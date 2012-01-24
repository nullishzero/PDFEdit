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

#include "kernel/static.h"
#include "kernel/factories.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/cpdf.h"

namespace pdfobjects
{

namespace utils
{

using namespace std;
using namespace boost;
	

int getIntFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	boost::shared_ptr<CInt> int_ptr=dict->getProperty<CInt>(name);
	return int_ptr->getValue();
}

IndiRef getRefFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	boost::shared_ptr<CRef> int_ptr=dict->getProperty<CRef>(name);
	return int_ptr->getValue();
}

std::string getStringFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	boost::shared_ptr<CString> str_ptr=dict->getProperty<CString>(name);
	return str_ptr->getValue();
}
	
std::string getNameFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	boost::shared_ptr<CName> name_ptr=dict->getProperty<CName>(name);
	std::string value;
	name_ptr->getValue(value);

	return value;
}

std::string getDictType(boost::shared_ptr<CDict> dict)
{
	try
	{
		return getNameFromDict("Type", dict);
	// FIXME change to proper exception
	}catch(std::exception &)
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
	IProperty::getSmartCObjectPtr<CRef>(refProp)->getValue(ref);
	boost::shared_ptr<CPdf> pdf = refProp->getPdf().lock();
	assert(pdf);
	boost::shared_ptr<IProperty> indirect_ptr=pdf->getIndirectProperty(ref);
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
	IProperty::getSmartCObjectPtr<SimpleClass>(val1)->getValue(value1);
	IProperty::getSmartCObjectPtr<SimpleClass>(val2)->getValue(value2);

	return value1==value2;
}

bool PropertyEquals::operator()(const boost::shared_ptr<IProperty> & val1, const boost::shared_ptr<IProperty> & val2)const
{
using namespace debug;
	
	// types must be same

	PropertyType val1Type=val1->getType(),
				 val2Type=val2->getType();
	
	if(val1Type!=val2Type)
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


//=========================================================
//	CObject* "delegate" helper methods
//=========================================================

//
//
//
boost::shared_ptr<IProperty>
getReferencedObject (boost::shared_ptr<IProperty> ip)
{
	if (ip)
	{
		if (isRef (ip))
		{// Fetch the right object
			assert (hasValidPdf(ip));
			assert (hasValidRef (ip));
			if (!hasValidPdf(ip) || !hasValidRef(ip))
				throw CObjInvalidObject ();

			IndiRef ref;
			IProperty::getSmartCObjectPtr<CRef>(ip)->getValue(ref);
			boost::shared_ptr<CPdf> pdf = ip->getPdf().lock();
			assert(pdf);
			return pdf->getIndirectProperty (ref);

		}else
			return ip;
	}else
		throw CObjInvalidOperation ();
}


boost::shared_ptr<IProperty> getIPropertyFromRectangle(const libs::Rectangle & rect)
{
using namespace boost;

	// pdf specification says that two diagonal corners should be used and
	// readers has to be prepared to normalize it
	boost::shared_ptr<CArray> array(CArrayFactory::getInstance());
	boost::scoped_ptr<IProperty> llx(CRealFactory::getInstance(rect.xleft));
	boost::scoped_ptr<IProperty> lly(CRealFactory::getInstance(rect.yleft));
	boost::scoped_ptr<IProperty> urx(CRealFactory::getInstance(rect.xright));
	boost::scoped_ptr<IProperty> ury(CRealFactory::getInstance(rect.yright));
	array->addProperty(0, *llx);
	array->addProperty(1, *lly);
	array->addProperty(2, *urx);
	array->addProperty(3, *ury);

	return array;
}

void getRectangleFromProperty(const boost::shared_ptr<IProperty> prop, libs::Rectangle & rect)
{
	if(!isArray(prop))
		throw CObjBadValue();
	
	boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray>(prop);
	if(array->getPropertyCount() != 4)
		throw CObjBadValue();

	rect.xleft = getDoubleFromArray(array, 0);
	rect.yleft = getDoubleFromArray(array, 1);
	rect.xright = getDoubleFromArray(array, 2);
	rect.yright = getDoubleFromArray(array, 3);
}

boost::shared_ptr<IProperty> getIPropertyFromDate(const tm * time)
{
using namespace boost;

	boost::shared_ptr<CString> dateString(CStringFactory::getInstance());
	char buffer[30];
	memset(buffer, '\0', sizeof(buffer));

	strftime(buffer, sizeof(buffer)-1, "D:%Y%m%d%H%M%S", time);
	
	// FIXME use timezone information
	
	// sets value from created buffer
	dateString->setValue(buffer);

	return dateString;
}

bool checkAndReplace(boost::shared_ptr<CDict> annotDict, std::string fieldName, IProperty & fieldValue)
{
using namespace boost;

	boost::shared_ptr<IProperty> value;
	try
	{
		value=annotDict->getProperty(fieldName);
		annotDict->setProperty(fieldName, fieldValue);
		return true;
	}catch(ElementNotFoundException &)
	{
		// annotDict doesn't contain fieldName so one has to be added
		annotDict->addProperty(fieldName, fieldValue);
		return false;
	}
}


//
//
//
::XRef*
getXRef (boost::shared_ptr<IProperty> ip)
{
	boost::shared_ptr<CPdf> pdf = ip->getPdf().lock();
	if(!pdf)
		return NULL;
	return pdf->getCXref(); 
}


 
} // end of utils namespace

} // end of pdfobjects namespace
