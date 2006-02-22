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
template<typename T, typename U, PropertyType Tp> struct WriteProcessorTraitSimple; 
template<typename T, typename U> struct WriteProcessorTraitSimple<T,U,pBool>  
	{public: typedef struct utils::BoolWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTraitSimple<T,U,pInt>   
	{public: typedef struct utils::IntWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTraitSimple<T,U,pReal>  
	{public: typedef struct utils::RealWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTraitSimple<T,U,pString>
	{public: typedef struct utils::StringWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTraitSimple<T,U,pName>  
	{public: typedef struct utils::NameWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTraitSimple<T,U,pRef>   
	{public: typedef struct utils::RefWriter<T,U> WriteProcessor;};


		
		
//=====================================================================================
// CObjectSimple
//


//
// Protected constructor, called when we have parsed an object
//
template<PropertyType Tp>
CObjectSimple<Tp>::CObjectSimple (CPdf* p, Object* o, IndiRef& ref, bool isDirect) : IProperty(o),pdf(p)
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pNull == Tp) || (pBool == Tp) || (pInt == Tp) || (pReal ==Tp) ||
				  (pString == Tp) || (pName == Tp) || (pRef == Tp),COBJECT_BAD_TYPE);
	assert (NULL != p);
	assert (NULL != o);
	printDbg (0,"CObjectSimple constructor.");
	
	// Save id and gen id
	IProperty::setIndiRef (&ref);
	// Save mapping between Object and IProperty
	pdf->setObjectMapping (o,this);
	
	if (isDirect)
	{// Direct object
			
		// Set that it is an indirect object
		IProperty::setIsDirect (true);
	
	}else
	{// Indirect object

		// Save indirect mapping 
		pdf->setIndMapping (&ref,this);
		// Set that it is an indirect object
		IProperty::setIsDirect (false);
	
	}
}


//
// Public constructor
//
template<PropertyType Tp>
CObjectSimple<Tp>::CObjectSimple (CPdf* p, bool isDirect) : pdf(p)
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pNull == Tp) || (pBool == Tp) || (pInt == Tp) || (pReal ==Tp) ||
				  (pString == Tp) || (pName == Tp) || (pRef == Tp),COBJECT_BAD_TYPE);
/**///assert (NULL != p);
	printDbg (0,"CObjectSimple constructor.");
	

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
	switch (Tp)
	{
		case pNull:
				assert (!"Setting value to NULL object.");
				break;

		case pBool:
				if (strO == "true")
					writeValue (true);
				else if (strO == "false")
					writeValue (false);
				else
					throw ObjBadValueE ();
				break;

		case pInt:
				{
				std::stringstream ss (strO);
				int val = 0;
				ss >> val;
				writeValue (val);
				}
				break;
				
		case pReal:
				{
				double n = atof (strO.c_str());
				writeValue (n);
				}
				break;
				
		case pString:
		case pName:
				writeValue (strO);
				break;

		case pRef:
				{
				int id = 0;
				int gen = 0;
				std::stringstream ss (strO);
				ss >> id;
				ss >> gen;
				IndiRef ref = {id,gen};
				writeValue (ref);
				}
				break;

		default:
				assert (!"Bad Tp value.");
				break;
	}
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

	typename WriteProcessorTraitSimple<Object*, WriteType, Tp>::WriteProcessor wp;
	
	// Write it with specific writer (functor) according to the template parameter
	wp (IProperty::obj,val);
		
}


//
//
//
template<> template<>
inline bool 
CObjectSimple<pBool>::getPropertyValue () const 
	{return (0 != IProperty::obj->getBool());}

template<PropertyType Tp> template<>
inline int 
CObjectSimple<Tp>::getPropertyValue () const
{
	STATIC_CHECK ((pBool == Tp) || (pInt == Tp), INCORRECT_USE_OF_getPropertyvalue_FUNCTION);
	
	switch (Tp)
	{
		case pBool:
			return IProperty::obj->getBool ();
			
		case pInt:
			return IProperty::obj->getInt ();
			
		default:
				assert (0);
				break;
	}
}

template<PropertyType Tp> template<>
inline  double 
CObjectSimple<Tp>::getPropertyValue () const
{
	STATIC_CHECK ((pBool == Tp) || (pInt == Tp) || (pReal == Tp), INCORRECT_USE_OF_getPropertyvalue_FUNCTION);
	
	switch (Tp)
	{
		case pBool:
			return IProperty::obj->getBool ();
			
		case pInt:
		case pReal:
			return IProperty::obj->getNum ();
				
		default:
				assert (0);
				break;
	}
}

template<PropertyType Tp> template<>
std::string
CObjectSimple<Tp>::getPropertyValue () const
{
	string str;
	getStringRepresentation (str);
	return str;
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
	if (IProperty::isChanged ())
		{printDbg (1,"Warning: CObjectSimple::release(). Object has been changed, but was not saved.");}
	printDbg (0,"release()");

	//
	// If it is direct, we can free it
	//
	if (IProperty::isDirect ())
	{	
		pdf->delObjectMapping (IProperty::obj);
		utils::objectFree (IProperty::obj);
		delete this;
			
	}else
	{
		//
		// TODO: This is an indirect object, what to do?
		//
	}
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
CObjectComplex<Tp>::CObjectComplex (CPdf* p,SpecialObjectType objTp) : specialObjectType(objTp),pdf(p)
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
		pdf = dynamic_cast<CPdf*>(this);
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
CObjectComplex<Tp>::CObjectComplex (CPdf* p, Object* o, IndiRef& ref, bool isDirect, SpecialObjectType objTp) 
						:  IProperty(o),specialObjectType(objTp),pdf(p)
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pArray == Tp) || (pStream == Tp) || (pDict == Tp),COBJECT_BAD_TYPE);
	assert (sPdf != objTp);
	assert (NULL != p);
	assert (NULL != o);
	printDbg (0,"CObjectComplex constructor.");
	
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
CObjectComplex<Tp>::CObjectComplex (CPdf* p, IProperty* ip) : pdf(p) 
{
	STATIC_CHECK ((pOther != Tp) && (pOther1 != Tp) && (pOther2 != Tp) && (pOther3 != Tp),
					COBJECT_BAD_TYPE);
	STATIC_CHECK ((pArray == Tp) || (pStream == Tp) || (pDict == Tp),COBJECT_BAD_TYPE);
/**///assert	(NULL != p);
	printDbg (0,"CObjectComplex constructor.");
	
	if (NULL == ip)
	{// Indirect object
		
		//Ref ref;
		// Create the object
		//IProperty::obj = pdf->getXref()->createObject(static_cast<ObjType>(Tp),&ref);
		// We have created an indirect object
		//IProperty::setIsDirect (false);
		//IProperty::setIndiRef (ref.num, ref.gen);
		// Save the mapping, because we have created new indirect object
		//pdf->setObjectMapping (IProperty::getIndiRef(),this);
		
	}else
	{// Direct object
		
		//IProperty::obj = pdf->getXref()->createObject(static_cast<ObjType>(Tp));
		// Inherit id's from parent and inform that it is a direct object
		IProperty::setIndiRef (ip->getIndiRef());
		IProperty::setIsDirect (true);
	}

		
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
		
}


//
// Just a hint that we can free this object
//
// This is a generic function for all types
//
template<PropertyType Tp>
void
CObjectComplex<Tp>::release()
{
	assert (NULL != IProperty::obj);
	if (IProperty::isChanged ())
		{printDbg (1,"Warning: CObject::release(). Object has been changed, but was not saved.");}
	printDbg (0,"release()");

	if (IProperty::isDirect ())
	{	//	
		// This is a simple direct object, we can free it recursively if complex
		// 		
		std::vector<Object*> objects;
		utils::getAllXpdfObjects (*IProperty::obj, objects);
		
		processObjectFamily (objects, utils::ObjectDeleteProcessor<utils::CheckDirectObject> (pdf) );

		// We have freed everything "below" this object, now free Object
		utils::objectFree (IProperty::obj);
		//IProperty::obj == NULL;
		
		// At last, free this
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
	assert (!"Called getPropertyCount() on not supporting type...");
	return 0;	
}

/*
template<>
PropertyCount
CObjectComplex<pArray>::getPropertyCount () const
{
	return 0;//IProperty::obj->arrayGetLength();	
}*/

/*
template<>
PropertyCount
CObjectComplex<pDict>::getPropertyCount () const
{
	return 0;//IProperty::obj->dictGetLength();	
}

template<>
PropertyCount
CObjectComplex<pStream>::getPropertyCount () const
{
	return 0;//return IProperty::obj->streamGetDict()->getLength ();
}
*/


//
// This is nor really a generic function, it is just for pDict 
//
template<PropertyType Tp>
template<typename T>
void
CObjectComplex<Tp>::getAllPropertyNames (T& container) const
{
	STATIC_CHECK(Tp==pDict,BAD_COBJECT_TYPE);
	assert (!"Called getAllPropertyNames() on not supporting type...");
}


/*template<>
template<typename T>
void
CObjectComplex<pDict>::getAllPropertyNames (T& container) const
{
	assert (NULL != obj);
	//int size = IProperty:: dictGetLength();
	//for (int i = 0; i < size; i++)
	{
	//	container.push_back (string (obj->dictGetKey (i)) );
	}
	
}

*/

} /* namespace pdfobjects */


#endif // COBJECTI_H
