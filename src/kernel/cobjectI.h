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

#include "iproperty.h"
#include "cpdf.h"

// CStream filters
#include "filters.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//=====================================================================================
// CObjectSimple
//=====================================================================================

//
// Constructors
//

//
// Protected constructor, called when we have parsed an object
//
template<PropertyType Tp, typename Checker>
CObjectSimple<Tp,Checker>::CObjectSimple (CPdf& p, Object& o, const IndiRef& rf) : IProperty (&p,rf), value(Value())
{
	Checker check; check.objectCreated (this);
	//kernelPrintDbg (debug::DBG_DBG,"CObjectSimple <" << debug::getStringType<Tp>() << ">(p,o,rf) constructor.");
	
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
	//kernelPrintDbg (debug::DBG_DBG,"CObjectSimple <" << debug::getStringType<Tp>() << ">(o) constructor.");
	
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
	//kernelPrintDbg (debug::DBG_DBG,"CObjectSimple <" << debug::getStringType<Tp>() << ">(val) constructor.");
}


//
// Get methods
//

//
// Turn object to string
//
template<PropertyType Tp, typename Checker>
void 
CObjectSimple<Tp,Checker>::getStringRepresentation (std::string& str) const
{
	utils::simpleValueToString<Tp> (value, str);
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
// Set methods
//

//
// Set string representation
//
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::setStringRepresentation (const std::string& strO)
{
	STATIC_CHECK ((Tp != pNull),INCORRECT_USE_OF_setStringRepresentation_FUNCTION_FOR_pNULL_TYPE);
	//kernelPrintDbg (debug::DBG_DBG,"text:" << strO);

	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// Create context in which the change occurs
		boost::shared_ptr<ObserverContext> context (this->_createContext());
		// Change our value
		utils::simpleValueFromString (strO, this->value);
		// notify observers and dispatch the change
		_objectChanged (context);

	}else
	{
		assert (!hasValidRef (this));

		// Change our value
		utils::simpleValueFromString (strO, this->value);
	}
}


//
// Write a value.
// 
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::writeValue (WriteType val)
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	//kernelPrintDbg (debug::DBG_DBG, "writeValue() type: " << Tp);

	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// Create context in which the change occurs
		boost::shared_ptr<ObserverContext> context (this->_createContext());
		// Change the value
		value = val;
		// notify observers and dispatch the change
		_objectChanged (context);
	
	}else
	{
		assert (!hasValidRef (this));

		// Change the value
		value = val;
	}
}	



//
// Helper function
//

//
//
//
template<PropertyType Tp, typename Checker>
::Object*
CObjectSimple<Tp,Checker>::_makeXpdfObject () const
{
	//kernelPrintDbg (debug::DBG_DBG,"_makeXpdfObject");
	return utils::simpleValueToXpdfObj<Tp,const Value&> (value);
}

//
//
//
template<PropertyType Tp, typename Checker>
void 
CObjectSimple<Tp,Checker>::_objectChanged (boost::shared_ptr<const ObserverContext> context)
{
	kernelPrintDbg (debug::DBG_DBG, "CObjectSimple");
	// Do not notify anything if we are not in a valid pdf
	if (!hasValidPdf (this))
		return;

	assert (hasValidRef(this));
	
	// Dispatch the change
	this->dispatchChange ();
	
	if (context)
	{
		// Clone this new value
		boost::shared_ptr<IProperty> newValue (this->clone());
		// Fill them with correct values
		newValue->setPdf (this->getPdf());
		newValue->setIndiRef (this->getIndiRef());
		// Notify everybody about this change
		this->notifyObservers (newValue, context);

	}else
	{
		assert (!"Invalid context");
		throw CObjInvalidOperation ();
	}
}

//
// Clone method
//
template<PropertyType Tp, typename Checker>
IProperty*
CObjectSimple<Tp,Checker>::doClone () const
{
	kernelPrintDbg (debug::DBG_DBG,"CObjectSimple::doClone ()");

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
// Constructors
//

//
// Protected constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex (CPdf& p, Object& o, const IndiRef& rf) : IProperty (&p,rf) 
{
	Checker check; check.objectCreated (this);
	//kernelPrintDbg (debug::DBG_DBG,"CObjectComplex <" << debug::getStringType<Tp>() << ">(p,o,rf) constructor.");
	
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
	//kernelPrintDbg (debug::DBG_DBG,"CObjectComplex <" << debug::getStringType<Tp>() << ">(o) constructor.");
	
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
	//kernelPrintDbg (debug::DBG_DBG,"CObjectComplex <" << debug::getStringType<Tp>() << ">() constructor.");
}


//
// Get methods
//

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
// Template  member functions can't be virutal (at least according to nowadays specification)
//
template<PropertyType Tp, typename Checker>
template<typename Container>
void
CObjectComplex<Tp,Checker>::getAllPropertyNames (Container& container) const
{
	STATIC_CHECK ((Tp != pArray), INCORRECT_USE_OF_getAllPropertyNames_FUNCTION);
	//kernelPrintDbg (debug::DBG_DBG, "getAllPropertyNames()");

	utils::getAllNames (container, value);
}

//
//
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::getProperty (PropertyId id) const
{
	//kernelPrintDbg (debug::DBG_DBG,"getProperty() " << id);
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

	// Set mode only if pdf is valid
	_setMode (ip,id);

	return ip;
}


//
// Set methods
//

//
//
//
template<PropertyType Tp, typename Checker>
void 
CObjectComplex<Tp,Checker>::setPdf (CPdf* pdf)
{
	// Set pdf to this object
	IProperty::setPdf (pdf);

	// Set new pdf to all its children
	typename Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
		utils::getIPropertyFromItem (*it)->setPdf (pdf);
}

//
//
//
template<PropertyType Tp, typename Checker>
void 
CObjectComplex<Tp,Checker>::setIndiRef (const IndiRef& rf)
{
	// Set pdf to this object
	IProperty::setIndiRef (rf);

	// Set new pdf to all its children
	typename Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
		utils::getIPropertyFromItem (*it)->setIndiRef (rf);
}


//
// 
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::delProperty (PropertyId id)
{
	kernelPrintDbg (debug::DBG_DBG,"delProperty(" << id << ")");

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
		throw ElementNotFoundException ("CObjectComplex", "item not found");
	
	boost::shared_ptr<IProperty> ip = cmp.getIProperty ();
	if (ip)
	{
		if (hasValidPdf (this))
		{
			assert (hasValidRef (this));
			
			// Create the context
			boost::shared_ptr<ObserverContext> context (this->_createContext(ip));
			// Delete that item
			value.erase (it);
			// Indicate that this object has changed
			boost::shared_ptr<IProperty> changedObj (new CNull());
			_objectChanged (changedObj, context);
			
			// Be sure
			ip->setPdf (NULL);
			ip->setIndiRef (IndiRef());
	
		}else
		{
			assert (!hasValidRef (this));
			// Delete that item
			value.erase (it);
		}
		
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
	kernelPrintDbg (debug::DBG_DBG,"addProperty(...)");

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
	kernelPrintDbg (debug::DBG_DBG,"addProperty(" << position << ")");

	//
	// Check if we add to a valid position
	//
	if (position > value.size())
		throw OutOfRange ();
	
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

		// Inherit id, gen number and pdf
		newIpClone->setPdf (this->getPdf());
		newIpClone->setIndiRef (this->getIndiRef());
		// Insert it
		value.insert (it,newIpClone);
		
	}else
		throw CObjInvalidObject ();
	
	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// Notify observers and dispatch change
		_objectChanged (newIpClone, 
					boost::shared_ptr<ObserverContext>
						(new BasicObserverContext 
						 	(boost::shared_ptr<IProperty> (new CNull ()))
						)
				   );
	}
	
	// Set mode only if pdf is valid
	_setMode (newIpClone, position);
	
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
	kernelPrintDbg (debug::DBG_DBG,"addProperty( " << propertyName << ",...)");

	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	if (newIpClone)
	{
		// Inherit id, gen number and pdf
		newIpClone->setIndiRef (this->getIndiRef());
		newIpClone->setPdf (this->getPdf());
	
		// Store it
		value.push_back (std::make_pair (propertyName,newIpClone));
		
	}else
		throw CObjInvalidObject ();

	//
	// Dispatch change if we are in valid pdf
	// 
	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// notify observers and dispatch change
		_objectChanged (newIpClone,
					boost::shared_ptr<ObserverContext> 
						(new BasicObserverContext 
						 	(boost::shared_ptr<IProperty> (new CNull ()))
						)
					);
	}

	// Set mode only if pdf is valid
	_setMode (newIpClone, propertyName);
	
	return newIpClone;
}


//
//
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::setProperty (PropertyId id, IProperty& newIp)
{
	kernelPrintDbg (debug::DBG_DBG, "setProperty(" << id << ")");
	
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

	// Save the old one
	boost::shared_ptr<IProperty> oldIp = cmp.getIProperty ();
	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	
	if (newIpClone)
	{
		// Inherit id, gen number and pdf
		newIpClone->setIndiRef (this->getIndiRef());
		newIpClone->setPdf (this->getPdf());

		// Construct item, and replace it with this one
		typename Value::value_type newVal = utils::constructItemFromIProperty (*it, newIpClone);
		std::fill_n (it, 1, newVal);
		
	}else
		throw CObjInvalidObject ();

	//
	// Dispatch change if we are in valid pdf
	// 
	if (hasValidPdf (this))
	{	
		assert (hasValidRef (this));
		
		// Create context
		boost::shared_ptr<ObserverContext> context (_createContext (oldIp));	
		// Notify observers and dispatch change
		_objectChanged (newIpClone, context);

		// Be sure
		oldIp->setPdf (NULL);
		oldIp->setIndiRef (IndiRef());
	}

	// Set mode only if pdf is valid
	_setMode (newIpClone, id);
	
	return newIpClone;
}


//
// Helper methods
//

//
//
//
template <PropertyType Tp, typename Checker>
::Object*
CObjectComplex<Tp,Checker>::_makeXpdfObject () const
{
	//kernelPrintDbg (debug::DBG_DBG,"_makeXpdfObject");
	
	std::string rpr;
	getStringRepresentation (rpr);

	if (hasValidPdf (this))
		return utils::xpdfObjFromString (rpr, this->getPdf()->getCXref());
	else
		return utils::xpdfObjFromString (rpr);
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
//
//
template<PropertyType Tp, typename Checker>
void 
CObjectComplex<Tp,Checker>::_objectChanged 
	(boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const ObserverContext> context)
{
	// Do not notify anything if we are not in a valid pdf
	if (!hasValidPdf (this))
		return;
	assert (hasValidRef (this));

	// Dispatch the change
	this->dispatchChange ();
	
	if (context)
	{
		// Notify everybody about this change
		this->notifyObservers (newValue, context);

	}else
	{
		assert (!"Invalid context");
		throw CObjInvalidOperation ();
	}
}

//
// Set mode
//
template <PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::_setMode (boost::shared_ptr<IProperty> ip, PropertyId id) const
{
	ModeController* modecontroller = NULL;
	if (hasValidPdf (this) && (NULL != (modecontroller=this->getPdf()->getModeController())))
	{
		assert (modecontroller);		
		PropertyMode mode = utils::getModeForComplexObjects (value, id, *modecontroller);
		if (mdUnknown == mode)
			ip->setMode (this->getMode());
		else
			ip->setMode (mode);
	}
}



//
// Clone method
//
template<PropertyType Tp, typename Checker>
IProperty*
CObjectComplex<Tp,Checker>::doClone () const
{
	//kernelPrintDbg (debug::DBG_DBG,"");
	
	// Make new complex object
	// NOTE: We do not want to inherit any IProperty variable
	CObjectComplex<Tp,Checker>* clone_ = _newInstance ();
	
	//
	// Loop through all items and clone them as well and finally add them to the new object
	//
	typename Value::const_iterator it = value.begin ();
	for (; it != value.end (); ++it)
	{
		boost::shared_ptr<IProperty> newIp = utils::getIPropertyFromItem(*it)->clone ();
		assert (newIp);
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
	//kernelPrintDbg (debug::DBG_DBG, "");

	// Create the context
	return new BasicObserverContext (changedIp);
}


//
// Destructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::~CObjectComplex ()
{
	Checker check; check.objectDeleted (this);
}

//=====================================================================================
// CObjectStream
//=====================================================================================

//
// Constructors
//

//
//
//
template<typename Checker>
CObjectStream<Checker>::CObjectStream (CPdf& p, ::Object& o, const IndiRef& rf) : IProperty (&p,rf), parser (NULL)
{
	Checker check; check.objectCreated (this);
	kernelPrintDbg (debug::DBG_DBG,"");
	// Make sure it is a stream
	assert (objStream == o.getType());
	if (objStream != o.getType())
		throw XpdfInvalidObject ();
	
	// Get the dictionary and init CDict with it
	::Object objDict;
	::Dict* dict = o.streamGetDict ();
	assert (NULL != dict);
	objDict.initDict (dict);
	utils::complexValueFromXpdfObj<pDict,CDict::Value&> (dictionary, objDict, dictionary.value);
	objDict.free ();

	// Set pdf and ref
	dictionary.setPdf (&p);
	dictionary.setIndiRef (rf);
	
	// Save the contents of the container
	utils::parseStreamToContainer (buffer, o);
}


//
//
//
template<typename Checker>
CObjectStream<Checker>::CObjectStream (::Object& o) : parser (NULL)
{
	Checker check; check.objectCreated (this);
	kernelPrintDbg (debug::DBG_DBG,"");
	// Make sure it is a stream
	assert (objStream == o.getType());
	if (objStream != o.getType())
		throw XpdfInvalidObject ();
	
	// Get the dictionary and init CDict with it
	::Object objDict;
	::Dict* dict = o.streamGetDict();
	assert (NULL != dict);
	objDict.initDict (dict);
	utils::complexValueFromXpdfObj<pDict,CDict::Value&> (dictionary, objDict, dictionary.value);
	objDict.free ();

	// Save the contents of the container
	utils::parseStreamToContainer (buffer, o);
}


//
//
//
template<typename Checker>
CObjectStream<Checker>::CObjectStream () : parser (NULL)
{
	Checker check; check.objectCreated (this);
	kernelPrintDbg (debug::DBG_DBG,"");

}

//
// Cloning
//

//
//
//
template<typename Checker>
IProperty*
CObjectStream<Checker>::doClone () const
{
	kernelPrintDbg (debug::DBG_DBG,"CObjectStream::doClone");
	assert (NULL == parser  || !"Want to clone opened stream.. Should the stream state be also copied?");
	//assert (getLength() == buffer.size());
	
	// Make new stream object
	// NOTE: We do not want to inherit any IProperty variable
	CObjectStream<Checker>* clone_ = _newInstance ();
	
	//
	// Loop through all items and clone them as well and finally add them to the new object
	//
	CDict::Value::const_iterator it = dictionary.value.begin ();
	for (; it != dictionary.value.end (); ++it)
	{
		boost::shared_ptr<IProperty> newIp = ((*it).second)->clone ();
		assert (newIp);
		CDict::Value::value_type item =  std::make_pair ((*it).first, newIp);
		clone_->dictionary.value.push_back (item);
	}

	std::copy (buffer.begin(), buffer.end(), std::back_inserter (clone_->buffer));
	
	return clone_;
}

//
// Set methods
//

//
//
//
template<typename Checker>
void 
CObjectStream<Checker>::setPdf (CPdf* pdf)
{
	// Set pdf to this object and dictionary it contains
	IProperty::setPdf (pdf);
	dictionary.setPdf (pdf);
}

//
//
//
template<typename Checker>
void 
CObjectStream<Checker>::setIndiRef (const IndiRef& rf)
{
	// Set pdf to this object and dictionary it contains
	IProperty::setIndiRef (rf);
	dictionary.setIndiRef (rf);
}


//
//
//
template<typename Checker>
void 
CObjectStream<Checker>::setRawBuffer (const Buffer& buf)
{
	assert (NULL == parser || !"Stream is open.");
	if (NULL != parser)
		throw CObjInvalidOperation ();
	
	// Create context
	boost::shared_ptr<ObserverContext> context (this->_createContext());

	// Copy buf to buffer
	buffer.clear ();
	std::copy (buf.begin(), buf.end(), std::back_inserter (buffer));
	// Change length
	setLength (buffer.size());
	
	//Dispatch change 
	_objectChanged (context);
}

//
//
//
template<typename Checker>
template<typename Container>
void 
CObjectStream<Checker>::setBuffer (const Container& buf)
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (NULL == parser || !"Stream is open.");
	if (NULL != parser)
		throw CObjInvalidOperation ();

	// Create context
	boost::shared_ptr<ObserverContext> context (this->_createContext());
	
	// Make buffer pdf valid, encode buf and save it to buffer
	std::string strbuf;
	utils::makeStreamPdfValid (buf.begin(), buf.end(), strbuf);
	encodeBuffer (strbuf);
	// Change length
	setLength (buffer.size());
	
	//Dispatch change 
	_objectChanged (context);
}


//
//
//
template<typename Checker>
void
CObjectStream<Checker>::setLength (size_t len) 
{
	
	//Lock
	dictionary.lockChange ();

	CInt _len (len);
	try
	{
		dictionary.setProperty ("Length", _len);
	
	}catch (ElementNotFoundException&)
	{
		dictionary.addProperty ("Length", _len);
	}

	// Unlock
	dictionary.unlockChange ();

}


//
// Helper methods
//

//
// 1) Dictionary of a BaseStream is always freed in ~BaseStream
// 2) In mem stream if needBuf variable is set, buf is freed
// 	needBuf is set in doDecryption
//
//
template<typename Checker>
::Object*
CObjectStream<Checker>::_makeXpdfObject () const
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (hasValidPdf (this));
	assert (hasValidRef (this));

	//
	// Set correct length. This can ONLY happen e.g. when length is an indirect
	// object
	// 
	if (getLength() != buffer.size())
	{
		kernelPrintDbg (debug::DBG_CRIT, "Length attribute of a stream is not valid. Changing it to buffer size.");

		// Get the reference
		boost::shared_ptr<IProperty> refLen = dictionary.getProperty ("Length");
		assert (isRef (refLen));
		if (!isRef(refLen))
			throw CObjInvalidObject ();
		
		// Get the length object
		boost::shared_ptr<IProperty> len = utils::getReferencedObject (refLen);
		if (isInt (len))
		{
			// Change is dispatched here 
			IProperty::getSmartCObjectPtr<CInt>(len)->writeValue (buffer.size());
			
		}else
		{
			assert (!"Length is not int object.");
			throw CObjInvalidOperation ();
		}
	}

	// Get xref
	XRef* xref = this->getPdf()->getCXref ();
	assert (xref);

	// Dictionary will be deallocated in ~BaseStream
	::Object* obj = utils::xpdfStreamObjFromBuffer (buffer, dictionary._makeXpdfObject());
	assert (NULL != obj);
	assert (objStream == obj->getType());
	return obj;
}


//
//
//
template<typename Checker>
template<typename Container>
void 
CObjectStream<Checker>::encodeBuffer (const Container& buf)
{
	kernelPrintDbg (debug::DBG_DBG, "");

	//
	// Create input filtes and add filters according to Filter item in
	// stream dictionary
	// 
	filters::InputStream in;
	std::vector<std::string> filters;
	getFilters (filters);
	
	// Try adding filters if one is not supported use none
	try {
		
		filters::CFilterFactory::addFilters (in, filters);
	
	}catch(FilterNotSupported&)
	{
		kernelPrintDbg (debug::DBG_DBG, "One of the filters is not supported, using none..");
		
		dictionary.delProperty ("Filter");
		// Clear buffer
		buffer.clear ();
		std::copy (buf.begin(), buf.end(), std::back_inserter (buffer));
		return;
	}
	
	// Clear buffer
	buffer.clear ();

	// Create input source from buffer
	boost::iostreams::stream<filters::buffer_source<Container> > input (buf);
	in.push (input);
	// Copy it to container
	Buffer::value_type c;
	in.get (c);
	while (!in.eof())
	{//\TODO Improve this !!!!
		buffer.push_back (c);
		in.get (c);
	}
	// Close the stream
	in.reset ();
}

//
//
//
template<typename Checker>
void
CObjectStream<Checker>::getStringRepresentation (std::string& str) const 
{
	kernelPrintDbg (debug::DBG_DBG, "");
	
	// Empty the string
	str.clear ();

	// Get dictionary string representation
	std::string strDict;
	dictionary.getStringRepresentation (str);

	// Put them together
	return utils::streamToString (strDict, buffer.begin(), buffer.end(), std::back_inserter(str));
}


//
//
//
template<typename Checker>
void
CObjectStream<Checker>::getDecodedStringRepresentation (std::string& str) const 
{
	kernelPrintDbg (debug::DBG_DBG, "");
	
	// Empty the string
	str.clear ();

	//
	// Make xpdf object and use its filters to get sane characters
	// 
	::Object* obj = _makeXpdfObject ();
	assert (NULL != obj);
	
	// Get the contents
	utils::getStringFromXpdfStream (str, *obj);

	// Clean-up
	utils::freeXpdfObject (obj);
}

//
//
//
template<typename Checker>
size_t
CObjectStream<Checker>::getLength () const 
{
	boost::shared_ptr<IProperty> len = utils::getReferencedObject (dictionary.getProperty("Length"));
	if (isInt (len))
	{
		int length;
		IProperty::getSmartCObjectPtr<CInt>(len)->getPropertyValue (length);
		assert (0 <= length);
		return length;
	
	}else
	{
		assert (!"Bad Length type in stream.");
		throw CObjInvalidObject ();
	}
}

//
//
//
template<typename Checker>
IProperty::ObserverContext* 
CObjectStream<Checker>::_createContext () const
{
	return new BasicObserverContext (boost::shared_ptr<IProperty> (new CNull ()));
}

//
//
//
template<typename Checker>
template<typename Container>
void
CObjectStream<Checker>::getFilters (Container& container) const
{
	boost::shared_ptr<IProperty> ip;
	//
	// Get optional value Filter
	//
	try	
	{
		ip = dictionary.getProperty ("Filter");
		
	}catch (ElementNotFoundException&)
	{
		// No filter found
		kernelPrintDbg (debug::DBG_DBG, "No filter found.");
		return;
	}

	//
	// If it is a name just store it
	// 
	if (isName (ip))
	{
		std::string fltr;
		boost::shared_ptr<const CName> name = IProperty::getSmartCObjectPtr<CName>(ip);
		name->getPropertyValue (fltr);
		container.push_back (fltr);
		
		kernelPrintDbg (debug::DBG_DBG, "Filter name:" << fltr);
	//
	// If it is an array, iterate through its properties
	//
	}else if (isArray (ip))
	{
		boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray>(ip);
		// Loop throug all children
		CArray::Value::iterator it = array->value.begin ();
		for (; it != array->value.end(); ++it)
		{
			std::string fltr;
			boost::shared_ptr<CName> name = IProperty::getSmartCObjectPtr<CName>(*it);
			name->getPropertyValue (fltr);
			container.push_back (fltr);
			
			kernelPrintDbg (debug::DBG_DBG, "Filter name:" << fltr);

		} // for (; it != array->value.end(); ++it)
	}
}

//
//
//
template<typename Checker>
template<typename Storage>
void 
CObjectStream<Checker>::_getAllChildObjects (Storage& store) const
{
	typename CDict::Value::const_iterator it = dictionary.value.begin ();
	for	(; it != dictionary.value.end (); ++it)
	{
		boost::shared_ptr<IProperty> ip = utils::getIPropertyFromItem (*it);
		store.push_back (ip);
	}
}

//
//
//
template<typename Checker>
void 
CObjectStream<Checker>::_objectChanged (boost::shared_ptr<const ObserverContext> context)
{
	// Do not notify anything if we are not in a valid pdf
	if (!hasValidPdf (this))
		return;
	assert (hasValidRef (this));

	// Set correct length
	if (getLength() != buffer.size())
	{
		kernelPrintDbg (debug::DBG_CRIT, "Length attribute of a stream is not valid. Changing it to buffer size.");
		setLength (buffer.size());
	}
	
	// Dispatch the change
	this->dispatchChange ();
	
	if (context)
	{
		// Clone new value
		boost::shared_ptr<IProperty> newValue (this->clone());
		// Fill it with correct values
		newValue->setPdf (this->getPdf());
		newValue->setIndiRef (this->getIndiRef());
		// Notify everybody about this change
		this->notifyObservers (newValue, context);

	}else
	{
		assert (!"Invalid context");
		throw CObjInvalidOperation ();
	}
}


//
// Parsing
//

//
//
//
template<typename Checker>
void
CObjectStream<Checker>::open ()
{
	kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser || !curObj.isNone ())
	{
		assert (!"Open an opened stream.");
		delete parser;
		parser = NULL;
		throw CObjInvalidOperation ();
	}
	
	::XRef* xref = (NULL != this->getPdf ()) ? this->getPdf ()->getCXref() : NULL;
	// Create xpdf object from current stream and parse it
	parser = new ::Parser (xref, new ::Lexer(xref, _makeXpdfObject()));
}

//
// 
//
template<typename Checker>
void
CObjectStream<Checker>::close ()
{
	kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser && !curObj.isNone())
	{
		curObj.free ();
		delete parser;
		parser = NULL;		

	}else
	{
		assert (!"Close a closed stream.");
		throw CObjInvalidOperation ();
	}
}


//
//
//
template<typename Checker>
bool
CObjectStream<Checker>::eof () const
{
	//kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser)
	{
		return (curObj.isEOF());

	}else
	{
		assert (!"Operation on closed stream.");
		throw CObjInvalidOperation ();
	}
}

//
//
//
template<typename Checker>
void
CObjectStream<Checker>::getXpdfObject (::Object& obj)
{
	assert (!eof());
	//kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser)
	{
		curObj.free ();
		
		parser->getObj (&curObj);
		assert (!curObj.isNone ());
		assert (!curObj.isNull ());
		assert (!curObj.isError ());
		
		curObj.copy (&obj);

	}else
	{
		assert (!"Operation on closed stream.");
		throw CObjInvalidOperation ();
	}
}

//
//
//
template<typename Checker>
::Stream*
CObjectStream<Checker>::getXpdfStream ()
{
	kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser)
	{
		return parser->getStream ();

	}else
	{
		assert (!"Operation on closed stream.");
		throw CObjInvalidOperation ();
	}
}

//
// Destructor
//
template<typename Checker>
CObjectStream<Checker>::~CObjectStream ()
{
	Checker check; check.objectDeleted (this);
	kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser)
	{
		assert (!"Stream not closed.");
		curObj.free ();
		delete parser;
		parser = NULL;
	}
}



//=====================================================================================
} // namespace pdfobjects
//=====================================================================================



#endif // _COBJECTI_H
