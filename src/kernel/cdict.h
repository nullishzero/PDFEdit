/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
#ifndef _CDICT_H
#define _CDICT_H

// all basic includes
#include "kernel/static.h"
#include "kernel/iproperty.h"
#include "kernel/cobjectsimple.h"
#include "kernel/carray.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

		
//=====================================================================================
// CDict
//=====================================================================================

/** 
 * Class representing dictionary object from pdf specification v1.5.
 *
 * We do not want to use xpdf dictionary because it is a real mess and it is really
 * not suitable for editing. We use xpdf object just for initializing.
 * 
 * According to pdf specification, dictionary is a container of keys and values.
 * Keys are strings identifying items. You can not rely on the position of a
 * value.
 * Value can be any object from pdf specification. Pdf objects form a tree like
 * structure.
 * 
 * This class does not provide default copy constructor because copying a
 * property could be understood either as deep copy or shallow copy. 
 * Copying complex types could be very expensive so we have made the decision to
 * avoid it.
 *
 * REMARK: It is similar to CArray but it has also too much differences to be
 * cleanly implemented as one template class. (It has been implemented like one
 * template class but later was seperated to CArray and CDict)

 * \see CObjectSimple, CArray, CStream
 */
class CDict : noncopyable, public IProperty
{
	friend class CStream;
	
public:
	typedef std::list<std::pair<std::string, boost::shared_ptr<IProperty> > > Value; 
	typedef const std::string& WriteType; 
	typedef const std::string& PropertyId;
	typedef observer::ComplexChangeContext<IProperty, PropertyId> CDictComplexObserverContext;

	/** 
	 * Type of this property object.
	 * This association allows us to get the PropertyType from object type.
	 */
	static const PropertyType type = pDict;
private:
	
	/** Dictionary representation. */
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
	CDict (boost::weak_ptr<CPdf> p, const Object& o, const IndiRef& rf);

	/**
	 * Constructor.
	 *
	 * @param o		Xpdf object. 
	 */
	CDict (const Object& o);


public:	
	
	/** 
	 * Public constructor. This object will not be associated with a pdf.
	 */
	CDict () {}


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
	{ 
		return new CDict; 
	}
	
	//
	// Get methods
	//
public:

	/**
	 * Return type of this property.
	 *
	 * @return Type of this property.
	 */
	virtual PropertyType getType () const {return type;}
	

	/**
	 * Returns string representation of this object according to pdf
	 * specification. 
	 * 
	 * @param str 	Output string that will hold the string representation 
	 * 				of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;


	/**
	 * Simple shallow copy constructor (does not copy pointers).
	 *
	 * @param dict dictionary which will be copied
	 */
	void init (const CDict& dict);

	/** 
	 * Returns property count.
	 * 
	 * @return Property count.
	 */
	size_t getPropertyCount () const 
	{
		//kernelPrintDbg (debug::DBG_DBG, "getPropertyCount(" << debug::getStringType<Tp>() << ") = " << value.size());
		return value.size();
	}
 

	/**
	 * Inserts all property names of this complex type to the container supplied by caller. 
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
	 * Returns value of property identified by its name.
   	 *
   	 * @param 	id 	Name of the property.
	 * @return	Output variable where the value will be stored.
   	 */
	boost::shared_ptr<IProperty> getProperty (PropertyId id) const;

	/** 
	 * Returns property identified by its name.
	 * This is a convenient method which also does the casting trickery
	 * and indirect object resolution if the given template type is not
	 * reference itself.
	 *
   	 * @param 	id 	Name of the property.
	 * @return	Output variable where the value will be stored.
	 */
	template<typename ItemType> boost::shared_ptr<ItemType> getProperty(PropertyId id)const {
		boost::shared_ptr<IProperty> ip = getProperty(id);

		if (ItemType::type != pRef && isRef(ip))
			ip = utils::getReferencedObject(ip);

		if(ItemType::type != ip->getType()) {
			kernelPrintDbg (debug::DBG_DBG, "wanted type " << ItemType::type 
					<< " got " << ip->getType () << " key[" << id << "]");
			throw ElementBadTypeException(id);
		}

		return IProperty::getSmartCObjectPtr<ItemType>(ip);
	}

	/** 
	 * Returns property identified by its name.
	 * This is a convenient method which also does the casting trickery
	 * and indirect object resolution if the given template type is not
	 * reference itself.
	 *
   	 * @param 	id 	Name of the property.
	 * @return	Output variable where the value will be stored.
	 */
	template<typename ItemType> boost::shared_ptr<ItemType> getProperty(const char *name)const {
		assert(name);
		PropertyId n(name);
		return getProperty<ItemType>(n);
	}

	/**
	 * Returns property type of an item identified by name.
	 *
	 * @param id Name of the property.
	 * @return Property type.	
	 *
	 * \exception ObjInvalidPositionInComplex When the id does not correctly identify an item.
	 *
	 */
	PropertyType getPropertyType (PropertyId id) const 
		{ return getProperty(id)->getType(); }

	
	//
	// Set methods
	//
public:
	/**
	 * Set pdf to this object and its children.
	 * @param pdf New pdf.
	 */
	virtual void setPdf (boost::weak_ptr<CPdf> pdf);

	/**
	 * Set ref to this object and its children.
	 * @param rf New indirect reference number.
	 */
	virtual void setIndiRef (const IndiRef& rf);

	/**
	 * Sets property type of an item.
	 * 
	 * Firstly, the property that is passed as argument is cloned and
	 * the cloned object replaces object specified by id. If the item does not
	 * exist it is added.
	 * 
	 * @param	id	Name/Index of property
	 * @param	ip	Value, for simple types (int,string,...) and for complex types IProperty*
	 * @return Pointer to the new property.
	 */
	boost::shared_ptr<IProperty> setProperty (PropertyId id, IProperty& ip);
	
	/**
	 * Add property to dictionary. 
	 *
	 * Firstly, the property that is passed as argument is cloned and 
	 * the cloned object is added.
	 * Indicate that this object has changed and return the pointer to the cloned object.
	 *
	 * If a property with the same name is already in this dictionary an exception is thrown.
	 *
	 * @param propertyName property name
	 * @param newIp new property
	 * @return Pointer to the new property.
	 *
	 * \exception OutOfRange Thrown when property not found.
	 * \exception CObjInvalid When the object is invalid
	 */
	boost::shared_ptr<IProperty> addProperty (const std::string& propertyName, const IProperty& newIp);

	/**
	 * Remove property from dictionary. 
	 *
	 * @param id Name/Index of property
	 *
	 * \exception ElementNotFoundException Thrown when object is not found.
	 */
	void delProperty (PropertyId id);


	//
	// Destructor
	//
public:
	/**
	 * Destructor
	 */
	~CDict () {}
	

	//
	// Helper methods
	//
public:

	/**
	 * Apply functor operator() on each element.
	 * The operator() will get std::pair<string, shared_ptr<IProperty>> as
	 * parameter. First item identifies the property.
	 * 
	 * @param fctor Functor that will do the work.
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
	 * Caller has to deallocate the xpdf Object.
	 *
	 * @return Xpdf object representing this object.
	 *
	 * \exception ObjBadValueE Thrown when xpdf can't parse the string representation of this
	 * object correctly.

	 */
	virtual ::Object* _makeXpdfObject () const;

private:
	/**
	 * Create context of a change.
	 *
	 * REMARK: Be carefull. Deallocate the object.
	 * 
	 * @param changedIp Pointer to old value.
	 * @param id		Id identifies changed property.
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
	 * @param id Key identifying property.
	 */
	void _setMode (boost::shared_ptr<IProperty> ip, PropertyId id) const;

public:
	/**
	 * Return all child objects.
	 *
	 * @param store Output container of all child objects.
	 */
	template <typename Storage>
	void _getAllChildObjects (Storage& store) const
	{
		Value::const_iterator it = value.begin ();
		for(; it != value.end (); ++it)
			store.push_back ((*it).second);
	}


};

//=====================================================================================
//	Find element functor
//=====================================================================================

/**
 * This functor is used as a parameter to a function equal to std::find_if algorithm.
 * 
 * Finds out an item specified by its name. std::find_if() CANNOT be used, because in c++ specification two 
 * important requirements are missing.
 * <ul>
 * <li> ordering - order of item traversal </li>
 * <li> using supplied instance - it is specified that find_if can not make copies of supplied functor </li>
 * </ul>
 * This functor relies on the first one.
 *
 * REMARK: More effective algorithms could be used but this approach is 
 * more generic.
 */
class DictIdxComparator : noncopyable
{
private:
	const std::string str;
	boost::shared_ptr<IProperty> ip;

public:
	DictIdxComparator (const std::string& s) : str(s) {}
		
	boost::shared_ptr<IProperty> getIProperty () {return ip;}
		
	bool operator() (const CDict::Value::value_type& item)
	{	
		if (item.first == str)
		{
			ip = item.second;
			return true;
		}
			
		return false;
	}
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
template <PropertyType Tp,typename T> void complexValueFromXpdfObj (IProperty& ip, const ::Object& obj, T val);

/**
 * This function is a slower equivalent to complexValueFromXpdfObj. But on the
 * other hand, complexValueFromXpdfObj() needs inner dictionary container that we
 * do not have always access to.
 *
 * @param resultDict Dictionary.
 * @param dict Xpdf object from which we init dictionary.
 */
void dictFromXpdfObj (CDict& resultDict, ::Object& dict);


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

/** Helper function which returns data from given stream.
 * @param str Stream to read (until it returns EOF).
 * @param dictLength Initial size reported by the stream dictionary.
 * @param size Output size of the buffer.
 *
 * This is low-level function which simply reads given stream from its 
 * beginning until EOF. It can be used to retrieve data from whatever
 * Stream filter stack layer. E.g. if we want to get encoded data we
 * can simply give stream->getBaseStream() as parameter and this function
 * will return buffer as it is written in the file.
 *
 * @return Buffer with stream data (size will contain number of bytes
 * stored) or NULL on error.
 */
unsigned char* bufferFromStream(Stream& str, size_t dictLength, size_t& size);

/** Helper function for removing filters from the given stream object.
 * @param obj Stream object.
 * @param size Size of returned buffer.
 * @return Buffer with raw stream data or NULL on error.
 *
 * If the given obj used some filters, they are all decoded to get orignal
 * data and the stream dictionary is updated so that it doesn't contain
 * any filters. 
 * <br>
 * Stream object stored in the given obj is not touched (because it doesn't
 * give much sense - it returns decoded data anyway, it doesn't use dictionary
 * to find out how to decode). Nevertheless this operation may be considered 
 * harmfull for later usage of given the object in the xpdf code paths!
 */
unsigned char* convertStreamToDecodedData (const Object& obj, size_t& size);

/** Function to be used for data extracting from the given stream object.
 * Note that implementation can apply additional filters to the stream
 * data currently stored in the obj.stream and update object accordingly
 * (change Filter and associated entry). Length entry doesn't have to 
 * be changed by the implementation as it is supposed to be updated by 
 * caller (because we want to prevent useless object updating when a
 * chain of this function is called for multiple filters).
 * <br>
 * Function is called from the streamToCharBuffer function to have transparent
 * access to the stream data without any knowledge about filters.
 * bufferFromStreamData is the referencial implementation which provides
 * encoded data without any filters applied (all other implementations
 * can use this function as the base and convert returned buffer into
 * their represenation).
 *
 * @param Stream object.
 * @param size Number of bytes written to the returned buffer.
 * @return Buffer (size bytes) with data (must be deallocated by caller) or 
 * NULL on error.
 */
typedef unsigned char* (*stream_data_extractor)(const Object& obj, size_t& size);

/** Makes a valid pdf indirect object representation of stream object.
 * @param streamObject Xpdf object representing stream.
 * @param ref Reference for this indirect object.
 * @param outputBuf Output byte buffer containing complete representation.
 * @param extractor Function to be used to extract data from the object's 
 * 	stream.
 *
 * Allocates and fills buffer in given outputBuf with pdf object format
 * representation of given stream object. Moreover adds indirect header and
 * footer if ref is non NULL. 
 * <br>
 * If you want some filters to be used to encode stream data, use extractor
 * function parameter for this purpose. bufferFromStreamData used by default
 * returns stream data without any filters applied.
 * <br>
 * Given buffer may contain NUL bytes inside. Caller should consume number of
 * returned bytes from outputBuf.
 * 
 * @return number of bytes used in outputBuf or 0 if problem occures.
 */
size_t streamToCharBuffer (const Object & streamObject, Ref* ref, CharBuffer & outputBuf, 
		stream_data_extractor extractor);
	
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
void getStringFromXpdfStream (std::string& str, ::Object& obj);


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
 * Get simple value from dictionary.
 *
 * \todo Can use MPL because ItemType and ItemPType depend on each other.!!
 *
 * @param dict	Dictionary.
 * @param id 	Position in the dictionary.
 */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromDict (const boost::shared_ptr<CDict>& dict, const std::string& id) DONOTUSE;

/** \copydoc getSimpleValueFromDict */
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
 * Get simple value from dictionary.
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
getSimpleValueFromDict (const CDict& dict, const std::string& id) DONOTUSE;

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
getSimpleValueFromDict (const boost::shared_ptr<IProperty>& dict, const std::string& id) DONOTUSE;
	
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

/** Get int from dictionary. */
template<typename IP>
inline int
getIntFromDict (const IP& ip, const std::string& id)
	{ return getSimpleValueFromDict<int, CInt, pInt> (ip, id);}

/** Get	double from dictionary. */
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

/** Get string from dictionary. */
template<typename IP>
inline std::string
getStringFromDict (const IP& ip, const std::string& id)
{ 
	return getSimpleValueFromDict<CString> (ip, id);
}

/** Get name from dictionary. */
template<typename IP>
inline std::string
getNameFromDict (const IP& ip, const std::string& id)
{ 
	return getSimpleValueFromDict<CName> (ip, id);
}

//=========================================================
//	CDict "set value" helper methods
//=========================================================


/** 
 * Set simple value in dictionary. 
 * If it is a reference, set fetch it and set it to the fetched object.
 *
 * @param dict Dict property.
 * @param name Name of property.
 * @param val Value to be written.
 */
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInDict (const CDict& dict, const std::string& name, const Value& val) DONOTUSE;

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
setSimpleValueInDict (const IProperty& dict, const std::string& name, const Value& val) DONOTUSE;
	
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

/** \copydoc setSimpleValueInDict */
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

/** Set int in dictionary. */
template<typename IP>
inline void
setIntInDict (const IP& ip, const std::string& name, int val)
{ 
	setSimpleValueInDict<int, CInt, pInt> (ip, name, val);
}

/** Set	double in dictioary. */
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

	return dict->getProperty<ItemType>(key);
}

//=====================================================================================
} /* namespace utils */
//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================


#endif // _CDICT_H
