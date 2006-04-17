// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/* 
 * =====================================================================================
 *        Filename:  cobject.h
 *     Description:  Header file containing definition of IProperty and CObject classes.
 *         Created:  01/18/2006 
 *        Revision:  none
 *          Author:  jmisutka (06/01/19), 
 *			\TODO:
 *					better public/protected dividing
 *					StreamReader
 *
 * =====================================================================================
 */
#ifndef _COBJECT_H
#define _COBJECT_H

// all basic includes
#include "static.h"
#include "iproperty.h"



//=====================================================================================
namespace pdfobjects {
//=====================================================================================

		
//
// Forward declarations of memory checkers
//
class NoMemChecker;
class BasicMemChecker;
class CXRef;
		
	
/**
 * CObjectSimple type trait.
 * Additional information that identifies variable type, e.g. for writeValue function.
 *
 * If someone tries to use unsupported type (pCmd,....), she should get compile error
 * because PropertyTraitSimple<> has no body.
 *
 * REMARK: BE CAREFUL when manipulating these ones.
 */
template<PropertyType T> struct PropertyTraitSimple; 
template<> struct PropertyTraitSimple<pNull>
{	public: typedef NullType		value;
	public: typedef NullType		writeType; 
};
template<> struct PropertyTraitSimple<pBool>
{	public: typedef bool			value;
	public: typedef bool			writeType; 
};
template<> struct PropertyTraitSimple<pInt>	
{	public: typedef int				value;
	public: typedef int				writeType; 
};
template<> struct PropertyTraitSimple<pReal>	
{	public: typedef double			value;
	public: typedef double 			writeType; 
};
template<> struct PropertyTraitSimple<pString> 
{	public: typedef std::string			value;
	public: typedef const std::string& 	writeType; 
};
template<> struct PropertyTraitSimple<pName>	
{	public: typedef std::string			value;
	public: typedef const std::string& 	writeType; 
};
template<> struct PropertyTraitSimple<pRef> 	
{	public: typedef IndiRef				value;
	public: typedef const IndiRef&	 	writeType; 
};


//=====================================================================================
// CObjectSimple
//=====================================================================================

/** 
 * Template class representing simple PDF objects from specification v1.5.
 *
 * This class represents simple objects like null, string, number etc.
 * It does not have so many special functions as CObjectComplex.
 *
 * Other xpdf objects like objCmd can't be instantiated although the PropertyType 
 * exists. It is because PropertyTraitSimple is not specified for these types.
 *
 * We can use memory checking with this class which save information about living IProperties.
 * Can be used to detect memory leaks etc.
 *
 */
template <PropertyType Tp, typename Checker = BasicMemChecker>
class CObjectSimple : public IProperty
{
public:
	typedef typename PropertyTraitSimple<Tp>::writeType	 WriteType;
	typedef typename PropertyTraitSimple<Tp>::value 	 Value;

private:
	/** Object's value. */
	Value value;
	

private:
	
	/** Copy constructor. */
	CObjectSimple (const CObjectSimple&);
	
public/*protected*/:
	
	/**
	 * Constructor. Only kernel can call this constructor. It depends on the object, that we have
	 * parsed. The object will read value from o and store it. We do NOT save any reference to o.
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CObjectSimple (CPdf& p, Object& o, const IndiRef& rf);

	
	/**
	 * Constructor. Only kernel can call this constructor. It depends on the object, that we have
	 * parsed. The object will read value from o and store it. We do NOT save any reference to o.
	 *
	 * @param o		Xpdf object. 
	 */
	CObjectSimple (Object& o);


public:	

	/**
	 * Public constructor. Can be used to create pdf objects. This object is created
	 * as a standalone and does not belong to any pdf. When added to a pdf
	 * the relation will be saved to IProperty::pdf.
	 *
	 * @param val	Value that will this object hold.
	 */
	CObjectSimple (const Value& val = Value());

	//
	// Cloning
	//
protected:

	/**
	 * Clone. Performs deep copy.
	 * REMARK: pRef DOES NOT copy the object to which it has reference.
	 *
	 * @return Deep copy of this object.
	 */
	virtual IProperty* doClone () const;


public:
	/**
	 * Return type of this property.
	 *
	 * @return Type of this property.
	 */
	virtual PropertyType getType () const {return Tp;};
			

	/**
	 * Returns string representation of (x)pdf object. 
	 * 
	 * @param str 	After successful call, it will hold the string representation 
	 * 				of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;
	
	
	/**
	 * Convert string to an object value.
	 * <exception cref="ObjBadValueE" /> Thrown when we can't parse the string correctly.
	 *
	 * @param str0 Object in a text form.
	 */
	void setStringRepresentation (const std::string& strO);

	
	/**
	 * Change the value of an object. The variable type depends
	 * on CObject type.
	 * 
	 * We can define the best to represent an pdf object.
	 *
	 * @param val	Value that will be set.
	 */
	void writeValue (WriteType val);
 

	/**
	 * Return property value. Each property has its own return type.
	 *
	 * @param val Out parameter where property value will be stored.
	 */
	void getPropertyValue (Value& val) const;

	
	/**
 	 * Notify Writer object that this object has changed. We have to call this
	 * function to make changes visible.
	 *
	 * It is necessary for undo operation, that means saving previous state of the object.
	 * We obtain reference to CXref from CObject::pdf.
	 */
	virtual void dispatchChange () const; 


	/**
	 * Destructor
	 */
	~CObjectSimple ();
	

	//
	// Helper functions
	//
public:	

	/**
	 * Make xpdf Object from this object. This function allocates xpdf object, caller has to free it.
	 * You have to call free() function on the Object to deallocate its resources.
	 *
	 * <exception cref="ObjBadValueE" /> Thrown when xpdf can't parse the string representation of this
	 * object correctly.
	 * 
	 * @return Xpdf object representing actual value of this simple object.
	 */
	virtual Object*	_makeXpdfObject () const;
	
private:

	/**
	 * Create context of a change.
	 *
	 * REMARK: Be carefull. Deallocate this object.
	 * 
	 * @return Context in which a change occured.
	 */
	ObserverContext* 
	_createContext () const
	{
		// Save original value for the context
		boost::shared_ptr<IProperty> oldValue (this->clone());
		// For safety
		oldValue->setPdf (NULL);
		oldValue->setIndiRef (0,0);
		// Create the context
		return new BasicObserverContext (oldValue);
	}

	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<const ObserverContext> context)
	{
		// Dispatch the change
		dispatchChange ();
		
		if (context)
		{
			// Clone this new value
			boost::shared_ptr<IProperty> newValue (this->clone());
			// For safety
			newValue->setPdf (NULL);
			newValue->setIndiRef (0,0);
			// Notify everybody about this change
			IProperty::notifyObservers (newValue, context);
		}else
			throw CObjInvalidOperation ();
	}

};




//=====================================================================================
// CObjectComplex
//=====================================================================================

//
// Forward declaration of element finders
//
class ArrayIdxComparator;
class DictIdxComparator;


/**
 * CObjectComplex type trait.
 * Additional information that identifies variable type, e.g. for writeValue function.
 *
 * If someone tries to use unsupported type (pCmd,....), she should get compile error
 * because PropertyTrait<> has no body.
 */
template<PropertyType T> struct PropertyTraitComplex; 
template<> struct PropertyTraitComplex<pArray>	
{	 
		typedef std::list<boost::shared_ptr<IProperty> > value; 
		typedef const std::string& 			writeType; 
		typedef unsigned int	 			propertyId;
		typedef class ArrayIdxComparator	indexComparator;
};
template<> struct PropertyTraitComplex<pDict>	
{	 
		typedef std::list<std::pair<std::string, boost::shared_ptr<IProperty> > > value; 
		typedef const std::string& 			writeType; 
		typedef const std::string& 			propertyId;
		typedef class DictIdxComparator		indexComparator;
};



/** 
 * Template class representing complex PDF objects from specification v1.5.
 *
 * The specific features are implemented using c++ feature called Incomplete Instantiation.
 * It means that, when it is not used, it is not instatiated, so e.g. CArray won't have
 * addProperty (IProperty& ,string&) method.
 *
 * This class can be both, a final class (no child objects) or a parent to a special class.
 *
 * When it is not a final class, it is a special object (CPdf, CPage,...). We can
 * find this out by calling virtual method getSpecialObjType(). 
 * This can be helpful for example for special manipulation with content stream, xobjects, ...
 */
template <PropertyType Tp, typename Checker = BasicMemChecker>
class CObjectComplex : public IProperty
{
public:
	typedef typename PropertyTraitComplex<Tp>::writeType  		WriteType;
	typedef typename PropertyTraitComplex<Tp>::propertyId 		PropertyId;
	typedef typename PropertyTraitComplex<Tp>::indexComparator	IndexComparator;
	typedef typename PropertyTraitComplex<Tp>::value 	  		Value;  
	
private:
	
	/** Object's value. */
	Value value;

	
private:

	/** Copy constructor */
	CObjectComplex (const CObjectComplex&) {};
	
protected:

/*debug*/public:
	/**
	 * Constructor. Only kernel can call this constructor
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CObjectComplex (CPdf& p, Object& o, const IndiRef& rf);

	/**
	 * Constructor. Only kernel can call this constructor
	 *
	 * @param o		Xpdf object. 
	 */
	CObjectComplex (Object& o);


public:	
	
	/** 
	 * Public constructor. This object will not be associated with a pdf.
	 */
	CObjectComplex ();

	//
	// Cloning
	//
protected:

	/**
	 * Clone. Performs deep copy.
	 * REMARK: It will not copy pdf indirect objects that are referenced from the pdf object tree 
	 * starting in this object.
	 *
	 * @return Deep copy of this object.
	 */
	virtual IProperty* doClone () const;

	
public:

	/**
	 * Return type of this property.
	 *
	 * @return Type of this property.
	 */
	virtual PropertyType getType () const {return Tp;};
	

	/**
	 * Returns string representation of (x)pdf object. 
	 * 
	 * @param str 	After successful call, it will hold string representation 
	 * 				of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;
	
	
	/**
	 * Try to make an (x)pdf object from string.
	 * <exception cref="..." />
	 *
	 * @param str0 Object in a text form.
	 */
	void setStringRepresentation (const std::string& strO) {assert (!"is this function really needed???");};

	
	/**
	 * Change the value of an object. The variable type depends
	 * on CObject type. 
	 * For complex types, it is equal to setStringRepresentation().
	 * 
	 * <exception cref="ObjBadValueE "/> Thrown When a value cannot be set due to bad value e.g. in complex types.
	 *
	 * @param val	Value that will be set.
	 */
	void writeValue (WriteType val) {assert (!"is this function really needed???");};
  
	
	/**
	 * Returns if it is one of special objects CPdf,CPage etc.
	 *
	 * @return Type of special object.
	 */
	virtual SpecialObjectType getSpecialObjType() const {return sNone;};

	
	/**
 	 * Notify Writer object that this object has changed. We have to call this
	 * function to make changes visible.
	 *
	 * It is necessary for undo operation, that means saving previous state of the object.
	 * We obtain reference to CXref from CObject::pdf.
	 */
	virtual void dispatchChange () const; 


	/**
	 * Destructor
	 */
	~CObjectComplex ();
	
	//
	//
	// Specific features by Incomplete Instantiation
	//
	//
public:
	/** 
	 * Returns property count.
	 * 
	 * @return Property count.
	 */
	size_t getPropertyCount () const 
	{
		printDbg (debug::DBG_DBG, "getPropertyCount(" << debug::getStringType<Tp>() << ") = " << value.size());
		return value.size();
	};
 

	/**
	 * Inserts all property names to container supplied by caller. 
	 * 
	 * REMARK: Specific for pDict.
   	 *
	 * @param container Container of string objects. STL vector,list,deque.
	 */
	template<typename Container>
	void getAllPropertyNames (Container& container) const;


	/**
	 * Returns value of property identified by its name/position depending on type of this object.
   	 *
   	 * @param 	id 	Variable identifying position of the property.
	 * @return	Variable where the value will be stored.
   	 */
	boost::shared_ptr<IProperty> getProperty (PropertyId id) const;

	
	/**
	 * Returns property type of an item identified by name/position.
	 *
	 * <exception cref="ObjInvalidPositionInComplex "/> When the id does not correctly identify an item.
	 *
	 * @param	name	Name of the property.
	 * @return		Property type.	
	 */
	PropertyType getPropertyType (PropertyId id) const 
		{return getProperty(id)->getType();};
	
	
	/**
	 * Sets property type of an item.
	 * 
	 * Firstly, the property that is passed as argument is cloned, the argument itself is not set.
	 * The cloned object replaces object specified by id.
	 * 
	 * @param	id		Name/Index of property
	 * @param	value	Value, for simple types (int,string,...) and for complex types IProperty*
	 *
	 * @return Pointer to the new property.
	 */
	boost::shared_ptr<IProperty> setProperty (PropertyId id, IProperty& ip);
	
	/**
	 * Adds property to array/dict/stream. 
	 *
	 * Firstly, the property that is passed as argument is cloned, the argument itself is not added. 
	 * The cloned object is added, automaticly associated with the pdf where the object is beeing added.
	 * Indicate that this object has changed and return the pointer to the cloned object.
	 *
	 * @param newIp 		New property.
	 * @param propertyName 	Name of the created property.
	 *
	 * @return Pointer to the new property.
	 */
	boost::shared_ptr<IProperty> addProperty (const IProperty& newIp);
	boost::shared_ptr<IProperty> addProperty (size_t position, const IProperty& newIp);
	boost::shared_ptr<IProperty> addProperty (const std::string& propertyName, const IProperty& newIp);

	
	/**
	 * Remove property from array/dict/stream. If the xpdf Object to be removed is 
	 * associated with an IProperty call release(). Otherwise just free the memory
	 * occupied by the xpdf object. Properties start with index 0.
	 * Finally indicate that this object has changed.
	 *
	 * <exception cref="ObjInvalidPositionInComplex "/> When the id does not correctly identify an item.
	 * 
	 * @param id Name/Index of property
	 */
	void delProperty (PropertyId id);


	/**
	 * Returns index of a property.
	 *
	 * @param ip Object for which we want to find out the position.
	 */
	PropertyId getPropertyId (const boost::shared_ptr<IProperty>& ip) const;

	//
	// Helper functions
	//
public:

	/**
	 * Make xpdf Object from this object.
	 * You have to call free() function on the Object to deallocate its resources.
	 *
	 * <exception cref="ObjBadValueE" /> Thrown when xpdf can't parse the string representation of this
	 * object correctly.

	 * @return Xpdf object representing actual value of this simple object.
	 */
	virtual Object*	_makeXpdfObject () const;

	
private:
	
	/**
	 * Create context of a change.
	 *
	 * REMARK: Be carefull. Deallocate this object.
	 * 
	 * @return Context in which a change occured.
	 */
	ObserverContext* _createContext (boost::shared_ptr<IProperty>& changedIp);

	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<IProperty> newValue, 
						 boost::shared_ptr<const ObserverContext> context)
	{
		// Dispatch the change
		dispatchChange ();
		
		if (context)
		{
			// Notify everybody about this change
			IProperty::notifyObservers (newValue, context);
		}else
			throw CObjInvalidOperation ();
	}


public:
	/**
	 * Return all object we have access to.
	 *
	 * @param store Container of objects.
	 */
	template <typename Storage>
	void _getAllChildObjects (Storage& store) const;

};




//=====================================================================================
// CObjectStream
//=====================================================================================

/**
 * Special class representing xpdf streams. It is neither a simple object, because it does not
 * contain just simple value, nor a complex object, because it can not be simple represented
 * in that generic class. It contains a dictionary and a stream. It does not have methods common
 * to complex objects.
 *
 * \TODO:
 * 			buf & showing buf
 * 				probably just STream::getChar() and try it
 * 			podla typu sa bude vyberat dynamicky
 * 				ked niekto zmeni typ, a potom ulozi dany stream, tak sa to prekoduje danym streamom
 * 			ako ukladat stream
 */
template <typename Checker = BasicMemChecker>
class CObjectStream : public IProperty
{
typedef CObjectComplex<pDict> CDict;
		
private:
	
	/** Object dictionary. */
	boost::scoped_ptr<CDict> dict;
	
	/** Xpdf object. */
	mutable Object xpdfDict;

		
private:

	/** Copy constructor */
	CObjectStream (const CObjectStream&) {};
	
	
protected:

/*debug*/public:
	/**
	 * Constructor. Only kernel can call this constructor
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CObjectStream (CPdf& p, Object& o, const IndiRef& rf);

	/**
	 * Constructor. Only kernel can call this constructor
	 *
	 * @param o		Xpdf object. 
	 */
	CObjectStream (Object& o);


public:	
	
	/** 
	 * Public constructor. This object will not be associated with a pdf.
	 */
	CObjectStream ();

	//
	// Cloning
	//
protected:

	/**
     * Implementation of clone method
     *
     * @param Deep copy of this object.
	 */
	virtual IProperty* doClone () const;

	
public:	
	
	/** 
     * Returns type of object. 
     *
     * @return Type of this class.
     */
    virtual PropertyType getType () const {return pStream;};

	
	/**
	 * Returns string representation of actual object.
	 * 
	 * If it is an indirect object, we have to notify CXref.
	 */
	virtual void getStringRepresentation (std::string& str) const;
 
	/**
	 * Convert string to an object value.
	 * <exception cref="ObjBadValueE" /> Thrown when we can't parse the string correctly.
	 *
	 * @param str0 Object in a text form.
	 */
	void setStringRepresentation (const std::string& strO);

	/**
 	 * Notify Writer object that this object has changed. We have to call this
	 * function to make changes visible.
	 */
	virtual void dispatchChange () const;

	/**
     * Create xpdf object.
	 *
	 * REMARK: Caller is responsible for deallocating the xpdf object.
     *
     * @return Xpdf object(s).
     */
    virtual Object* _makeXpdfObject () const; 

public:

	/**
	 * Destructor.
	 */
	~CObjectStream ();

//
// Special functions
//
public:

	/**
	 * Return the list of all supported streams
	 *
	 * @return List of supported stream filters.
	 */
	static const std::list<std::string>& getSupportedStreams () 
	{
		std::list<std::string> supported;
		//
		// Initialize the list
		//
		if (supported.empty())
		{
			supported.push_back ("NoFilter");
		}
	
		return &supported;
	}

	
private:
	
	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<const ObserverContext> context)
	{
		// Dispatch the change
		dispatchChange ();
			
		if (context)
		{
			assert (!"Not implemented yet");
			// Notify everybody about this change
			IProperty::notifyObservers (*this, context);
		}else
			throw CObjInvalidOperation ();
	}

};




//=====================================================================================
//
// CObject types
//

typedef CObjectSimple<pNull>	CNull;
typedef CObjectSimple<pBool>	CBool;
typedef CObjectSimple<pInt>		CInt;
typedef CObjectSimple<pReal>	CReal;
typedef CObjectSimple<pString> 	CString;
typedef CObjectSimple<pName> 	CName;
typedef CObjectSimple<pRef> 	CRef;

typedef CObjectComplex<pArray>	CArray;
typedef CObjectComplex<pDict>	CDict;
typedef CObjectStream<>			CStream;



//=====================================================================================
//
//	Memory checker classes
//

/**
 * No memory checks done.
 */
class NoMemChecker 
{public: 
	NoMemChecker () {};
	void objectCreated (IProperty*) {};
	void objectDeleted (IProperty*) {};
};

/**
 * This class stores pointer to every created class in a container. When a class is destroyed, it is removed
 * from the container.
 *
 * After the end of a program, we can count how many objects have not been released.
 * 
 */
class BasicMemChecker
{
public:
	typedef std::list<const IProperty*> _IPsList;

private:
	_IPsList& getList () {static _IPsList ips; return ips;};
	size_t& getMax () {static size_t mx; return mx;};

public:
	//
	BasicMemChecker () {};

	//
	//
	//
	void objectCreated (IProperty* ip)
	{
		#if MEM_CHECKER_OUTPUT
		_printHeader (std::cerr);
		std::cerr << "IProperty [0x"<< (unsigned)ip << "] created.";
		_printFooter (std::cerr);
		#endif
		
		getList().push_back (ip);
		++getMax ();
	};

	//
	//
	//
	void objectDeleted (IProperty* ip)
	{
		#if MEM_CHECKER_OUTPUT
		_printHeader (std::cerr);
		std::cerr << "IProperty [0x"<< (unsigned)ip << "] deleted.";
		#endif

		_IPsList::iterator it = find (getList().begin(), getList().end(), ip);
		if (it != getList().end())
		{
				getList().erase (it);
		}
		else
		{
				printDbg (debug::DBG_CRIT, "!!!!!!!!!! deleting what was not created !!!!!!!!!!");
		}
		#if MEM_CHECKER_OUTPUT
		_printFooter (std::cerr);
		#endif
	};
		
	//
	// Get living IProperty count
	//
	size_t getCount () {return getList().size (); };
	size_t getMaxCount () {return getMax(); };

private:
	void _printHeader (std::ostream& oss)
	{
		oss << std::setw (10) << std::setfill ('<') << "\t";
		oss << std::setbase (16);
	}

	void _printFooter (std::ostream& oss)
	{
		oss << std::setbase (10);
		oss << "\t" << std::setw (10) << std::setfill ('>') << "" << std::endl;
	}
	
};



//=====================================================================================
//
//	Find element functors
//


/**
 * This class is used as functor to an equal algorithm to std::find_if algorithm.
 * Finds out an item specified by its position. find_if CANNOT be used, because it 
 * does not meet 2 main requirements. a) ordering b) not making COPIES of the functor and
 * this functor RELIES on these requirements.
 * 
 *
 * More effective algorithms could be used but this approach is 
 * used to get more generic.
 */
class ArrayIdxComparator
{
private:
	unsigned int pos;
	boost::shared_ptr<IProperty> ip;

private:
	//
	// We have a stateful object, so prevent copying
	//
	ArrayIdxComparator (const ArrayIdxComparator&);

public:
	ArrayIdxComparator (unsigned int p) : pos(p) {};
	
	boost::shared_ptr<IProperty> getIProperty () {return ip;};
		
	bool operator() (const PropertyTraitComplex<pArray>::value::value_type& _ip)
	{	
		if (0 == pos)
		{
			ip = _ip;
			return true;
		}
		--pos;
		return false;
	}
};


/**
 * This class is used as functor to an equal algorithm to std::find_if algorithm.
 * Finds out an item specified by its position. find_if CANNOT be used, because it 
 * does not meet 2 main requirements. a) ordering b) not making COPIES of the functor and
 * this functor RELIES on these requirements.
 *
 * Perhaps more effective algorithms could be used but this approach is 
 * used to get more generic.
 */
class DictIdxComparator
{
private:
	const std::string str;
	boost::shared_ptr<IProperty> ip;

private:
	//
	// We have a stateful object, so prevent copying
	//
	DictIdxComparator (const DictIdxComparator&);

public:
	DictIdxComparator (const std::string& s) : str(s) {};
		
	boost::shared_ptr<IProperty> getIProperty () {return ip;};
		
	bool operator() (const PropertyTraitComplex<pDict>::value::value_type& item)
	{	
		if (item.first == str)
		{
			ip = item.second;
			return true;
		}
			
		return false;
	};
};



//=====================================================================================
//
//  Helper functions located in cobject.cc



//=====================================================================================
namespace utils {
//=====================================================================================


/**
 * Creates CObject* from xpdf object.
 *
 * @param pdf Pdf in which the created object will live. Parameter to CObject* constructor.
 * @param obj Xpdf object from which the object will be created. Parameter to CObject* constructor.
 * @param ref Indirect reference number of this object (or its parent).
 *
 * @return Pointer to newly created object.
 */
IProperty* createObjFromXpdfObj (CPdf& pdf, Object& obj,const IndiRef& ref);

/**
 * Creates CObject* from xpdf object.
 *
 * @param obj Xpdf object from which the object will be created. Parameter to CObject* constructor.
 *
 * @return Pointer to newly created object.
 */
IProperty* createObjFromXpdfObj (Object& obj);


		
/**
 * Return simple xpdf object (null,number,string...) in string representation.
 * 
 * REMARK: String can represent more different objects, so we have to distinguish among them.
 * This is done at compile time with use of templates, but because of this we have to
 * make other functions also template.
 *
 * @param Value that will be converted to string.
 * @param Output string
 */
template <PropertyType Tp> void simpleValueToString (bool val,std::string& str);
template <PropertyType Tp> void simpleValueToString (int val,std::string& str);
template <PropertyType Tp> void simpleValueToString (double val,std::string& str);
template <PropertyType Tp> void simpleValueToString (const std::string& val,std::string& str);
template <PropertyType Tp> void simpleValueToString (const NullType& val,std::string& str);
template <PropertyType Tp> void simpleValueToString (const IndiRef& val,std::string& str);

/**
 * Return complex xpdf object (null,number,string...) in string representation.
 *
 * REMARK: It is a template function because I think stream won't be converted to string
 * as easily as a dictionary. So we specialize these function for pArray and pDict.
 *
 * @param Value that will be converted to string.
 * @param Output string
 */
template <PropertyType Tp> void complexValueToString (const typename PropertyTraitComplex<pArray>::value& val, 
													  std::string& str);
template <PropertyType Tp> void complexValueToString (const typename PropertyTraitComplex<pDict>::value& val, 
													  std::string& str);

/**
 * Save real xpdf object value to val.
 * 
 * @param obj	Xpdf object which holds the value.
 * @param val	Variable where the value will be stored.
 */
template <PropertyType Tp,typename T> void simpleValueFromXpdfObj (Object& obj, T val);
template <PropertyType Tp,typename T> void complexValueFromXpdfObj (IProperty& ip, Object& obj, T val);

/**
 * Create xpdf Object which represents value.
 * 
 * @param obj	Value where the value is stored.
 * @return 		Xpdf object where the value is stored.
 */
template <PropertyType Tp,typename T> Object* simpleValueToXpdfObj (T val);

/**
 * Template functions can't be virutal, so this is a helper
 * function that has the same functionality as getAllPropertyNames() but
 * can take as parameter any container type that supports push_back function.
 *
 * @param container Container to which all names will be added.
 */
template<typename T>
void 
getAllNames (T& container, const typename PropertyTraitComplex<pDict>::value& store)
{
	for (typename PropertyTraitComplex<pDict>::value::const_iterator it = store.begin();
		it != store.end(); ++it)
	{
			container.push_back ((*it).first);
	}
}


/**
 * Create xpdf object from string.
 *
 * @param str String that should represent an xpdf object.
 * @param xref Xref of actual pdf if any.
 *
 * @return Xpdf object whose string representation is in str.
 */
Object* xpdfObjFromString (const std::string& str, XRef* xref = NULL);


/**
 * Parses string to get simple values like int, name, bool etc.
 * 
 * <a cref="ObjBadValueE" /> Thrown when the string, can't be parsed correctly.
 * 
 * @param str	String to be parsed.
 * @param val	Desired value.
 */
void simpleValueFromString (const std::string& str, bool& val);
void simpleValueFromString (const std::string& str, int& val);
void simpleValueFromString (const std::string& str, double& val);
void simpleValueFromString (const std::string& str, std::string& val);
void simpleValueFromString (const std::string& str, IndiRef& val);


/**
 * Free an object. We assume that all child objects (if any)
 * have been already freed.
 *
 * Copy & paste from Object.h
 * 
 * @param obj	Object to be freed.
 */
void freeXpdfObject (Object* obj);


/**
 * Constructs an item containing IProperty of a special container from a value that we want to replace.
 *
 * @param item  Item that will be replaced
 * @patam ip	IProperty that will be inserted;
 */
inline PropertyTraitComplex<pArray>::value::value_type 
constructItemFromIProperty (const PropertyTraitComplex<pArray>::value::value_type&,
							PropertyTraitComplex<pArray>::value::value_type ip) {return ip;}

inline PropertyTraitComplex<pDict>::value::value_type 
constructItemFromIProperty (const PropertyTraitComplex<pDict>::value::value_type& item,
							boost::shared_ptr<IProperty> ip) {return std::make_pair(item.first,ip);}

/**
 * Get IProperty from an item of a special container.
 *
 * @param item Item of a special container.
 */
inline boost::shared_ptr<IProperty>  
getIPropertyFromItem (PropertyTraitComplex<pArray>::value::value_type item) {return item;}
inline boost::shared_ptr<IProperty>  
getIPropertyFromItem (const PropertyTraitComplex<pDict>::value::value_type& item) {return item.second;}

/**
 * Return property id from an container item. It can be either the name or the position.
 *
 * @param item Item of a container.
 * 
 * @return Property Id.
 */
inline size_t
getPropertyIdFromItem (PropertyTraitComplex<pArray>::value::value_type, size_t pos) {return pos;}
inline PropertyTraitComplex<pDict>::propertyId  
getPropertyIdFromItem (const PropertyTraitComplex<pDict>::value::value_type& item, size_t) {return item.first;}


/**
 * Convert xpdf object to string
 *
 * @param obj Xpdf object that will be converted.
 * @param str This will hold the string representation of the object.
 */
void xpdfObjToString (Object& obj, std::string& str);

/**
 * Returns true if object has a parent.
 *
 * <cref exception="ObjInvalidOperation"> Thrown when the object does not belong
 * to a valid pdf.
 *
 * @param ip	IProperty of object. 
 * @param indiObj Out parameter wheren indirect object will be saved, because
 * finding the object can be an expensive operation.
 */
bool objHasParent (const IProperty& ip);
bool objHasParent (const IProperty& ip, boost::shared_ptr<IProperty>& indiObj);



//=====================================================================================
} /* namespace utils */
//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================




//
// Include the actual implementation of CObject class
//
#include "cobjectI.h"


#endif // _COBJECT_H

