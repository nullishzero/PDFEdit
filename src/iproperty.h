/*
 * =====================================================================================
 *        Filename:  iproperty.h
 *     Description:  IProperty class 
 *         Created:  01/19/2006 11:23:44 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef IPROPERTY_H
#define IPROPERTY_H

#include <vector>
#include <list>
#include <map>
#include <string>
#include <iostream>

// xpdf
#include "Object.h"

//
#include "debug.h"



//=====================================================================================

namespace pdfobjects
{
using namespace std;
/**
 * Null and empty types.
 */
class NullType {};
struct EmptyType {};

//
// Forward definitions
//
class IObserver;


/**
 * Enum describing the type of a property.
 */
enum PropertyType 
{
		pNull 	= objNull, 
		pBool 	= objBool, 
		pInt 	= objInt, 
		pReal 	= objReal, 
		pString = objString,
		pName	= objName,
		pArray	= objArray, 
		pDict	= objDict, 
		pStream	= objStream, 
		pRef	= objRef,

		// debug
		pOther	= objCmd,
		pOther1 = objError,
		pOther2 = objEOF,
		pOther3 = objNone
};

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


/** 
 * Narrow interface describing properties of every pdf object. We use this 
 * interface when we want to access or change properties of (x)pdf object.
 *
 * Typically, we will start by CPage object which is a dictionary. We have 
 * direct access to all simple properties like ints, strings, reals etc.
 * So we won't call e.g. getAllPropertyNames on objects that do not have
 * properties. However, if we call e.g. a method accessing properties and 
 * the object does not support it, we simply get an exception.
 *
 * Each IProperty is associated with one xpdf object. All modifying operations
 * are directly performed on this object. The object represents current state.
 * However, these changes are not visible by the (x)pdf till they are registered
 * in CXref.
 *
 * When accessing complex properties, we have to know with which type we
 * are working. According to the type, we can cast this object to CObject<type> 
 * to get more functionality.
 *
 * With this interface, we get all property names, get the propery value types,
 * get property count and finally cast it to CObject.
 */
class IProperty
{
typedef string			PropertyName;
typedef unsigned int	PropertyIndex;
typedef unsigned int	PropertyCount;
typedef std::list<IObserver*> ObserverList;

private:
 Object* 	  obj;			/*< Xpdf object */
 ObserverList observers;	/*< List of observers */

  
private:	
  /**
   * @param o Xpdf object.
   */
  IProperty (Object* o): obj(o) {assert (NULL != o);};
  
  
public:
  /** 
   * Returns pointer to derived object. 
   *
   * @return Object casted to desired type.
   */
  template<typename T>
  T* getCObjectPtr () 
  {
	STATIC_CHECK(sizeof(T)>=sizeof(IProperty),DESTINATION_TYPE_TOO_NARROW); 
	return dynamic_cast<T*>(this);
  }

  /** 
   * Returns type of object. 
   *
   * @return Type of this class.
   */
  PropertyType getCObjectType () 
  {
	assert (obj->getType() != objCmd);
	assert (obj->getType() != objEOF);
	assert (obj->getType() != objNone);
	assert (obj->getType() != objError);
	
	return static_cast<PropertyType>(obj->getType());
  };

  /**
   * When destroying IProperty, CObject associated with this IProperty (should be?) 
   * also destroyed.
   */
  virtual ~IProperty () {};


public:
  /**
   * Attaches an observers.
   *
   * @param observer Observer beeing attached.
   */
  virtual void registerObserver (IObserver* o) {assert(NULL != o); observers.push_back (o);}
  
  /**
   * Detaches an observer.
   * 
   * @param observer Observer beeing detached.
   */
  virtual void unregisterObserver (IObserver* o)
  {
	assert(NULL != o);
	ObserverList::iterator it = find (observers.begin (), observers.end(),o);
	if (it != observers.end ())
			observers.erase (it);
  };
  
  /**
   * Notifies all observers about a change.
   */
  virtual void notifyObservers () = 0;


}; /* class IProperty */



/** 
 * Interface for Observer.
 * Implementator should implement notify, which is called each time the
 * value of property is changed.
 */
class IObserver
{
public:
	virtual void Update (IProperty*) = 0;
    virtual ~IObserver();
};


}// namespace pdfobjects


#endif  //IPROPERTY_H
