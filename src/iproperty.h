/*
 * =====================================================================================
 * 
 *        Filename:  iproperty.h
 * 
 *     Description:  IProperty class 
 * 
 *         Version:  1.0
 *         Created:  01/19/2006 11:23:44 PM CET
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  jmisutka ()
 *         Company:  
 *           Email:  
 * 
 * =====================================================================================
 */

#ifndef IPROPERTY_H
#define IPROPERTY_H

#include <vector>
#include <list>
#include <map>
#include <string>

//
#include "debug.h"

// xpdf
#include <Object.h>


using namespace std;



//=====================================================================================



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


class PropertyObserver;

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
typedef string			PropertyName;
typedef unsigned int	PropertyIndex;
typedef unsigned int	PropertyCount;


private:
  Object* obj;								/*< Xpdf object */
  std::list<PropertyObserver*> observers;	/*< List of observers */

  
public:	
  /**
   * @param o Xpdf object.
   */
  IProperty (Object* o): obj(o) {assert (NULL != o);};
  

  /** 
   * Returns pointer to derived object. 
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
  virtual void registerObserver (PropertyObserver* /*observer*/);
  
  /**
   * Detaches an observer.
   * 
   * @param observer Observer beeing detached.
   */
  virtual void unregisterObserver (PropertyObserver* /*observer*/);
  
  /**
   * Notifies all observers about a change.
   */
  virtual void notifyObservers ();


}; /* class IProperty */



}// namespace pdfobjects

#endif  //IPROPERTY_H
