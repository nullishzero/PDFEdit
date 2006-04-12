// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *		  Filename:  iproperty.cc
 *	   Description:  IProperty.
 *		   Created:  12/04/2006 00:48:14 PM CET
 *			Author:  jmisutka (), 
 * =====================================================================================
 */

#include "static.h"

#include "iproperty.h"
#include "cobject.h"
#include "cpdf.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//
// Constructor
//
IProperty::IProperty (CPdf* _pdf) : mode(mdUnknown), pdf(_pdf)
{
	printDbg (debug::DBG_DBG, "IProperty () constructor.");

	ref.num = ref.gen = 0;
}

IProperty::IProperty (CPdf* _pdf, const IndiRef& rf) : ref(rf), mode(mdUnknown), pdf(_pdf)
{
	printDbg (debug::DBG_DBG, "IProperty () constructor.");
}

//
// Deep copy
//
boost::shared_ptr<IProperty>
IProperty::clone () const
{
	boost::shared_ptr<IProperty> ip (doClone ());
	assert (typeid (*ip) == typeid (*this) && "doClone INCORRECTLY overriden!!" );
	
	return ip;
}


//
// Set/Get pdf
//
void 
IProperty::setPdf (CPdf* p)
{
	assert (NULL == pdf);	// modify existing association with a pdf?
	if (NULL != pdf)
		throw CObjInvalidOperation ();
	
	pdf = p;
}


//
// Dispatch change
//
void 
IProperty::dispatchChange () const
{
	printDbg (debug::DBG_DBG,"CObjectComplex::dispatchChange() [" << (int)this << "]" );

	//
	// Check if we are in a pdf. If not, we have nowhere to dispatch the change
	//
	CPdf* pdf = IProperty::getPdf ();
	if (NULL == pdf)
		return;

	//
	// If this is an indirect object inform xref about the change
	// else find the closest indirect object and call dispatchChange on that object
	//
	if (utils::objHasParent (*this))
	{
		printDbg (debug::DBG_DBG, "TRUE");
		boost::shared_ptr<IProperty> pIp = pdf->getIndirectProperty (IProperty::getIndiRef());
		if (pIp)
		{
			assert (IProperty::getIndiRef().num == pIp->getIndiRef().num);
			assert (IProperty::getIndiRef().gen == pIp->getIndiRef().gen);

			pIp->dispatchChange ();
			
		}else
		{
			assert (!"Bad parent.");
			throw ElementBadTypeException ("Bad pointer type.");
		}
	}else
	{
		printDbg (debug::DBG_DBG, "TRUE");
		Object* obj = _makeXpdfObject ();
		// This function saves a COPY of xpdf object(s) do we have to delete it
		//pdf->getXrefWriter()->changeObject (ind->num, ind->gen, obj);
		utils::freeXpdfObject (obj);
	}
}

//
// Observer interface
//

//
//
//
void 
IProperty::registerObserver (boost::shared_ptr<const Observer> o) 
{
	if (o)
		observers.push_back (o);
	else
		throw ElementBadTypeException ("IProperty::registerObserver got invalid observer.");
}

//
//
//
void 
IProperty::unregisterObserver (boost::shared_ptr<const Observer> o)
{
	if (o)
	{
		ObserverList::iterator it = find (observers.begin (), observers.end(), o);
		if (it != observers.end ())
			observers.erase (it);
		else
		{
			printDbg (debug::DBG_DBG, "unregisterObserver did not find the element to erase.");
			ElementNotFoundException ("","");
		}
	
	}else
		throw ElementBadTypeException ("IProperty::unregisterObserver got invalid observer.");
}

//
//
//
void
IProperty::notifyObservers (boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const ObserverContext> context)
{
	ObserverList::iterator it = IProperty::observers.begin ();
	for (; it != IProperty::observers.end(); ++it)
		(*it)->notify (newValue, context);
}

//=====================================================================================
} // namespace pdfobjects
//=====================================================================================
