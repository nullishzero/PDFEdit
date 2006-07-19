// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
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


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

//
// Forward declarations
// 
class CPdf;
class IProperty;
typedef observer::ObserverHandler<IProperty> IPropertyObserverSubject;


/** Enum describing property type. */
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


/** Object identification number. */
typedef unsigned int ObjNum;
/** Object generation number. */
typedef unsigned int GenNum;

/** Values specifying indirect reference to an (x)pdf object. */
typedef struct IndiRef
{
	ObjNum	num; /**< Object's pdf identification number. */
	GenNum	gen; /**< Object's pdf generation number. */

	static const ObjNum invalidnum = 0;	/**< Invalid object identification number. */
	static const GenNum invalidgen = 0; /**< Invalid object generation number. */

	/** 
	 * Empty constructor.
	 * Initialize identification and generation number to invalid reference.
	 */
	IndiRef() : num(invalidnum), gen(invalidgen) {}
	
	/** 
	 * Constructor.
	 * Stores supplied identification and generation numbers.
	 * 
	 * @param ref Indirect Reference.
	 */
	IndiRef(const IndiRef& ref) : num(ref.num), gen(ref.gen)	{}

	/** 
	 * Constructor.
	 * Stores supplied identification and generation numbers.
	 * 
	 * @param ref Xpdf reference.
	 */
	IndiRef(const ::Ref& ref) : num(ref.num), gen(ref.gen) {}

	/** 
	 * Constructor.
	 * Stores supplied identification and generation numbers.
	 * 
	 * @param _num Object number.
	 * @param _gen Generation number.
	 */
	IndiRef(int _num, int _gen) : num(_num), gen(_gen) {}

	/** Copy constructor. */
	IndiRef& operator= (const IndiRef& _r) { num = _r.num; gen = _r.gen; return *this;};
	/** Equality operator. */
	bool operator== (const IndiRef& _r) const { return (num == _r.num && gen == _r.gen) ? true : false;};
			
} IndiRef;



//=====================================================================================
// class IProperty
//=====================================================================================

/** 
 * Narrow interface describing properties of every pdf object. We use this 
 * interface when we access properties of pdf object.
 *
 * Each IProperty is associated with one pdf object.
 * Changes made to this object are visible by xpdf only after registered
 * dispatchChange() method.
 *
 * When accessing complex properties, we have to know their type.
 * According to the type, we can cast this object to CObjectComplex<type> 
 * to get more functionality.
 *
 * REMARK: The connection to CPdf is stored in CPdf* and not a smart pointer. This has a good reason
 * namely cyclic references of smart pointers.
 *
 * This object implements Observer interface which means we can observe changes
 * made to all CObjects.
 */
class IProperty : public IPropertyObserverSubject
{
private:
	IndiRef 		ref;		/**< Object's pdf identification and generation number. */
	PropertyMode	mode;		/**< Mode of this property. */
	CPdf* 			pdf;		/**< This object belongs to this pdf. */	
	bool			wantDispatch;/**< If true changes are dispatched. */

	//
	// Constructors
	//
private:
	/** Copy constructor. */
	IProperty (const IProperty&) : IPropertyObserverSubject() {};

protected:	

	/** Basic constructor. */
	IProperty (CPdf* _pdf = NULL);

	/** Constructor. */
	IProperty (CPdf* _pdf, const IndiRef& rf);


	//
	// Deep Copy
	//
public:
  
	/**
	 * Copy constructor. Returns deep copy.
	 *
	 * REMARK: This is an example of Factory  method design pattern where we do
	 * not know the real type of the instance of this object.
	 * 
	 * @return Deep copy of this object.
	 */
	boost::shared_ptr<IProperty> clone () const;

protected:

	/**
	 * Implementation of clone method.
	 *
	 * REMARK: This is an example of Template method design pattern.
 	 *
 	 * @return Deep copy of this object.
	 */
	virtual IProperty* doClone () const = 0;

	//
	// Pdf
	//
public:
  
	/**
	 * Set pdf.
	 * <exception cref="ObjInvalidOperation"> Thrown when we want to set pdf association to 
	 * already associated object.
	 *
	 * @param p pdf that this object belongs to
	 */
	virtual void setPdf (CPdf* p);

	/**
	 * Returns pdf in which this object resides.
	 *
	 * @return Pdf that this object is associated with.
	 */
	CPdf* getPdf () const {return pdf;};

	//
	// IndiRef
	//
public:
	/**
	 * Returns object's identification number. If it is a direct object
	 * returns identification and generation number of parent object.
	 *
	 * @return Identification and generation number.
	 */
	const IndiRef& getIndiRef () const {return ref;};


	/**
	 * Set object identification number and generation number.
	 *
	 * @param rf Indirect reference identification and generation number.
	 */
	virtual void setIndiRef (const IndiRef& rf) {ref = rf;};

 
	/**
	 * Set object identification number and generation number.
	 *
	 * @param n Object's identification number.
	 * @param g Object's generation number.
	 */
	void setIndiRef (ObjNum n, GenNum g) {ref.num = n; ref.gen = g;};

	//
	// PropertyMode
	//	
public:

	/**
	 * Get mode of this property.
	 *
	 * @return Mode.
	 */
	PropertyMode getMode () const {return mode;};

	/**
	 * Set mode of this property.
	 *
	 * @param md Mode.
	 */
	void setMode (PropertyMode md) {mode = md;};
	

public:
  
    /** 
     * Returns pointer to derived object. 
     *
	 * @param ptr Pointer to an IProperty.
	 * 
     * @return Object casted to desired type.
     */
    template<typename T>
    static
	boost::shared_ptr<T> getSmartCObjectPtr (const boost::shared_ptr<IProperty>& ptr) 
    {
    	STATIC_CHECK(sizeof(T)>=sizeof(IProperty),DESTINATION_TYPE_TOO_NARROW); 
		boost::shared_ptr<T> newptr = boost::dynamic_pointer_cast<T, IProperty> (ptr);
		if (newptr)
		{
	  	  	return newptr;
		}
		else
		{
			assert (!"doClone INCORRECTLY overriden!! ");
			throw CObjInvalidObject (); 
		}
    }

	/** 
     * Returns type of instance of this object. 
     *
     * @return Type of this instance.
     */
    virtual PropertyType getType () const = 0;

	/**
	 * Returns string representation according to pdf specification
	 * of this object or its children.
	 *
	 * @param str Output string.
	 */
	virtual void getStringRepresentation (std::string& str) const = 0;

	//
	// Dispatch change
	//
public:
	/**
 	 * Notify Writer object that this object has changed. 
	 *
	 * We have to call this function to make changes visible.
	 *
	 * REMARK: This is an example of Template method design pattern.
	 */
	void dispatchChange () const;

	/**
	 * Lock changes. Do not dispatch changes.
	 */
	void lockChange () {assert (true == wantDispatch); wantDispatch = false;}

	/**
	 * Unlock changes. Do dispatch changes.
	 */
	void unlockChange () {assert (false == wantDispatch); wantDispatch = true;}

	/**
	 * Create xpdf object from this object. This is a factory method because we
	 * do not know the type of instance of this object.
	 *
	 * @return Xpdf object(s).
	 */
	virtual Object* _makeXpdfObject () const = 0;

	/**
	 * Destructor.
	 */
	virtual ~IProperty () {};

}; /* class IProperty */


//=====================================================================================
// Helper functions
//=====================================================================================


/** 
 * Checks whether pdf is valid.
 * 
 * @param pdf Pdf isntance to check.
 * @return true if pdf is not NULL, false otherwise.
 */
inline bool isPdfValid(CPdf* pdf)
	{ return (NULL !=pdf); }

/** 
 * Checks whether iproprety belongs to a valid pdf.
 * 
 * @param ip Property to check.
 * @return isPdfValid(ip-&getPdf()).
 */
inline bool hasValidPdf(const IProperty& ip)
	{ return isPdfValid(ip.getPdf()); }

/** \copydoc hasValidPdf */
template<typename T> inline bool hasValidPdf(T ip)
	{ return isPdfValid(ip->getPdf()); }


/** 
 * Checks whether given indirect reference is valid.
 * Reference is valid, if it is non NULL and object number is greater than 0 and 
 * object number is not invalid.
 * 
 * @param ref Reference to check.
 * @return true if reference is valid, false otherwise.
 */
inline bool isRefValid(const IndiRef* ref)
	{ return (ref) && (ref->num>0) && (IndiRef::invalidnum != ref->num); }

template<typename T> inline bool hasValidRef (T ip) 
	{ return isRefValid(&ip->getIndiRef()); }

inline bool hasValidRef (IProperty& ip) 
	{ return isRefValid(& ip.getIndiRef()); }


/** Is iproperty of a specified type. */
template<PropertyType Type>
inline bool isIPType (const IProperty& ip) {return (Type == ip.getType());}
/** Is iproperty of a specified type. */
template<PropertyType Type>
inline bool isIPType (boost::shared_ptr<IProperty> ip) {return (Type == ip->getType());}
/** Is IProperty of specified type. */
template<typename T> inline bool isNull	 (T& ip) {return isIPType<pNull> (ip);}
/** \copydoc isNull*/
template<typename T> inline bool isInt 	 (T& ip) {return isIPType<pInt> (ip);}
/** \copydoc isNull*/
template<typename T> inline bool isReal  (T& ip) {return isIPType<pReal> (ip);}
/** \copydoc isNull*/
template<typename T> inline bool isString(T& ip) {return isIPType<pString> (ip);}
/** \copydoc isNull*/
template<typename T> inline bool isName  (T& ip) {return isIPType<pName> (ip);}
/** \copydoc isNull*/
template<typename T> inline bool isRef 	 (T& ip) {return isIPType<pRef> (ip);}
/** \copydoc isNull*/
template<typename T> inline bool isDict  (T& ip) {return isIPType<pDict> (ip);}
/** \copydoc isNull*/
template<typename T> inline bool isArray (T& ip) {return isIPType<pArray> (ip);}
/** \copydoc isNull*/
template<typename T> inline bool isStream(T& ip) {return isIPType<pStream> (ip);}
	

//=====================================================================================
// Output functions
//=====================================================================================


//
// String representation of object type
//
/** Get string representation of specified type. */
template<int i> inline
std::string getStringType () {return "Unknown";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<0> () {return "Bool";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<1> () {return "Int";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<2> () {return "Real";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<3> () {return "String";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<4> () {return "Name";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<5> () {return "Null";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<9> () {return "Ref";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<6> () {return "Array";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<7> () {return "Dict";}
/** Get string representation of specified type. */
template<> inline
std::string getStringType<8> () {return "Stream";}

/** 
 * Prints property type.
 *  
 * Prints given type in human readable from instead of just number.
 * Uses getStringType method to get string representation.
 *
 * @param out String where to print.
 * @param type Type to print.
 * @return Reference to given string.
 */
std::ostream& operator<< (std::ostream& out, PropertyType type);

/** 
 * Prints xpdf object type.
 *  
 * Prints given type in human readable from instead of just number.
 * Uses getStringType method to get string representation.
 *
 * @param out String where to print.
 * @param type Xpdf type to print.
 * @return Reference to given string.
 */
std::ostream& operator<< (std::ostream& out, ::ObjType type);

/** 
 * Prints reference.
 * Prints given reference in ref[num, gen] format.
 * 
 * @param out String where to print.
 * @param ref Reference to print.
 * @return reference to given string.
 */
std::ostream & operator << (std::ostream & out, const IndiRef & ref);

/** 
 * Prints reference.
 * Prints given xpdf reference in ref[num, gen] format.
 * 
 * @param out String where to print.
 * @param ref Reference to print.
 * @return reference to given string.
 */
std::ostream & operator << (std::ostream & out, const ::Ref & ref);


// =====================================================================================
} // namespace pdfobjects
// =====================================================================================


#endif  //_IPROPERTY_H
