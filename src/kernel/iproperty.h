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
#include <string>

// xpdf
#include <Object.h>

#include "exceptions.h"
#include "observer.h"

// Debug routine
#include "utils/debug.h"



//=====================================================================================

namespace pdfobjects
{
/**
 * Null and empty types.
 */
class NullType {};
struct EmptyType {};

/**
 * Forward declarations
 */
class CPdf;

/**
 * Enum describing the type of a property.
 */
enum PropertyType 
{
		// Simple
		pNull 	= objNull, 
		pBool 	= objBool, 
		pInt 	= objInt, 
		pReal 	= objReal, 
		pString = objString,
		pName	= objName,
		pRef	= objRef,
		
		// Complex
		pArray	= objArray, 
		pDict	= objDict, 
		pStream	= objStream, 

		//
		pInvalid,

		// Debug
		pOther	= objCmd,
		pOther1 = objError,
		pOther2 = objEOF,
		pOther3 = objNone
};

/**
 * Enum describing the type of a special object.
 */
enum SpecialObjectType {sNone, sPdf, sPage, sPageTree, sContentStream};

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
typedef std::vector<const IObserver*> ObserverList;

protected:
 PropertyType	type;		/**< Property type. */
 IndiRef 		ref;		/**< Object's pdf id and generation number. */
 CPdf* 			pdf;		/**< This object belongs to this pdf. */	

private:
 ObserverList 	observers;	/**< List of observers. */

private:
  /**
   * Copy constructor
   */
  IProperty (const IProperty&) {};

protected:	

  /**
   * Default constructor. We suppose that obj will be (was) set by CObject class.
   * This constructor is used, when someone wants to create a CObject.
   */
  IProperty () : type(pInvalid), pdf(NULL)
  {
	printDbg (0,"IProperty () constructor.");

	ref.num = 0;
	ref.gen = 0;
  };
  
  /**
   * @param o Xpdf object.
   */
  explicit
  IProperty (PropertyType tp): type(tp), pdf(NULL)
  { 
	printDbg (0,"IProperty constructor."); 

	ref.num = 0;
	ref.gen = 0;
  };


public:
  
	/**
	 * Set member variable pdf.
	 *
	 * @param p pdf that this object belongs to
	 */
	void  setPdf (CPdf* p)
	{
		assert (NULL != p); 	// set NULL?
		assert (NULL == pdf);	// modify existing association with a pdf?
		
		if (NULL ==p || NULL != pdf)
				throw ObjInvalidOperation ();
		
		pdf = p;
	};

	/**
	 * Returns pdf in which this object lives.
	 *
	 * @return Pdf that this object is associated with.
	 */
	CPdf* getPdf () const	{return pdf;};

public:
	/**
	 * Returns object's identification number. If it is an inline object
	 * returns id of parent object.
	 *
	 * @return Indirect identification number and generation number.
	 */
	const IndiRef& getIndiRef () const {return ref;};


	/**
	 * Set object identification number and generation number.
	 *
	 * @param _r Indirect reference id and generation number.
	 */
	void setIndiRef (const IndiRef& _r) {ref = _r;};

 
	/**
	 * Set object identification number and generation number.
	 *
	 * @param n Object's id.
	 * @param g Object's generation number.
	 */
	void setIndiRef (ObjNum n, GenNum g) {ref.num = n; ref.gen = g;};


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
  PropertyType getType () const  {return type;};

	/**
	* Indicate that you do not want to use this object again.
	* 
	* If it is an indirect object, we have to notify CXref.
	*/
	virtual void release () = 0;

	/**
	 * Returns string representation of actual object.
	 * 
	 * If it is an indirect object, we have to notify CXref.
	 */
	virtual void getStringRepresentation (std::string& str) const = 0;
 
	/**
	 * When destroying IProperty, CObject associated with this IProperty (should be?) 
	 * also destroyed.
	 */
	virtual ~IProperty () {};

public:
	/**
	 * Attaches an observers.
	 *
	 * @param observer Observer being attached.
	 */
	void registerObserver (const IObserver* o) 
	{
		assert(NULL != o);
		observers.push_back (o);
	}
  
       /**
	* Detaches an observer.
	* 
	* @param observer Observer beeing detached.
	*/
	void unregisterObserver (const IObserver* o)
	{
		assert(NULL != o);
		ObserverList::iterator it = find (observers.begin (), observers.end(),o);
		if (it != observers.end ())
			observers.erase (it);
	};

protected:
  /**
   * Notify all observers that a property has changed.
   */
  inline void notifyObservers ()
  {
	ObserverList::iterator it = IProperty::observers.begin ();
	for (; it != IProperty::observers.end(); it++)
		  (*it)->notify (this);
  }

  /**
   * Create xpdf object.
   *
   * @return Xpdf object(s).
   */
  virtual Object* _makeXpdfObject ()const = 0;


}; /* class IProperty */


} /* namespace pdfobjects */


#endif  //IPROPERTY_H
