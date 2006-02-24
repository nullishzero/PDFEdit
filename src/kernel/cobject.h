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
 *			2006/02/20	changed constructor, addProperty has to be more clever
 *						getStringRepresentation,writeValue,release finished
 *			2006/02/21	changed getPropertyValue to non - template, bacause we can't partially specialize member functions
 *					also the way of passing return from return value to function argument, because
 *						* do not want to alloc something in kernel and dealloc elsewhere
 *						* pass by stack copying can be tricky when copying big amounts and we do not know 
 *						  in pdf
 *			2006/02/23	cobjectsimple implementation finished
 *			2006/02/23	cobjectcomplex 
 *					- getStringRepresentation
 *					- writeValue
 *					- release?, getPropertyCount
 *					- getAllPropertyNames -- problem templates virtual
 *			2006/02/24	- implementing addProperty for
 *					IProperty*. (template f. can't be virtual)
 *					all other functions, problems witch Object internal structure
 *					  not public functions etc., etc., etc.
 *			
 * 
 *			TODO:
 *					CObjectSimple testing
 *					addProperty ... other than IProperty*
 *					INDIRECT -- refcounting
 *					!! CPdf::addObject and addIndObject to replace mapping* functions
 *					setPropertyValue what is its purpose? just simple types?
 *					when shall we call notifyObservers
 *					can't add IProperty when pdf is not NULL (just to simplify things)
 *					   no real obstruction for removing this
 *					allow adding indirect values (directly :))
 *					utils::getObjectAtPos functions
 *					utils::removeObjectAtPos functions
 *
 * =====================================================================================
 */
#ifndef COBJECT_H
#define COBJECT_H

#include <string>
#include <list>
#include <vector>
#include <deque>
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
 *
 * REMARK: BE CAREFUL when manipulating this ones.
 * It is used in setStringRepresentation(), writeValue(), getPropertyValue()
 * 
 */
template<PropertyType T> struct PropertyTraitSimple; 
template<> struct PropertyTraitSimple<pNull>
{	public: typedef NullType		writeType; 
	public: typedef NullType		returnType; 
};
template<> struct PropertyTraitSimple<pBool>
{	public: typedef const bool		writeType; 
	public: typedef bool			returnType; 
};
template<> struct PropertyTraitSimple<pInt>	
{	public: typedef const int		writeType; 
	public: typedef int			returnType; 
};
template<> struct PropertyTraitSimple<pReal>	
{	public: typedef const double 		writeType; 
	public: typedef double			returnType; 
};
template<> struct PropertyTraitSimple<pString> 
{	public: typedef const std::string& 	writeType; 
	public: typedef std::string		returnType; 
};
template<> struct PropertyTraitSimple<pName>	
{	public: typedef const std::string& 	writeType; 
	public: typedef std::string		returnType; 
};
template<> struct PropertyTraitSimple<pRef> 	
{	public: typedef const IndiRef& 		writeType; 
	public: typedef IndiRef			returnType; 
};


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
	/** Write type for getPropertyValue function. */
	typedef typename PropertyTraitSimple<Tp>::returnType ReturnType;
		
private:
	/** Type indicating whether this object is a special object, like Cpdf, CAnnotation... */
	// DO NOT NEED THIS IN SIMPLE TYPE // SpecialObjectType specialObjectType;
	
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
	CObjectSimple (CPdf& p, Object& o, IndiRef& ref, bool isDirect);


public:	

	/**
	 * Public constructor. Can be used to creted direct/indirect objects.
	 *
	 * @param p		Pointer to pdf object in which this object will exist.
	 * @param isDirect	Indicates whether this is a direct/indirect object.
	 */
	CObjectSimple (CPdf& p, bool isDirect);

#ifdef DEBUG
CObjectSimple (){};
#endif
	
	
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
	 * We can define how best we want to represent an pdf object.
	 *
	 * <exception cref="ObjBadValueE "/> Thrown when a value cannot be set.
	 *
	 * @param val	Value that will be set.
	 */
	void writeValue (WriteType val);
 

	/**
	 * Return property value. Each property has its own return value.
	 */
	void getPropertyValue (ReturnType& val) const;
	

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
	void dispatchChange (bool makeValidCheck=false) const; 

	
	/**
	 * Returns if it is one of special objects CPdf,CPage etc.
	 *
	 * @return Type of special object.
	 */
	// virtual SpecialObjectType getSpecialObjType() const {return sNone;};

	
	/**
	 * Returns pointer to derived object. 
	 */
	/*template<typename T>
	T* getSpecialObjectPtr () const
	{
		STATIC_CHECK(sizeof(T)>=sizeof(CObjectSimple<Tp>),DESTINATION_TYPE_TOO_NARROW); 
		return dynamic_cast<T*>(this);
	}*/

	
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


	//
	// Helper functions
	//
private:
	/**
	 *
	 */
	inline void
	_objectChanged ()
	{
		// The object has been changed
		IProperty::setIsChanged (true);
		// Notify everybody about this change
		IProperty::notifyObservers ();
	}

};




//=====================================================================================
// CObjectComplex
//

/*
 * Typedefs
 */
typedef unsigned int	PropertyCount;
typedef unsigned int	PropertyIndex;
typedef std::string	PropertyName;

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
		typedef const unsigned int 	propertyId;
};
template<> struct PropertyTraitComplex<pStream> 
{	public: 
		typedef const std::string& 	writeType; 
		typedef const std::string& 	propertyId;
};
template<> struct PropertyTraitComplex<pDict>	
{	public: 
		typedef const std::string& 	writeType; 
		typedef const std::string& 	propertyId;
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
 */
template <PropertyType Tp>
class CObjectComplex : public IProperty
{
public:
	/** Write type for writeValue function. */
	typedef typename PropertyTraitComplex<Tp>::writeType WriteType;
	/** This type identifies a property. */
	typedef typename PropertyTraitComplex<Tp>::propertyId PropertyId;
	
private:
	/** Type indicating whether this object is a special object, like Cpdf, CAnnotation... */
	SpecialObjectType specialObjectType;
	
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
	CObjectComplex (CPdf& p, Object& o, IndiRef& ref, bool isDirect, SpecialObjectType objTp = sNone);


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
	 */
	CObjectComplex (CPdf& p, bool isDirect);
#ifdef DEBUG
CObjectComplex (){};
#endif

	
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
	virtual void getAllPropertyNames (std::vector<std::string>& container) const;
	virtual void getAllPropertyNames (std::list<std::string>& container) const;
	virtual void getAllPropertyNames (std::deque<std::string>& container) const;


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
    	//getPropertyValue (ReturnType& val, typename PropertyTraitComplex<Tp>::PropertyId name) const;
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
	virtual void setPropertyValue (PropertyId id, const IProperty& ip);
	
	/**
	 * Adds property to complex type. The property that should be added will
	 * automaticly be associated with the pdf this object lives in.
	 *
	 * @param newIp 	New property.
	 * @param propertyName 	Name of the created property
	 */
	void addProperty (IProperty& newIp);
	void addProperty (IProperty& newIp, const std::string& propertyName);
	
	
	/**
	 * Removes property from array/dict.
	 * REMARK: This can be tricky when having a complex type as a value.
	 *
	 * @param id Name/Index of property
	 */
	void delProperty (PropertyId /*id*/);



	//
	// Helper functions
	//
private:
	/**
	 *
	 */
	template<typename T>
	void 
	_getAllPropertyNames (T& container) const;

	/**
	 *
	 */
	inline void
	_objectChanged ()
	{
		// The object has been changed
		IProperty::setIsChanged (true);
		// Notify everybody about this change
		IProperty::notifyObservers ();
	}
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
 * Parses string to get simple values like int, name, bool etc.
 * 
 * <a cref="ObjBadValueE" /> Thrown when the string, can't be parsed correctly.
 * 
 * @param strO	String to be parsed.
 * @param val	Desired value.
 */
void getSimpleValueFromString (const std::string& str, bool& val);
void getSimpleValueFromString (const std::string& str, int& val);
void getSimpleValueFromString (const std::string& str, double& val);
void getSimpleValueFromString (const std::string& str, std::string& val);
void getSimpleValueFromString (const std::string& str, IndiRef& val);




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
// ReadProcessor
//
template<typename Storage, typename Val>
struct BoolReader
{public:
		void operator() (Storage obj, Val val)
			{val = (0 != obj->getBool());}
};

template<typename Storage, typename Val>
struct IntReader
{public:
		void operator() (Storage obj, Val val)
			{val = obj->getInt ();}
};

template<typename Storage, typename Val>
struct RealReader
{public:
		void operator() (Storage obj, Val val)
			{val = obj->getNum ();}
};

template<typename Storage, typename Val>
struct StringReader
{public:
		void operator() (Storage obj, Val val)
			{val = obj->getString ();}
};

template<typename Storage, typename Val>
struct NameReader
{public:
		void operator() (Storage obj, Val val)
			{val = obj->getName ();}
};


template<typename Storage, typename Val>
struct RefReader
{public:
		void operator() (Storage obj, Val val)
			{val.num = obj->getRefNum();
			 val.gen = obj->getRefGen();}
};


/*template<typename Storage, typename Val, typename CObject>
struct ComplexReader
{
	public:
		void 
		operator() (Storage obj, Val val, CObject* cobj)
		{
		}
};
*/


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
		operator() (Storage /*obj*/, Val val, CObject* cobj)
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

