// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/* 
 * =====================================================================================
 *        Filename:  cobject.h
 *     Description:  Header file containing definition of IProperty and CObject classes.
 *         Created:  01/18/2006 
 *          Author:  jmisutka (06/01/19), 
 *
 * =====================================================================================
 */
#ifndef _COBJECT_H
#define _COBJECT_H

// all basic includes
#include "static.h"
#include "iproperty.h"
// Filters
#include "filters.h"


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
 *
 * This is an advance c++ template technique which allows us to store
 * additional information that identify variable type (e.g. specification of function types of a function template --
 * function in a template class).
 *
 * If someone tries to use unsupported type (e.g pCmd, etc.), she should get compile error
 * because PropertyTraitSimple<> has no body.
 *
 * REMARK: BE CAREFUL when manipulating these ones. Because a small change could
 * be hard to find.
 *
 * This class does not provide default copy constructor because copying a
 * property could be understood either as deep copy or shallow copy. 
 * Copying complex types could be very expensive so we have made this decision to
 * avoid it.
 *
 * \see CObjectComplex, CObjectStream
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
 * This is a very generic class representing simple objects like null, string, number etc.
 * It does not have so many special functions as CObjectComplex.
 *
 * Other xpdf objects like objCmd can not be instantiated although the PropertyType 
 * exists. It is because PropertyTraitSimple is not specialized for these types.
 *
 * We use memory checking with this class which save information about existing IProperties.
 * This technique can be used to detect memory leaks etc. 
 *
 * Xpdf object is real mess so we just instantiate this object with xpdf object and
 * use this object. They were definitely not meant for changing, actually there
 * are meany places which clearly prohibit dissolve any hope for a sane way to change the object.
 *
 */
template <PropertyType Tp, typename Checker = BasicMemChecker>
class CObjectSimple : noncopyable, public IProperty
{
public:
	typedef typename PropertyTraitSimple<Tp>::writeType	 WriteType;
	typedef typename PropertyTraitSimple<Tp>::value 	 Value;

private:
	/** Object's valuei holder. */
	Value value;
	

	//
	// Constructors
	//
public:
	
	/**
	 * Constructor. 
	 * The object will read value from o and store it. We do NOT save any reference to o.
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CObjectSimple (CPdf& p, Object& o, const IndiRef& rf);

	
	/**
	 * Constructor.
	 * The object will read value from o and store it. We do NOT save any reference to o.
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
	 * Clone. 
	 *
	 * Performs deep copy.
	 * REMARK: CRef does NOT copy the referenced object just itself.
	 *
	 * @return Deep copy of this object.
	 */
	virtual IProperty* doClone () const;


	//
	// Get methods
	//
public:
	/**
	 * Return type of this property.
	 *
	 * @return Type of this property.
	 */
	virtual PropertyType getType () const {return Tp;};
			

	/**
	 * Returns string representation of this object according to pdf
	 * specification. 
	 * 
	 * @param str 	Output string that will hold the string representation 
	 * 				of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;
	
	/**
	 * Return property value. Each property has its own return type.
	 *
	 * @param val Out parameter where property value will be stored.
	 */
	void getValue (Value& val) const;

	/**
	 * Return property value. Each property has its own return type.
	 *
	 * @return Property value.
	 */
	Value getValue () const;

	//
	// Set methods
	//
public:
	/**
	 * Convert string to an object value.
	 * \exception ObjBadValueE Thrown when we can't parse the string correctly.
	 *
	 * @param strO String object that will be parsed and saved to this object's value.
	 */
	void setStringRepresentation (const std::string& strO);

	
	/**
	 * Change the value of an object. The variable type depends
	 * on CObject type.
	 * 
	 * We can define the best type to represent an pdf object in
	 * PropertyTraitSimple.
	 *
	 * \see PropertyTraitSimple
	 *
	 * @param val	Value that will be set.
	 */
	void setValue (WriteType val);

	//
	// Destructor
	//
public:
	/**
	 * Destructor
	 */
	~CObjectSimple ();
	

	//
	// Helper methods
	//
public:	

	/**
	 * Make xpdf Object from this object. This function allocates and initializes xpdf object.
	 * Caller has to free the xpdf Object (call Object::free and then
	 * deallocating)
	 *
	 * \exception ObjBadValueE Thrown when xpdf can't parse the string representation of this
	 * object correctly.
	 * 
	 * @return Xpdf object representing value of this simple object.
	 */
	virtual ::Object* _makeXpdfObject () const;
	
private:

	/**
	 * Create context of a change.
	 *
	 * REMARK: Be carefull. Deallocate this object.
	 * 
	 * @return Context in which a change occured.
	 */
	ObserverContext* _createContext () const
	{
		// Save original value for the context
		boost::shared_ptr<IProperty> oldValue (this->clone());
		// Set original values
		oldValue->setPdf (this->getPdf());
		oldValue->setIndiRef (this->getIndiRef());
		// Create the context
		return new BasicObserverContext (oldValue);
	}

	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<const ObserverContext> context);

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
 *
 * CObjectComplex type trait.
 *
 * This is an advance c++ template technique which allows us to store
 * additional information that identify variable type (e.g. specification of function types of a function template --
 * function in a template class).
 *
 * If someone tries to use unsupported type (e.g pCmd, etc.), she should get compile error
 * because PropertyTraitSimple<> has no body.
 *
 * REMARK: BE CAREFUL when manipulating these ones. Because a small change could
 * be hard to find.
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

//
// Forward declarations
//
namespace utils 
{
std::pair<size_t, PropertyTraitComplex<pArray>::value::value_type>
	constructIdPairFromIProperty (size_t pos, const PropertyTraitComplex<pArray>::value::value_type& item);
PropertyTraitComplex<pDict>::value::value_type
	constructIdPairFromIProperty (size_t, const PropertyTraitComplex<pDict>::value::value_type& item);
}

template <typename Checker> class CObjectStream;


/** 
 * Template class representing complex PDF objects from specification v1.5.
 *
 * This is a generic class joining implementation of dictionary and array together in in one place.
 *
 * Other xpdf objects like objCmd can not be instantiated although the PropertyType 
 * exists. It is because PropertyTraitSimple is not specialized for these types.
 *
 * We use memory checking with this class which save information about existing IProperties.
 * This technique can be used to detect memory leaks etc. 
 *
 * Xpdf object is real mess so we just instantiate this object with xpdf object and
 * use this object. They were definitely not meant for changing, actually there
 * are meany places which clearly prohibit dissolve any hope for a sane way to change the object.
 * 
 * This class uses another advance c++ technique called Incomplete
 * Instantiation. The specific features are implemented using this feature.
 * It simply means that when a function in a template class is not used, it is not instatiated
 * (e.g. CArray won't have addProperty (IProperty& ,string&) method.)
 *
 * This class does not provide default copy constructor because copying a
 * property could be understood either as deep copy or shallow copy. 
 * Copying complex types could be very expensive so we have made this decision to
 * avoid it.
 *
 * \see CObjectSimple, CObjectStream
 */
template <PropertyType Tp, typename Checker = BasicMemChecker>
class CObjectComplex : noncopyable, public IProperty
{
	template<typename T> friend class CObjectStream;
	
public:
	typedef typename PropertyTraitComplex<Tp>::writeType  		WriteType;
	typedef typename PropertyTraitComplex<Tp>::propertyId 		PropertyId;
	typedef typename PropertyTraitComplex<Tp>::indexComparator	IndexComparator;
	typedef typename PropertyTraitComplex<Tp>::value 	  		Value;  
	
private:
	
	/** Object's value. */
	Value value;


	//
	// Constructors
	//
public:
	/**
	 * Constructor.
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CObjectComplex (CPdf& p, Object& o, const IndiRef& rf);

	/**
	 * Constructor.
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

	/** 
	 * Return new instance. 
	 *
	 * This function is a factory method design pattern for creating complex instances.
	 *
	 * @return New complex object.
	 */
	virtual CObjectComplex<Tp,Checker>* _newInstance () const
		{ return new CObjectComplex<Tp,Checker>; }
	
	//
	// Get methods
	//
public:

	/**
	 * Return type of this property.
	 *
	 * @return Type of this property.
	 */
	virtual PropertyType getType () const {return Tp;};
	

	/**
	 * Returns string representation of this object according to pdf
	 * specification. 
	 * 
	 * @param str 	Output string that will hold the string representation 
	 * 				of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;

	
	/** 
	 * Returns property count.
	 * 
	 * @return Property count.
	 */
	size_t getPropertyCount () const 
	{
		//kernelPrintDbg (debug::DBG_DBG, "getPropertyCount(" << debug::getStringType<Tp>() << ") = " << value.size());
		return value.size();
	};
 

	/**
	 * Inserts all property names of this complex type to the container supplied by caller. 
	 * 
	 * REMARK: Specific for pDict.
   	 *
	 * @param container Output container of string objects. Could be vector,list,deque etc.
	 */
	template<typename Container>
	void getAllPropertyNames (Container& container) const;

	/**
	 * Returns true if the property name is present in the dictionary.
	 *
	 * @param name Property name
	 *
	 * @return True if the property exists, false otherwise.
	 */
	bool containsProperty (const std::string& name) const;
	
	/**
	 * Returns value of property identified by its name/position depending on the type of this object.
   	 *
   	 * @param 	id 	Variable identifying position of the property.
	 * @return	Output variable where the value will be stored.
   	 */
	boost::shared_ptr<IProperty> getProperty (PropertyId id) const;

	/**
	 * Returns property type of an item identified by name/position.
	 *
	 * \exception ObjInvalidPositionInComplex When the id does not correctly identify an item.
	 *
	 * @param	name	Name of the property.
	 * @return Property type.	
	 */
	PropertyType getPropertyType (PropertyId id) const 
		{return getProperty(id)->getType();};

	
	//
	// Set methods
	//
public:
	/**
	 * Set pdf to this object and also to all its children.
	 *
	 * @param pdf New pdf.
	 */
	virtual void setPdf (CPdf* pdf);

	/**
	 * Set ref to this object and also to all its children.
	 *
	 * @param pdf New indirect reference numbers.
	 */
	virtual void setIndiRef (const IndiRef& rf);

	/**
	 * Sets property type of an item.
	 * 
	 * Firstly, the property that is passed as argument is cloned, the argument itself is not used.
	 * The cloned object replaces object specified by id. If the item does not
	 * exist it is added.
	 * 
	 * @param	id		Name/Index of property
	 * @param	value	Value, for simple types (int,string,...) and for complex types IProperty*
	 *
	 * @return Pointer to the new property.
	 */
	boost::shared_ptr<IProperty> setProperty (PropertyId id, IProperty& ip);
	
	/**
	 * Adds property to array/dict. 
	 *
	 * Firstly, the property that is passed as argument is cloned, the argument itself is not added. 
	 * The cloned object is added, automatically associated with the pdf where the object is beeing added.
	 * Indicate that this object has changed and return the pointer to the cloned object.
	 *
	 * \exception OutOfRange Thrown when position is out of range.
	 *
	 * @param newIp 		New property.
	 * @param propertyName 	Name of the created property.
	 *
	 * @return Pointer to the new property.
	 */
	boost::shared_ptr<IProperty> addProperty (const IProperty& newIp);

	/** \copydoc addProperty */
	boost::shared_ptr<IProperty> addProperty (size_t position, const IProperty& newIp);
	
	/** \copydoc addProperty */
	boost::shared_ptr<IProperty> addProperty (const std::string& propertyName, const IProperty& newIp);

	
	/**
	 * Remove property from array/dict. 
	 *
	 * \exception ElementNotFoundException Thrown when object is not found.
	 * 
	 * @param id Name/Index of property
	 */
	void delProperty (PropertyId id);


	//
	// Destructor
	//
public:
	/**
	 * Destructor
	 */
	~CObjectComplex ();
	

	//
	// Helper methods
	//
public:

	/**
	 * Perform an action on each element.
	 *
	 * Fctor::operator () (std::pair<int/string, shared_ptr<IProperty> >)
	 * 
	 * @param fnc Functor that will do the work.
	 */
	template<typename Fctor>
	void forEach (Fctor& fctor)
	{
		int pos = 0;
		typename Value::iterator it = value.begin ();
		for (; it != value.end (); ++it, ++pos)
		{
			fctor (utils::constructIdPairFromIProperty (pos, *it));
		}
	}

	/**
	 * Make xpdf Object from this object. This function allocates and initializes xpdf object.
	 * Caller has to free the xpdf Object (call Object::free and then
	 * deallocating)
	 *
	 * \exception ObjBadValueE Thrown when xpdf can't parse the string representation of this
	 * object correctly.
	 * 
	 * @return Xpdf object representing value of this simple object.
	 */
	virtual ::Object* _makeXpdfObject () const;

private:
	/**
	 * Create context of a change.
	 *
	 * REMARK: Be carefull. Deallocate this object.
	 * 
	 * @param changedIp Pointer to old value.
	 * 
	 * @return Context in which a change occured.
	 */
	ObserverContext* _createContext (boost::shared_ptr<IProperty>& changedIp);

	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param newValue Pointer to new value of an object.
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<IProperty> newValue, 
						 boost::shared_ptr<const ObserverContext> context);

	//
	// Mode interface
	//
protected:
	/**
	 * Set mode of a property.
	 *
	 * @param ip IProperty which mode will be set.
	 * @param name Identification of the property. String for dicts, number for
	 * arrays.
	 */
	void _setMode (boost::shared_ptr<IProperty> ip, PropertyId id) const;

public:
	/**
	 * Return all object we have access to.
	 *
	 * @param store Output container of all child objects.
	 */
	template <typename Storage>
	void _getAllChildObjects (Storage& store) const;

};




//=====================================================================================
// CObjectStream
//=====================================================================================

//
// Forward declaration
//
template<typename T> class CStreamXpdfReader;

/**
 * Template class representing stream PDF objects from specification v1.5.
 *
 * It is neither a simple object, because it does not
 * contain just simple value, nor a complex object, because it can not be simple represented
 * in that generic class. It contains a dictionary and a stream. It does not have methods common
 * to complex objects.
 *
 * This is a generic class joining implementation of dictionary and array together in in one place.
 *
 * Other xpdf objects like objCmd can not be instantiated although the PropertyType 
 * exists. It is because PropertyTraitSimple is not specialized for these types.
 *
 * We use memory checking with this class which save information about existing IProperties.
 * This technique can be used to detect memory leaks etc. 
 *
 * Xpdf stream objects are the worst from all xpdf objects because of their deallocation politics.
 * It is really not easy to say when, where and who should deallocate an xpdf stream, its buffer etc...
 * 
 * This class does not provide default copy constructor because copying a
 * property could be understood either as deep copy or shallow copy. 
 * Copying complex types could be very expensive so we have made this decision to
 * avoid it.
 */
template <typename Checker = BasicMemChecker>
class CObjectStream : noncopyable, public IProperty
{
	typedef CObjectSimple<pName> 	CName;
	typedef CObjectSimple<pInt> 	CInt;
	typedef CObjectComplex<pDict> 	CDict;
	typedef CObjectComplex<pArray> 	CArray;
	typedef boost::iostreams::filtering_streambuf<boost::iostreams::output> filtering_ostreambuf;
	typedef std::string PropertyId;

public:
	// We can access xpdf stream only through CStreamXpdfReader 
	template<typename T> friend class CStreamXpdfReader;

	typedef std::vector<filters::StreamChar> Buffer;

protected:

	/** Object dictionary. */
	CDict dictionary;
	
	/** Buffer. */
	Buffer buffer;

	/** XpdfParser. */
	::Parser* parser;
	/** Current object in an opened stream. */
	mutable ::Object curObj;
		

	//
	// Constructors
	//
public:
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
	 * It adds one required property to objects dictionary namely "Length". This
	 * is according to the pdf specification.
	 *
	 * @param makeReqEntries If true required entries are added to stream
	 * dictionary.
	 */
	CObjectStream (bool makeReqEntries = true);

	
	//
	// Cloning
	//
protected:

	/**
     * Implementation of clone method. 
	 *
     * @param Deep copy of this object.
	 */
	virtual IProperty* doClone () const;

	/** Return new instance. */
	virtual CObjectStream<Checker>* _newInstance () const
		{ return new CObjectStream<Checker> (false); }

	/** Create required entries. */
	void createReqEntries ();

	//
	// Dictionary methods, delegated to CDict
	//
public:
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	size_t getPropertyCount () const
		{return dictionary.getPropertyCount ();}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	template<typename Container> 
	void getAllPropertyNames (Container& container) const
		{ dictionary.getAllPropertyNames (container); }
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	boost::shared_ptr<IProperty> getProperty (PropertyId id) const
		{return dictionary.getProperty (id);}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	PropertyType getPropertyType (PropertyId id) const
		{return dictionary.getPropertyType (id);}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	boost::shared_ptr<IProperty> setProperty (PropertyId id, IProperty& ip)
		{return dictionary.setProperty (id, ip);}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	boost::shared_ptr<IProperty> addProperty (PropertyId id, const IProperty& newIp)
		{return dictionary.addProperty (id, newIp);}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	void delProperty (PropertyId id)
		{dictionary.delProperty (id);}


	//
	// Get methods
	//
public:	
	
	/** 
     * Returns type of object. 
     *
     * @return Type of this class.
     */
    virtual PropertyType getType () const {return pStream;}

	/**
	 * Returns string representation of this object.
	 *
	 * @param str Output string representation.
	 */
	virtual void getStringRepresentation (std::string& str) const;

	/**
	 * Returns decoded string representation of this object.
	 *
	 * @param str Output string representation.
	 */
	virtual void getDecodedStringRepresentation (std::string& str) const;

	/**
	 * Get encoded buffer. Can contain non printable characters.
	 *
	 * @return Buffer.
	 */
	const Buffer& getBuffer () const {return buffer;}
	
	/**
	 * Get filters.
	 *
	 * @param container Container of filter names.
	 */
	template<typename Container>
	void getFilters (Container& container) const;


	//
	// Set methods
	//
public:
	/**
	 * Set pdf to itself and also tu all children
	 *
	 * @param pdf New pdf.
	 */
	virtual void setPdf (CPdf* pdf);

	/**
	 * Set ref to itself and also tu all children
	 *
	 * @param pdf New indirect reference numbers.
	 */
	virtual void setIndiRef (const IndiRef& rf);

	/**
	 * Set encoded buffer.
	 *
	 * @param buf New buffer.
	 */
	void setRawBuffer (const Buffer& buf);

	/**
	 * Set decoded buffer. 
	 * Use avaliable filters. If a filter is not avaliable an exception is thrown.
	 *
	 * @param buf New buffer.
	 */
	template<typename Container>
	void setBuffer (const Container& buf);

	//
	// Parsing (use friend CStreamXpdfReader)
	//
private:
	
	/**
	 * Initialize parsing mechanism.
	 *
	 * REMARK: if CObject is not in a pdf, we MUST be sure that it does not
	 * use indirect objects.
	 */
	void open ();

	/**
	 * Close parser.
	 */
	void close ();
	
	/**
	 * Get xpdf object and copy it to obj.
	 *
	 * REMARK: We can not do any buffering (caching) of xpdf objects, because
	 * xpdf already does caching and it will NOT work correctly with inline
	 * images. We would buffer WRONG data.
	 *
	 * @param obj Next xpdf object.
	 */
	void getXpdfObject (::Object& obj);
	
	/**
	 * Get xpdf stream. Be carefull this is not a copy.
	 * 
	 * @return Stream.
	 */
	 ::Stream* getXpdfStream ();

	/**
	 * Is the last object end of stream.
	 *
	 * This is not very common behaviour, but we can not use caching 
	 * \see getXpdfObject
	 * so we can tell if it is the end after fetching an object which means
	 * after calling getXpdfObject.
	 * 
	 * @return True if we no more data avaliable, false otherwise.
	 */
	bool eof () const;
	
	
	//
	// Destructor
	//
public:

	/**
	 * Destructor.
	 */
	~CObjectStream ();



	//
	// Helper methods
	//
public:
	/**
	 * Make xpdf Object from this object. This function allocates and initializes xpdf object.
	 * Caller has to free the xpdf Object (call Object::free and then
	 * deallocating)
	 *
	 * \exception ObjBadValueE Thrown when xpdf can't parse the string representation of this
	 * object correctly.
	 * 
	 * @return Xpdf object representing value of this simple object.
	 */
	virtual Object* _makeXpdfObject () const; 


private:
	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<const ObserverContext> context);

	/**
	 * Get encoded buffer.
	 *
	 * @param container Output container.
	 */
	template<typename Container>
	void encodeBuffer (const Container& container);

private:
	/**
	 * Get length.
	 *
	 * @return Stream length.
	 */
	size_t getLength () const;

	/**
	 * Set length.
	 *
	 * @param len Stream Length.
	 */
	void setLength (size_t len);

private:
	/**
	 * Create context of a change.
	 *
	 * REMARK: Be carefull. Deallocate this object.
	 * 
	 * @return Context in which a change occured.
	 */
	ObserverContext* _createContext () const;

public:
	/**
	 * Return all object we have access to.
	 *
	 * @param store Container of objects.
	 */
	template <typename Storage>
	void _getAllChildObjects (Storage& store) const;

	
	//
	// Special functions
	//
public:

	/**
	 * Return the list of all supported streams
	 *
	 * @return List of supported stream filters.
	 */
	template<typename Container>
	static void getSupportedStreams (Container& supported) 
		{ filters::CFilterFactory::getSupportedStreams (supported); }

}; /* CObjectStream */




//=====================================================================================
// CObject typedefs
//=====================================================================================

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


//
// Forward declaration
//
namespace utils{
void xpdfObjToString (Object& obj, std::string& str);
}

/**
 * Adapter which is able to read sequentially from more CStreams.
 *
 * It stores a container of streams and when the actual stream does not
 * contain more objects, tries to read the next one if any.
 *
 * This class is an example of Adapter design pattern. We have to be able to
 * read from more streams sequentially and this class provides us with the
 * interface.
 */
template<typename Container>
class CStreamXpdfReader
{
public:
	typedef std::vector<boost::shared_ptr<CStream> > CStreams;

private:
	CStreams streams;	/**< All streams. */
	boost::shared_ptr<CStream> actstream;	/**< Actual stream that is beeing parsed. */
	size_t pos;			/**< Position of actual parsed stream in the stream container. */
	size_t objread; 	/**< Helper variable for debugging. Number of read objects. */
	

public:

	/** Constructor. */
	CStreamXpdfReader (Container& strs) : pos(0), objread (0)
		{ assert (!strs.empty()); std::copy (strs.begin(), strs.end(), std::back_inserter(streams));}
	CStreamXpdfReader (boost::shared_ptr<CStream> str) : pos(0), objread (0)
		{ assert (str); streams.push_back (str); }

	/** Open. */
	void open ()
	{
		assert (!streams.empty());
		if (actstream && 0 != pos)
			{ assert (!"Stream opened before."); }

		actstream = streams.front ();
		actstream->open ();
	}

	/** Close. */
	void close ()
	{
		assert (!streams.empty());
		assert (streams.size() == pos + 1);
		assert (actstream == streams.back());
		assert (actstream->eof());
		
		actstream->close ();
	}

	/** 
	 * Close. 
	 * Save parsed streams to container.
	 *
	 * @param parsedstreams Output buffer that will contain all streams we have
	 * really parsed.
	 */
	template<typename Ctr>
	void close (Ctr& parsedstreams)
	{
		assert (!streams.empty());
		assert (actstream->eof());
		
		for (size_t i = 0; i <= pos; ++i)
			parsedstreams.push_back (streams[i]);
		
		actstream->close ();
	}

	/** Get xpdf object. */
	void getXpdfObject (::Object& obj)
	{
		assert (!actstream->eof());
		
		// Get an object
		actstream->getXpdfObject (obj);

		// If we are at the end of this stream but another stream is not empty 
		// get the object
		if (actstream->eof() && !eof())
		{
			assert (obj.isEOF());
			actstream->getXpdfObject (obj);
		}
		
		/** debugging \TODO remove. */
		objread ++;
	}

	/** 
	 * Is end of stream. 
	 *
	 * We can not cache and due to this fact we can not tell if a stream
	 * is empty without fetching an object. 
	 */
	bool eof ()
	{ 
		if (eofOfActualStream())
		{
			// Do we have another stream
			while (actstream != streams.back())
			{
				assert (pos < streams.size());
				actstream->close();
				// Take next stream
				++pos;
				actstream = streams[pos];
				actstream->open ();
				// Fetch an object and look at it
				xpdf::XpdfObject obj;
				actstream->getXpdfObject (*obj);
				if (!actstream->eof())
				{
					actstream->close();
					actstream->open();
					break;
				}
			}
			return  (actstream == streams.back() && eofOfActualStream()); 
		
		}else
		{
			return false;
		}
	}

	/** End of actual stream. */
	bool eofOfActualStream ()
		{ return (actstream->eof()); }

	/** Get xpdf stream. */
	::Stream* getXpdfStream ()
		{ return actstream->getXpdfStream(); }

	
};

//=====================================================================================
//	Memory checker classes -- DEBUGGING PURPOSES
//=====================================================================================

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
 * After the end of a program, we can count how many objects have not been released. If zero left, we know 
 * that we do not have a memory leak.
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
		std::cerr << "IProperty created.";
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
		getList().pop_back ();
		return;
		#if MEM_CHECKER_OUTPUT
		_printHeader (std::cerr);
		std::cerr << "IProperty deleted.";
		#endif

		_IPsList::iterator it = find (getList().begin(), getList().end(), ip);
		if (it != getList().end())
		{
				getList().erase (it);
		}
		else
		{
				utilsPrintDbg (debug::DBG_CRIT, "!!!!!!!!!! deleting what was not created !!!!!!!!!!");
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
//	Find element functors
//=====================================================================================


/**
 * This class is used as functor to analgorithm similar to std::find_if.
 *
 * Finds out an item specified by its position. find_if CANNOT be used, because it 
 * does not meet 2 main requirements. a) ordering b) not making COPIES of the functor and
 * this functor RELIES on these requirements.
 *
 * More effective algorithms could be used but this approach is 
 * used to be more generic.
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
 * 
 * Finds out an item specified by its position. find_if CANNOT be used, because it 
 * does not meet 2 main requirements. a) ordering b) not making COPIES of the functor and
 * this functor RELIES on these requirements.
 *
 * Perhaps more effective algorithms could be used but this approach is 
 * used to be more generic.
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
/** Namespace includes all helper functions and classes.*/
namespace utils {
//=====================================================================================

//=========================================================
// Validate functions
//=========================================================

/**
 * Make name valid.
 * 
	Beginning with PDF 1.2, any character except null (character code 0) may be included
	in a name by writing its 2-digit hexadecimal code, preceded by the number
	sign character (#); see implementation notes 3 and 4 in Appendix H. This
	syntax is required to represent any of the delimiter or white-space characters or
	the number sign character itself; it is recommended but not required for characters
	whose codes are outside the range 33 (!) to 126 (~). The examples shown in
	Table 3.3 are valid literal names in PDF 1.2 and later.

 *	@param it Start iterator.
 *	@param end End iterator.
 */
template<typename Iter>
std::string 
makeNamePdfValid (Iter it, Iter end)
{
	typedef std::string::value_type Item;
	std::string tmp;
	for (; it != end; ++it)
	{
		if ( '!' > (*it) || '~' < (*it))
		{ // Convert it to ascii
		
			Item a = (Item) (*it) >> sizeof (Item) * 4;
			Item b = (Item) (*it) & ((unsigned) 1 << sizeof(Item) * 4);
			tmp += std::string ("#") + a + b;
			
		}else
			tmp += *it;
	}
	
	return tmp;
}

/**
 * Make name valid.
 * 
	Beginning with PDF 1.2, any character except null (character code 0) may be included
	in a name by writing its 2-digit hexadecimal code, preceded by the number
	sign character (#); see implementation notes 3 and 4 in Appendix H. This
	syntax is required to represent any of the delimiter or white-space characters or
	the number sign character itself; it is recommended but not required for characters
	whose codes are outside the range 33 (!) to 126 (~). The examples shown in
	Table 3.3 are valid literal names in PDF 1.2 and later.

 *	@param str Character string.
 */
inline std::string 
makeNamePdfValid (const char* str)
	{ return makeNamePdfValid (&str[0], &str[strlen(str)]); }

/**
 * Make string pdf valid.
 *
 Any characters may appear in a string except unbalanced parentheses and
 the backslash, which must be treated specially./
 *
 * @param it Start iterator.
 * @param end End iterator.
 */
template<typename Iter>
std::string 
makeStringPdfValid (Iter it, Iter end)
{
	typedef typename std::string::value_type Item;
	std::string tmp;
	for (; it != end; ++it)
	{
		if ( '\\' == (*it))
		{ // Escape every backslash
			tmp += '\\';
		}
		else if ( '(' == (*it) || ')' == (*it))
		{ // Prepend \ before ( or )
			if ('\\' != tmp[tmp.length() - 1])
				tmp += '\\';
		}
		
		tmp += *it;
	}
	
	return tmp;
}

/**
 * 
 * Make string pdf valid.
 *
 Any characters may appear in a string except unbalanced parentheses and
 the backslash, which must be treated specially./
 *
 * @param str Character string.
 */
inline std::string 
makeStringPdfValid (const char* str)
	{ return makeStringPdfValid (&str[0], &str[strlen(str)]); }

/**
 * Make stream pdf valid.
 *
 * Not needed now.
 * 
 * @param it Start insert iterator.
 * @param end End iterator.
 */
template<typename Iter>
void
makeStreamPdfValid (Iter it, Iter end, std::string& out)
{
	for (; it != end; ++it)
	{
		//if ( '\\' == (*it))
		//{ // "Escape" every occurence of '\'
		//		out += '\\';
		//}

		out += *it;
	}
}
	

//=========================================================
// Creation functions
//=========================================================

/**
 * Creates CObject* from xpdf object.
 *
 * @param pdf Pdf in which the created object will live. Parameter to CObject* constructor.
 * @param obj Xpdf object from which the object will be created. Parameter to CObject* constructor.
 * @param ref Indirect reference number of this object (or its parent).
 *
 * @return Pointer to newly created object.
 */
IProperty* createObjFromXpdfObj (CPdf& pdf, ::Object& obj,const IndiRef& ref);

/**
 * Creates CObject* from xpdf object.
 *
 * @param obj Xpdf object from which the object will be created. Parameter to CObject* constructor.
 *
 * @return Pointer to newly created object.
 */
IProperty* createObjFromXpdfObj (::Object& obj);

/**
 * Save real xpdf object value to val.
 * 
 * @param obj	Xpdf object which holds the value.
 * @param val	Variable where the value will be stored.
 */
template <PropertyType Tp,typename T> void simpleValueFromXpdfObj (::Object& obj, T val);

/**
 * Save real xpdf object value to val.
 * 
 * @param ip 	IProperty that will be initialized by obj.
 * @param obj	Xpdf object which holds the value.
 * @param val	Variable where the value will be stored.
 */
template <PropertyType Tp,typename T> void complexValueFromXpdfObj (IProperty& ip, ::Object& obj, T val);

/**
 * This function is a slower equivalent to complexValueFromXpdfObj. But on the
 * other hand, complexValueFromXpdfObj needs inner dictionary container that we
 * do not have always access to.
 *
 * @param resultDict Dictionary.
 * @param dict Xpdf object from which we init dict.
 */
void dictFromXpdfObj (CDict& dict, ::Object& objDict);

/**
 * Create xpdf Object which represents value.
 * 
 * @param obj	Value where the value is stored.
 * @return 		Xpdf object where the value is stored.
 */
template <PropertyType Tp,typename T> Object* simpleValueToXpdfObj (T val);

/**
 * Create xpdf object from string.
 *
 * @param str String that should represent an xpdf object.
 * @param xref Xref of actual pdf if any.
 *
 * @return Xpdf object whose string representation is in str.
 */
::Object* xpdfObjFromString (const std::string& str, ::XRef* xref = NULL);


/**
 * Create xpdf object from string.
 *
 * @param buffer Stream buffer.
 * @param dict Stream dictionary.
 * @param xref Xref of actual pdf if any.
 *
 * @return Xpdf object.
 */
::Object* xpdfStreamObjFromBuffer (const CStream::Buffer& buffer, ::Object* dict);


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

inline std::pair<size_t, PropertyTraitComplex<pArray>::value::value_type>
constructIdPairFromIProperty (size_t pos, const PropertyTraitComplex<pArray>::value::value_type& item)
	{return std::make_pair (pos, item);}

inline PropertyTraitComplex<pDict>::value::value_type
constructIdPairFromIProperty (size_t, const PropertyTraitComplex<pDict>::value::value_type& item)
	{return item;}


//=========================================================
// To string functions
//=========================================================
		
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

/** \copydoc simpleValueToString */
template <PropertyType Tp> void simpleValueToString (int val,std::string& str);

/** \copydoc simpleValueToString */
template <PropertyType Tp> void simpleValueToString (double val,std::string& str);

/** \copydoc simpleValueToString */
template <PropertyType Tp> void simpleValueToString (const std::string& val,std::string& str);

/** \copydoc simpleValueToString */
template <PropertyType Tp> void simpleValueToString (const NullType& val,std::string& str);

/** \copydoc simpleValueToString */
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

/** \copydoc complexValueToString */
template <PropertyType Tp> void complexValueToString (const typename PropertyTraitComplex<pDict>::value& val, 
													  std::string& str);

/**
 * CStream object to string
 *
 * @param strDict Dictionary string representation.
 * @param begin Buffer begin
 * @param end Buffer end
 * @param out Output string.
 */
template<typename ITERATOR, typename OUTITERATOR>
void streamToString (const std::string& strDict, ITERATOR begin, ITERATOR end, OUTITERATOR out);

/** Makes a valid pdf representation of xpdf string object.
 * @param stringObject Xpdf object with objString type.
 * @param outputBuf Output buffer where final representation is stored.
 *
 * This method creates correct pdf representation of given xpdf string object.
 * It should be used for string objects which may contain NUL characters inside.
 * Otherwise simpleValueToString method can be used.
 * 
 * @return Number of bytes stored in outputBuf.
 */
size_t stringToCharBuffer(Object & stringObject, CharBuffer & outputBuf);

/**
 * Makes a valid pdf representation of a stream using streamToString function.
 * 
 * @param strDict Dictionary string representation.
 * @param streambuf Raw stream buffer.
 * @param outbuf Output buffer.
 *
 * @param Length of data.
 */
size_t streamToCharBuffer (const std::string& strDict, const CStream::Buffer& streambuf, CharBuffer& outbuf);

/** Makes a valid pdf indirect object representation of stream object.
 * @param streamObject Xpdf object representing stream.
 * @param ref Reference for this indirect object.
 * @param outputBuf Output byte buffer containing complete representation.
 * @param asIndirect Flag for indirect output.
 *
 * Allocates and fill buffer in given outputBuf with pdf object format
 * representation of given stream object. Moreover adds indirect header and
 * footer if asIndirect parameter is true. 
 * <br>
 * Given buffer may contain NUL bytes inside. Caller should consume number of
 * returned bytes from outputBuf.
 * <br>
 * Reference parameter is ignored if asIndirect is false, because it is used
 * only for indirect object header.
 * 
 * @return number of bytes used in outputBuf or 0 if problem occures.
 */
size_t streamToCharBuffer (Object & streamObject, Ref ref, CharBuffer & outputBuf, bool asIndirect=true);
	
/**
 * Convert xpdf object to string
 *
 * @param obj Xpdf object that will be converted.
 * @param str This will hold the string representation of the object.
 */
void xpdfObjToString (Object& obj, std::string& str);


/**
 * Get string from xpdf stream object.
 *
 * @param str Output string.
 * @param obj Xpdf stream object.
 */
void
getStringFromXpdfStream (std::string& str, ::Object& obj);


//=========================================================
// From string functions
//=========================================================

/**
 * Create text representation of an indirect object from string and IndiRef.
 *
 * @param rf IndiRef.
 * @param val Value of an object.
 * @param output Output string.
 */
 void createIndirectObjectStringFromString (const IndiRef& rf, const std::string& val, std::string& output);

/**
 * Parse stream object to a container
 *
 * @param container Container of characters (e.g. ints).
 * @param obj Stream object.
 */
template<typename T>
void parseStreamToContainer (T& container, ::Object& obj);


//=========================================================
// Get functions
//=========================================================

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
 * Get IProperty from an item of a special container.
 *
 * @param item Item of a special container.
 */
inline boost::shared_ptr<IProperty>  
getIPropertyFromItem (PropertyTraitComplex<pArray>::value::value_type item) {return item;}

/** \copydoc getIPropertyFromItem */
inline boost::shared_ptr<IProperty>  
getIPropertyFromItem (const PropertyTraitComplex<pDict>::value::value_type& item) {return item.second;}


//=========================================================
// Other functions
//=========================================================

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
 * Returns true if object has a parent.
 *
 * \exception ObjInvalidOperation Thrown when the object does not belong
 * to a valid pdf.
 *
 * @param ip	IProperty of object. 
 */
bool objHasParent (const IProperty& ip);

/**
 * Returns true if object has a parent.
 *
 * \exception ObjInvalidOperation Thrown when the object does not belong
 * to a valid pdf.
 *
 * @param ip	IProperty of object. 
 * @param indiObj Out parameter wheren indirect object will be saved, because
 * finding the object can be an expensive operation.
 */
bool objHasParent (const IProperty& ip, boost::shared_ptr<IProperty>& indiObj);


//=========================================================
//	CObject* "delegate" helper methods
//=========================================================

/**
 * If the object is a reference, fetch the "real" object
 *
 * @param ip IProperty.
 */
boost::shared_ptr<IProperty> getReferencedObject (boost::shared_ptr<IProperty> ip);


//=========================================================
//	CObjectSimple "get value" helper methods
//=========================================================

/**
 * Get simple value from simple cobject.
 *
 * @param ip IProperty.
 *
 * @return Value.
 */
template<typename ItemType, PropertyType ItemPType, typename Value>
inline Value
getValueFromSimple (const boost::shared_ptr<IProperty>& ip)
{
	if (ItemPType == ip->getType ())
	{
		// Cast it to the correct type and return it
		boost::shared_ptr<ItemType> item = IProperty::getSmartCObjectPtr<ItemType> (ip);
		return item->getValue ();

	}else
	{
		utilsPrintDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
		throw ElementBadTypeException ("getValueFromSimple");
	}
}


/** Get int from ip. */
inline int 
getIntFromIProperty (const boost::shared_ptr<IProperty>& ip)
	{return getValueFromSimple<CInt, pInt, int> (ip);}

/** Get double from ip. */
inline double 
getDoubleFromIProperty (const boost::shared_ptr<IProperty>& ip)
{
	return (isInt (ip)) ? getValueFromSimple<CInt, pInt, int> (ip) :
						 getValueFromSimple<CReal, pReal, double> (ip);
}

/** Get string from ip. */
inline std::string
getStringFromIProperty (const boost::shared_ptr<IProperty>& ip)
		{return getValueFromSimple<CString, pString, std::string> (ip);}
	

//=========================================================
//	CObjectSimple "set value" helper methods
//=========================================================

/**
 * Set simple value.
 *
 * @param ip IProperty.
 *
 * @return Value.
 */
template<typename ItemType, PropertyType ItemPType, typename Value>
inline void
setValueToSimple (const boost::shared_ptr<IProperty>& ip, const Value& val)
{
	if (ItemPType != ip->getType ())
	{
		utilsPrintDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
		throw ElementBadTypeException ("");
	}

	// Cast it to the correct type and set value
	boost::shared_ptr<ItemType> item = IProperty::getSmartCObjectPtr<ItemType> (ip);
	item->setValue (val);
}


//=========================================================
//	CDict "get value" helper methods
//=========================================================

/**
 * Get simple value from dict.
 *
 * \todo Use MPL because ItemType and ItemPType depend on each other.!!
 *
 * @param dict	Dictionary.
 * @param id 	Position in the dictionary.
 */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromDict (const boost::shared_ptr<CDict>& dict, const std::string& id)
{
	//utilsPrintDbg (debug::DBG_DBG, "dict[" << id << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = getReferencedObject (dict->getProperty (id));
	// Check the type and get the value
	return getValueFromSimple<ItemType, ItemPType, SimpleValueType> (ip);
}
/** \copydoc getSimpleValueFromDict */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromDict (const CDict& dict, const std::string& id)
{
	//utilsPrintDbg (debug::DBG_DBG, "dict[" << id << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = getReferencedObject (dict.getProperty (id));
	// Check the type and get the value
	return getValueFromSimple<ItemType, ItemPType, SimpleValueType> (ip);
}

/** \copydoc getSimpleValueFromDict */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromDict (const boost::shared_ptr<IProperty>& ip, const std::string& id)
{
	assert (isDict(ip));
	if (!isDict (ip))
	{
		assert (!"IProperty is not a dict.");
		throw ElementBadTypeException ("getSimpleValueFromDict()");
	}

	// Cast it to dict
	boost::shared_ptr<CDict> dict = IProperty::getSmartCObjectPtr<CDict> (ip);
	// Get value from dict
	return getSimpleValueFromDict<SimpleValueType, ItemType, ItemPType> (dict, id);
}

/** Get int from dict. */
template<typename IP>
inline int
getIntFromDict (const IP& ip, const std::string& id)
	{ return getSimpleValueFromDict<int, CInt, pInt> (ip, id);}

/** Get	double from dict. */
template<typename IP>
inline double
getDoubleFromDict (const IP& ip, const std::string& id)
{ 
	// Try getting int, if not successful try double
	try {
		
		return getIntFromDict (ip, id);
		
	}catch (ElementBadTypeException&) {}

	return getSimpleValueFromDict<double, CReal, pReal> (ip, id);
}

/** Get string from dict. */
template<typename IP>
inline std::string
getStringFromDict (const IP& ip, const std::string& id)
	{ return getSimpleValueFromDict<std::string, CString, pString> (ip, id);}

/** Get name from dict. */
template<typename IP>
inline std::string
getNameFromDict (const IP& ip, const std::string& id)
	{ return getSimpleValueFromDict<std::string, CName, pName> (ip, id);}

//=========================================================
//	CDict "set value" helper methods
//=========================================================


/** 
 * Set simple value in dict. 
 * If it is a reference, set fetch it and set it to the fetched object.
 *
 * @param ip Dict property.
 * @param name Name of property.
 * @param val Value to be written.
 */
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInDict (const CDict& dict, const std::string& name, const Value& val)
{
	utilsPrintDbg (debug::DBG_DBG, "dict[" << name << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = dict.getProperty (name);
	// If it is a ref DELEGATE it
	ip = getReferencedObject (ip);
	
	// Cast it to the correct type and set value
	setValueToSimple<ItemType, ItemPType, Value> (ip, val);
}

/** \copydoc setSimpleValueInDict */
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInDict (const IProperty& ip, const std::string& name, const Value& val)
{
	assert (isDict (ip));
	if (!isDict (ip))
	{
		assert (!"Ip is not a dictionary.");
		throw ElementBadTypeException ("");
	}

	// Cast it to dict
	CDict* dict = dynamic_cast<const CDict*> (&ip);
	setSimpleValueInDict<Value, ItemType, ItemPType> (*dict, name, val);
}


/** Set int in array. */
template<typename IP>
inline void
setIntInDict (const IP& ip, const std::string& name, int val)
	{ setSimpleValueInDict<int, CInt, pInt> (ip, name, val);}

/** Set	double in array. */
template<typename IP>
inline void
setDoubleInDict (const IP& ip, const std::string& name, double val)
{ 
	// Try setting double, if not successful try int
	try {
		
		return setSimpleValueInDict<double, CReal, pReal> (ip, name, val);
		
	}catch (ElementBadTypeException&) {}

	setIntInDict (ip, name, static_cast<int>(val));
}

//=========================================================
//	CDict "get type" helper methods
//=========================================================

/**
 * Get iproperty casted to specific type from dictionary.
 *
 * @param dict Dictionary.
 * @param id   Position in the array.
 */
template<typename ItemType, PropertyType ItemPType>
inline boost::shared_ptr<ItemType>
getTypeFromDictionary (const boost::shared_ptr<CDict>& dict, const std::string& key)
{
	utilsPrintDbg (debug::DBG_DBG, "dict[" << key << "]");
	
	// Get the item that is associated with specified key 
	boost::shared_ptr<IProperty> ip = dict->getProperty (key);
	//
	// If it is a Ref forward it to the real object
	// 
	ip = getReferencedObject (ip);

	// Check the type
	if (ItemPType != ip->getType ())
	{
		utilsPrintDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType () << " key[" << key << "]");
		std::string err= "getTypeFromDictionary() [" + key + "]";
		throw ElementBadTypeException (err);
	}

	// Cast it to the correct type and return it
	return IProperty::getSmartCObjectPtr<ItemType> (ip);
}

/** \copydoc getTypeFromDictionary */
template<typename ItemType, PropertyType ItemPType>
inline boost::shared_ptr<ItemType>
getTypeFromDictionary (const boost::shared_ptr<IProperty>& ip, const std::string& key)
{
	assert (isDict (ip));
	if (!isDict(ip))
	{
		assert (!"Ip is not a dictionary.");
		throw ElementBadTypeException ("getTypeFromDictionary()");
	}

	// Cast it to dict
	boost::shared_ptr<CDict> dict = IProperty::getSmartCObjectPtr<CDict> (ip);

	return getTypeFromDictionary<ItemType, ItemPType> (dict, key);
}

/** 
 * Get stream from dictionary. If it is CRef fetch the object pointed at.
 */
template<typename IP>
inline boost::shared_ptr<CStream>
getCStreamFromDict (IP& ip, const std::string& key)
	{return getTypeFromDictionary<CStream,pStream> (ip, key);}

/** 
 * Get dictionary from dictionary. If it is an indirect object, fetch the object.
 */
template<typename IP>
inline boost::shared_ptr<CDict>
getCDictFromDict (IP& ip, const std::string& key)
	{return getTypeFromDictionary<CDict,pDict> (ip, key);}

/** 
 * Get array from dictionary. If it is an indirect object, fetch the object.
 */
template<typename IP>
inline boost::shared_ptr<CArray>
getCArrayFromDict (IP& ip, const std::string& key)
	{return getTypeFromDictionary<CArray,pArray> (ip, key);}


/** 
 * Get dictionary from dictionary. If it is an indirect object, fetch the object.
 */
inline boost::shared_ptr<CDict>
getCDictFromDict (boost::shared_ptr<CDict> dict, const std::string& key)
	{return getTypeFromDictionary<CDict,pDict> (dict, key);}


//=========================================================
//	CArray "get value" helper methods
//=========================================================

/**
 * Get simple value from array.
 *
 * \todo Use MPL because ItemType and ItemPType depend on each other.!!
 *
 * @param array	Array.
 * @param id 	Position in the array.
 */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromArray (const boost::shared_ptr<CArray>& array, size_t position)
{
	utilsPrintDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array->getProperty (position);
	// Check the type and get the value
	return getValueFromSimple<ItemType, ItemPType, SimpleValueType> (ip);
}

/** \copydoc getSimpleValueFromArray */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromArray (const boost::shared_ptr<IProperty>& ip, size_t position)
{
	assert (isArray (ip));
	if (!isArray (ip))
	{
		assert (!"Ip is not an array.");
		throw ElementBadTypeException ("getSimpleValueFromArray()");
	}

	// Cast it to array
	boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (ip);
	// Get value from array
	return getSimpleValueFromArray<SimpleValueType, ItemType, ItemPType> (array, position);
}

/** Get int from array. */
template<typename IP>
inline int
getIntFromArray (const IP& ip, size_t position)
	{ return getSimpleValueFromArray<int, CInt, pInt> (ip, position);}

/** Get	double from array. */
template<typename IP>
inline double
getDoubleFromArray (const IP& ip, size_t position)
{ 
	// Try getting int, if not successful try double
	try {
		
		return getIntFromArray (ip, position);
		
	}catch (ElementBadTypeException&) {}

	return getSimpleValueFromArray<double, CReal, pReal> (ip, position);
}


//=========================================================
//	CArray "set value" helper methods
//=========================================================


/** 
 * Set simple value in array. 
 * If it is a reference, set fetch it and set it to the fetched object.
 *
 * @param ip Array property.
 * @param position Position in the array.
 * @param val Value to be written.
 */
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInArray (const CArray& array, size_t position, const Value& val)
{
	utilsPrintDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array.getProperty (position);
	// If it is a ref DELEGATE it
	ip = getReferencedObject (ip);
	
	// Cast it to the correct type and set value
	setValueToSimple<ItemType, ItemPType, Value> (ip, val);
}

/** \copydoc setSimpleValueInArray */
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInArray (const IProperty& ip, size_t position, const Value& val)
{
	assert (isArray (ip));
	if (!isArray (ip))
	{
		assert (!"Ip is not an array.");
		throw ElementBadTypeException ("");
	}

	// Cast it to array
	const CArray* array = dynamic_cast<const CArray*> (&ip);
	
	setSimpleValueInArray<Value, ItemType, ItemPType> (*array, position, val);
}


/** Set int in array. */
template<typename IP>
inline void
setIntInArray (const IP& ip, size_t position, int val)
	{ setSimpleValueInArray<int, CInt, pInt> (ip, position, val);}

/** Set	double in array. */
template<typename IP>
inline void
setDoubleInArray (const IP& ip, size_t position, double val)
{ 
	// Try setting double, if not successful try int
	try {
		
		return setSimpleValueInArray<double, CReal, pReal> (ip, position, val);
		
	}catch (ElementBadTypeException&) {}

	setIntInArray (ip, position, static_cast<int>(val));
}


//=========================================================
//	CArray "get type" helper methods
//=========================================================

/**
 * Get iproperty casted to specific type from array.
 *
 * @param array Array.
 * @param id   Position in the array.
 */
template<typename ItemType, PropertyType ItemPType>
inline boost::shared_ptr<ItemType>
getTypeFromArray (const boost::shared_ptr<CArray>& array, size_t pos)
{
	//utilsPrintDbg (debug::DBG_DBG, "array[" << pos << "]");
	
	// Get the item that is associated with specified key 
	boost::shared_ptr<IProperty> ip = array->getProperty (pos);
	//
	// If it is a Ref forward it to the real object
	// 
	ip = getReferencedObject (ip);

	// Check the type
	if (ItemPType != ip->getType ())
	{
		utilsPrintDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
		throw ElementBadTypeException ("getTypeFromArray()");
	}

	// Cast it to the correct type and return it
	return IProperty::getSmartCObjectPtr<ItemType> (ip);
}

/** \copydoc getTypeFromArray */
template<typename ItemType, PropertyType ItemPType>
inline boost::shared_ptr<ItemType>
getTypeFromArray (const boost::shared_ptr<IProperty>& ip, size_t pos)
{
	assert (isArray (ip));
	if (!isArray (ip))
	{
		assert (!"Ip is not an array.");
		throw ElementBadTypeException ("getTypeFromArray()");
	}

	// Cast it to array
	boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (ip);

	return getTypeFromArray<ItemType, ItemPType> (array, pos);
}

/** 
 * Get stream from array. If it is CRef fetch the object pointed at.
 */
template<typename IP>
inline boost::shared_ptr<CStream>
getCStreamFromArray (IP& ip, size_t pos)
	{return getTypeFromArray<CStream,pStream> (ip, pos);}


	
//=========================================================
// Mode helper functions
//=========================================================
	
/**
 * Find the mode of a property.
 *
 * @param
 * @param id Property number.
 * @param modecontroller Mode controller
 * 
 * @return Property mode.
 */
inline PropertyMode 
getModeForComplexObjects (const CArray::Value&, size_t id, const configuration::ModeController& modecontroller)
{ 
	std::ostringstream oss;
	oss << id;
	return modecontroller.getMode ("", oss.str());
}
	
/**
 * Find the mode of a property.
 *
 * @param dict CDict dictionary.
 * @param name Property name.
 * @param modecontroller Mode controller
 * 
 * @return Property mode.
 */
inline PropertyMode 
getModeForComplexObjects (const CDict::Value& dict, const std::string& name, const configuration::ModeController& modecontroller)
{ 
	DictIdxComparator cmp ("Type");
	CDict::Value::const_iterator it = dict.begin();
	for (; it != dict.end(); ++it)
	{
			if (cmp (*it))
					break;
	}
	if (it == dict.end())
	{ // No type found
		return modecontroller.getMode ("", name);
		
	}else	
	{ // We have found a type
		boost::shared_ptr<IProperty> ip = getReferencedObject (cmp.getIProperty());
		std::string tmp;
		ip->getStringRepresentation (tmp);
		return modecontroller.getMode (tmp, name);
	}
}



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

