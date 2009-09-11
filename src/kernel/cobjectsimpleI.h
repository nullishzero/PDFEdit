/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
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
template<PropertyType Tp>
CObjectSimple<Tp>::CObjectSimple (boost::weak_ptr<CPdf> p, const Object& o, const IndiRef& rf) : IProperty (p,rf), value(Value())
{
	//kernelPrintDbg (debug::DBG_DBG,"CObjectSimple <" << debug::getStringType<Tp>() << ">(p,o,rf) constructor.");
	
	// Set object's value
	utils::simpleValueFromXpdfObj<Tp,Value&> (o,value);
}


//
// Protected constructor, called when we have parsed an object
//
template<PropertyType Tp>
CObjectSimple<Tp>::CObjectSimple (const Object& o) : value(Value())
{
	//kernelPrintDbg (debug::DBG_DBG,"CObjectSimple <" << debug::getStringType<Tp>() << ">(o) constructor.");
	
	// Set object's value
	utils::simpleValueFromXpdfObj<Tp,Value&> (o,value);
}


//
// Public constructor
//
template<PropertyType Tp>
CObjectSimple<Tp>::CObjectSimple (const Value& val) : IProperty(), value(val)
{
	//kernelPrintDbg (debug::DBG_DBG,"CObjectSimple <" << debug::getStringType<Tp>() << ">(val) constructor.");
}


//
// Get methods
//

//
// Turn object to string
//
template<PropertyType Tp>
void 
CObjectSimple<Tp>::getStringRepresentation (std::string& str) const
{
	utils::simpleValueToString<Tp> (value, str);
}

//
// Get the value of an property
//
template<PropertyType Tp>
void
CObjectSimple<Tp>::getValue (Value& val) const
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	val = value;
}

//
// Get the value of an property
//
template<PropertyType Tp>
typename PropertyTraitSimple<Tp>::value
CObjectSimple<Tp>::getValue () const
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
template<PropertyType Tp>
void
CObjectSimple<Tp>::setStringRepresentation (const std::string& strO)
{
	STATIC_CHECK ((Tp != pNull),INCORRECT_USE_OF_setStringRepresentation_FUNCTION_FOR_pNULL_TYPE);
	//kernelPrintDbg (debug::DBG_DBG,"text:" << strO);

	// Check whether we can make the change
	this->canChange();

	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));

		// Create context in which the change occurs
		boost::shared_ptr<ObserverContext> context (this->_createContext());
		// Change our value
		utils::simpleValueFromString (strO, value);
		
		try {
			// notify observers and dispatch the change
			_objectChanged (context);
			
		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}

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
template<PropertyType Tp>
void
CObjectSimple<Tp>::setValue (WriteType val)
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_setValue_FUNCTION_FOR_pNULL_TYPE);
	//kernelPrintDbg (debug::DBG_DBG, "setValue() type: " << Tp);
	
	// Check whether we can make the change
	this->canChange();

	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// Create context in which the change occurs
		boost::shared_ptr<ObserverContext> context (this->_createContext());
		// Change the value
		value = val;
		
		try {
			// notify observers and dispatch the change
			_objectChanged (context);
			
		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}
	
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
template<PropertyType Tp>
::Object*
CObjectSimple<Tp>::_makeXpdfObject () const
{
	//kernelPrintDbg (debug::DBG_DBG,"_makeXpdfObject");
	return utils::simpleValueToXpdfObj<Tp,const Value&> (value);
}

//
//
//
template<PropertyType Tp>
void 
CObjectSimple<Tp>::_objectChanged (boost::shared_ptr<const ObserverContext> context)
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
		// Return new value (mh wanted it this way)
		boost::shared_ptr<IProperty> newValue (this, EmptyDeallocator<IProperty> ());
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
template<PropertyType Tp>
IProperty*
CObjectSimple<Tp>::doClone () const
{
	kernelPrintDbg (debug::DBG_DBG,"CObjectSimple::doClone ()");

	// Make new complex object
	// NOTE: We do not want to preserve any IProperty variable
	return new CObjectSimple<Tp> (value);
}


//=====================================================================================
} // namespace pdfobjects
//=====================================================================================



#endif // _COBJECTSIMPLEI_H
