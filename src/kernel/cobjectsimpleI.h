// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cobjectsimpleI.h
 *         Created:  02/02/2006 
 *          Author:  jmisutka (06/01/19), 
 * 			
 * =====================================================================================
 */

#ifndef _COBJECTSIMPLEI_H
#define _COBJECTSIMPLEI_H

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
CObjectSimple<Tp,Checker>::getValue (Value& val) const
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	val = value;
}

//
// Get the value of an property
//
template<PropertyType Tp, typename Checker>
typename PropertyTraitSimple<Tp>::value
CObjectSimple<Tp,Checker>::getValue () const
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	return value;
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
CObjectSimple<Tp,Checker>::setValue (WriteType val)
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_setValue_FUNCTION_FOR_pNULL_TYPE);
	//kernelPrintDbg (debug::DBG_DBG, "setValue() type: " << Tp);

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
} // namespace pdfobjects
//=====================================================================================



#endif // _COBJECTSIMPLEI_H
