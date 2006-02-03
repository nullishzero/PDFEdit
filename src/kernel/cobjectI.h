/** 
 * =====================================================================================
 *        Filename:  cobjectI.h
 *     Description:  Header file containing implementation of IProperty class.
 *         Created:  02/02/2006 
 *        Revision:  none
 *          Author:  jmisutka (06/01/19), 
 * 			
 * 				2006/02/02   -- writeValue finished (not tested)
 * =====================================================================================
 */

#ifndef COBJECTI_H
#define COBJECTI_H


#include <xpdf/Object.h>

#include "cpdf.h"
#include "observer.h"
#include "exceptions.h"


//=====================================================================================

namespace pdfobjects 
{

//
// WriteProcessors
//
// We know that Storage is xpdf Object and value type depends on each writer type
//
namespace
{
	template<typename Storage, typename Val>
	struct BoolWriter
	{
		public:
			void 
			operator() (Storage obj, Val val)
			{
				obj->initBool (GBool(val));
			}
	};

	template<typename Storage, typename Val>
	struct IntWriter
	{
		public:
			void 
			operator() (Storage obj, Val val)
			{
				obj->initInt (val);
			}
	};

	template<typename Storage, typename Val>
	struct RealWriter
	{
		public:
			void 
			operator() (Storage obj, Val val)
			{
				obj->initReal (val);
			}
	};

	template<typename Storage, typename Val>
	struct StringWriter
	{
		public:
			void 
			operator() (Storage obj, Val val)
			{
				obj->initString (GString(val.c_str()));
			}
	};

	template<typename Storage, typename Val>
	struct NameWriter
	{
		public:
			void 
			operator() (Storage obj, Val val)
			{
				obj->initName (val.c_str());		
			}
	};

	template<typename Storage, typename Val>
	struct ComplexWriter
	{
		public:
			void 
			operator() (Storage obj, Val val)
			{
				//
				// We will get string here
				// -- if setStringRepresentation throws exception, just propagate it
				// 
				setStringRepresentation (val);
			}
	};

	template<typename Storage, typename Val>
	struct RefWriter
	{
		public:
			void 
			operator() (Storage obj, Val val)
			{
				obj->initRef (val.num, val.gen);
			}
	};

/**
 * This type trait holds information which writer class to use with specific CObject class.
 * REMARK: If we want to abandon using Object as information holder, you need to rewrite
 * these classes to be able to write values.
 * 
 * One class can be used with more CObject classes.
 */
template<typename T, typename U, PropertyType Tp> struct WriteProcessorTrait; 
template<typename T, typename U> struct WriteProcessorTrait<T,U,pBool> 	{public: typedef struct BoolWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTrait<T,U,pInt> 	{public: typedef struct IntWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTrait<T,U,pReal> 	{public: typedef struct RealWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTrait<T,U,pString>{public: typedef struct StringWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTrait<T,U,pName> 	{public: typedef struct NameWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTrait<T,U,pArray> {public: typedef struct ComplexWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTrait<T,U,pStream>{public: typedef struct ComplexWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTrait<T,U,pDict> 	{public: typedef struct ComplexWriter<T,U> WriteProcessor;};
template<typename T, typename U> struct WriteProcessorTrait<T,U,pRef> 	{public: typedef struct RefWriter<T,U> WriteProcessor;};

};

		
//============================== CObject implementation ===============================================



//
// Protected constructor
//
template<PropertyType Tp>
CObject<Tp>::CObject (CPdf* p,SpecialObjectType objTp) : specialObjectType(objTp),pdf(p)
{
	STATIC_CHECK (pOther != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther1 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther2 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther3 != Tp,COBJECT_BAD_TYPE);
	assert ((NULL != p) || (sPdf == objTp));
	printDbg (0,"CObject constructor.");
		
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
	//pdf->setPropertyMapping (IProperty::getIndiRef(),this);
};


//
// Protected constructor
//
template<PropertyType Tp>
CObject<Tp>::CObject (CPdf* p, Object* o, IndiRef& ref, bool isDirect, SpecialObjectType objTp) 
						:  IProperty(o),specialObjectType(objTp),pdf(p)
{
	STATIC_CHECK (pOther != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther1 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther2 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther3 != Tp,COBJECT_BAD_TYPE);
	assert (sPdf != objTp);
	assert (NULL != p);
	assert (NULL != o);
	printDbg (0,"CObject constructor.");
	
	// Save id and gen id
	IProperty::setIndiRef (&ref);
	
	if (isDirect)
	{// Direct object
			
		// Set that it is an indirect object
		IProperty::setIsDirect (true);
	
	}else
	{// Indirect object

		// Save the mapping 
		pdf->setPropertyMapping (&ref,this);
		// Set that it is an indirect object
		IProperty::setIsDirect (false);
	
	}
};


//
// Public constructor
//
template<PropertyType Tp>
CObject<Tp>::CObject (CPdf* p, IProperty* ip) 
{
	STATIC_CHECK (pOther != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther1 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther2 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther3 != Tp,COBJECT_BAD_TYPE);
/**///assert	(NULL != p);
	printDbg (0,"CObject constructor.");
	
	if (NULL == ip)
	{// Indirect object
		
		//Ref ref;
		// Create the object
		//IProperty::obj = pdf->getXref()->createObject(static_cast<ObjType>(Tp),&ref);
		// We have created an indirect object
		//IProperty::setIsDirect (false);
		//IProperty::setIndiRef (ref.num, ref.gen);
		// Save the mapping, because we have created new indirect object
		//pdf->setPropertyMapping (IProperty::getIndiRef(),this);
		
	}else
	{// Direct object
		
		//IProperty::obj = pdf->getXref()->createObject(static_cast<ObjType>(Tp));
		// Inherit id's from parent and inform that it is a direct object
		IProperty::setIndiRef (ip->getIndiRef());
		IProperty::setIsDirect (true);
	}

		
};



//
// Better reimplement Object::print(File* f) because of
// the File parameter.
//  
template<PropertyType Tp>
void
CObject<Tp>::getStringRepresentation (string& str) const
{
  assert (NULL != IProperty::obj);
  assert (obj->getType() == (ObjType)Tp);
  
  CPdf::objToString (IProperty::obj,str);
};



//
// Notify all observers that this object has changed
//
template<PropertyType Tp>
void
CObject<Tp>::notifyObservers ()
{
  ObserverList::iterator it = IProperty::observers.begin ();

  for (; it != IProperty::observers.end(); it++)
      (*it)->notify (this);
};




/**
 * DO NOT use this function, you really DO NOT want to write to Null object.
 */
template<>
void
CObject<pNull>::writeValue (WriteType)
{
	STATIC_CHECK (false,INCORRECT_USE_OF_FUNCTION);		
};

//
// Write a value.
// 
template<PropertyType Tp>
void
CObject<Tp>::writeValue (WriteType val)
{
	assert (obj->getType() == (ObjType)pBool);
	printDbg (0,"writeValue()");

	typename WriteProcessorTrait<Object*, WriteType, Tp>::WriteProcessor wp;
	
	// Write it with specific writer (functor) according to template parameter
	wp (IProperty::obj,val);
		
};


//
// Just a hint that we can free this object
//
template<PropertyType Tp>
void
CObject<Tp>::release()
{
	printDbg (0,"release()");

	
};


template<>
void
CObject<pRef>::release()
{
	cout << "pRef";;
};


template<>
void
CObject<pDict>::release()
{
	cout << "pDict:";;
};



}; /* namespace pdfobjects */


#endif // COBJECTI_H
