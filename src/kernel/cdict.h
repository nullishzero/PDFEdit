// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/* 
 * =====================================================================================
 *        Filename:  cdict.h
 *     Description:  Header file containing definition of IProperty and CObject classes.
 *         Created:  01/18/2006 
 *          Author:  jmisutka (06/01/19), 
 *
 * =====================================================================================
 */
#ifndef _CDICT_H
#define _CDICT_H

// all basic includes
#include "static.h"
#include "iproperty.h"
#include "cobjectsimple.h"
#include "carray.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

		
//=====================================================================================
// CDict
//=====================================================================================

//class CStream;


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
class CDict : noncopyable, public IProperty
{
	friend class CStream;
	
public:
	typedef std::list<std::pair<std::string, boost::shared_ptr<IProperty> > > Value; 
	typedef const std::string& 			WriteType; 
	typedef const std::string& 			PropertyId;
	typedef observer::ComplexChangeContext<IProperty, PropertyId> CDictComplexObserverContext;

	/** Type of the property.
	 * This fields holds pDict value. It is used by template functions to get to
	 * property type from template type.
	 */
	static const PropertyType type=pDict;
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
	CDict (CPdf& p, Object& o, const IndiRef& rf);

	/**
	 * Constructor.
	 *
	 * @param o		Xpdf object. 
	 */
	CDict (Object& o);


public:	
	
	/** 
	 * Public constructor. This object will not be associated with a pdf.
	 */
	CDict () {};


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
	virtual CDict* _newInstance () const
		{ return new CDict; }
	
	//
	// Get methods
	//
public:

	/**
	 * Return type of this property.
	 *
	 * @return Type of this property.
	 */
	virtual PropertyType getType () const {return type;};
	

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
	void getAllPropertyNames (Container& container) const
	{
		for (Value::const_iterator it = value.begin();it != value.end(); ++it)
			container.push_back ((*it).first);
	}

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
	~CDict () {};
	

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
		Value::iterator it = value.begin ();
		for (; it != value.end (); ++it)
			fctor (*it);
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
	ObserverContext* _createContext (boost::shared_ptr<IProperty> changedIp, PropertyId id);

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
	void _getAllChildObjects (Storage& store) const
	{
		Value::const_iterator it = value.begin ();
		for	(; it != value.end (); ++it)
			store.push_back ((*it).second);
	}


};

//=====================================================================================
//	Find element functor
//=====================================================================================

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
		
	bool operator() (const CDict::Value::value_type& item)
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


//=========================================================
// To string functions
//=========================================================

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
getSimpleValueFromDict (const boost::shared_ptr<CDict>& dict, const std::string& id)__attribute__((deprecated));

// gcc doesn't like __attribute__ in function definition
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromDict (const boost::shared_ptr<CDict>& dict, const std::string& id)
{
	//utilsPrintDbg (debug::DBG_DBG, "dict[" << id << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = getReferencedObject (dict->getProperty (id));
	// Check the type and get the value
	return getValueFromSimple<ItemType> (ip);
}

/**
 * Get simple value from dict.
 *
 * Uses getReferencedObject to property with given id to get target property (if
 * it is reference) and getValueFromSimple with target property.
 *
 * @param dict	Dictionary.
 * @param id 	Position in the dictionary.
 */
template<typename ItemType>
inline typename ItemType::Value
getSimpleValueFromDict (const boost::shared_ptr<CDict>& dict, CDict::PropertyId id)
{
	utilsPrintDbg (debug::DBG_DBG, "dict[" << id << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = getReferencedObject (dict->getProperty (id));
	// Check the type and get the value
	return getValueFromSimple<ItemType> (ip);
}

/** \copydoc getSimpleValueFromDict */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromDict (const CDict& dict, const std::string& id)__attribute__((deprecated));

template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromDict (const CDict& dict, const std::string& id)
{
	//utilsPrintDbg (debug::DBG_DBG, "dict[" << id << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = getReferencedObject (dict.getProperty (id));
	// Check the type and get the value
	return getValueFromSimple<ItemType> (ip);
}

/** \copydoc getSimpleValueFromDict */
template<typename ItemType>
inline typename ItemType::Value
getSimpleValueFromDict (const CDict& dict, const std::string& id)
{
	utilsPrintDbg (debug::DBG_DBG, "dict[" << id << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = getReferencedObject (dict.getProperty (id));

	// Check the type and get the value
	return getValueFromSimple<ItemType> (ip);
}

/** \copydoc getSimpleValueFromDict */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromDict (const boost::shared_ptr<IProperty>& ip, const std::string& id)__attribute__((deprecated));
	
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
	return getSimpleValueFromDict<ItemType> (dict, id);
}

/** @copydoc getSimpleValueFromDict 
 */
template<typename ItemType>
inline typename ItemType::Value
getSimpleValueFromDict (const boost::shared_ptr<IProperty>& ip, const std::string& id)
{
	if (!isDict (ip))
		throw ElementBadTypeException ("getSimpleValueFromDict()");

	// Cast it to dict
	boost::shared_ptr<CDict> dict = IProperty::getSmartCObjectPtr<CDict> (ip);

	// Get value from dict
	return getSimpleValueFromDict<ItemType> (dict, id);
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
	{ return getSimpleValueFromDict<CString> (ip, id);}

/** Get name from dict. */
template<typename IP>
inline std::string
getNameFromDict (const IP& ip, const std::string& id)
	{ return getSimpleValueFromDict<CName> (ip, id);}

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
setSimpleValueInDict (const CDict& dict, const std::string& name, const Value& val)__attribute__((deprecated));

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

/** Sets value of property with given name.
 *
 * Gets property with given name and dereference (uses getReferencedObject) it 
 * (if it is reference) and sets its value (uses setValueToSimple). 
 *
 * @param dict Dictionary.
 * @param name Property name.
 * @param val Value for property.
 */
template<typename ItemType>
inline void
setSimpleValueInDict (const CDict& dict, CDict::PropertyId name, const typename ItemType::Value& val)
{
	utilsPrintDbg (debug::DBG_DBG, "dict[" << name << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = dict.getProperty (name);

	// If it is a ref DELEGATE it
	ip = getReferencedObject (ip);
	
	// Cast it to the correct type and set value
	setValueToSimple<ItemType> (ip, val);
}

/** \copydoc setSimpleValueInDict */
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInDict (const IProperty& ip, const std::string& name, const Value& val)__attribute__((deprecated));
	
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

/** @copydoc setSimpleValueInDict
 */
template<typename ItemType>
inline void
setSimpleValueInDict (const IProperty& ip, CDict::PropertyId name, const typename ItemType::Value& val)
{
	if (!isDict (ip))
		throw ElementBadTypeException ("");

	// Cast it to dict
	CDict* dict = dynamic_cast<const CDict*> (&ip);

	setSimpleValueInDict<ItemType> (*dict, name, val);
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
getTypeFromDictionary (const boost::shared_ptr<CDict>& dict, const std::string& key)__attribute__((deprecated));

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

/** Get property from dictionary.
 *
 * If property is reference, dereferences it (uses getReferencedObject
 * function).
 * @param dict Dictionary property.
 * @param key Property name.
 * @return Property with give type.
 * @throw ElementBadTypeException if found (dereferenced) property has 
 * different type than given.
 */
template<typename ItemType>
inline boost::shared_ptr<ItemType>
getTypeFromDictionary (const boost::shared_ptr<CDict>& dict, CDict::PropertyId key)
{
	utilsPrintDbg (debug::DBG_DBG, "dict[" << key << "]");
	
	// Get the item that is associated with specified key 
	boost::shared_ptr<IProperty> ip = dict->getProperty (key);

	//
	// If it is a Ref forward it to the real object
	// 
	ip = getReferencedObject (ip);

	// Check the type
	if (ItemType::type != ip->getType ())
	{
		utilsPrintDbg (debug::DBG_DBG, "wanted type " << ItemType::type << " got " << ip->getType () << " key[" << key << "]");
		std::string err= "getTypeFromDictionary() [" + key + "]";
		throw ElementBadTypeException (err);
	}

	// Cast it to the correct type and return it
	return IProperty::getSmartCObjectPtr<ItemType> (ip);
}

/** \copydoc getTypeFromDictionary */
template<typename ItemType, PropertyType ItemPType>
inline boost::shared_ptr<ItemType>
getTypeFromDictionary (const boost::shared_ptr<IProperty>& ip, const std::string& key)__attribute__((deprecated));

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

/** Gets property from dictionary.
 *
 * Checks type of given ip. If it is CDict instance, casts it and calls
 * getTypeFromDictionary with CDict parameter.
 *
 * @param ip Dictionary property.
 * @param key Property name.
 * @return Property with given key and ItemType.
 * @throw ElementBadTypeException if given ip is not CDict instance or target
 * property has not ItemType.
 *
 */
template<typename ItemType>
inline boost::shared_ptr<ItemType>
getTypeFromDictionary (const boost::shared_ptr<IProperty>& ip, CDict::PropertyId key)
{
	if (!isDict(ip))
		throw ElementBadTypeException ("getTypeFromDictionary()");

	// Cast it to dict
	boost::shared_ptr<CDict> dict = IProperty::getSmartCObjectPtr<CDict> (ip);

	return getTypeFromDictionary<ItemType> (dict, key);
}

/** 
 * Get dictionary from dictionary. If it is an indirect object, fetch the object.
 */
template<typename IP>
inline boost::shared_ptr<CDict>
getCDictFromDict (IP& ip, const std::string& key)
	{return getTypeFromDictionary<CDict> (ip, key);}

/** 
 * Get array from dictionary. If it is an indirect object, fetch the object.
 */
template<typename IP>
inline boost::shared_ptr<CArray>
getCArrayFromDict (IP& ip, const std::string& key)
	{return getTypeFromDictionary<CArray> (ip, key);}


/** 
 * Get dictionary from dictionary. If it is an indirect object, fetch the object.
 */
inline boost::shared_ptr<CDict>
getCDictFromDict (boost::shared_ptr<CDict> dict, const std::string& key)
	{return getTypeFromDictionary<CDict> (dict, key);}


//=====================================================================================
} /* namespace utils */
//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================


#endif // _CDICT_H
