// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _COBJECTHELPERS_H_
#define _COBJECTHELPERS_H_

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.13  2006/04/19 18:40:23  hockm0bm
 * * getPropertyId method implemented
 *         - compilation problems in linking stage (probably because of bad getAllPropertyNames implementation)
 * * operatator== for properties
 * 	- supports only pRef comparing now
 * * simpleEquals helper method for simple value comparing
 *
 * Revision 1.12  2006/04/13 18:14:09  hockm0bm
 * getDictFromRef with reference and pdf parameters
 *
 * Revision 1.11  2006/04/12 21:08:13  hockm0bm
 * printProperty method signature changed
 *         - no implicit parameter
 *         - output stream parameter is reference now
 *
 * Revision 1.10  2006/04/10 23:04:51  misuj1am
 *
 *
 * -- ADD: setValueToSimple, improved the code
 *
 * Revision 1.9  2006/04/09 21:27:13  misuj1am
 *
 *
 * -- getTypeFromDictionary returns shared_ptr<Type> that we want (if possible)
 * 	-- if CRef then try to fetch the indirect object and return it
 *
 * Revision 1.8  2006/04/07 10:23:02  misuj1am
 *
 * -- new observers, they are NOT called if an object changes
 *
 * Revision 1.7  2006/04/03 14:35:13  misuj1am
 *
 *
 * -- ADD: set (int/.../) methods for array
 *
 * Revision 1.6  2006/04/02 17:12:14  misuj1am
 *
 *
 * -- ADD get*FromArray, generic function, avaliable specification for Int and double
 *
 * Revision 1.5  2006/04/02 08:21:03  hockm0bm
 * printProperty helper method signature changed
 *         - stream parameter is not reference
 *         - default parameter is std::cout
 * implementation moved to cc file
 *
 * Revision 1.4  2006/04/01 20:43:48  hockm0bm
 * new output stream parameter for printProperty
 *
 * Revision 1.3  2006/04/01 20:30:03  hockm0bm
 * printProperty helper method for property printing
 *
 * Revision 1.2  2006/03/29 06:13:43  hockm0bm
 * getDictFromRef helper method added
 *
 * Revision 1.1  2006/03/24 17:37:57  hockm0bm
 * new file for helper methods for cobjects
 * in this time only for simple methods from dictionary
 *
 *
 */

#include "cobjectI.h"
#include "iproperty.h"

namespace pdfobjects
{
namespace utils
{

/** Gets type of the dictionary.
 * @param dict Dictionary wrapped in smart pointer.
 *
 * Tries to get /Type field from dictionary and returns its string value. If not
 * present, returns an empty string.
 *
 * @return string name of the dictionary type or empty string if not able to
 * find out.
 */
std::string getDictType(boost::shared_ptr<CDict> dict);
        
/** Helper method for getting int property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pInt, gets its int value which is returned.
 *
 * @throw ElementNotFoundException if property is not found.
 * @throw ElementBadTypeException if property is found but doesn't contain 
 * integer value.
 * @return int value of the property.
 */
int getIntFromDict(std::string name, boost::shared_ptr<CDict> dict);

/** Helper method for getting reference property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pRef, gets its IndiRef value which is returned.
 *
 * @throw ElementNotFoundException if property is not found.
 * @throw ElementBadTypeException if property is found but doesn't contain 
 * reference value.
 * @return IndiRef value of the property.
 */
IndiRef getRefFromDict(std::string name, boost::shared_ptr<CDict> dict);

/** Helper method for getting string property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pString, gets its string value which is returned.
 *
 * @throw ElementNotFoundException if property is not found.
 * @throw ElementBadTypeException if property is found but doesn't contain 
 * string value.
 * @return std::string value of the property.
 */
std::string getStringFromDict(std::string name, boost::shared_ptr<CDict> dict);

/** Helper method for getting name property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pName, gets its string value which is returned.
 *
 * @throw ElementNotFoundException if property is not found.
 * @throw ElementBadTypeException if property is found but doesn't contain 
 * string value.
 * @return std::string value of the property.
 */
std::string getNameFromDict(std::string name, boost::shared_ptr<CDict> dict);

/** Gets dictionary from reference property.
 * @param refProp Reference property (myst be pRef typed).
 *
 * Gets reference value from property and dereferences indirect object from it.
 * Uses refProp's pdf for dereference.
 * Finaly casts (if indirect is dictionary) to CDict and returns.
 *
 * @throw ElementBadTypeException if refProp is not CRef instance or indirect
 * object is not CDict instance.
 * @return CDict instance wrapper by shared_ptr smart pointer.
 */
boost::shared_ptr<CDict> getDictFromRef(boost::shared_ptr<IProperty> refProp);

/** Gets dictionary from reference and pdf instance.
 * @param pdf Pdf istance.
 * @param ref Indirect reference.
 *
 * Uses CPdf::getIndirectProperty to get dereferenced object from given
 * reference on given pdf instance. Checks object type and if it is CDict,
 * returns it, otherwise throws an axception.
 *
 * @throw ElementBadTypeException if indirect object is not CDict instance.
 * @return CDict instance wrapper by shared_ptr smart pointer.
 */
boost::shared_ptr<CDict> getDictFromRef(CPdf & pdf, IndiRef ref);

/** Helper method for property printing.
 * @param ip Property to print.
 * @param out Output stream, where to print (implicit value is std::cout).
 *
 * Gets string representation of given property and dumps it to standard output.
 * TODO output stream as parameter.
 */
void printProperty(boost::shared_ptr<IProperty> ip, std::ostream &out);




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
		printDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
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
	return (pInt == ip->getType()) ? getValueFromSimple<CInt, pInt, int> (ip) :
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
		printDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
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
	printDbg (debug::DBG_DBG, "dict[" << key << "]");
	
	// Get the item that is associated with specified key 
	boost::shared_ptr<IProperty> ip = dict->getProperty (key);
	//
	// If it is a Ref forward it to the real object
	// 
	if (pRef == ip->getType())
	{
		IndiRef ref;
		IProperty::getSmartCObjectPtr<CRef>(ip)->getPropertyValue(ref);
		ip = dict->getPdf()->getIndirectProperty (ref);
	}

	// Check the type
	if (ItemPType != ip->getType ())
	{
		printDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
		throw ElementBadTypeException ("getValueFromDictionary()");
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
	assert (pDict == ip->getType ());
	if (pDict != ip->getType ())
		throw ElementBadTypeException ("getTypeFromDictionary()");

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
	printDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array->getProperty (position);
	// Check the type and get the value
	return getValueFromSimple<ItemType, ItemPType, SimpleValueType> (ip);
}

template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromArray (const boost::shared_ptr<IProperty>& ip, size_t position)
{
	assert (pArray == ip->getType ());
	if (pArray != ip->getType ())
		throw ElementBadTypeException ("getSimpleValueFromArray()");

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
 *
 * @param ip Array property.
 * @param position Position in the array.
 * @param val Value to be written.
 */
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInArray (const boost::shared_ptr<CArray>& array, size_t position, const Value& val)
{
	printDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array->getProperty (position);
	// Cast it to the correct type and set value
	setValueToSimple<ItemType, ItemPType, Value> (ip, val);
}


template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInArray (const boost::shared_ptr<IProperty>& ip, size_t position, const Value& val)
{
	assert (pArray == ip->getType ());
	if (pArray != ip->getType ())
		throw ElementBadTypeException ("");

	// Cast it to array
	boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (ip);
	
	setSimpleValueInArray<Value, ItemType, ItemPType> (array, position, val);
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

/** Equality operator for T type.
 * @param val1 Value to compare (with T type wrapped by smart poiter).
 * @param val2 Value to compare (with T type wrapped by smart poiter).
 *
 * Two values are equal iff:
 * <ul>
 * <li> have same types (getType returns same value)
 * <ul> They have same values
 * 	<li> if value is simple - direct values are same (getPropertyValue is used)
 * 	<li> if value is complex - same number of elements and elements with same
 * 	identifier are equal
 * </ul>
 * </ul>
 *
 * <b>Supported types</b><br>
 * <ul>
		<li> pNull
		<li> pBool
		<li> pInt
		<li> pReal
		<li> pString
		<li> pName
		<li> pRef
 * </ul>
 * @throw NotImplementedException if value type is not supported (see supported
 * types table).
 *
 * @return true if values are equal, false otherwise.
 */
bool operator==(const boost::shared_ptr<IProperty> val1, const boost::shared_ptr<IProperty> val2);

/** Gets all identificators of property in parent complex type.
 * @param parent Complex value where to search.
 * @param child Property to search.
 * @param container Container, where to place all identificators (array
 * indexes).
 *
 * Goes through whole array elements and compares child width all of them and
 * which matches (using operator== TODO link), their indexes are stored to
 * container (it has to provide push_back and clear methods for int type value).
 * <br>
 * Complex template parameter stands for type of CObjectComplex where to search.
 * This type has to provide typedef for propertyId and getAllPropertyNames,
 * getProperty methods.
 * <br>
 * Container template parameter stands for type of storage where to place found
 * indexes. This has to provide clear and push_back methods. Given container is
 * cleared at start.
 *
 */
template<typename Complex, typename Container > void 
getPropertyId(boost::shared_ptr<Complex> parent, boost::shared_ptr<IProperty> child, Container & container) throw()
{
using namespace std;
using namespace boost;
using namespace debug;

	printDbg(DBG_DBG, "");

	// type of the storage where to place indexes
	typedef vector<typename Complex::PropertyId> IdStorage;

	// collects all properties identificators
	IdStorage ids;
	parent->getAllPropertyNames(ids);

	// clears given container to prevent mixing with original data
	container.clear();
	
	// goes over all identificators and compare each with given child
	typename IdStorage::iterator i=ids.begin();
	for(; i!=ids.end(); i++)
	{
		shared_ptr<IProperty> element=parent->getProperty(*i);
		try
		{
			// uses == operator to compare properties
			// it may throw, if element is not supported by operator, we will
			// simply ignore such values
			if(operator==(element, child))
			{
				printDbg(DBG_DBG, "Element matches at id="<<*i);
				container.push_back(*i);
			}
		}catch(NotImplementedException & e)
		{
			// type of element is not supported
			printDbg(DBG_WARN, "Element comparing not supported for typ="<<element->getType()<<". Ignoring element.");
		}
	}
}

}// end of utils namespace

}// end of pdfobjects namespace
#endif

