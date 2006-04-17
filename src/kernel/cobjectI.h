// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
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

// CStream filters
#include "filters.h"


//=====================================================================================

namespace pdfobjects 
{


//=====================================================================================
// CObjectSimple
//=====================================================================================


//
// Protected constructor, called when we have parsed an object
//
template<PropertyType Tp, typename Checker>
CObjectSimple<Tp,Checker>::CObjectSimple (CPdf& p, Object& o, const IndiRef& rf) : IProperty (&p,rf), value(Value())
{
	Checker check; check.objectCreated (this);
	//assert (NULL == p.getIndirectProperty (rf).get()); TODO
	printDbg (debug::DBG_DBG,"CObjectSimple <" << debug::getStringType<Tp>() << ">(p,o,rf) constructor.");
	
	// Set object's value
	utils::simpleValueFromXpdfObj<Tp,Value&> (o,value);
}


//
// Protected constructor, called when we have parsed an object
//
template<PropertyType Tp, typename Checker>
CObjectSimple<Tp,Checker>::CObjectSimple (Object& o) : value(Value())
{
	Checker check; check.objectCreated (this);
	printDbg (debug::DBG_DBG,"CObjectSimple <" << debug::getStringType<Tp>() << ">(o) constructor.");
	
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
	printDbg (debug::DBG_DBG,"CObjectSimple <" << debug::getStringType<Tp>() << ">(val) constructor.");
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
	printDbg (debug::DBG_DBG,"text:" << strO);

	// Create context in which the change occurs
	boost::shared_ptr<ObserverContext> context (this->_createContext());
	// Change our value
	utils::simpleValueFromString (strO, this->value);
	// notify observers and dispatch the change
	_objectChanged (context);
}



//
// Write a value.
// 
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::writeValue (WriteType val)
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	printDbg (debug::DBG_DBG,"writeValue()");

	// Create context in which the change occurs
	boost::shared_ptr<ObserverContext> context (this->_createContext());
	// Change the value
	value = val;
	// notify observers and dispatch the change
	_objectChanged (context);
}	


//
// Get the value of an property
//
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::getPropertyValue (Value& val) const
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	val = value;
}

//
// Inform cxwriter that the value has changed
//
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::dispatchChange() const
{
	printDbg (debug::DBG_DBG,"CObjectSimple::dispatchChange() [" << (int)this << "]" );

	//
	// Check if we are in a pdf. If not, we have nowhere to dispatch the change
	//
	CPdf* pdf = IProperty::getPdf ();
	if (NULL == pdf || pNull == Tp)
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
		Ref ref = {IProperty::getIndiRef().num, IProperty::getIndiRef().gen};
		//pdf->getCXref()->changeObject (ref, obj);
		utils::freeXpdfObject (obj);
	}
}

//
// Helper function
//
template<PropertyType Tp, typename Checker>
Object*
CObjectSimple<Tp,Checker>::_makeXpdfObject () const
{
	printDbg (debug::DBG_DBG,"_makeXpdfObject");
	return utils::simpleValueToXpdfObj<Tp,const Value&> (value);
}


//
// Clone method
//
template<PropertyType Tp, typename Checker>
IProperty*
CObjectSimple<Tp,Checker>::doClone () const
{
	printDbg (debug::DBG_DBG,"CObjectSimple::doClone ()");

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
	Checker check; check.objectDeleted (this);
}


//=====================================================================================
// CObjectComplex
//=====================================================================================


//
// Protected constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex (CPdf& p, Object& o, const IndiRef& rf) : IProperty (&p,rf) 
{
	Checker check; check.objectCreated (this);
	// assert (NULL == p.getIndirectProperty (rf)); TODO
	printDbg (debug::DBG_DBG,"CObjectComplex <" << debug::getStringType<Tp>() << ">(p,o,rf) constructor.");
	
	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<Tp,Value&> (*this, o, value);
}

//
// Protected constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex (Object& o)
{
	Checker check; check.objectCreated (this);
	// assert (NULL == p.getIndirectProperty (rf)); TODO
	printDbg (debug::DBG_DBG,"CObjectComplex <" << debug::getStringType<Tp>() << ">(o) constructor.");
	
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
	printDbg (debug::DBG_DBG,"CObjectComplex <" << debug::getStringType<Tp>() << ">() constructor.");
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
		Ref ref = {IProperty::getIndiRef().num, IProperty::getIndiRef().gen};
		//pdf->getCXref()->changeObject (ref, obj);
		utils::freeXpdfObject (obj);
	}
}

//
// Destructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::~CObjectComplex ()
{
	Checker check; check.objectDeleted (this);
}


//
//
//
template <PropertyType Tp, typename Checker>
Object*
CObjectComplex<Tp,Checker>::_makeXpdfObject () const
{
	printDbg (debug::DBG_DBG,"_makeXpdfObject");
	
	std::string rpr;
	getStringRepresentation (rpr);

	if (NULL != IProperty::getPdf())
		return utils::xpdfObjFromString (rpr, IProperty::getPdf()->getCXref());
	else
		return utils::xpdfObjFromString (rpr);
}


//
// Template  member functions can't be virutal (at least according to nowadays specification)
//
// It seems that virtual functions are always instantiated
//
template<PropertyType Tp, typename Checker>
template<typename Container>
void
CObjectComplex<Tp,Checker>::getAllPropertyNames (Container& container) const
{
	STATIC_CHECK ((Tp != pArray), INCORRECT_USE_OF_getAllPropertyNames_FUNCTION);
	printDbg (debug::DBG_DBG, "getAllPropertyNames()");

	utils::getAllNames (container, value);
}


//
// 
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::delProperty (PropertyId id)
{
	printDbg (debug::DBG_DBG,"delProperty(" << id << ")");

	//
	// BEWARE using std::find_if with stateful functors !!!!!
	// We could have used getProperty but we also need the iterator
	//
	IndexComparator cmp (id);
	typename Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
			if (cmp (*it))
					break;
	}
	
	if (it == value.end())
		throw ElementNotFoundException ("", "");
	
	boost::shared_ptr<IProperty> ip = cmp.getIProperty ();
	if (ip)
	{
		// Create the context
		boost::shared_ptr<ObserverContext> context (this->_createContext(ip));
		// Delete that item
		value.erase (it);
		// Indicate that this object has changed
		boost::shared_ptr<IProperty> changedObj (new CNull());
		_objectChanged (changedObj, context);
		
	}else
		throw CObjInvalidObject ();
}


//
// Correctly to add an object (without name) can be done only to Array object
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::addProperty (const IProperty& newIp)
{
	STATIC_CHECK ((Tp == pArray), INCORRECT_USE_OF_addProperty_FUNCTION);
	printDbg (debug::DBG_DBG,"addProperty(...)");

	return addProperty (value.size(), newIp);
}

//
// Correctly to add an object (without name) can be done only to Array object
//
// REMARK: because of the compiler, we can't put PropertyId here
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::addProperty (size_t position, const IProperty& newIp)
{
	STATIC_CHECK ((Tp == pArray), INCORRECT_USE_OF_addProperty_FUNCTION);
	printDbg (debug::DBG_DBG,"addProperty(" << position << ")");

	//
	// Check if we add to a valid position
	//
	if (position > value.size())
		throw CObjInvalidOperation ();
	
	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	
	if (newIpClone)
	{
		typename Value::iterator it;
		// Find the correct position
		if (value.size() == position)
		{
			it = value.end ();
			
		}else
		{
			for (it = value.begin(); 0 != position; ++it, --position)
				;
		}

		// Insert it
		value.insert (it,newIpClone);
		// Inherit id and gen number
		newIpClone->setIndiRef (IProperty::getIndiRef());
		// Inherit pdf
		newIpClone->setPdf (IProperty::getPdf());
		
	}else
		throw CObjInvalidObject ();
	
	// Notify observers and dispatch change
	_objectChanged (newIpClone, 
					boost::shared_ptr<ObserverContext>
						(new BasicObserverContext 
						 	(boost::shared_ptr<IProperty> (new CNull ()))
						)
				   );

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
	STATIC_CHECK ((Tp == pDict), INCORRECT_USE_OF_addProperty_FUNCTION);
	printDbg (debug::DBG_DBG,"addProperty( " << propertyName << ",...)");

	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	if (newIpClone)
	{
		// Store it
		value.push_back (std::make_pair (propertyName,newIpClone));
		// Inherit id and gen number
		newIpClone->setIndiRef (IProperty::getIndiRef());
		// Inherit pdf
		newIpClone->setPdf (IProperty::getPdf());
		
	}else
		throw CObjInvalidObject ();

	// notify observers and dispatch change
	_objectChanged (newIpClone,
					boost::shared_ptr<ObserverContext> 
						(new BasicObserverContext 
						 	(boost::shared_ptr<IProperty> (new CNull ()))
						)
					);

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
CObjectComplex<Tp,Checker>::setProperty (PropertyId id, IProperty& newIp)
{
	printDbg (debug::DBG_DBG,"setProperty(" << id << ")");
	
	//
	// Find the item we want
	// BEWARE using std::find_if with stateful functors !!!!!
	//
	IndexComparator cmp (id);
	typename Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
			if (cmp (*it))
					break;
	}

	// Check the bounds, if fails an exception is thrown
	if (it == value.end())
			throw ElementNotFoundException ("", "");

	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	
	if (newIpClone)
	{
		//
		// Insert the element we want to add at the end, swap with the element we want to
		// delete and delete the last one
		//	
		typename Value::value_type newVal = utils::constructItemFromIProperty (*it, newIpClone);
		std::fill_n (it, 1, newVal);
		
	}else
		throw CObjInvalidObject ();
		

	return newIpClone;
	// newVal holds pointer to the object, that should be released, so now it should be deleted
}


//
//
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::getProperty (PropertyId id) const
{
	printDbg (debug::DBG_DBG,"getProperty() " << id);

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

	if (it == value.end())
			throw ElementNotFoundException ("", "");
	
	boost::shared_ptr<IProperty> ip = cmp.getIProperty ();

	//
	// \TODO Find out the mode
	//
	
	return ip;
}

//
//
//
template<PropertyType Tp, typename Checker>
typename PropertyTraitComplex<Tp>::propertyId
CObjectComplex<Tp,Checker>::getPropertyId (const boost::shared_ptr<IProperty>& ip) const
{
	printDbg (debug::DBG_DBG,"getPropertyId ()");

	typename Value::const_iterator it = value.begin();
	for (size_t i = 0; it != value.end(); ++it, ++i)
	{
		// Have we found the item
		if (utils::getIPropertyFromItem (*it) == ip)
			return utils::getPropertyIdFromItem (*it, i);
	}

	throw ElementNotFoundException ("", "");
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
	printDbg (debug::DBG_DBG,"CObjectComplex::doClone");
	
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

//
//
//
template<PropertyType Tp, typename Checker>
IProperty::ObserverContext* 
CObjectComplex<Tp,Checker>::_createContext (boost::shared_ptr<IProperty>& changedIp)
{
	printDbg (debug::DBG_DBG, "");

	// Save original value for the context
	boost::shared_ptr<IProperty> oldValue (changedIp->clone());
	// For safety up to 1 level deeper
	oldValue->setPdf (NULL);
	oldValue->setIndiRef (0,0);
	for (typename Value::iterator it = this->value.begin (); it != value.end (); ++it)
	{
		utils::getIPropertyFromItem(*it)->setPdf (NULL);
		utils::getIPropertyFromItem(*it)->setIndiRef (0,0);
	}
	// Create the context
	return new BasicObserverContext (oldValue);
}

//=====================================================================================
// CObjectStream
//=====================================================================================


//
//
//
template<typename Checker>
CObjectStream<Checker>::CObjectStream (CPdf& p, Object& o, const IndiRef& rf) : IProperty (&p,rf) 
{
	Checker check; check.objectCreated (this);
	printDbg (debug::DBG_DBG,"CObjectComplex <pStream> >() constructor.");

	// Copy the stream
	o.copy (&xpdfDict);
}


//
//
//
template<typename Checker>
CObjectStream<Checker>::CObjectStream (Object& o)
{
	Checker check; check.objectCreated (this);
	printDbg (debug::DBG_DBG,"CObjectComplex <pStream> >() constructor.");

	// Copy the stream
	o.copy (&xpdfDict);
}


//
//
//
template<typename Checker>
CObjectStream<Checker>::CObjectStream ()
{
	Checker check; check.objectCreated (this);
	printDbg (debug::DBG_DBG,"CObjectComplex <pStream> >() constructor.");

}


//
//
//
template<typename Checker>
IProperty*
CObjectStream<Checker>::doClone () const
{
	return NULL;
}

//
//
//
template<typename Checker>
void
CObjectStream<Checker>::getStringRepresentation (std::string& str) const
{
	str = "<what shall i return?>";
}
	

//
//
//
template<typename Checker>
void 
CObjectStream<Checker>::setStringRepresentation (const std::string& strO)
{
	assert (!"Not implemented yet.");
	// just an example
	
	// find the type
	// find the params
	// create appropriate filter and endcode data	
	boost::scoped_ptr<filters::CFilter> filter = new filters::NoFilter ();
	//save it
	_objectChanged ();
}


//
//
//
template<typename Checker>
void
CObjectStream<Checker>::dispatchChange () const
{
	printDbg (debug::DBG_DBG,"CObjectStream::dispatchChange<pStream>() [" << (int)this << "]" );

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
		Ref ref = {IProperty::getIndiRef().num, IProperty::getIndiRef().gen};
		//pdf->getCXref()->changeObject (ref, obj);
		utils::freeXpdfObject (obj);
	}
}	

//
//
//
template<typename Checker>
Object*
CObjectStream<Checker>::_makeXpdfObject () const
{
	Object* obj = new Object ();
	xpdfDict.copy (obj);
	return obj;
}


//
//
//
template<typename Checker>
CObjectStream<Checker>::~CObjectStream ()
{
	Checker check; check.objectDeleted (this);
	printDbg (debug::DBG_DBG,"~CObjectStream()");

	// Free xpdf object
	xpdfDict.free ();
}



} /* namespace pdfobjects */



#endif // _COBJECTI_H
