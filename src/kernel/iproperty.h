/*
 * =====================================================================================
 *        Filename:  iproperty.h
 *     Description:  IProperty class 
 *         Created:  01/19/2006 11:23:44 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _IPROPERTY_H_
#define _IPROPERTY_H_

#include <vector>
#include <list>
#include <map>
#include <string>
#include <iostream>

// xpdf
#include <xpdf/Object.h>

// Debug routine
#include "utils/debug.h"



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
// Forward declaration
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


/** Observer list for IProperty. */
typedef std::list<IObserver*> ObserverList;

/** Object id number. */
typedef unsigned int ObjNum;
/** Object generation number. */
typedef unsigned int GenNum;

/** Values specifying indirect reference to an (x)pdf object. */
typedef struct IndiRef
{
	ObjNum	num; /**< Object's pdf identification number */
	GenNum	gen; /**< Object's pdf generation number */
	IndiRef& operator= (const IndiRef& _r) { num = _r.num; gen = _r.gen; return *this;};
			
} IndiRef;


/** 
 * Narrow interface describing properties of every pdf object. We use this 
 * interface when we want to access or change properties of (x)pdf object.
 *
 * Typically, we will start by CPage object which is a dictionary. We have 
 * direct access to all simple properties like ints, strings, reals etc.
 *
 * Each IProperty is associated with one xpdf object. All modifying operations
 * are directly performed on this object. The object represents current state.
 * However, these changes are not visible by the (x)pdf till they are registered
 * in CXref.
 *
 * When accessing complex properties, we have to know the type with which type we
 * are working. According to the type, we can cast this object to CObject<type> 
 * to get more functionality.
 */
class IProperty
{
typedef std::list<IObserver*> ObserverList;

protected:
 Object* 	  obj;			/**< Xpdf object. */
 ObserverList observers;	/**< List of observers. */
 IndiRef 	  ref;			/**< Object's pdf id and generation number. */
 bool		  isDrect;		/**< Set, if this object is a direct object. */

protected:
 bool		  isChngd;		/**< DEBUGGING purposes: Set, if this object has been changed but not saved. */

private:
 	/**
	 * Copy constructor
	 */
 	IProperty (const IProperty&) {};

protected:	

  /**
   * Default constructor. We suppose that obj will be(was) set by CObject class.
   */
  IProperty () : obj(NULL), isDrect(true), isChngd (false)
  {
	printDbg (0,"IProperty () constructor.");

	ref.num = 0;
	ref.gen = 0;
  };
  
  /**
   * @param o Xpdf object.
   */
  explicit
  IProperty (Object* o, bool _isDirect): obj(o), isDrect(_isDirect), isChngd (false)
  { 
	assert (NULL != o);
  	assert (obj->getType() != objCmd);
	assert (obj->getType() != objEOF);
	assert (obj->getType() != objNone);
	assert (obj->getType() != objError);
	printDbg (0,"IProperty constructor."); 

	ref.num = 0;
	ref.gen = 0;
  };

  /**
   * Indicate that the object has changed.
   *
   * @param changed TRUE if the object has been changed, but not saved. 
   * 				FALSE if the object is to be saved.
   */
  void setIsChanged (bool changed) {isChngd = changed;};

  /**
   * Indicate whether the object has been changed.
   * 
   * @preturn TRUE if the object has been changed, but not saved. FALSE otherwise.
   */
  bool isChanged () const {return isChngd;};


 /**
   * Returns object's identification number. If it is an inline object
   * returns id of parent object.
   *
   * @return Indirect identification number and generation number.
   */
  const IndiRef* getIndiRef () const {return &ref;};

  
 /**
  * Set object identification number and generation number.
  *
  * @param _r Indirect reference id and generation number.
  */
 void setIndiRef (IndiRef* _r) {ref = *_r;};

 
 /**
  * Set object identification number and generation number.
  *
  * @param n Object's id.
  * @param g Object's generation number.
  */
  void setIndiRef (ObjNum n, GenNum g) {ref.num = n; ref.gen = g;};

  
  /**
   * Set whether this object is direct or not.
   *
   * @param isDirect	This object is direct object.
   */
  void setIsDirect (bool _isDirect) {isDrect = _isDirect;};

  /**
   * Return whether this object is direct or not. This is
   * crucial when deallocating objects. A direct object can
   * be destroyed at will. 
   * 
   * @return True if this object is a direct one.
   */
  bool isDirect () {return isDrect;};


public:
  
  /** 
   * Returns pointer to derived object. 
   *
   * @return Object casted to desired type.
   */
  template<typename T>
  T* getCObjectPtr () const
  {
	STATIC_CHECK(sizeof(T)>=sizeof(IProperty),DESTINATION_TYPE_TOO_NARROW); 
	return dynamic_cast<T*>(this);
  }

  
  /** 
   * Returns type of object. 
   *
   * @return Type of this class.
   */
  PropertyType getCObjectType () const
  {
	assert (obj->getType() != objCmd);
	assert (obj->getType() != objEOF);
	assert (obj->getType() != objNone);
	assert (obj->getType() != objError);
	
	return static_cast<PropertyType>(obj->getType());
  };

  
  /**
   * Indicate that you do not want to use this object again.
   * 
   * If it is an indirect object, we have to notify CXref.
   */
  virtual void release () = 0;

  
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
  void registerObserver (IObserver* o) {assert(NULL != o); observers.push_back (o);}
  
  /**
   * Detaches an observer.
   * 
   * @param observer Observer beeing detached.
   */
  void unregisterObserver (IObserver* o)
  {
	assert(NULL != o);
	ObserverList::iterator it = find (observers.begin (), observers.end(),o);
	if (it != observers.end ())
			observers.erase (it);
  };
  
}; /* class IProperty */



}// namespace pdfobjects


#endif  //IPROPERTY_H
