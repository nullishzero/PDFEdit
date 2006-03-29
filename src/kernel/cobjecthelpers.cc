// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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

	shared_ptr<IProperty> prop_ptr=dict->getPropertyValue(name);	
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

	shared_ptr<IProperty> prop_ptr=dict->getPropertyValue(name);	
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

	shared_ptr<IProperty> prop_ptr=dict->getPropertyValue(name);	
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

	shared_ptr<IProperty> prop_ptr=dict->getPropertyValue(name);	
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

} // end of utils namespace

} // end of pdfobjects namespace
