/** 
 * =====================================================================================
 *        Filename:  cobjectI.h
 *     Description:  Header file containing implementation of CObject class.
 *         Created:  02/02/2006 
 *        Revision:  none
 *          Author:  jmisutka (06/01/19), 
 * 			
 * =====================================================================================
 */

#ifndef _COBJECTI_H
#define _COBJECTI_H

#include "static.h"

// xpdf
#include "xpdf.h"

#include "iproperty.h"
#include "cobject.h"
#include "cpdf.h"




//=====================================================================================

namespace pdfobjects 
{



		
//=====================================================================================
// CObjectSimple
//


//
// Protected constructor, called when we have parsed an object
//
template<PropertyType Tp, typename Checker>
CObjectSimple<Tp,Checker>::CObjectSimple (CPdf& p, Object& o, const IndiRef& rf) : IProperty (&p,rf), value(Value())
{
	Checker check; check.objectCreated (this);
	assert (NULL == p.getExistingProperty (rf));
	printDbg (0,"CObjectSimple <" << debug::getStringType<Tp>() << ">(p,o,rf) constructor.");
	
	// Set object's value
	utils::simpleValueFromXpdfObj<Tp,Value&> (o,value);
}


//
// Public constructor
//
template<PropertyType Tp, typename Checker>
CObjectSimple<Tp,Checker>::CObjectSimple (const Value& val) : IProperty(), value(val)
{
	Checker check; check.objectCreated (this);
	printDbg (0,"CObjectSimple <" << debug::getStringType<Tp>() << ">(val) constructor.");
}


//
// Turn object to string
//
template<PropertyType Tp, typename Checker>
void 
CObjectSimple<Tp,Checker>::getStringRepresentation (std::string& str) const
{
	utils::simpleValueToString<Tp> (value,str);
}


//
// Set string representation
//
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::setStringRepresentation (const std::string& strO)
{
	STATIC_CHECK ((Tp != pNull),INCORRECT_USE_OF_setStringRepresentation_FUNCTION_FOR_pNULL_TYPE);
	printDbg (0,"setStringRepresentation() text:" << strO);
	
	utils::simpleValueFromString (strO,value);

	// notify observers and dispatch the change
	_objectChanged ();
}



//
// Write a value.
// 
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::writeValue (WriteType val)
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	printDbg (0,"writeValue()");

	value = val;

	// notify observers and dispatch the change
	_objectChanged ();
}	


//
// Get the value of an property
//
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::getPropertyValue (Value& val) const
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	printDbg (0,"getPropertyValue()");

	val = value;
}

//
// Inform cxwriter that the value has changed
//
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::dispatchChange() const
{
	printDbg (0,"dispatchChange() [" << (int)this << "]" );

	//
	// Check if we are in a pdf. If not, we have nowhere to dispatch the change
	//
	CPdf* pdf = IProperty::getPdf ();
	if (NULL == pdf)
			return;

	//
	// If this object is indirect infrom xref about the change
	// else find the closest indirect object and call dispatchChange on that object
	// 
	if (_isIndirect())
	{
		Object* obj = _makeXpdfObject ();
		// This function saves a COPY of xpdf object(s) do we have to delete it
		//pdf->getXrefWriter()->changeObject (ind->num, ind->gen, obj);
		utils::freeXpdfObject (obj);

	}else
	{
		IProperty* ip = pdf->getExistingProperty (IProperty::getIndiRef());
		assert (NULL != ip);
		assert (IProperty::getIndiRef().num == ip->getIndiRef().num);
		assert (IProperty::getIndiRef().gen == ip->getIndiRef().gen);
		ip->dispatchChange ();
	}
}

//
// Helper function
//
template<PropertyType Tp, typename Checker>
Object*
CObjectSimple<Tp,Checker>::_makeXpdfObject () const
{
	printDbg (0,"_makeXpdfObject");
	return utils::simpleValueToXpdfObj<Tp,const Value&> (value);
}

//
// Helper function
//
template<PropertyType Tp, typename Checker>
bool
CObjectSimple<Tp,Checker>::_isIndirect () const
{
	CPdf* pdf = IProperty::getPdf ();
	assert (NULL != pdf);
	if (NULL == pdf)
		throw ObjInvalidOperation ();

	if (this == pdf->getExistingProperty (IProperty::getIndiRef ()))
		return true;
	else
		return false;
}


//
// Clone method
//
template<PropertyType Tp, typename Checker>
IProperty*
CObjectSimple<Tp,Checker>::doClone () const
{
	printDbg (0,"CObjectSimple::doClone ()");

	// Make new complex object
	// NOTE: We do not want to preserve any IProperty variable
	return new CObjectSimple<Tp,Checker> (value);
}


//
// Destructor
//
template<PropertyType Tp, typename Checker>
CObjectSimple<Tp,Checker>::~CObjectSimple ()
{
	Checker check; check.objectDeleted(this);
	printDbg (0,"~CObjectSimple()");
}


//=====================================================================================
// CObjectComplex
//


//
// Protected constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex (CPdf& p, Object& o, const IndiRef& rf) : IProperty (&p,rf) 
{
	Checker check; check.objectCreated (this);
	assert (NULL == p.getExistingProperty (rf));
	printDbg (0,"CObjectComplex <" << debug::getStringType<Tp>() << ">(p,o,rf) constructor.");
	
	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<Tp,Value&> (*this, o, value);
}


//
// Public constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex ()
{
	Checker check; check.objectCreated (this);
	printDbg (0,"CObjectComplex <" << debug::getStringType<Tp>() << ">() constructor.");
}


//
//
//
template<PropertyType Tp, typename Checker>
void 
CObjectComplex<Tp,Checker>::getStringRepresentation (std::string& str) const 
{
	utils::complexValueToString<Tp> (value,str);
}


//
//
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::dispatchChange() const
{
	printDbg (0,"dispatchChange() [" << (int)this << "]" );

	//
	// Check if we are in a pdf
	//
	CPdf* pdf = IProperty::getPdf ();
	if (NULL == pdf)
		return;

	//
	// If this object is indirect infrom xref about the change
	// else find the closest indirect object and call dispatchChange on that object
	// 
	if (_isIndirect())
	{
		Object* obj = _makeXpdfObject ();
		// This function saves a COPY of xpdf object(s) do we have to delete it
		//pdf->getXrefWriter()->changeObject (ind->num, ind->gen,obj);
		utils::freeXpdfObject (obj);

	}else
	{
		IProperty* ip = pdf->getExistingProperty (IProperty::getIndiRef());
		assert (NULL != ip);
		assert (IProperty::getIndiRef().num == ip->getIndiRef().num);
		ip->dispatchChange ();
	}
}

//
// Destructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::~CObjectComplex ()
{
	Checker check; check.objectDeleted(this);
	printDbg (0,"~CObjectSimple()");
}


//
//
//
template <PropertyType Tp, typename Checker>
Object*
CObjectComplex<Tp,Checker>::_makeXpdfObject () const
{
	printDbg (0,"_makeXpdfObject");
	
	std::string rpr;
	getStringRepresentation (rpr);
	printDbg (0,"\tfrom" << rpr);

	return utils::xpdfObjFromString (rpr);
}


//
// Template  member functions can't be virutal (at least according to nowadays specification)
//
// It seems that virtual functions are always instantiated
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::getAllPropertyNames (std::list<std::string>& container) const
{
	STATIC_CHECK ((Tp != pArray), INCORRECT_USE_OF_getAllPropertyNames_FUNCTION);
	printDbg (0, "getAllPropertyNames()");

	utils::getAllNames (container,value);
}


//
// 
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::delProperty (PropertyId id)
{
	printDbg (0,"delProperty(" << id << ")");

	//
	// BEWARE using std::find_if with stateful functors !!!!!
	// We could have used getPropertyValue but we also need the iterator
	//
	IndexComparator cmp (id);
	typename Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
			if (cmp (*it))
					break;
	}
	
	if (it == value.end())
		throw ObjInvalidPositionInComplex ();
	
	boost::shared_ptr<IProperty> ip = cmp.getIProperty ();
	assert (NULL != ip.get());
	
	// Delete that item
	value.erase (it);
	
	// Indicate that this object has changed
	_objectChanged ();

	// deallocates ip, also *ip should be deallocated
}


//
// Correctly to add an object (without name) can be done only to Array object
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::addProperty (const IProperty& newIp)
{
	STATIC_CHECK ((Tp == pArray), INCORRECT_USE_OF_addProperty_FUNCTION);
	printDbg (0,"addProperty(...)");

	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	assert (NULL != newIpClone.get());
	
	// Add it
	value.push_back (newIpClone);
	
	// Inherit id and gen number
	newIpClone->setIndiRef (IProperty::getIndiRef());
	// Inherit pdf
	newIpClone->setPdf (IProperty::getPdf());
	
	// notify observers and dispatch change
	_objectChanged ();

	//
	// \TODO: Find out the mode
	//

	return newIpClone;
}


//
// Correctly add an object (with name) can be done only to Dict and Stream object
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::addProperty (const std::string& propertyName, const IProperty& newIp)
{
	STATIC_CHECK ((Tp == pDict) || (Tp == pStream), INCORRECT_USE_OF_addProperty_FUNCTION);
	printDbg (0,"addProperty( " << propertyName << ",...)");

	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	assert (NULL != newIpClone.get());

	// Store it
	value.push_back (std::make_pair (propertyName,newIpClone));
	
	// Inherit id and gen number
	newIpClone->setIndiRef (IProperty::getIndiRef());
	// Inherit pdf
	newIpClone->setPdf (IProperty::getPdf());

	// notify observers and dispatch change
	_objectChanged ();

	//
	// \TODO: Find out the mode
	//

	return newIpClone;
}


//
//
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::setPropertyValue (PropertyId id, IProperty& newIp)
{
	printDbg (0,"setPropertyValue(" << id << ")");
	
	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	assert (NULL != newIpClone.get());

	//
	// BEWARE using std::find_if with stateful functors !!!!!
	//
	IndexComparator cmp (id);
	typename Value::const_iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
			if (cmp (*it))
					break;
	}

	// Check the bounds, if fails an exception is thrown
	if (it == value.end())
			throw ObjInvalidPositionInComplex ();

	boost::shared_ptr<IProperty> oldIp = cmp.getIProperty ();
	assert (NULL != oldIp.get());
	//
	// Insert the element we want to add at the end, swap with the element we want to
	// delete and delete the last one
	//	
	typename Value::iterator itemNext = it; ++itemNext;
	typename Value::value_type newVal = utils::constructItemFromIProperty (*it, newIpClone);
	std::fill (it, itemNext, newVal);

	return newIpClone;

	// newVal holds pointer to the object, that should be released, so now it should be deleted
}


//
//
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::getPropertyValue (PropertyId id) const
{
	printDbg (0,"getPropertyValue() " << id);

	//
	// BEWARE using std::find_if with stateful functors !!!!!
	//
	IndexComparator cmp (id);
	typename Value::const_iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
			if (cmp (*it))
					break;
	}

	boost::shared_ptr<IProperty> ip = cmp.getIProperty ();
	assert (NULL != ip.get());
	
	if (it == value.end())
			throw ObjInvalidPositionInComplex ();
	
	//
	// \TODO Find out the mode
	//
	
	return ip;
}


//
// Helper function
//
template<PropertyType Tp, typename Checker>
bool
CObjectComplex<Tp,Checker>::_isIndirect () const
{
	CPdf* pdf = IProperty::getPdf ();
	assert (NULL != pdf);
	if (NULL == pdf)
		throw ObjInvalidOperation ();

	if (this == pdf->getExistingProperty (IProperty::getIndiRef ()))
		return true;
	else
		return false;
}

//
//
//
template<PropertyType Tp, typename Checker>
template <typename Storage>
void 
CObjectComplex<Tp,Checker>::_getAllChildObjects (Storage& store) const
{
	typename Value::const_iterator it = value.begin ();
	for	(; it != value.end (); ++it)
	{
		boost::shared_ptr<IProperty> ip = utils::getIPropertyFromItem (*it);
		store.push_back (ip);
	}
}


//
// Clone method
//
template<PropertyType Tp, typename Checker>
IProperty*
CObjectComplex<Tp,Checker>::doClone () const
{
	printDbg (0,"CObjectComplex::doClone");
	
	// Make new complex object
	// NOTE: We do not want to inherit any IProperty variable
	CObjectComplex<Tp,Checker>* clone_ = new CObjectComplex<Tp,Checker> ();
	
	//
	// Loop through all items and clone them as well and finally add them to the new object
	//
	typename Value::const_iterator it = value.begin ();
	for (; it != value.end (); ++it)
	{
		boost::shared_ptr<IProperty> newIp = utils::getIPropertyFromItem(*it)->clone ();
		assert (NULL != newIp.get ());
		typename Value::value_type item =  utils::constructItemFromIProperty (*it, newIp);
		clone_->value.push_back (item);
	}

	return clone_;
}




} /* namespace pdfobjects */



#endif // _COBJECTI_H
