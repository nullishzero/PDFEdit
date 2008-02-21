/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

// static
#include "kernel/static.h"
// xpdf
#include "kernel/xpdf.h"
//
#include "kernel/pdfspecification.h"
#include "kernel/cobject.h"
#include "kernel/cpdf.h"
#include "kernel/cxref.h"


using namespace std;
using namespace xpdf;
using namespace boost;

// =====================================================================================
namespace pdfobjects{
// =====================================================================================
namespace utils {
// =====================================================================================


// =====================================================================================
//  CObject 2 String / String 2 CObject functions
// =====================================================================================


//
//
//
void
simpleValueFromString (const std::string& str, bool& val)
{
	static const string __true ("true");
	static const string __false ("false");
	
	if (str.size() == __true.size() && equal(str.begin(), str.end(), __true.begin(), nocase_compare))
		val = true;
	else if (str.size() == __false.size() && equal (str.begin(), str.end(), __false.begin(), nocase_compare)) 
		val = false;
	else
		throw CObjBadValue ();
}

void
simpleValueFromString (const std::string& str, int& val)
{
	std::stringstream ss (str);
	ss.exceptions (stringstream::failbit | stringstream::badbit);
	try {
		ss >> val;
	}catch (stringstream::failure&) 
	{
		throw CObjBadValue ();
	}					
}

void
simpleValueFromString (const std::string& str, double& val)
{
	shared_ptr<Object> ptrObj (xpdfObjFromString(str), xpdf::object_deleter());
	
		assert (objReal == ptrObj->getType ());
		if (objReal != ptrObj->getType() && objInt != ptrObj->getType())
			throw CObjBadValue ();
					
	simpleValueFromXpdfObj<pReal, double&> (*ptrObj, val);
}

void
simpleValueFromString (const std::string& str, std::string& val)
{
	val = str;
}

void
simpleValueFromString (const std::string& str, IndiRef& val)
{
	std::stringstream ss (str);
	ss.exceptions (stringstream::failbit | stringstream::badbit);
	try {
		ss >> val.num;
		ss >> val.gen;
	}catch (stringstream::failure&) 
	{
		throw CObjBadValue ();
	}
}



//
// To string functions
// 


//
// Convert simple value from CObjectSimple to string
//
template <>
void
simpleValueToString<pBool> (bool val, string& str)
{
	str = ((val) ? Specification::CBOOL_TRUE : Specification::CBOOL_FALSE);
}
//
//
//
template <>
void
simpleValueToString<pInt> (int val, string& str)
{
	char buf[24];
	sprintf(buf,"%d",val);
	str=buf;
}
//
//
//
template <>
void
simpleValueToString<pReal> (double val, string& str)
{
	char buf[64];
	sprintf(buf, "%g", val);
 	str.assign(buf);
}

//
// Special case for pNull
//
template<>
void
simpleValueToString<pNull> (const NullType&, string& str)
{
	str = Specification::CNULL_NULL;
}
//
// Special case for pRef
//
template<>
void
simpleValueToString<pRef> (const IndiRef& ref, string& str)
{
	ostringstream oss;
	oss << ref.num << Specification::CREF_MIDDLE << ref.gen << Specification::CREF_SUFFIX;
	// convert oss to string
	str = oss.str ();
}

//
// Special case for pString and pName
//
template<PropertyType Tp>
void
simpleValueToString (const std::string& val, std::string& str)
{
	STATIC_CHECK ((pString == Tp) || (pName == Tp), COBJECT_INCORRECT_USE_OF_simpleObjToString_FUNCTION);

	switch (Tp)
	{
		case pString:
			str = Specification::CSTRING_PREFIX + makeStringPdfValid (val.begin(), val.end()) + Specification::CSTRING_SUFFIX;
			break;

		case pName:
			str = Specification::CNAME_PREFIX + makeNamePdfValid (val.begin(), val.end());
			break;

		default:
			assert (0);
			break;
	}
}

template void simpleValueToString<pString> (const string& val, string& str);
template void simpleValueToString<pName> (const string& val, string& str);


//
// Convert complex value from CObjectComplex to string
//
template<>
void
complexValueToString<CArray> (const CArray::Value& val, string& str)
{
	utilsPrintDbg (debug::DBG_DBG,"complexValueToString<pArray>()" );
		
	ostringstream oss;

	// start tag
	str = Specification::CARRAY_PREFIX;
		
	//
	// Loop through all items and get each string and append it to the result
	//
	CArray::Value::const_iterator it = val.begin();
	for (; it != val.end(); ++it) 
	{
		str += Specification::CARRAY_MIDDLE;
		string tmp;
		(*it)->getStringRepresentation (tmp);
		str += tmp;
	}
		
	// end tag
	str += Specification::CARRAY_SUFFIX;
}
//
//
//
template<>
void
complexValueToString<CDict> (const CDict::Value& val, string& str)
{
	utilsPrintDbg (debug::DBG_DBG,"complexValueToString<pDict>()");

	// start tag
	str = Specification::CDICT_PREFIX;

	//
	// Loop through all items and get each items name + string representation
	// and append it to the result
	//
	CDict::Value::const_iterator it = val.begin ();
	for (; it != val.end(); ++it) 
	{
		str += Specification::CDICT_MIDDLE + (*it).first + Specification::CDICT_BETWEEN_NAMES;
		string tmp;
		(*it).second->getStringRepresentation (tmp);
		str += tmp;
	}

	// end tag
	str += Specification::CDICT_SUFFIX;
}


// =====================================================================================
} /* namespace utils */
// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
