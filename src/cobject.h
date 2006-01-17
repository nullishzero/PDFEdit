#ifndef COBJECT_H
#define COBJECT_H

#include <vector>
#include <list>
#include <map>
#include <string>

using namespace std;


namespace pdfobjects
{


/**
 * Null and empty types.
 */
class NullType {};
struct EmptyType {};


/**
 * Enum describing the type of a property.
 */
enum PropertyType {pNull, pBool, pInt, pReal, pString, pArray, pDict, pStream, pRef};


/**
 * Enum describing the type of a special object.
 */
enum SpecialObjectType {sNone, sPdf, sPage, sPageTree, sContentStream};


/** 
 * Narrow interface describing unique pdf identification of every pdf object.
 *
 * Can be used when identifying direct objects within another objects.
 */
class IId
{
typedef unsigned int ObjNum;
typedef unsigned int GenNum;

	
private:
  const ObjNum objNum;	/**< Object's pdf identification number */
  const GenNum genNum;	/**< Object's pdf generation number */

public:
  IId (const ObjNum objN=0, const GenNum genN=0): objNum(objN),genNum(genN) {};

  /**
   * Returns object's identification number. If it is an inline object
   * returns id of parent object.
   *
   * @return Identification number of object.
   */
  ObjNum getObjNum () const {return objNum;};
  
  /**
   * Returns object's generation number. If it is an inline object
   * returns generation number of parent object.
   *
   * @return Generation number of object.
   */
  GenNum getGenNum () const {return genNum;};
  
protected:
  ~IId () {};

}; /* class IId */



class PropertyObserver;
typedef string		PropertyName;
typedef unsigned int	PropertyIndex;
typedef unsigned int	PropertyCount;

/** 
 * Narrow interface describing properties of every pdf object. We use this 
 * interface when we want to access or change properties of (x)pdf object.
 *
 * Typically, we will start by CPage object which is a dictionary. We have 
 * direct access to all simple properties like ints, strings, reals etc.
 * So we won't call e.g. getAllPropertyNames on objects that do not have
 * properties. However, if we call e.g. a method accessing properties and 
 * the object does not support it, we simply get an error/exception.
 *
 * When accessing complex properties, we have to know with which type we
 * are working. According to the type, we can cast this object to CObject<type> 
 * to get more functionanlity.
 *
 * Each IProperty is associated with one xpdf object. All modifying operations
 * are directly performed on this object.
 *
 * With this interface, we get all property names, get the propery value types,
 * get property count and finally cast it to CObject.
 */
class IProperty
{

private:
  //Object* obj;				/*< Xpdf object */
  std::list<PropertyObserver*> observers;	/*< List of observers */

  
public:	
  /**
   * @param o Xpdf object.
   */
  //IProperty (const Object& o): obj(o) {};
  
  /**
   * Inserts all property names to container supplied by caller. This container must support 
   * push_back() function.
   *
   * @param container Container of string objects.
   */
  template <typename T>
  void getAllPropertyNames (T& container) const
  {
	  //
	  // add names to the container with function e.g. T::push_back
	  //
	 /**/ container.push_back ("name1");
	  container.push_back ("name2");
	  container.push_back ("name3");
	  container.push_back ("name4");
  };
  
  /**
   * Returns value of property identified by its name.
   *
   * @param name Name of the property
   */
  template <typename T>
  T getPropertyValue (const PropertyName& /*name*/) const
  {
	  /**/return "value";
  };

 /**
   * Returns property type of an item.
   *
   * @param	name	Name of the property.
   * @return		Property type.	
   */
  PropertyType getPropertyType (const PropertyName& /*name*/) const
  {
	  //
	  // return type read from obj
	  //
	  return pString;
  };


  /** 
   * Returns property count.
   *
   * @return Property count.
   */
  PropertyCount getPropertyCount () {/**/return 4;};


  /** 
   * Returns pointer to derived object. 
   */
  template<typename T>
  T* getCObjectPtr () {return dynamic_cast<T*>(this);}

  /** 
   * Returns type of object. 
   */
  PropertyType getCObjectType () {/**/return pInt;}

  /**
   * When destroying IProperty, CObject associated with this IProperty is also
   * destroyed.
   */
  virtual ~IProperty () {};


public:
  
  /**
   * Notify something(CObject,XrefWriter?) object that this object has changed. It is necessary for 
   * saving previous state of the object.
   * REMARK: Changing specific properties can affect other objects. Return value
   *		 will indicate whether it is necessary to reparse property tree again.
   *
   * @param 
   * @return
   */
  virtual int dispatchChange (/*CXpdfWriter&*/) const /*=0;*/{/**/return 0;};


public:
  /**
   * Attaches an observers.
   *
   * @param observer Observer beeing attached.
   */
  virtual void registerObserver (PropertyObserver* /*observer*/) {};
  
  /**
   * Detaches an observer.
   * 
   * @param observer Observer beeing detached.
   */
  virtual void unregisterObserver (PropertyObserver* /*observer*/) {};
  
  /**
   * Notifies all observers about a change.
   */
  virtual void notifyObservers () {};


}; /* class IProperty */


/* Specialization has to be in the same namespace (most likely gcc not conforming to c++ standard) */
template<>
IProperty* IProperty::getPropertyValue<IProperty*> (const PropertyName& /*name*/) const
{
	return new IProperty;
};


/**
 * Additional information that identifies variable type for writeValue function
 */
template<PropertyType T> class PropertyTrait;
template<> class PropertyTrait<pNull>	{public: typedef NullType 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pBool>	{public: typedef const bool	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pInt>	{public: typedef const int	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pReal>	{public: typedef const double 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pString> {public: typedef const string 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pStream> {public: typedef const string 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pArray>	{public: typedef const string 	writeType; typedef const unsigned int 	PropertyId;};
template<> class PropertyTrait<pDict>	{public: typedef const string 	writeType; typedef const string 	PropertyId;};
//template<> class PropertyTrait<pRef> {public: typedef writeType;}


/** 
 * Template class representing all PDF objects from specificaton v1.5.
 *
 * This dividing is crucial when making specific changes to specific objects
 * e.g. adding/deleting of properties in Dictionary.
 *
 * The specific features are implemented using c++ feature called Incomplete Instantiation.
 * It means that, when it is not used, it is not instatiated, so e.g. CInt won't have
 * addDictProperty() method.
 *
 * This class can be either a final class (no child objects) or not final class 
 * (has child object). 
 *
 * When it is not a final class, it is a special object (CPdf, CPage,...). We can
 * find out the object by calling  virtual method getSpecialObjType(). This is necessary
 * for example for special manipulation with content stream, xobjects, ...
 *
 * REMARK: it will be the responsibility of XrefWriter to remember a mapping
 * between each xpdf Object and our representation. e.g. when we want to save
 * a pdf, we also want to save undo's, and if we would have multiple our objects 
 * pointing to one xpdf Object we would have multiple undos and it could cause 
 * problems
 *
 */
template <PropertyType Tp>
class CObject : public IId, public IProperty
{

private:
	//
	// We want to support undo operation... The question is, where shall the change be 
	// saved. I decided to keep it here.
	//
	// std::vector<Object*> undoObjects;
	
	SpecialObjectType specialObjectType;	/*< Type indicating whether this object is a special object, like Cpdf, CAnnotation...*/

	
public:
	/**
	 *
	 */
	CObject () {};
CObject
	
//	CObject (const Object& o) : IId(o.,o.), IProperty (o) {};

	/**
	 * Returns string representation of (x)pdf object.
	 *
	 * @return String representing (x)pdf object.
	 */
	string& getStringRepresentation () const;
	
	/**
	 * Make object from string.
	 *
	 * @param txt object in string form
	 */
	void setStringRepresentation (const string& strO);

	/**
	 * We can define how best we want to represent an pdf object. E.g.
	 * we can represent a dictionary with list<pair<string,string> > etc...
	 */
	void writeValue (typename PropertyTrait<Tp>::writeType* val) {};
  
	/**
 	 * Notify Writer object that this object has changed. It is necessary for 
 	 * saving previous state of the object.
 	 * REMARK: Changing specific properties can affect other objects. Return value
 	 *		 indicates whether it is necessary to reparse the property tree again.
	 *
	 * @param 
	 * @return
	 */
	virtual int dispatchChange (/*CXpdfWriter&*/) const {/**/return 0;};

	/**
	 * Returns xpdf object.
	 *
	 * @return Xpdf object.
	 */
//	const Object* getRawObject () const {return obj};


	//
	// Specific features by Incomplete Instantiation
	//

	/**
	 * Sets property type of an item.
	 *
	 * @param	id		Name/Index of property
	 * @param	value	Value, for simple types (int,string,...) and for complex types IProperty*
	 */
	template<typename T>
	void setPropertyValue (typename PropertyTrait<Tp>::PropertyId& /*name*/, const T& /*val*/)
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


	/**
	 * Returns if it is one of special objects CPdf,CPage etc.
	 *
	 * @return Type of special object.
	 */
	virtual SpecialObjectType getSpecialObjType() {return sNone;}

	/**
	 * Returns pointer to derived object. 
	 */
	template<typename T>
	T* getSpecialObjectPtr () {return dynamic_cast<T*>(this);}


	~CObject ()	{};

private:
	void createObj () const;

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



/** 
 * Interface for Observer.
 * Implementator should implement notify, which is called each time the
 * value of property is changed.
 */
class PropertyObserver
{
	
public:
    virtual ~PropertyObserver();
    virtual void notify(IProperty* changedObj) = 0;

protected:
    PropertyObserver();
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

} /* namespace pdfobjects */



#endif // COBJECT_H

