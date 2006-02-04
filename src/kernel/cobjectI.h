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
// General object functions
//
namespace {

/**
 * Returns object in string representation.
 */
void
objToString (Object* /*obj*/,string& /*str*/)
{
/*  assert (NULL != obj);
  printDbg (0,"objToString(" << (unsigned int)obj << ")");
  printDbg (0,"\tobjType = " << obj->getTypeName() );

  ostringstream oss;
  Object o;
  int i;

  switch (obj->getType()) 
  {
  
  case objBool:
    oss << ((obj->getBool()) ? "true" : "false");
    break;
  
  case objInt:
    oss << obj->getInt();
    break;
  
  case objReal:
    oss << obj->getReal ();
    break;
  
  case objString:
    oss << "("  << obj->getString()->getCString() << ")";
    break;
  
  case objName:
    oss << "/" << obj->getName();
    break;
  
  case objNull:
    oss << "null";
    break;
  
  case objArray:
    oss << "[";
	for (i = 0; i < obj->arrayGetLength(); ++i) 
	{
      if (i > 0)
		oss << " ";
      obj->arrayGetNF (i,&o);
      string tmp;
	  CPdf::objToString (&o,tmp);
	  oss << tmp;
      o.free();
    }
    oss << "]";
    break;
  
  case objDict:
    oss << "<<";
    for (i = 0; i <obj-> dictGetLength(); ++i) 
	{
      oss << " /" << obj->dictGetKey(i) << " ";
      obj->dictGetValNF(i, &o);
	  string tmp;
	  CPdf::objToString (&o,tmp);
	  oss << tmp;
      o.free();
    }
    oss << " >>";
    break;
  
  case objStream:
    oss << "<stream>";
    break;
  
  case objRef:
    oss << obj->getRefNum() << " " << obj->getRefGen() << " R";
    break;
  
  case objCmd:
  case objError:
  case objEOF:
  case objNone:
  default:
	assert (false);	
    break;
  }

  // convert oss to string
  str = oss.str ();
*/
}


	/**
	 * Get all objects that are "in" an object with recursion
	 * up to level 1. That means just direct descendats.
	 *
	 * @param o		xpdf Object.
	 * @param store storage that implements push_back() function.
	 */
	template <typename Storage>
	void getAllXpdfObjects (const Object& obj, Storage store)
	{
		assert (objCmd!=obj.getType());
		assert (objError!=obj.getType());
		assert (objEOF!=obj.getType());
		assert (objNone!=obj.getType());
		
		switch (obj.getType())
		{
			case objArray:
				int size = obj.arrayGetLength ();
				for (int i = 0; i < size; i++)
				{
					//store.push_back (obj->
				}
						
				break;
			
			case objDict:
				break;

			case objStream:
				assert (!"not implemented yet...");
				break;

			case objRef:
				assert (!"not implemented yet...");
				break;

			default:	// Null, Bool, Int, Real, String, Name
				return;
				break;
		}	
	}

};


		
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
// This is a generic function for all Simple Types
//
template<PropertyType Tp>
void
CObject<Tp>::release()
{
	assert (NULL != obj);
	if (IProperty::isChanged ())
		{printDbg (1,"Warning: CObject::release(). Object has been changed, but was not saved.");}
	printDbg (0,"release()");

	if (IProperty::isDirect ())
	{	//	
		// This is a simple direct object, we can free it recursivly if complex
		// 		
		//processObjectFamily (IProperty::obj,ObjectDeleteProcessor());
			
	}else
	{
		//
		// TODO: This is a simple indirect object, what to do?
		//
	}
	
};



}; /* namespace pdfobjects */


#endif // COBJECTI_H
