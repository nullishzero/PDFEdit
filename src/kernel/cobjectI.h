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

#ifndef COBJECTI_H
#define COBJECTI_H

#include <sstream>

// xpdf
#include <xpdf/Object.h>

// debug
#include "debug.h"

#include "observer.h"
#include "exceptions.h"

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
CObjectSimple<Tp,Checker>::CObjectSimple (CPdf& p, Object& o, const IndiRef& ref) : IProperty (&p), value(Value())
{
	Checker check (this, OPER_CREATE);
	printDbg (0,"CObjectSimple constructor.");
	
	
	// Save id and gen id
	IProperty::setIndiRef (ref);
	
	// Set object's value
	utils::simpleValueFromXpdfObj<Tp,Value&> (o,value);
}


//
// Public constructor
//
template<PropertyType Tp, typename Checker>
CObjectSimple<Tp,Checker>::CObjectSimple (CPdf& p) : IProperty(&pdf), value(Value())
{
	Checker check (this, OPER_CREATE);
	printDbg (0,"CObjectSimple constructor.");
}


//
//
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
	printDbg (0,"setStringRepresentation() " << strO);
	
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
//
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
// Just a hint that we can free this object
// This is a generic function for all types
//
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::release()
{
	assert (NULL != IProperty::pdf);
	printDbg (0,"release()");

	if (!_isIndirect())
	{
		delete this;

	}else
	{
		assert (!"not implemented yet");
	}
}

//
//
//
template<PropertyType Tp, typename Checker>
void
CObjectSimple<Tp,Checker>::dispatchChange() const
{
	STATIC_CHECK ((pNull != Tp), INCORRECT_USE_OF_dispatchChange_FUNCTION_FOR_pNULL_TYPE);
	assert (NULL != IProperty::pdf);
	printDbg (0,"dispatchChange() [" << (int)this << "]" );
	
	// Dispatch the change
	//IndiRef* ref = IProperty::getIndiRef ();
	// Get IProperty to nearest indirect object
	//IProperty* ip = pdf->getExistingProperty (ref);
	//assert (ref == ip->getIndiRef());
	//Object* obj = ip->_makeXpdfObject();
	// This function saves a COPY of xpdf object(s) do we have to delete it
	//pdf->getXrefWriter()->changeObject (ind->num, ind->gen,obj);
	//freeXpdfObject (obj);
}

//
// Helper function
//
template<PropertyType Tp, typename Checker>
inline Object*
CObjectSimple<Tp,Checker>::_makeXpdfObject () const
{
	return utils::simpleValueToXpdfObj<Tp,const Value&> (value);
}

//
// Helper function
//
template<PropertyType Tp, typename Checker>
inline bool
CObjectSimple<Tp,Checker>::_isIndirect () const
{
	if (this == pdf->getExistingProperty (IProperty::getIndiRef ()))
		return true;
	else
		return false;
}



//=====================================================================================
// CObjectComplex
//



//
// Special constructor for CPdf
//
// This is the only place, where Value is not what it should be after executing the constructor, 
// but it will be set in CPdf constructor.
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex ()
{
	STATIC_CHECK (pDict == Tp, COBJECT_BAD_TYPE);
	Checker check (this,OPER_CREATE);
	printDbg (0,"CObjectComplex pdf constructor.");
		
	IProperty::pdf = dynamic_cast<CPdf*>(this);
}


//
// Protected constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex (CPdf& p, Object& o, const IndiRef& ref) : IProperty (&p) 
{
	Checker check (this,OPER_CREATE);
	printDbg (0,"CObjectComplex constructor.");
	
	// Save id and gen id
	IProperty::setIndiRef (ref);

	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<Tp,Value&> (*this, o, value);
}


//
// Public constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex (CPdf& p) : IProperty (&p)
{
	Checker check (this,OPER_CREATE);
	printDbg (0,"CObjectComplex constructor.");
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
CObjectComplex<Tp,Checker>::setStringRepresentation (const std::string& strO)
{
	assert (!"not implemented");	
}

//
// Write a value.
// 
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::writeValue (WriteType val)
{
	printDbg (0,"writeValue()" << val);

//	typename WriteProcessorTraitComplex<WriteType, Tp>::WriteProcessor wp;
	// Write it with specific writer (functor) according to template parameter
//	wp (val,this);

	// notify observers and dispatch change
	_objectChanged ();
}


//
//
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::dispatchChange() const
{
	typedef std::vector<IProperty*> IPList;
	
	assert (NULL != IProperty::pdf);
	printDbg (0,"dispatchChange() [" << (int)this << "]" );
	
	// Dispatch the change
	//IndiRef* ref = IProperty::getIndiRef ();
	// Get IProperty to nearest indirect object
	//IProperty* ip = pdf->getExistingProperty (ref);
	//assert (ref == ip->getIndiRef());
	//Object* obj = ip->_makeXpdfObject();
	// This function saves a COPY of xpdf object(s) do we have to delete it
	//pdf->getXrefWriter()->changeObject (ind->num, ind->gen,obj);
	//freeXpdfObject (obj);
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
// Just a hint that we can free this object
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::release ()
{
	assert (NULL != IProperty::pdf);
	printDbg (0,"release()");

	if (!_isIndirect())
	{
		delete this;

	}else
	{
		assert (!"not implemented yet");
	}
}



//
// Template  member functions can't be virutal (at least according to nowadays specification)
//
// It seems that virtual funcions are always instantiated
//
template<>
inline void
CObjectComplex<pArray>::getAllPropertyNames (std::list<std::string>&) const
	{assert (0);}

template<PropertyType Tp, typename Checker>
inline void
CObjectComplex<Tp,Checker>::getAllPropertyNames (std::list<std::string>& container) const
{
	STATIC_CHECK ((pArray != Tp), INCORRECT_USE_OF_getAllNames_FUNCTION); 
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
	// We could have used getPropertyValue but we also need the iterator
	// 
	IndexComparator cmp (id);
	typename Value::iterator it = find_if (value.begin(),value.end(),cmp);
	if (it == value.end())
	{
		throw ObjInvalidPositionInComplex ();
	}
	IProperty* ip = cmp.getIProperty ();
	assert (ip);	
	
	// Delete that item
	value.erase (it);
	
	// Indicate that it can be freed
	ip->release ();

	// Indicate that this object has changed
	_objectChanged ();
}


//
// Correctly to add an object (without name) can be done only to Array object
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::addProperty (IProperty& newIp)
{
	STATIC_CHECK ((Tp == pArray), INCORRECT_USE_OF_addProperty_FUNCTION);
	assert (NULL == newIp.getPdf());
	printDbg (0,"addProperty("<< (unsigned int) &newIp << ")");

	// Add it
	value.push_back (&newIp);
	
	// Inherit id and gen number
	newIp.setIndiRef (IProperty::ref);
	// Inherit pdf
	newIp.setPdf (IProperty::pdf);
	
	// notify observers and dispatch change
	_objectChanged ();
}


//
// Correctly add an object (with name) can be done only to Dict and Stream object
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::addProperty (const std::string& propertyName, IProperty& newIp)
{
	STATIC_CHECK ((Tp == pDict) || (Tp == pStream), INCORRECT_USE_OF_addProperty_FUNCTION);
	assert (NULL == newIp.getPdf());
	printDbg (0,"addProperty(, " << (unsigned int) &newIp << ", " << propertyName << ")");

	// Store it
	value.push_back (std::make_pair (propertyName,&newIp));
	
	// Inherit id and gen number
	newIp.setIndiRef (IProperty::ref);
	// Inherit pdf
	newIp.setPdf (IProperty::pdf);

	// notify observers and dispatch change
	_objectChanged ();
}


//
//
//
template<PropertyType Tp, typename Checker>
inline IProperty*
CObjectComplex<Tp,Checker>::setPropertyValue (PropertyId id, IProperty& newIp)
{
	assert (NULL != IProperty::pdf);
	assert (NULL == newIp.getPdf());
	printDbg (0,"setPropertyValue()");

	//
	// Check the bounds, if fails an exception is thrown
	//
	IndexComparator cmp (id);
	typename Value::iterator itExist = find_if (value.begin(),value.end(),cmp);

	if (itExist == value.end())
			throw ObjInvalidPositionInComplex ();
	IProperty* oldIp = cmp.getIProperty ();
	//
	// Insert the element we want to add at the end, swap with the element we want to
	// delete and delete the last one
	//
	value.push_back (std::make_pair((*itExist).first,&newIp));
	typename Value::reverse_iterator itNew = value.rbegin ()++;
	
	// Swap and delete
	iter_swap (itExist,itNew);
	value.pop_back();

	// Delete IProperty
	oldIp->release ();

	return &newIp;
}

//
//
//
template<>
inline IProperty*
CObjectComplex<pArray>::setPropertyValue (PropertyId id, IProperty& newIp)
{
	assert (NULL != IProperty::pdf);
	assert (NULL == newIp.getPdf());
	printDbg (0,"setPropertyValue<pArray>()");

	//
	// Check the bounds, if fails an exception is thrown
	//
	IndexComparator cmp (id);
	Value::iterator itExist = find_if (value.begin(),value.end(),cmp);
	
	if (itExist == value.end())
			throw ObjInvalidPositionInComplex ();
	IProperty* oldIp = cmp.getIProperty ();
	//
	// Insert the element we want to add at the end, swap with the element we want to
	// delete and delete the last one
	//
	value.push_back (&newIp);
	Value::iterator itNew = value.end () - 1;
	
	// Swap and delete
	iter_swap (itExist,itNew);
	value.pop_back();
	// Delete the IProperty
	oldIp->release ();
	
	return &newIp;
}


//
//
//
template<PropertyType Tp, typename Checker>
IProperty*
CObjectComplex<Tp,Checker>::getPropertyValue (PropertyId id) const
{
	printDbg (0,"getPropertyValue()");

	IndexComparator cmp (id);
	typename Value::iterator it = find_if (value.begin(),value.end(),cmp);

	if (it != value.end())
			return cmp.getIProperty ();
	else
			throw ObjInvalidPositionInComplex ();
}


//
// Helper function
//
template<PropertyType Tp, typename Checker>
inline bool
CObjectComplex<Tp,Checker>::_isIndirect () const
{
	if (this == pdf->getExistingProperty (IProperty::getIndiRef ()))
		return true;
	else
		return false;
}




} /* namespace pdfobjects */


#endif // COBJECTI_H
