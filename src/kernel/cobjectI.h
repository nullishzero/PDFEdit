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


/**
 * This type trait holds information which writer class to use with specific CObject class.
 * REMARK: If we want to abandon using Object as information holder, you need to rewrite
 * these classes to be able to write values.
 * 
 * One class can be used with more CObject classes.
 */
template<typename T, typename U, PropertyType Tp> struct ProcessorTraitSimple; 
template<typename T, typename U> struct ProcessorTraitSimple<T,U,pBool>  
	{public: typedef struct utils::BoolWriter<T,U> WriteProcessor;
		 typedef struct utils::BoolReader<T,U> ReadProcessor;};
template<typename T, typename U> struct ProcessorTraitSimple<T,U,pInt>   
	{public: typedef struct utils::IntWriter<T,U> WriteProcessor;
		 typedef struct utils::IntReader<T,U> ReadProcessor;};
template<typename T, typename U> struct ProcessorTraitSimple<T,U,pReal>  
	{public: typedef struct utils::RealWriter<T,U> WriteProcessor;
		 typedef struct utils::RealReader<T,U> ReadProcessor;};
template<typename T, typename U> struct ProcessorTraitSimple<T,U,pString>
	{public: typedef struct utils::StringWriter<T,U> WriteProcessor;
		 typedef struct utils::StringReader<T,U> ReadProcessor;};
template<typename T, typename U> struct ProcessorTraitSimple<T,U,pName>  
	{public: typedef struct utils::NameWriter<T,U> WriteProcessor;
		 typedef struct utils::NameReader<T,U> ReadProcessor;};
template<typename T, typename U> struct ProcessorTraitSimple<T,U,pRef>   
	{public: typedef struct utils::RefWriter<T,U> WriteProcessor;
		 typedef struct utils::RefReader<T,U> ReadProcessor;};


		
		
//=====================================================================================
// CObjectSimple
//


//
// Protected constructor, called when we have parsed an object
//
template<PropertyType Tp>
CObjectSimple<Tp>::CObjectSimple (CPdf& p, Object& o, IndiRef& ref, bool isDirect) : IProperty(o)
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pNull == Tp) || (pBool == Tp) || (pInt == Tp) || (pReal ==Tp) ||
				  (pString == Tp) || (pName == Tp) || (pRef == Tp),COBJECT_BAD_TYPE);
	printDbg (0,"CObjectSimple constructor.");
	
	// Save pdf
	IProperty::pdf = &p;	
	// Save id and gen id
	IProperty::setIndiRef (&ref);
	// Save mapping between Object and IProperty
	IProperty::pdf->setObjectMapping (&o,this);
	
	if (isDirect)
	{// Direct object
			
		// Set that it is an indirect object
		IProperty::setIsDirect (true);
	
	}else
	{// Indirect object

		// Save indirect mapping 
		IProperty::pdf->setIndMapping (&ref,this);
		// Set that it is an indirect object
		IProperty::setIsDirect (false);
	
	}
}


//
// Public constructor
//
template<PropertyType Tp>
CObjectSimple<Tp>::CObjectSimple (CPdf& p, bool isDirect)
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pNull == Tp) || (pBool == Tp) || (pInt == Tp) || (pReal ==Tp) ||
				  (pString == Tp) || (pName == Tp) || (pRef == Tp),COBJECT_BAD_TYPE);
	printDbg (0,"CObjectSimple constructor.");

	// Save pdf
	IProperty::pdf = &p;

	if (isDirect)
	{// Direct object
			
		//IProperty::obj = pdf->getXref()->createObject(static_cast<ObjType>(Tp));
		IProperty::setIsDirect (true);
	
	}else
	{// Indirect object

		//Ref ref;
		// Create the object
		//IProperty::obj = pdf->getXref()->createObject(static_cast<ObjType>(Tp),&ref);
		// We have created an indirect object
		IProperty::setIsDirect (false);
		//IProperty::setIndiRef (ref.num, ref.gen);
		//pdf->setIndMapping (ref,this);
	}

	// Save the mapping
	//pdf->setObjectMapping (IProperty::obj,this);
}


//
// Better reimplement Object::print(File* f) because of
// the File parameter.
//  
template<PropertyType Tp>
void
CObjectSimple<Tp>::getStringRepresentation (std::string& str) const
{
  assert (NULL != IProperty::obj);
  assert (obj->getType() == (ObjType)Tp);
  
  utils::simpleObjToString (IProperty::obj,str);
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
	
	ReturnType val;
	utils::getSimpleValueFromString (strO,val); 
	
	writeValue (val);	
}



//
// Write a value.
// 
template<PropertyType Tp>
void
CObjectSimple<Tp>::writeValue (WriteType val)
{
	assert (obj != NULL);
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	printDbg (0,"writeValue()");

	typename ProcessorTraitSimple<Object*, WriteType, Tp>::WriteProcessor wp;
	
	// Write it with specific writer (functor) according to the template parameter
	wp (IProperty::obj,val);

	// set isChanged and notify observers
	_objectChanged ();
}	


//
//
//
template<PropertyType Tp>
void
CObjectSimple<Tp>::getPropertyValue (ReturnType& val) const
{
	STATIC_CHECK ((pNull != Tp),INCORRECT_USE_OF_writeValue_FUNCTION_FOR_pNULL_TYPE);
	printDbg (0,"getPropertyValue()");

	typename ProcessorTraitSimple<Object*, ReturnType&, Tp>::ReadProcessor rp;
	
	// Read it with specific reader (functor)
	rp (IProperty::obj,val);
}


//
// Just a hint that we can free this object
// This is a generic function for all types
//
template<PropertyType Tp>
void
CObjectSimple<Tp>::release()
{
	assert (NULL != obj);
	assert (NULL != IProperty::pdf);
	if (IProperty::isChanged ())
		{printDbg (1,"Warning: CObjectSimple::release(). Object has been changed, but was not saved.");}
	printDbg (0,"release()");

	//
	// If it is direct, we can free it
	//
	if (IProperty::isDirect ())
	{	
		pdf->delObjectMapping (IProperty::obj);
		//IndiRef* ind = IProperty::getIndiRef ();
		//pdf->getXrefWriter()->releaseObject (ind->num, ind->gen);
		utils::objectFree (IProperty::obj);
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
CObjectSimple<Tp>::dispatchChange(bool makeValidCheck) const
{
	STATIC_CHECK ((pNull != Tp), INCORRECT_USE_OF_dispatchChange_FUNCTION_FOR_pNULL_TYPE);
	assert (IProperty::isChanged());
	assert (NULL != IProperty::pdf);
	printDbg (0,"dispatchChange() [" << (int)this << "]" );
	

	if (makeValidCheck)
	{
		// Validate the object
		//if (!CPDF::vali....(IProperty::obj))
		//	throw ObjInvalidObject;
	}
	
	// Dispatch the change
	IndiRef* ind = IProperty::getIndiRef ();
	//pdf->getXrefWriter()->changeObject (ind->num, ind->gen,getRawObject ());
	IProperty::setIsChanged (false);
	
}





//=====================================================================================
// CObjectComplex
//

template<typename T, typename U, PropertyType Tp> struct WriteProcessorTraitComplex; 
template<typename T, typename U> struct WriteProcessorTraitComplex<T,U,pStream>
	{public: typedef struct utils::ComplexWriter<T,U,CObjectComplex<pStream> > WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTraitComplex<T,U,pArray>
	{public: typedef struct utils::ComplexWriter<T,U,CObjectComplex<pArray> > WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTraitComplex<T,U,pDict>
	{public: typedef struct utils::ComplexWriter<T,U,CObjectComplex<pDict> > WriteProcessor;};



//
//
//
template<PropertyType Tp>
CObjectComplex<Tp>::CObjectComplex (CPdf* p,SpecialObjectType objTp) : specialObjectType(objTp)
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pArray == Tp) || (pStream == Tp) || (pDict == Tp),COBJECT_BAD_TYPE);
	assert ((NULL != p) || (sPdf == objTp));
	printDbg (0,"CObjectComplex constructor.");
		
	//
	// If CPdf is beeing created, we don't have valid p
	// 
	if (sPdf == objTp)
	{
		assert (NULL == p);
		IProperty::pdf = dynamic_cast<CPdf*>(this);
	}else
	{
		IProperty::pdf = p;
	}
	
	//Ref ref;
	// Create the object
	//IProperty::obj = pdf->getXref()->createObject(static_cast<ObjType>(Tp,)&ref);
	// We have created an indirect object
	//IProperty::setIsDirect (false);
	//IProperty::setIndiRef (ref.num, ref.gen);
	// Save the mapping, because we have created new indirect object
	//pdf->setObjectMapping (IProperty::getIndiRef(),this);
}


//
// Protected constructor
//
template<PropertyType Tp>
CObjectComplex<Tp>::CObjectComplex (CPdf& p, Object& o, IndiRef& ref, bool isDirect, SpecialObjectType objTp) 
						:  IProperty(o),specialObjectType(objTp)
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pArray == Tp) || (pStream == Tp) || (pDict == Tp),COBJECT_BAD_TYPE);
	assert (sPdf != objTp);
	printDbg (0,"CObjectComplex constructor.");
	
	// Save pdf
	IProperty::pdf = &p;
	// Save id and gen id
	IProperty::setIndiRef (&ref);
	
	if (isDirect)
	{// Direct object
			
		// Set that it is an indirect object
		IProperty::setIsDirect (true);
	
	}else
	{// Indirect object

		// Save the mapping 
		pdf->setObjectMapping (&ref,this);
		// Set that it is an indirect object
		IProperty::setIsDirect (false);
	}
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
	
	if (isDirect)
	{// Direct object

		//IProperty::obj = pdf->getXref()->createObject(static_cast<ObjType>(Tp));
		IProperty::setIsDirect (true);
	}else
	{// Indirect object
		
		//Ref ref;
		// Create the object
		//IProperty::obj = pdf->getXref()->createObject(static_cast<ObjType>(Tp),&ref);
		// We have created an indirect object
		IProperty::setIsDirect (false);
		//IProperty::setIndiRef (ref.num, ref.gen);
		//pdf->setIndMapping (ref,this);
	}
	
	// Save the mapping
	//pdf->setObjectMapping (IProperty::getIndiRef(),this);
}



//
// Better reimplement Object::print(File* f) because of
// the File parameter.
//  
template<PropertyType Tp>
void
CObjectComplex<Tp>::getStringRepresentation (std::string& str) const
{
  assert (NULL != IProperty::obj);
  assert (obj->getType() == (ObjType)Tp);
  
  utils::complexObjToString (IProperty::obj,str);
}


//
// Write a value.
// 
template<PropertyType Tp>
void
CObjectComplex<Tp>::writeValue (WriteType val)
{
	assert (NULL != obj);
	printDbg (0,"writeValue()");

	typename WriteProcessorTraitComplex<Object*, WriteType, Tp>::WriteProcessor wp;
	
	// Write it with specific writer (functor) according to template parameter
	wp (IProperty::obj,val,this);

	// set isChanged and notify observers
	_objectChanged ();
}

//
// Just a hint that we can free this object
// This is a generic function for all types
//
template<PropertyType Tp>
void
CObjectComplex<Tp>::release()
{
	assert (NULL != obj);
	assert (NULL != IProperty::pdf);
	if (IProperty::isChanged ())
		{printDbg (1,"Warning: CObject::release(). Object has been changed, but was not saved.");}
	printDbg (0,"release()");

	//
	// If it is direct, we can free it
	//
	if (IProperty::isDirect ())
	{	
		std::vector<Object*> objects;
		utils::getAllXpdfObjects (*IProperty::obj, objects);
		
		processObjectFamily (objects, utils::ObjectDeleteProcessor<utils::CheckDirectObject> (pdf) );
	
		pdf->delObjectMapping (IProperty::obj);
		//IndiRef* ind = IProperty::getIndiRef ();
		//pdf->getXrefWriter()->releaseObject (ind->num, ind->gen);

		// We have freed everything "below" this object, now free Object
		utils::objectFree (IProperty::obj);
		delete this;
			
	}else
	{
		//
		// TODO: This is an indirect object, what to do?
		//
	}
	
}

//
// Generic function is disabled for now...
//
template<PropertyType Tp>
PropertyCount
CObjectComplex<Tp>::getPropertyCount () const
{
	STATIC_CHECK ( (pArray==Tp) || (Tp==pDict) || (Tp==pStream),
					BAD_COBJECT_TYPE);
	
	switch (Tp)
	{
		case pArray:
			return IProperty::obj->arrayGetLength ();

		case pDict:
			return IProperty::obj->dictGetLength ();

		case pStream:
			assert (!"...not supported...");
			IProperty::obj->streamGetDict()->getLength ();
			break;

		default:
			assert (0);
			break;
	}
}



//
// Template  member functions can't be virutal (at least according to nowadays specification)
//
template<PropertyType Tp>
inline void
CObjectComplex<Tp>::getAllPropertyNames (std::vector<std::string>& container) const
	{_getAllPropertyNames (container);}
template<PropertyType Tp>
inline void
CObjectComplex<Tp>::getAllPropertyNames (std::list<std::string>& container) const
	{_getAllPropertyNames (container);}
template<PropertyType Tp>
inline void
CObjectComplex<Tp>::getAllPropertyNames (std::deque<std::string>& container) const
	{_getAllPropertyNames (container);}


//
//
//
template<PropertyType Tp>
template<typename T>
inline void
CObjectComplex<Tp>::_getAllPropertyNames (T& container) const
{
/*	STATIC_CHECK((Tp == pDict) || (Tp == pStream),INCORRECT_USE_OF_getAllPropertyNames_FUNCTION);
	
	Dict* dict = (Tp==pStream) ?  IProperty::obj->streamGetDict () : IProperty::obj->getDict ();
	assert (NULL != dict);

	int size = dict->dictGetLength();
	for (int i = 0; i < size; i++)
	{
		container.push_back (std::string (dict->getKey (i)) );
	}
*/}



//
// 
//
template<PropertyType Tp>
void
CObjectComplex<Tp>::delProperty (PropertyId id)
{
	assert (NULL != IProperty::pdf);
	assert (NULL != IProperty::obj);
	printDbg (0,"delProperty()");

	Object* o = utils::getObjectAtPos (*IProperty::obj,id);
	IProperty* ip = pdf->getExistingProperty (o);
	
	if (NULL == ip)
	{ // No mapping exists
		
		utils::objectFree (o);
	}else
	{
		ip->release ();
	}
	
	utils::removeObjectAtPos (*IProperty::obj,id);
	
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
	assert (NULL != IProperty::obj);
	assert (NULL != newIp.getRawObject() );
	assert (NULL == newIp.getPdf());
	printDbg (0,"addProperty(" << id << ", " << (unsigned int) &newIp << ")");

	if (IProperty::isDirect())
	{
		// Add it
		IProperty::obj->arrayAdd (newIp.getRawObject());	
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
CObjectComplex<Tp>::addProperty (IProperty& newIp, const std::string& propertyName)
{
	STATIC_CHECK ((Tp == pDict) || (Tp == pStream), INCORRECT_USE_OF_addProperty_FUNCTION);
	assert (NULL != IProperty::obj);
	assert (NULL != newIp.getRawObject() );
	assert (NULL == newIp.getPdf());
	printDbg (0,"addProperty(, " << (unsigned int) &newIp << ", " << propertyName << ")");

	if (IProperty::isDirect())
	{
		// Alloc name and fill it
		char* name = new char[propertyName.length() + 1];
		strncpy (name,propertyName.c_str(),propertyName.length());
		name [propertyName.length()] = '\0';
		
		//Dict* dict = (pStream == newIp.getCObjectType()) ? obj->getDict() : obj->streamGetDict();
		//dict->add (name, const_cast<Object*>(newIp.getRawObject()));
		
		// Inherit id and gen number
		newIp.setIndiRef (IProperty::ref);
		
	}else
	{
		assert (!"not implemented yet");	
	}

	// Inherit pdf
	newIp.setPdf (IProperty::pdf);
	// set isChanged and notify observers
	_objectChanged ();
}


//
//
//
template<PropertyType Tp>
void
CObjectComplex<Tp>::setPropertyValue (PropertyId /*id*/, const IProperty& /*ip*/)
{
//
// ??? what here?
//
}





} /* namespace pdfobjects */


#endif // COBJECTI_H
