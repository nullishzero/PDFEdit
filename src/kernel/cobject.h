/** 
 * =====================================================================================
 *        Filename:  cobject.h
 *     Description:  Header file containing definition of IProperty and CObject classes.
 *         Created:  01/18/2006 
 *       Revision:  none
 *          Author:  jmisutka (06/01/19), 
 * 			
 * 			2006/01/29 added CPdf here, because of the recursive include problem + templates
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

using namespace std;


/**
 * Additional information that identifies variable type, e.g. for writeValue function.
 *
 * If someone tries to use unsupported type (pCmd,....), she should get compile error
 * because PropertyTrait<> has no body.
 */
template<PropertyType T> struct PropertyTrait; 
template<> struct PropertyTrait<pNull>
{	public: 
		typedef NullType		writeType; 
		typedef const NullType 	PropertyId;
};
template<> struct PropertyTrait<pBool>
{	public: 
		typedef const bool		writeType; 
		typedef const NullType 	PropertyId;
};
template<> struct PropertyTrait<pInt>	
{	public: 
		typedef const int		writeType; 
		typedef const NullType 	PropertyId;
};
template<> struct PropertyTrait<pReal>	
{	public: 
		typedef const double 	writeType; 
		typedef const NullType 	PropertyId;
};
template<> struct PropertyTrait<pString> 
{	public: 
		typedef const string& 	writeType; 
		typedef const NullType 	PropertyId;
};
template<> struct PropertyTrait<pName>	
{	public: 
		typedef const string& 	writeType; 
		typedef const NullType 	PropertyId;
};
template<> struct PropertyTrait<pArray>	
{	public: 
		typedef const string& 	writeType; 
		typedef const unsigned int 	PropertyId;
};
template<> struct PropertyTrait<pStream> 
{	public: 
		typedef const string& 	writeType; 
		typedef const NullType 	PropertyId;
};
template<> struct PropertyTrait<pDict>	
{	public: 
		typedef const string& 	writeType; 
		typedef const string& 	PropertyId;
};
template<> struct PropertyTrait<pRef> 	
{	public: 
		typedef const IndiRef& 	writeType; 
		typedef const NullType	PropertyId;
};



/*
 * Typedefs
 */
typedef unsigned int	PropertyCount;
typedef unsigned int	PropertyIndex;
typedef string			PropertyName;


/*
 * Forward declarations
 */
class CPdf;
class Observer;


/** 
 * Template class representing all PDF objects from specification v1.5.
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
class CObject : public IProperty
{
public:
	/** Write type for writeValue function. */
	typedef typename PropertyTrait<Tp>::writeType WriteType;
		
private:
	
	/** Type indicating whether this object is a special object, like Cpdf, CAnnotation... */
	SpecialObjectType specialObjectType;

	/** This object belongs to this pdf. */	
	CPdf* pdf;
	
private:
	/**
	 * Copy constructor
	 */
	CObject (const CObject&) {};
	
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
	CObject (CPdf* p,Object* o,IndiRef& ref,bool isDirect, SpecialObjectType objTp = sNone);


	/**
	 * Constructor. If no object is given, one is created.
	 *
	 * @param p		Pointer to pdf object.
	 * @param objTp	Type of this object. Indicates whether it is a special object (CPdf,CPage,..) or not.
	 */
	CObject (CPdf* p,SpecialObjectType objTp = sNone);

public:	

	/**
	 * Public constructor. Can be used to creted direct/indirect objects.
	 *
	 * @param p		Pointer to pdf object in which this object will exist.
	 * @param ip	If NULL an indirect object will be created. Otherwise, ip will 
	 * 				point to an existing IProperty where the direct object will be placed by
	 * 				a call to e.g. addProperty().
	 */
	CObject (CPdf* p, IProperty* ip = NULL);

	
	/**
	 * Returns string representation of (x)pdf object. 
	 * 
	 * @param str 	After successful call, it will hold string representation 
	 * 				of current object.
	 */
	void getStringRepresentation (string& str) const;
	friend void objToString (const Object* o,string& str);
	
	
	/**
	 * Make object from string.
	 * <exception cref="..." />
	 *
	 * @param str0 Object in a text form.
	 */
	void setStringRepresentation (const string& strO);

	
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
	 * @return Xpdf object.
	 */
	const Object* getRawObject () const {assert (NULL != obj); return obj;};
	
	
	/**
	 * Returns if it is one of special objects CPdf,CPage etc.
	 *
	 * @return Type of special object.
	 */
	virtual SpecialObjectType getSpecialObjType() const {return (SpecialObjectType)0;};

	
	/**
	 * Returns pointer to derived object. 
	 */
	template<typename T>
	T* getSpecialObjectPtr () const
	{
		STATIC_CHECK(sizeof(T)>=sizeof(CObject<Tp>),DESTINATION_TYPE_TOO_NARROW); 
		return dynamic_cast<T*>(this);
	}

	
	/**
	 * Indicate that you do not want to use this object again.
	 * 
	 * If it is an indirect object, we have to notify CXref.
	 */
  	virtual void release ();
	

protected:
    /**
     * Notify all observers that a property has changed.
     */
    void notifyObservers ();
 
	/**
	 * Destructor
	 */

/*DEBUG*/public:	~CObject ()	{};


	//
	// Specific features by Incomplete Instantiation
	//
public:
	/** 
     * Returns property count.
     *
     * @return Property count.
     */
    PropertyCount getPropertyCount () const;
 

	/**
     * Inserts all property names to container supplied by caller. This container must support 
     * push_back() function.
     *
     * @param container Container of string objects. STL vector,list,deque,... must implement 
     * pust_back() function.
     */
    template <typename T>
    void getAllPropertyNames (T& container) const {};


	/**
     * Returns value of property identified by its name/position depending on type of this object.
	 *
	 * So f.e. when we want to access dicionary item, we use string representation, when array
	 * we use integer.
   	 *
     * @param name Name of the property
     */
    template <typename T>
    T getPropertyValue (typename PropertyTrait<Tp>::PropertyId name) const;
	//
	// Be carefull to inherit id's from parent
	//

	
	/**
     * Returns property type of an item.
     *
     * @param	name	Name of the property.
     * @return		Property type.	
     */
    PropertyType getPropertyType (const PropertyName& name) const
    {
  	   	//return static_cast<PropertyType>(obj->getType());
    	return pNull;
    }

	
	/**
	 * Sets property type of an item.
	 *
	 * @param	id		Name/Index of property
	 * @param	value	Value, for simple types (int,string,...) and for complex types IProperty*
	 */
	template<typename T>
	void setPropertyValue (typename PropertyTrait<Tp>::PropertyId /*name*/, const T& /*val*/)
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
	};

	
	/**
	 * Adds property to array/dict.
	 *
	 * @param id Name/Index of property
	 */
	template<typename T>
	void addProperty (typename PropertyTrait<Tp>::PropertyId /*id*/, const T& /*val*/ ) {};
	
	
	/**
	 * Removes property from array/dict.
	 * REMARK: This can be tricky when having a complex type as a value.
	 *
	 * @param id Name/Index of property
	 */
	void delProperty (typename PropertyTrait<Tp>::PropertyId /*id*/) {};


private:
	//void createObj () const;

};



//
// CObject types
//
typedef CObject<pNull>	 CNull;
typedef CObject<pBool>	 CBoolean;
typedef CObject<pInt>	 CInt;
typedef CObject<pReal>	 CReal;
typedef CObject<pString> CString;
typedef CObject<pStream> CStream;
typedef CObject<pArray>	 CArray;
typedef CObject<pDict>	 CDict;
typedef CObject<pRef>	 CRef;




}; /* namespace pdfobjects */



#endif // COBJECT_H

