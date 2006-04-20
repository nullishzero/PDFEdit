// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.9  2006/04/20 18:02:59  hockm0bm
 * * operator== replaced by propertyEquals method
 * * parameters for getPropertyId and propertyEquals are const referencies
 * * getPropertyId - not useable now, because I don't have mechanism to get
 *         all available ids - but it is compileable
 *
 * Revision 1.8  2006/04/19 18:40:23  hockm0bm
 * * getPropertyId method implemented
 *         - compilation problems in linking stage (probably because of bad getAllPropertyNames implementation)
 * * operatator== for properties
 * 	- supports only pRef comparing now
 * * simpleEquals helper method for simple value comparing
 *
 * Revision 1.7  2006/04/13 18:14:09  hockm0bm
 * getDictFromRef with reference and pdf parameters
 *
 * Revision 1.6  2006/04/12 21:08:13  hockm0bm
 * printProperty method signature changed
 *         - no implicit parameter
 *         - output stream parameter is reference now
 *
 * Revision 1.5  2006/04/02 08:21:03  hockm0bm
 * printProperty helper method signature changed
 *         - stream parameter is not reference
 *         - default parameter is std::cout
 * implementation moved to cc file
 *
 * Revision 1.4  2006/03/30 23:22:57  misuj1am
 *
 *
 * -- CObjectComplex : get/set PropertyValue -> get/set Property
 * -- ADD: addProperty (size_t position, IProperty)
 *
 * Revision 1.3  2006/03/29 06:18:37  hockm0bm
 * syntax error corrected in getDictFromRef
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

#include <string>
#include "cobjecthelpers.h"

namespace pdfobjects
{

namespace utils
{


int getIntFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	using namespace boost;

	shared_ptr<IProperty> prop_ptr=dict->getProperty(name);	
	if(prop_ptr->getType() != pInt)
	{
		// malformed dictionary
		throw ElementBadTypeException(name/*, prop_ptr->getType()*/);
	}

	shared_ptr<CInt> int_ptr=IProperty::getSmartCObjectPtr<CInt>(prop_ptr);
	int value;
	int_ptr->getPropertyValue(value);

	return value;
}

IndiRef getRefFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	using namespace boost;

	shared_ptr<IProperty> prop_ptr=dict->getProperty(name);	
	if(prop_ptr->getType() != pRef)
	{
		// malformed dictionary
		throw ElementBadTypeException(name/*, prop_ptr->getType()*/);
	}

	shared_ptr<CRef> int_ptr=IProperty::getSmartCObjectPtr<CRef>(prop_ptr);
	IndiRef value;
	int_ptr->getPropertyValue(value);

	return value;
}

std::string getStringFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	using namespace boost;

	shared_ptr<IProperty> prop_ptr=dict->getProperty(name);	
	if(prop_ptr->getType() != pString)
	{
		// malformed dictionary
		throw ElementBadTypeException(name);
	}

	shared_ptr<CString> str_ptr=IProperty::getSmartCObjectPtr<CString>(prop_ptr);
	std::string value;
	str_ptr->getPropertyValue(value);

	return value;
}
	
std::string getNameFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	using namespace boost;

	shared_ptr<IProperty> prop_ptr=dict->getProperty(name);	
	if(prop_ptr->getType() != pName)
	{
		// malformed dictionary
		throw ElementBadTypeException(name);
	}

	shared_ptr<CName> name_ptr=IProperty::getSmartCObjectPtr<CName>(prop_ptr);
	std::string value;
	name_ptr->getPropertyValue(value);

	return value;
}

std::string getDictType(boost::shared_ptr<CDict> dict)
{
	try
	{
		return getNameFromDict("/Type", dict);
	// FIXME change to proper exception
	}catch(std::exception & e)
	{
		// not found so returns empty string
	}

	return std::string();
}

boost::shared_ptr<CDict> getDictFromRef(boost::shared_ptr<IProperty> refProp)
{
	// TODO exception parameters
	
	if(refProp->getType()!=pRef)
		throw ElementBadTypeException("");
	
	// gets reference value and dereferences indirect object
	IndiRef ref;
	IProperty::getSmartCObjectPtr<CRef>(refProp)->getPropertyValue(ref);
	boost::shared_ptr<IProperty> indirect_ptr=refProp->getPdf()->getIndirectProperty(ref);
	if(indirect_ptr->getType() != pDict)
		throw ElementBadTypeException("");
	return IProperty::getSmartCObjectPtr<CDict>(indirect_ptr);
}


boost::shared_ptr<CDict> getDictFromRef(CPdf & pdf, IndiRef ref)
{
	boost::shared_ptr<IProperty> indirect_ptr=pdf.getIndirectProperty(ref);
	if(indirect_ptr->getType() != pDict)
		throw ElementBadTypeException("");
	return IProperty::getSmartCObjectPtr<CDict>(indirect_ptr);
}


void printProperty(boost::shared_ptr<IProperty> ip, std::ostream & out)
{
	std::string str;
	ip->getStringRepresentation(str);
	out << str << std::endl;
}

/** Compares two simple values.
 * @param val1 value to compare.
 * @param val2 value to compare.
 *
 * Helper methods for simple value comparing. Uses getValueFromSimple method to
 * get value from property. Simple properties are same if values, which they
 * holds are same.
 * <br>
 * SimpleClass template parameter stands for property class type (CRef, CInt,
 * ...)
 * <br>
 * simpleType is property type - constant value returned by getType() method.
 * <p>
 * NOTE: no checking of parameters is done here.
 */
template<typename SimpleClass, PropertyType SimpleType> 
bool simpleEquals(const boost::shared_ptr<IProperty> & val1, const boost::shared_ptr<IProperty> & val2)throw()
{
	// type for value
	typedef typename SimpleClass::Value Value;

	// gets values
	Value value1, value2;
	IProperty::getSmartCObjectPtr<SimpleClass>(val1)->getPropertyValue(value1);
	IProperty::getSmartCObjectPtr<SimpleClass>(val2)->getPropertyValue(value2);

	return value1==value2;
}

bool propertyEquals(const boost::shared_ptr<IProperty> & val1, const boost::shared_ptr<IProperty> & val2)
{
	// types must be same
	if(val1->getType()!=val2->getType())
		return false;

	// we have same type, so value has to be same too
	switch(val1->getType())
	{
		case pNull:
			// CNulls are all same
			return true;
		case pBool:
			return simpleEquals<CBool, pBool>(val1, val2);
		case pInt:
			return simpleEquals<CInt, pInt>(val1, val2);
		case pReal:
			return simpleEquals<CReal, pReal>(val1, val2);
		case pString:
			return simpleEquals<CString, pString>(val1, val2);
		case pName:
			return simpleEquals<CName, pName>(val1, val2);
		case pRef:
			return simpleEquals<CRef, pRef>(val1, val2);
		default:
			// complex are not supported
			throw NotImplementedException("propertyEquals for type="+val1->getType());
	}
	
}

} // end of utils namespace

} // end of pdfobjects namespace
