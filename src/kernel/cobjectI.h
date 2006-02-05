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
	/* 	assert (NULL != obj);
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
	 * REMARK: Object can't be const because of the superb xpdf implementation.
	 *
	 * @param o		xpdf Object.
	 * @param store storage that implements push_back() function.
	 */
	template <typename Storage>
	void getAllXpdfObjects (Object& obj, Storage& /*store*/) 
	{
		assert (objCmd!=obj.getType());
		assert (objError!=obj.getType());
		assert (objEOF!=obj.getType());
		assert (objNone!=obj.getType());
		
		switch (obj.getType())
		{
			case objArray:
			{	//int size = obj.arrayGetLength ();
				//for (int i = 0; i < size; i++)
				{
					//store.push_back (obj.getArray()->elems[i]);
				}
			}	return;	
				break;
			
			case objDict:
			{	//int size = obj.dictGetLength ();
				//for (int i = 0; i < size; i++)
				{
					//store.push_back (obj.getDict()->entries[i].val);
				}
			}	return;	
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

	/**
	 * Performs Processor.operator() action on all supplied objects
	 *
	 * @param store 		Objects in a container. Iterator must be implemented.
	 * @param objProcessor	Object processor.
	 */
	template<typename ObjectStorage, typename ObjectProcessor>
	void
	processObjectFamily (ObjectStorage& store, ObjectProcessor objProcessor)
	{
		//
		// Perform an action on all objects
		// 
		for (typename ObjectStorage::iterator it = store.begin(); it != store.end(); it++)
		{
			objProcessor (*it);
		}
	}


	/**
	 * Empty check struct.
	 */
	struct NoCheck
	{public: void operator() (Object*){};};
	
	/**
	 * Direct object check struct.
	 */
	struct CheckDirectObject
	{
	public: 
		void operator() (Object* obj)
		{
			switch (obj->getType())
			{
				case objDict:
					//assert (0 == obj->getDict()->decRef());
					//obj->getDict()->incRef();
					break;
						
				case objArray:
				//	assert (0 == obj->getArray()->decRef());
				//	obj->getArray()->incRef();
					break;
						
				case objStream:
				//	assert (0 == obj->getStream()->decRef());
				//	obj->getStream()->incRef();
					break;
						
				default:
					return;
			}
		};
	};

	
	/**
	 * Performs a Release() function on an IProperty obtained from Object.
	 *
	 * This is a specialization of Xpdf object processor with the purpose to
	 * destroy these objects.
	 *
	 * REMARK: We have to be a bit careful because Object::free() is a recursive
	 * function which destroys e.g. all members of Dict etc...
	 *
	 * THIS CAN LEAD TO MEMORY LEAKS. E.g. we want to destroy a direct object which is a 
	 * dictionary and has reference counter set to > 1. Then it will be not deleted.
	 *
	 * We can make a checks, with a class passed to this struct as a template argument
	 */
	template<typename Check = NoCheck>
	class ObjectDeleteProcessor
	{
	 private:
		CPdf& pdf;
		Check check;

	 public:
		ObjectDeleteProcessor (CPdf& _pdf) : pdf(_pdf) {};
		
		void operator() (Object* obj)
		{
			IProperty* ip = pdf.getExistingProperty (obj);
			
			//
			// There is no IProperty associated with this obj so free it
			// If complex, then it will be freed recursively
			// 
			if (NULL == ip)
			{
				// make a check if any
				check (obj);	
				// free it
				obj->free ();
			
			}else
			{
				ip->release ();
			}
		};
	};


	/**
	 * Free an object. We assume that all child objects (if any)
	 * have been already freed.
	 *
	 * Copy & paste from Object.h
	 * 
	 * @param obj	Object to be freed.
	 */
	void
	objectFree (Object* obj)
	{
		printDbg (1,"objectFree(" << (unsigned int)obj << ")");
		std::string str;
		objToString (obj,str); 
		printDbg (1,"\t..." << str);
		
  		switch (obj->getType()) 
		{
		  
		  case objString:
		    //delete obj->string;
			//obj->string = NULL;
		    break;
		 
		  case objName:
		    //gfree(name);
		    break;

		  case objArray:
			//
			// A bit of a hack. That means do not free any nested
			// objects, we have already done this.
			//
			//obj->getArray()->length = 0;
		    //delete obj->getArray();
			//obj->array = NULL;
		    break;
			
		  case objDict:
 			//
			// Another xpdf hack. That means do not free any nested
			// objects, we have already done this.
			//
			//for (i = 0; i < obj->getDict()->length; ++i) 
			{
			//	gfree(obj->getDict()->entries[i].key);
			//	obj->getDict()->entries[i].key = NULL;
			}
			//obj->getDict()->length = 0;
		    //delete obj->getDict();
			//obj->dict = NULL;
		    break;
			
		  case objStream:
			//
			// really Everything??
			//
			//for (i = 0; i < obj->getStream()->getDict()->length; ++i) 
			{
			//	gfree(obj->getStream()getDict()->entries[i].key);
			//	obj->getStream()->getDict()->entries[i].key = NULL;
			}
		    //obj->getStream()->getDict()->length = 0;
		    //delete obj->getStream()->getDict();
			break;
			
		  default:
		    break;
		  }

		// delete main obj
		delete obj;
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
}


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
}


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

		
}



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
  
  objToString (IProperty::obj,str);
}



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
}




/**
 * DO NOT use this function, you really DO NOT want to write to Null object.
 */
template<>
void
CObject<pNull>::writeValue (WriteType)
{
	STATIC_CHECK (false,INCORRECT_USE_OF_FUNCTION);		
}

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
		
}


//
// Just a hint that we can free this object
//
// This is a generic function for all types
//
template<PropertyType Tp>
void
CObject<Tp>::release()
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
		getAllXpdfObjects (*IProperty::obj, objects);
		processObjectFamily (objects, ObjectDeleteProcessor<CheckDirectObject> (*pdf) );

		// We have freed everything "below" this object, now free Object
		objectFree (IProperty::obj);
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
CObject<Tp>::getPropertyCount () const
{
	STATIC_CHECK ( (pArray==Tp) || (Tp==pDict) || (Tp==pStream),
					BAD_COBJECT_TYPE);
	assert (!"Called getPropertyCount() on not supporting type...");
	return 0;	
}

template<>
PropertyCount
CObject<pArray>::getPropertyCount () const
{
	return 0;//IProperty::obj->arrayGetLength();	
}

template<>
PropertyCount
CObject<pDict>::getPropertyCount () const
{
	return 0;//IProperty::obj->dictGetLength();	
}

template<>
PropertyCount
CObject<pStream>::getPropertyCount () const
{
	return 0;//return IProperty::obj->streamGetDict()->getLength ();
}



//
// This is nor really a generic function, it is just for pDict 
//
template<PropertyType Tp>
template<typename T>
void
CObject<Tp>::getAllPropertyNames (T& container) const
{
	STATIC_CHECK(Tp==pDict,BAD_COBJECT_TYPE);
	assert (!"Called getAllPropertyNames() on not supporting type...");
}


template<>
template<typename T>
void
CObject<pDict>::getAllPropertyNames (T& container) const
{
	assert (NULL != obj);
	//int size = IProperty:: dictGetLength();
	//for (int i = 0; i < size; i++)
	{
	//	container.push_back (string (obj->dictGetKey (i)) );
	}
	
}












}; /* namespace pdfobjects */


#endif // COBJECTI_H
