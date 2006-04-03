// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _COBJECTHELPERS_H_
#define _COBJECTHELPERS_H_

/*
 * $RCSfile$
 *
 * $Log$
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

#include"cobjectI.h"

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
 * @return CDict instance wrapper ny shared_ptr smart pointer.
 */
boost::shared_ptr<CDict> getDictFromRef(boost::shared_ptr<IProperty> refProp);

/** Helper method for property printing.
 * @param ip Property to print.
 * @param out Output stream, where to print (implicit value is std::cout).
 *
 * Gets string representation of given property and dumps it to standard output.
 * TODO output stream as parameter.
 */
void printProperty(boost::shared_ptr<IProperty> ip, std::ostream out=std::cout);


//=========================================================
//	Array helper "get" methods
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
SimpleValueType
getSimpleValueFromArray (const boost::shared_ptr<CArray>& array, size_t position)
{
	printDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array->getProperty (position);
	if (ItemPType != ip->getType ())
	{
		printDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
		throw ElementBadTypeException ("");
	}

	// Cast it to the correct type and return it
	boost::shared_ptr<ItemType> item = IProperty::getSmartCObjectPtr<ItemType> (ip);
	SimpleValueType val = SimpleValueType ();
	item->getPropertyValue (val);
	return val;
}

template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
SimpleValueType
getSimpleValueFromArray (const boost::shared_ptr<IProperty>& ip, size_t position)
{
	assert (pArray == ip->getType ());
	if (pArray != ip->getType ())
		throw ElementBadTypeException ("");

	// Cast it to array
	boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (ip);

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
	double val = 0;
	try {
		
		return getIntFromArray (ip, position);
		
	}catch (ElementBadTypeException&) {}

	return getSimpleValueFromArray<double, CReal, pReal> (ip, position);
}


//=========================================================
//	Array helper "set" methods
//=========================================================


/** 
 * Set simple value in array. 
 *
 * @param ip Array property.
 * @param position Position in the array.
 * @param val Value to be written.
 */
template<typename Value, typename ItemType, PropertyType ItemPType>
void
setSimpleValueInArray (const boost::shared_ptr<CArray>& array, size_t position, Value val)
{
	printDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array->getProperty (position);
	if (ItemPType != ip->getType ())
	{
		printDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
		throw ElementBadTypeException ("");
	}

	// Cast it to the correct type and set value
	boost::shared_ptr<ItemType> item = IProperty::getSmartCObjectPtr<ItemType> (ip);
	item->writeValue (val);
}


template<typename Value, typename ItemType, PropertyType ItemPType>
void
setSimpleValueInArray (const boost::shared_ptr<IProperty>& ip, size_t position, Value val)
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


}// end of utils namespace

}// end of pdfobjects namespace
#endif

