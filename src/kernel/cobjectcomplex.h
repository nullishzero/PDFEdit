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
#ifndef _COBJECTCOMPLEX_H
#define _COBJECTCOMPLEX_H

// all basic includes
#include "static.h"
#include "iproperty.h"
#include "cobjectsimple.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

		
//
// Forward declarations of memory checkers
//
class NoMemChecker;
class BasicMemChecker;
class CXRef;

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

//
// Typedefs
//
typedef CObjectComplex<pArray>	CArray;
typedef CObjectComplex<pDict>	CDict;

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
// Creation functions
//=========================================================

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
		std::string tmp;
		boost::shared_ptr<IProperty> type = cmp.getIProperty ();
		if (isName (type))
			IProperty::getSmartCObjectPtr<CName>(type)->getValue(tmp);
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
#include "cobjectcomplexI.h"


#endif // _COBJECTCOMPLEX_H

