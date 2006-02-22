/** 
 * =====================================================================================
 *        Filename:  cobject.h
 *     Description:  Header file containing definition of IProperty and CObject classes.
 *         Created:  01/18/2006 
 *        Revision:  none
 *          Author:  jmisutka (06/01/19), 
 * 			
 * 			2006/01     constructor, getStringRepresentation, observer stuff, writeValue,
 *			2006/02/04	started implementing release () function (not trivial)
 *						implemented several easy functions for complex types
 *			2006/02/08  for the past several days i have had a battle with g++ and ls + collect
 *						i think that i have won at last...
 *
 *			FILE ORGANIZATION:
 *				cobject.h  -- definition of cobject + definition and a few implementations of helper funcions
 *								+ helper classes, all this is in a nested namespace pdfobjects::utils, it is
 *								because e.g. in cpdf.cc we can have the same functions...
 *								
 *				cobject.cc -- helper function implementation
 *				cobjectI.h -- implementation of CObject class
 *
 *			OBJECTS:
 *				Because simple types can be implemented generic really nicely, i separated them. It also ensures
 *				better checking whether a member function was called from an appropriate CObject* class.
 *
 * 			REMARKS: 
 *
 * 				1)  templates have to be in the same file or files (else there will be a linkage error)
 * 					but they have to be included as well we address this problem with the inclusion model 
 * 					solution. include the implementation in the cobject.h file. (the suffix of the file is .h but can be .cc, whatever..)
 *				2)  nondependand classes derived from template classes behave like normal classes so the
 *					definition should be in .h file and the implementation in .cc file.
 *					we address this in cpdf.h and cpdf.cc files, etc. many more
 *				3) 	linkage problems (multiple definitions) are handled automaticly which means CFRONT model
 *					we could add -frepo to the g++ compiler options, but do not have to at the moment.
 *					The second solution is the explicit template specialization, we use this in
 *					pdfobjects::utils::getAllXpdfObjects () function. The linker would not find the function
 *					if not specialized explicitely, because definition and declaration are not in the same file 
 *					nor are they both included (cobject.h,cobject.cc)
 *				4) 	it is possible that we won't be able to make a dynamic library if classes and functions not
 *					explicitely instantiated. C++ standard is very primitive because of the nature of current compilers. 
 * 					most of the gcc front model has to be reimplemented ONLY to support export function and we do not talk
 * 					about the generic libraries.... so perhaps in future
 *				
 *			 NEAR FUTURE:  start to test new functions with full linkage against the xpdf prject...
 *				 		add/complete objStream, objRef support
 *
 *			2006/02/20	changed constructor, addProperty has to be more clever
 *						getStringRepresentation,writeValue,release finished
 *
 *			TODO:
 *					INDIRECT -- refcounting
 *
 * =====================================================================================
 */
#ifndef COBJECT_H
#define COBJECT_H

#include <string>
#include <iostream>

#include "debug.h"
#include "iproperty.h"


//=====================================================================================
namespace pdfobjects
{


/**
 * Additional information that identifies variable type, e.g. for writeValue function.
 *
 * If someone tries to use unsupported type (pCmd,....), she should get compile error
 * because PropertyTraitSimple<> has no body.
 */
template<PropertyType T> struct PropertyTraitSimple; 
template<> struct PropertyTraitSimple<pNull>
{	public: typedef NullType		writeType; 
};
template<> struct PropertyTraitSimple<pBool>
{	public: typedef const bool		writeType; 
};
template<> struct PropertyTraitSimple<pInt>	
{	public: typedef const int		writeType; 
};
template<> struct PropertyTraitSimple<pReal>	
{	public: typedef const double 	writeType; 
};
template<> struct PropertyTraitSimple<pString> 
{	public: typedef const std::string& 	writeType; 
};
template<> struct PropertyTraitSimple<pName>	
{	public: typedef const std::string& 	writeType; 
};
template<> struct PropertyTraitSimple<pRef> 	
{	public: typedef const IndiRef& 	writeType; 
};

/*
 * Forward declarations
 */
class CPdf;



//=====================================================================================
// CObjectSimple
//


/** 
 * Template class representing simple PDF objects from specification v1.5.
 *
 * REMARK: it will be the responsibility of CXref to remember a mapping
 * between each xpdf Object and our representation. e.g. when we want to save
 * a pdf, we also want to save undo's, and if we would have more of our objects 
 * pointing to one xpdf Object we would have multiple undos and it could cause 
 * problems.
 */
template <PropertyType Tp>
class CObjectSimple : public IProperty
{
public:
	/** Write type for writeValue function. */
	typedef typename PropertyTraitSimple<Tp>::writeType WriteType;
		
private:
	/** Type indicating whether this object is a special object, like Cpdf, CAnnotation... */
	// DO NOT NEED THIS IN SIMPLE TYPE // SpecialObjectType specialObjectType;
	/** This object belongs to this pdf. */	
	CPdf* pdf;
	
private:
	/**
	 * Copy constructor
	 */
	CObjectSimple (const CObjectSimple&);
	
protected:
	/**
	 * Constructor. Only kernel can call this constructor. It is dependant on the object, that we have
	 * parsed.
	 *
	 * @param p		Pointer to pdf object. If we create CPdf, p will be NULL so use CObject::this pointer.
	 * @param o		Xpdf object. 
	 * @param ref	Indirect id and gen id.
	 * @param isDirect	Indicates whether this is a direct/indirect object.
	 */
	CObjectSimple (CPdf* p, Object* o, IndiRef& ref, bool isDirect);


public:	

	/**
	 * Public constructor. Can be used to creted direct/indirect objects.
	 *
	 * @param p		Pointer to pdf object in which this object will exist.
	 * @param isDirect	Indicates whether this is a direct/indirect object.
	 */
	CObjectSimple (CPdf* p, bool isDirect);

	
	/**
	 * Returns string representation of (x)pdf object. 
	 * 
	 * @param str 	After successful call, it will hold string representation 
	 * 				of current object.
	 */
	void getStringRepresentation (std::string& str) const;
	
	
	/**
	 * Make object from string.
	 * <exception cref="..." />
	 *
	 * @param str0 Object in a text form.
	 */
	void setStringRepresentation (const std::string& strO);

	
	/**
	 * Change the value of an object. The variable type depends
	 * on CObject type. For complex types, it is equal to setStringRepresentation().
	 * 
	 * We can define how best we want to represent an pdf object. E.g.
	 * we can represent a dictionary with list<pair<string,string> > etc...
	 *
	 * <exception cref="ObjBadValueE "/> When a value cannot be set due to bad value e.g. in complex types.
	 *
	 * @param val	Value that will be set.
	 */
	void writeValue (WriteType val);
 

	/**
	 * Return property value
	 */
    WriteType getPropertyValue () const;
	

	/**
 	 * Notify Writer object that this object has changed. We have to call this
	 * function to make changes visible.
	 *
	 * It is necessary for undo operation, that means saving previous state of the object.
	 *
	 * We obtain reference to CXref from CObject::pdf.
	 *
	 * @param makeValidCheck True if we want to verify that our changes preserve
	 * 						 pdf validity.
	 */
	void dispatchChange (bool /*makeValidCheck*/) const {}; 

	
	/**
	 * Returns xpdf object.
	 *
	 * @return Xpdf object associated with this CObject.
	 */
	const Object* getRawObject () const {assert (NULL != obj); return obj;};
	
	
	/**
	 * Returns if it is one of special objects CPdf,CPage etc.
	 *
	 * @return Type of special object.
	 */
	// virtual SpecialObjectType getSpecialObjType() const {return sNone;};

	
	/**
	 * Returns pointer to derived object. 
	 */
	template<typename T>
	T* getSpecialObjectPtr () const
	{
		STATIC_CHECK(sizeof(T)>=sizeof(CObjectSimple<Tp>),DESTINATION_TYPE_TOO_NARROW); 
		return dynamic_cast<T*>(this);
	}

	
	/**
	 * Indicate that you do not want to use this object again.
	 * 
	 * If it is an indirect object, we have to notify CXref.
	 */
  	virtual void release ();
	

//protected: 
/*DEBUG*/public:	
	/**
	 * Destructor
	 */
	~CObjectSimple ()	{};

};


//=====================================================================================
// CObjectComplex
//

/*
 * Typedefs
 */
typedef unsigned int	PropertyCount;
typedef unsigned int	PropertyIndex;
typedef std::string		PropertyName;

/**
 * Additional information that identifies variable type, e.g. for writeValue function.
 *
 * If someone tries to use unsupported type (pCmd,....), she should get compile error
 * because PropertyTrait<> has no body.
 */
template<PropertyType T> struct PropertyTraitComplex; 
template<> struct PropertyTraitComplex<pArray>	
{	public: 
		typedef const std::string& 	writeType; 
		typedef const unsigned int 	PropertyId;
};
template<> struct PropertyTraitComplex<pStream> 
{	public: 
		typedef const std::string& 	writeType; 
		typedef const std::string& 	PropertyId;
};
template<> struct PropertyTraitComplex<pDict>	
{	public: 
		typedef const std::string& 	writeType; 
		typedef const std::string& 	PropertyId;
};


/** 
 * Template class representing complex PDF objects from specification v1.5.
 *
 * This dividing is crucial when making specific changes to specific objects
 * e.g. adding/deleting of properties can be done to Dictionary but not to Integer.
 *
 * The specific features are implemented using c++ feature called Incomplete Instantiation.
 * It means that, when it is not used, it is not instatiated, so e.g. CInt won't have
 * addDictProperty() method
 *
 * This class can be both, a final class (no child objects) or not final class 
 * (has child object). 
 *
 * When it is not final class, it is a special object (CPdf, CPage,...). We can
 * find out the object by calling  virtual method getSpecialObjType(). 
 * This can be helpful for example for special manipulation with content stream, xobjects, ...
 *
 * REMARK: it will be the responsibility of CXref to remember a mapping
 * between each xpdf Object and our representation. e.g. when we want to save
 * a pdf, we also want to save undo's, and if we would have more of our objects 
 * pointing to one xpdf Object we would have multiple undos and it could cause 
 * problems.
 */
template <PropertyType Tp>
class CObjectComplex : public IProperty
{
public:
	/** Write type for writeValue function. */
	typedef typename PropertyTraitComplex<Tp>::writeType WriteType;
		
private:
	/** Type indicating whether this object is a special object, like Cpdf, CAnnotation... */
	SpecialObjectType specialObjectType;
	/** This object belongs to this pdf. */	
	CPdf* pdf;
	
private:
	/**
	 * Copy constructor
	 */
	CObjectComplex (const CObjectComplex&) {};
	
protected:
	/**
	 * Constructor. Only kernel can call this constructor
	 *
	 * @param p		Pointer to pdf object. If we create CPdf, p will be NULL so use CObject::this pointer.
	 * @param o		Xpdf object. 
	 * @param ref	Indirect id and gen id.
	 * @param isDirect	Indicates whether this is a direct/indirect object.
	 * @param objTp	Type of this object. Indicates whether it is a special object (CPdf,CPage,..) or not.
	 */
	CObjectComplex (CPdf* p, Object* o, IndiRef& ref, bool isDirect, SpecialObjectType objTp = sNone);


	/**
	 * Constructor. If no object is given, one is created.
	 *
	 * @param p		Pointer to pdf object.
	 * @param objTp	Type of this object. Indicates whether it is a special object (CPdf,CPage,..) or not.
	 */
	CObjectComplex (CPdf* p, SpecialObjectType objTp = sNone);

public:	

	/**
	 * Public constructor. Can be used to creted direct/indirect objects.
	 *
	 * @param p		Pointer to pdf object in which this object will exist.
	 * @param ip	If NULL an indirect object will be created. Otherwise, ip will 
	 * 				point to an existing IProperty where the direct object will be placed by
	 * 				a call to e.g. addProperty().
	 */
	CObjectComplex (CPdf* p, IProperty* ip = NULL);

	
	/**
	 * Returns string representation of (x)pdf object. 
	 * 
	 * @param str 	After successful call, it will hold string representation 
	 * 				of current object.
	 */
	void getStringRepresentation (std::string& str) const;
	
	
	/**
	 * Make object from string.
	 * <exception cref="..." />
	 *
	 * @param str0 Object in a text form.
	 */
	void setStringRepresentation (const std::string& strO);

	
	/**
	 * Change the value of an object. The variable type depends
	 * on CObject type. For complex types, it is equal to setStringRepresentation().
	 * 
	 * We can define how best we want to represent an pdf object. E.g.
	 * we can represent a dictionary with list<pair<string,string> > etc...
	 *
	 * <exception cref="ObjBadValueE "/> When a value cannot be set due to bad value e.g. in complex types.
	 *
	 * @param val	Value that will be set.
	 */
	void writeValue (WriteType val);
  
	
	/**
 	 * Notify Writer object that this object has changed. We have to call this
	 * function to make changes visible.
	 *
	 * It is necessary for undo operation, that means saving previous state of the object.
	 *
	 * We obtain reference to CXref from CObject::pdf.
	 *
	 * @param makeValidCheck True if we want to verify that our changes preserve
	 * 						 pdf validity.
	 */
	void dispatchChange (bool /*makeValidCheck*/) const {}; 

	
	/**
	 * Returns xpdf object.
	 *
	 * @return Xpdf object associated with this CObject.
	 */
	const Object* getRawObject () const {assert (NULL != obj); return obj;};
	
	
	/**
	 * Returns if it is one of special objects CPdf,CPage etc.
	 *
	 * @return Type of special object.
	 */
	virtual SpecialObjectType getSpecialObjType() const {return specialObjectType;};

	
	/**
	 * Returns pointer to derived object. 
	 */
	template<typename T>
	T* getSpecialObjectPtr () const
	{
		STATIC_CHECK(sizeof(T)>=sizeof(CObjectComplex<Tp>),DESTINATION_TYPE_TOO_NARROW); 
		return dynamic_cast<T*>(this);
	}

	
	/**
	 * Indicate that you do not want to use this object again.
	 * 
	 * If it is an indirect object, we have to notify CXref.
	 */
  	virtual void release ();
	

//protected:
/*DEBUG*/public:
	/**
	 * Destructor
	 */
	~CObjectComplex ()	{};


	//
	//
	// Specific features by Incomplete Instantiation
	//
	//
public:
	/** 
     * Returns property count.
	 *
	 * REMARK: Specific for pArray, pDict and pStream.
     *
     * @return Property count.
     */
    inline PropertyCount getPropertyCount () const;
 

	/**
     * Inserts all property names to container supplied by caller. This container must support 
     * push_back() function.
	 *
	 * REMARK: Specific for pDict and pStream.
     *
     * @param container Container of string objects. STL vector,list,deque,... must implement 
     * pust_back() function.
     */
    template <typename T>
    inline void getAllPropertyNames (T& container) const;


	/**
     * Returns value of property identified by its name/position depending on type of this object.
	 *
	 * So f.e. when we want to access dicionary item, we use string representation, when array
	 * we use integer.
	 *
	 * REMARK: Specific for pArray, pDict and pStream
   	 *
     * @param 	name Name of the property
	 * @return 	T	  That type which we want
     */
    template <typename T>
    T getPropertyValue (typename PropertyTraitComplex<Tp>::PropertyId name) const;
	//
	// Be carefull to inherit id's from parent if we return IProperty
	//

	
	/**
     * Returns property type of an item.
     *
     * @param	name	Name of the property.
     * @return		Property type.	
     */
    PropertyType getPropertyType (const PropertyName& name) const
    {
		assert (NULL != IProperty::obj);
  	   	return static_cast<PropertyType>(obj->getType());
    }

	
	/**
	 * Sets property type of an item.
	 *
	 * @param	id		Name/Index of property
	 * @param	value	Value, for simple types (int,string,...) and for complex types IProperty*
	 */
	template<typename T>
	void setPropertyValue (typename PropertyTraitComplex<Tp>::PropertyId /*name*/, const T& /*val*/)
	{
		//
		// Find the name in Object obj dictionary and change the value (type) of simple Object
		// simple conversions from int to string etc. can be done here
		//
		// If we want to set complex type we just get the Object from IProperty passed to this function
		// and save the pointer
		//
		// All operations are directly written to xpdf Objects
		//
	}

	
	/**
	 * Adds property to array/dict.
	 *
	 * @param id Name/Index of property
	 */
	template<typename T>
	void addProperty (typename PropertyTraitComplex<Tp>::PropertyId /*id*/, const T& /*val*/ ) {}
	
	
	/**
	 * Removes property from array/dict.
	 * REMARK: This can be tricky when having a complex type as a value.
	 *
	 * @param id Name/Index of property
	 */
	void delProperty (typename PropertyTraitComplex<Tp>::PropertyId /*id*/) {};

};



//=====================================================================================
//
// CObject types
//

typedef CObjectSimple<pNull>	CNull;
typedef CObjectSimple<pBool>	CBoolean;
typedef CObjectSimple<pInt>	 	CInt;
typedef CObjectSimple<pReal>	CReal;
typedef CObjectSimple<pString> 	CString;
typedef CObjectSimple<pRef>	 	CRef;

typedef CObjectComplex<pStream> CStream;
typedef CObjectComplex<pArray>	CArray;
typedef CObjectComplex<pDict>	CDict;



//=====================================================================================
//
//  Helper functions located in cobject.cc
//
namespace utils {


/**
 * Returns xpdf object string representation.
 */
void simpleObjToString (Object* obj,std::string& str);

/**
 * Returns xpdf object string representation.
 */
void complexObjToString (Object* /*obj*/,std::string& /*str*/);


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
void getAllXpdfObjects (Object& obj, Storage& /*store*/);



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
	// Perform an action on all objects
	for_each (store.begin(), store.end(), objProcessor);
}


/**
 * Free an object. We assume that all child objects (if any)
 * have been already freed.
 *
 * Copy & paste from Object.h
 * 
 * @param obj	Object to be freed.
 */
void objectFree (Object* obj);


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
template<typename Check>
class ObjectDeleteProcessor
{
private:
	CPdf* pdf;
	Check check;

public:
	ObjectDeleteProcessor (CPdf* _pdf) : pdf(_pdf) {};
	void operator() (Object* obj);
};


//
// WriteProcessors
//
// We know that Storage is xpdf Object and value type depends on each writer type
//
template<typename Storage, typename Val>
struct BoolWriter
{public:
		void operator() (Storage obj, Val val)
			{obj->initBool (GBool(val));}
};

template<typename Storage, typename Val>
struct IntWriter
{public:
		void operator() (Storage obj, Val val)
			{obj->initInt (val);}
};

template<typename Storage, typename Val>
struct RealWriter
{public:
		void operator() (Storage obj, Val val)
			{obj->initReal (val);}
};

template<typename Storage, typename Val>
struct StringWriter
{public:
		void operator() (Storage obj, Val val)
			{obj->initString (GString(val.c_str()));}
};

template<typename Storage, typename Val>
struct NameWriter
{public:
		void operator() (Storage obj, Val val)
			{obj->initName (val.c_str());}
};


template<typename Storage, typename Val>
struct RefWriter
{public:
		void operator() (Storage obj, Val val)
			{obj->initRef (val.num, val.gen);}
};


template<typename Storage, typename Val, typename CObject>
struct ComplexWriter
{
	public:
		void 
		operator() (Storage obj, Val val, CObject* cobj)
		{
			//
			// We will get string here
			// -- if setStringRepresentation throws exception, just propagate it
			// 
			cobj->setStringRepresentation (val);
		}
};

} /* namespace utils */

} /* namespace pdfobjects */




//
// Include the actual implementation of CObject class
//
#include "cobjectI.h"


#endif // COBJECT_H

