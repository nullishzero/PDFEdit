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

//
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
template<PropertyType Tp>
CObjectSimple<Tp>::CObjectSimple (CPdf& p, Object& o, const IndiRef& ref, bool isDirect) 
	: IProperty(static_cast<PropertyType>(o.getType()),isDirect), value(Value())
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pNull == Tp) || (pBool == Tp) || (pInt == Tp) || (pReal ==Tp) ||
				  (pString == Tp) || (pName == Tp) || (pRef == Tp),COBJECT_BAD_TYPE);
	assert (IProperty::type == Tp);
	printDbg (0,"CObjectSimple constructor.");
	
	
	// Save pdf
	IProperty::pdf = &p;	
	// Save id and gen id
	IProperty::setIndiRef (ref);
	
	if (!isDirect)
	{// Indirect object
			
		// Save indirect mapping 
		IProperty::pdf->setIndMapping (ref,this);
	}

	// Set object's value
	utils::simpleValueFromXpdfObj<Tp,Value&> (o,value);
}


//
// Public constructor
//
template<PropertyType Tp>
CObjectSimple<Tp>::CObjectSimple (CPdf& p, bool isDirect) : value(Value())
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pNull == Tp) || (pBool == Tp) || (pInt == Tp) || (pReal ==Tp) ||
				  (pString == Tp) || (pName == Tp) || (pRef == Tp),COBJECT_BAD_TYPE);
	printDbg (0,"CObjectSimple constructor.");

	// Save pdf
	IProperty::pdf = &p;
	// Save direct/indirect
	IProperty::setIsDirect (isDirect);

	if (!isDirect)
	{// Direct object
	
	}else
	{// Indirect object

		Ref ref = {42,43};
		//TODO !! Create ref
		// We have created an indirect object
		IProperty::setIndiRef (ref.num, ref.gen);
		pdf->setIndMapping (ref,this);
	}
}


//
//
//
template<PropertyType Tp>
void 
CObjectSimple<Tp>::getStringRepresentation (std::string& str) const
{
	utils::simpleValueToString<Tp> (value,str);
}


//
// Set string representation
//
template<PropertyType Tp>
void
CObjectSimple<Tp>::setStringRepresentation (const std::string& strO)
{
	STATIC_CHECK ((Tp != pNull),INCORRECT_USE_OF_setStringRepresentation_FUNCTION_FOR_pNULL_TYPE);
	STATIC_CHECK ((Tp == pBool) || (Tp == pInt) || (Tp == pReal) || (Tp == pString) || (Tp == pName) ||
			(Tp == pRef), INCORRECT_USE_OF_setStringRepresentation_FUNCTION_FOR_pNULL_TYPE);
	printDbg (0,"setStringRepresentation() " << strO);
	
	utils::simpleValueFromString (strO,value);

	// notify observers and dispatch the change
	_objectChanged ();
}



//
// Write a value.
// 
template<PropertyType Tp>
void
CObjectSimple<Tp>::writeValue (WriteType val)
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
template<PropertyType Tp>
void
CObjectSimple<Tp>::getPropertyValue (Value& val) const
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	printDbg (0,"getPropertyValue()");

	val = value;
}


//
// Just a hint that we can free this object
// This is a generic function for all types
//
template<PropertyType Tp>
void
CObjectSimple<Tp>::release()
{
	assert (NULL != IProperty::pdf);
	printDbg (0,"release()");

	assert (!"not implemented yet");
	
	//
	// If it is direct, we can free it
	//
	if (IProperty::isDirect ())
	{	
		//IndiRef* ind = IProperty::getIndiRef ();
		//pdf->getXrefWriter()->releaseObject (ind->num, ind->gen);
		delete this;
			
	}else
	{
		//
		// TODO: This is an indirect object, what to do?
		//
	}
}

//
//
//
template<PropertyType Tp>
void
CObjectSimple<Tp>::dispatchChange() const
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
//
//
template<PropertyType Tp>
Object*
CObjectSimple<Tp>::_makeXpdfObject () const
{
	return utils::simpleValueToXpdfObj<Tp,const Value&> (value);
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
template<PropertyType Tp>
CObjectComplex<Tp>::CObjectComplex ()
{
	STATIC_CHECK (pDict == Tp, COBJECT_BAD_TYPE);
	printDbg (0,"CObjectComplex pdf constructor.");
		
	IProperty::pdf = dynamic_cast<CPdf*>(this);
	IProperty::setIsDirect (false);
}


//
// Protected constructor
//
template<PropertyType Tp>
CObjectComplex<Tp>::CObjectComplex (CPdf& p, Object& o, const IndiRef& ref, bool isDirect) 
	: IProperty(static_cast<PropertyType>(o.getType()),isDirect)
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pArray == Tp) || (pStream == Tp) || (pDict == Tp),COBJECT_BAD_TYPE);
	assert (IProperty::type == Tp);
	printDbg (0,"CObjectComplex constructor.");
	
	// Save pdf
	IProperty::pdf = &p;
	// Save id and gen id
	IProperty::setIndiRef (ref);
	// Save direct/indirect
	IProperty::setIsDirect (isDirect);

	if (!isDirect)
	{// Indirect object

		// Save the mapping 
		pdf->setIndMapping (ref,this);
	}

	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<Tp,Value&> (*this, o, value);
}


//
// Public constructor
//
template<PropertyType Tp>
CObjectComplex<Tp>::CObjectComplex (CPdf& p, bool isDirect)
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pArray == Tp) || (pStream == Tp) || (pDict == Tp),COBJECT_BAD_TYPE);
	printDbg (0,"CObjectComplex constructor.");
	

	// Save pdf
	IProperty::pdf = &p;
	// Save direct/indirect
	IProperty::setIsDirect (isDirect);

	if (!isDirect)
	{// Indirect object
		
		Ref ref;
		//TODO !! get ref from somewhere
		//IProperty::setIndiRef (ref.num, ref.gen);
		pdf->setIndMapping (ref,this);
	}
}


//
//
//
template<PropertyType Tp>
void 
CObjectComplex<Tp>::getStringRepresentation (std::string& str) const 
{
	utils::complexValueToString<Tp> (value,str);
}


//
//
//
template<PropertyType Tp>
void 
CObjectComplex<Tp>::setStringRepresentation (const std::string& strO)
{
	assert (!"not implemented");	
}

//
// Write a value.
// 
template<PropertyType Tp>
void
CObjectComplex<Tp>::writeValue (WriteType val)
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
template<PropertyType Tp>
void
CObjectComplex<Tp>::dispatchChange() const
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
template <PropertyType Tp>
Object*
CObjectComplex<Tp>::_makeXpdfObject () const
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
template<PropertyType Tp>
void
CObjectComplex<Tp>::release()
{
	assert (NULL != IProperty::pdf);
	printDbg (0,"release()");

	assert (!"not implemented yet");
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

template<PropertyType Tp>
inline void
CObjectComplex<Tp>::getAllPropertyNames (std::list<std::string>& container) const
{
	STATIC_CHECK ((pArray != Tp), INCORRECT_USE_OF_getAllNames_FUNCTION); 
	utils::getAllNames (container,value);
}


//
// 
//
template<PropertyType Tp>
void
CObjectComplex<Tp>::delProperty (PropertyId id)
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
template<PropertyType Tp>
void
CObjectComplex<Tp>::addProperty (IProperty& newIp)
{
	STATIC_CHECK ((Tp == pArray), INCORRECT_USE_OF_addProperty_FUNCTION);
	assert (NULL == newIp.getPdf());
	printDbg (0,"addProperty("<< (unsigned int) &newIp << ")");

	if (IProperty::isDirect())
	{
		// Add it
		value.push_back (&newIp);
		// Inherit id and gen number
		newIp.setIndiRef (IProperty::ref);
		
	}else
	{
		assert (!"not implemented yet");	
	}

	// Inherit pdf
	newIp.setPdf (IProperty::pdf);
	// set isDirect and notify observers
	_objectChanged ();
}


//
// Correctly add an object (with name) can be done only to Dict and Stream object
//
template<PropertyType Tp>
void
CObjectComplex<Tp>::addProperty (const std::string& propertyName, IProperty& newIp)
{
	STATIC_CHECK ((Tp == pDict) || (Tp == pStream), INCORRECT_USE_OF_addProperty_FUNCTION);
	assert (NULL == newIp.getPdf());
	printDbg (0,"addProperty(, " << (unsigned int) &newIp << ", " << propertyName << ")");

	if (IProperty::isDirect())
	{
		// Store it
		value.push_back (std::make_pair (propertyName,&newIp));
		// Inherit id and gen number
		newIp.setIndiRef (IProperty::ref);
		
	}else
	{
		assert (!"not implemented yet");	
	}

	// Inherit pdf
	newIp.setPdf (IProperty::pdf);
	// notify observers and dispatch change
	_objectChanged ();
}


//
//
//
template<PropertyType Tp>
inline IProperty*
CObjectComplex<Tp>::setPropertyValue (PropertyId id, IProperty& newIp)
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
template<PropertyType Tp>
IProperty*
CObjectComplex<Tp>::getPropertyValue (PropertyId id) const
{
	printDbg (0,"getPropertyValue()");

	IndexComparator cmp (id);
	typename Value::iterator it = find_if (value.begin(),value.end(),cmp);

	if (it != value.end())
			return cmp.getIProperty ();
	else
			throw ObjInvalidPositionInComplex ();
}


} /* namespace pdfobjects */


#endif // COBJECTI_H
