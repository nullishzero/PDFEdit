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
class CObjectSimple : noncopyable, public IProperty
{
public:
	typedef typename PropertyTraitSimple<Tp>::writeType	 WriteType;
	typedef typename PropertyTraitSimple<Tp>::value 	 Value;

private:
	/** Object's value. */
	Value value;
	

	//
	// Constructors
	//
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
	 * REMARK: pRef DOES NOT copy the referenced object.
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
	 * Returns string representation of (x)pdf object. 
	 * 
	 * @param str 	After successful call, it will hold the string representation 
	 * 				of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;
	
	/**
	 * Return property value. Each property has its own return type.
	 *
	 * @param val Out parameter where property value will be stored.
	 */
	void getPropertyValue (Value& val) const;

	//
	// Set methods
	//
public:
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
	 * Make xpdf Object from this object. This function allocates xpdf object, caller has to free it.
	 * You have to call free() function on the Object to deallocate its resources.
	 *
	 * <exception cref="ObjBadValueE" /> Thrown when xpdf can't parse the string representation of this
	 * object correctly.
	 * 
	 * @return Xpdf object representing value of this simple object.
	 */
	virtual ::Object*	_makeXpdfObject () const;
	
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
	void _objectChanged (boost::shared_ptr<const ObserverContext> context)
	{
		kernelPrintDbg (debug::DBG_DBG, "CObjectSimple");
		// Do not notify anything if we are not in a valid pdf
		if (!isInValidPdf (this))
			return;
		assert (hasValidRef(this));
		
		// Dispatch the change
		this->dispatchChange ();
		
		if (context)
		{
			// Clone this new value
			boost::shared_ptr<IProperty> newValue (this->clone());
			// Fill them with correct values
			newValue->setPdf (this->getPdf());
			newValue->setIndiRef (this->getIndiRef());
			// Notify everybody about this change
			this->notifyObservers (newValue, context);
		}else
		{
			assert (!"Invalid context");
			throw CObjInvalidOperation ();
		}
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
 * The specific features are implemented using c++ feature called Incomplete Instantiation.
 * It means that, when it is not used, it is not instatiated, so e.g. CArray won't have
 * addProperty (IProperty& ,string&) method.
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
	 * Returns string representation of (x)pdf object. 
	 * 
	 * @param str 	After successful call, it will hold string representation 
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
	 * Try to make an (x)pdf object from string.
	 * <exception cref="..." />
	 *
	 * @param str0 Object in a text form.
	 */
	void setStringRepresentation (const std::string& strO) {assert (!"is this function really needed???");}

	
	/**
	 * Change the value of an object. The variable type depends
	 * on CObject type. 
	 * For complex types, it is equal to setStringRepresentation().
	 * 
	 * <exception cref="ObjBadValueE "/> Thrown When a value cannot be set due to bad value e.g. in complex types.
	 *
	 * @param val	Value that will be set.
	 */
	void writeValue (WriteType val) {assert (!"is this function really needed???");}
 
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
	 * Make xpdf Object from this object.
	 * You have to call free() function on the Object to deallocate its resources.
	 *
	 * <exception cref="ObjBadValueE" /> Thrown when xpdf can't parse the string representation of this
	 * object correctly.

	 * @return Xpdf object representing value of this simple object.
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
		// Do not notify anything if we are not in a valid pdf
		if (!isInValidPdf (this))
			return;
		assert (hasValidRef (this));

		// Dispatch the change
		this->dispatchChange ();
		
		if (context)
		{
			// Notify everybody about this change
			this->notifyObservers (newValue, context);

		}else
		{
			assert (!"Invalid context");
			throw CObjInvalidOperation ();
		}
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
	typedef std::vector<filters::StreamChar> Buffer;

//\debug \TODO remove protected:
	
	/** Object dictionary. */
	CDict dictionary;
	
	/** Buffer. */
	Buffer buffer;

	/** Parser. */
	Parser* parser;
	/** Next object in opened stream. */
	mutable ::Object curObj;
		

	//
	// Constructors
	//
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
	// Dictionary methods, delegated to CDict
	//
public:
	//
	//
	//
	size_t getPropertyCount () const
		{return dictionary.getPropertyCount ();}
	//
	//
	//
	template<typename Container> 
	void getAllPropertyNames (Container& container) const
		{ dictionary.getAllPropertyNames (container); }
	//
	//
	//
	boost::shared_ptr<IProperty> getProperty (PropertyId id) const
		{return dictionary.getProperty (id);}
	//
	//
	//
	PropertyType getPropertyType (PropertyId id) const
		{return dictionary.getPropertyType (id);}
	//
	//
	//
	boost::shared_ptr<IProperty> setProperty (PropertyId id, IProperty& ip)
		{return dictionary.setProperty (id, ip);}
	//
	//
	//
	boost::shared_ptr<IProperty> addProperty (PropertyId id, const IProperty& newIp)
		{return dictionary.addProperty (id, newIp);}
	//
	//
	//
	void delProperty (PropertyId id)
		{dictionary.delProperty (id);}

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
	 * @param str String representation.
	 */
	virtual void getStringRepresentation (std::string& str) const
		{ getStringRepresentation (str, false); }

	/**
	 * Returns printable string representation of this object.
	 *
	 * @param str String representation.
	 */
	virtual void getPritnableStringRepresentation (std::string& str) const
		{ getStringRepresentation (str, true); }

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
	void getFilters (Container& container) const
	{
		boost::shared_ptr<IProperty> ip;
		//
		// Get optional value Filter
		//
		try	
		{
			ip = dictionary.getProperty ("Filter");
			
		}catch (ElementNotFoundException&)
		{
			// No filter found
			kernelPrintDbg (debug::DBG_DBG, "No filter found.");
			return;
		}
	
		//
		// If it is a name just store it
		// 
		if (isName (ip))
		{
			std::string fltr;
			boost::shared_ptr<const CName> name = IProperty::getSmartCObjectPtr<CName>(ip);
				
			name->getPropertyValue (fltr);
			container.push_back (fltr);
			
			kernelPrintDbg (debug::DBG_DBG, "Filter name:" << fltr);
		//
		// If it is an array, iterate through its properties
		//
		}else if (isArray (ip))
		{
			boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray>(ip);
			if (array)
			{
				CArray::Value::iterator it = array->value.begin ();
				for (; it != array->value.end(); ++it)
				{
					if (isName (*it))
					{
						boost::shared_ptr<CName> name = IProperty::getSmartCObjectPtr<CName>(*it);
						std::string fltr;
						name->getPropertyValue (fltr);
						container.push_back (fltr);
					
						kernelPrintDbg (debug::DBG_DBG, "Filter name:" << fltr);

					}else
					{
						assert (!"One of the filters is not a name.");
						throw CObjInvalidObject ();
					}
				
				} // for (; it != array->value.end(); ++it)
				
			}else // if (array)
				throw CObjInvalidObject ();
		}
	}


	//
	// Set methods
	//
public:
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
	void setBuffer (const Buffer& buf);

	//
	// Parsing
	//
public:
	
	/**
	 * Initialize parsing mechanism.
	 *
	 * REMARK: if CObject is not in a pdf, we MUST be sure that it does not
	 * use indirect objects.
	 */
	void open ();

	/**
	 * Close parsing.
	 */
	void close ();
	
	/**
	 * Get xpdf object. It also frees obj passes as argument.
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
	 * Is end of stream.
	 */
	bool eof () const;
	
	/**
	 * Is stream opened.
	 *
	 * @return True if the stream has been opened, false otherwise.
	 */
	bool is_open () const {return (NULL != parser);};

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
     * Create xpdf object.
	 *
	 * REMARK: Caller is responsible for deallocating the xpdf object.
     *
     * @return Xpdf object(s).
     */
    virtual Object* _makeXpdfObject () const; 


private:
	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<const ObserverContext> context)
	{
		// Do not notify anything if we are not in a valid pdf
		if (!isInValidPdf (this))
			return;
		assert (hasValidRef (this));

		// Set correct length
		if (getLength() != buffer.size())
			setLength (buffer.size());
		
		// Dispatch the change
		this->dispatchChange ();
		
		if (context)
		{
			// Clone this new value
			boost::shared_ptr<IProperty> newValue (this->clone());
			// Fill them with correct values
			newValue->setPdf (this->getPdf());
			newValue->setIndiRef (this->getIndiRef());
			// Notify everybody about this change
			this->notifyObservers (newValue, context);

		}else
		{
			assert (!"Invalid context");
			throw CObjInvalidOperation ();
		}
	}

	/**
	 * Get encoded string representation.
	 *
	 * @param container Output container.
	 */
	void encodeBuffer (const Buffer& container);

	/**
	 * Returns representation of this object.
	 *
	 * @param str String representation.
	 * @param wantraw True if we want raw content, false if we want only
	 * printable characters.
	 */
	virtual void getStringRepresentation (std::string& str, bool wantraw) const;

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

protected:
	/**
	 * Lock dictionary. It will not dispatch any changes.
	 */
	void lockDictionary ()
	{ 
		kernelPrintDbg (debug::DBG_DBG, ""); 
		dictionary.setPdf (NULL); 
		dictionary.setIndiRef (IndiRef());
	}

	/**
	 * Unlock dictionary. It will dispatch all changes.
	 */
	void unlockDictionary ()
	{ 
		assert (!isInValidPdf(&dictionary));
		kernelPrintDbg (debug::DBG_DBG, ""); 
		dictionary.setPdf (this->getPdf()); 
		dictionary.setIndiRef (this->getIndiRef());
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

//
// Creation functions
//

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
template <PropertyType Tp,typename T> void complexValueFromXpdfObj (IProperty& ip, ::Object& obj, T val);

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


//
// To string functions
// 
		
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
 * CStream object to string
 *
 * @param strDict Dictionary string representation.
 * @param buf Buffer string representation
 * @param str Output string.
 */
void streamToString (const std::string& strDict, const std::string& buf, std::string& str);

/**
 * Convert xpdf object to string
 *
 * @param obj Xpdf object that will be converted.
 * @param str This will hold the string representation of the object.
 */
void xpdfObjToString (Object& obj, std::string& str);

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
void parseStreamToContainer (CStream::Buffer& container, ::Object& obj);


//
// Get functions
//

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
inline boost::shared_ptr<IProperty>  
getIPropertyFromItem (const PropertyTraitComplex<pDict>::value::value_type& item) {return item.second;}


//
// Other funcions
//

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
 * <cref exception="ObjInvalidOperation"> Thrown when the object does not belong
 * to a valid pdf.
 *
 * @param ip	IProperty of object. 
 * @param indiObj Out parameter wheren indirect object will be saved, because
 * finding the object can be an expensive operation.
 */
bool objHasParent (const IProperty& ip);
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
		Value val = Value ();
		item->getPropertyValue (val);
		return val;

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
	item->writeValue (val);
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
//
// If we got iproperty, cast it to Dict
//
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
//	CArray "get value" helper methods
//=========================================================

/**
 * Get simple value from array.
 *
 * \TODO Use MPL because ItemType and ItemPType depend on each other.!!
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
	utilsPrintDbg (debug::DBG_DBG, "array[" << pos << "]");
	
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
//
// If we got iproperty, cast it to Dict
//
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

