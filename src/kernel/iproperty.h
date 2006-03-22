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

// basic includes
#include "static.h"

// property modes
#include "modecontroller.h"

// stl
//#include <vector>
//#include <string>

// xpdf
#include "xpdf.h"

// our stuff
//#include "exceptions.h"
//#include "observer.h"



//=====================================================================================

namespace pdfobjects
{


//
// Forward declarations
// 
class CPdf;



/** Enum describing the type of a property. */
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

		// Debug
		pOther	= objCmd,
		pOther1 = objError,
		pOther2 = objEOF,
		pOther3 = objNone
};


/** Enum describing the type of a special object. */
enum SpecialObjectType 
{
		sNone, sPdf, sPage, sPageTree, sContentStream
};


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
 * interface when we want to access properties of pdf object.
 *
 * Each IProperty is associated with one pdf object. The object represents current state.
 * However, these changes are not visible by the (x)pdf till they are registered
 * in CXref with dispatchChange() method.
 *
 * When accessing complex properties, we have to know the type with which type we
 * are working. According to the type, we can cast this object to CObjectComplex<type> 
 * to get more functionality.
 *
 * REMARK: The connection to CPdf is stored in CPdf* and not smart pointer. This has a good reason
 * namely cyclic references of smart pointers.
 */
class IProperty
{
typedef std::vector<const IObserver*> ObserverList;

private:
 IndiRef 		ref;		/**< Object's pdf id and generation number. */
 PropertyMode	mode;		/**< Mode of this property. */
 ObserverList 	observers;	/**< List of observers. */
 CPdf* 			pdf;		/**< This object belongs to this pdf. */	


private:
  /** Copy constructor. */
  IProperty (const IProperty&) {};

protected:	

  /** Basic constructor. */
  IProperty (CPdf* _pdf = NULL) : mode(mdUnknown), pdf(_pdf)
  {
	printDbg (debug::DBG_DBG, "IProperty () constructor.");

	ref.num = ref.gen = 0;
  };

  /** Constructor. */
  IProperty (CPdf* _pdf, const IndiRef& rf) : ref(rf), mode(mdUnknown), pdf(_pdf)
  {
	printDbg (debug::DBG_DBG, "IProperty () constructor.");
  };

public:
  
  /**
   * Copy constructor. Returns deep copy.
   * 
   * @param Deep copy of this object.
   */
  boost::shared_ptr<IProperty> clone () const
  {
		boost::shared_ptr<IProperty> ip (doClone ());
		assert (typeid (*ip) == typeid (*this) && "doClone INCORRECTLY overriden!!" );
		
		return ip;
  }

protected:

  /**
   * Implementation of clone method
   *
   * @param Deep copy of this object.
   */
  virtual IProperty* doClone () const = 0;

public:
  
	/**
	 * Set member variable pdf.
	 * <exception cref="ObjInvalidOperation"> Thrown when we want to set pdf association to 
	 * already associated object.
	 *
	 * @param p pdf that this object belongs to
	 */
	void setPdf (CPdf* p)
	{
		assert (NULL == pdf);	// modify existing association with a pdf?
		if (NULL != pdf)
				throw ObjInvalidOperation ();
		
		pdf = p;
	};

	/**
	 * Returns pdf in which this object lives.
	 *
	 * @return Pdf that this object is associated with.
	 */
	CPdf* getPdf () const {return pdf;};


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
	void setIndiRef (const IndiRef& rf) {ref = rf;};

 
	/**
	 * Set object identification number and generation number.
	 *
	 * @param n Object's id.
	 * @param g Object's generation number.
	 */
	void setIndiRef (ObjNum n, GenNum g) {ref.num = n; ref.gen = g;};

public:

	/**
	 * Get mode of this property.
	 *
	 * @return Mode.
	 */
	PropertyMode getMode () const {return mode;};

	/**
	 * Set mode of a property.
	 *
	 * @param md Mode.
	 */
	void setMode (PropertyMode md) {mode = md;};
	

public:
  
    /** 
     * Returns pointer to derived object. 
     *
     * @return Object casted to desired type.
     */
    template<typename T>
    static
	boost::shared_ptr<T> getSmartCObjectPtr (const boost::shared_ptr<IProperty>& ptr) 
    {
    	STATIC_CHECK(sizeof(T)>=sizeof(IProperty),DESTINATION_TYPE_TOO_NARROW); 
  	  	return boost::static_pointer_cast<T, IProperty> (ptr);
    }

  
    /** 
     * Returns type of object. 
     *
     * @return Type of this class.
     */
    virtual PropertyType getType () const = 0;

	/**
	 * Returns string representation of actual object.
	 * 
	 * If it is an indirect object, we have to notify CXref.
	 */
	virtual void getStringRepresentation (std::string& str) const = 0;
 

	/**
 	 * Notify Writer object that this object has changed. We have to call this
	 * function to make changes visible.
	 */
	virtual void dispatchChange () const = 0; 
	
	/**
	 * Destructor.
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
  void notifyObservers ()
  {
	ObserverList::iterator it = IProperty::observers.begin ();
	for (; it != IProperty::observers.end(); it++)
		  (*it)->notify (this);
  }


public:

  /**
   * Create xpdf object.
   *
   * @return Xpdf object(s).
   */
  virtual Object* _makeXpdfObject () const = 0;


}; /* class IProperty */


} /* namespace pdfobjects */


#endif  //_IPROPERTY_H
