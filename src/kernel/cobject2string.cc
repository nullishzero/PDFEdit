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

// static
#include "kernel/static.h"
// xpdf
#include "kernel/xpdf.h"
//
#include "kernel/pdfspecification.h"
#include "kernel/cobject.h"


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
	snprintf(buf, sizeof(buf)-1, "%d",val);
	str=buf;
}

/** Removes trailing zeros from given number in string
 * form.
 *
 * @param number_str Number in string representation.
 *
 * Note that number must be in [-+]nnnn.nnnn form.
 */
void trim_trailing_zero(char * number_str)
{
	char * p, *decimal = strchr(number_str, '.');
	if(!decimal)
		return;
	for(p=&number_str[strlen(number_str)-1];
			decimal < p && *p=='0'; --p)
		*p= '\0';

	// get rid of decimal point with no trailing decimal numbers
	if(p == decimal)
		*p='\0';
	
}

//
//
//
template <>
void
simpleValueToString<pReal> (double val, string& str)
{
	char buf[64];

	/* PDF specification doesn't allow [-]dd.dddE+-EXP
	 * real number format. So we have to make sure that
	 * all numbers have [-]dd.dddd format.
	 * PDF specification also says that we are using 5
	 * significant decimal digits.
	 * Note that %g produce required output only if 
	 * exponent is lower than precision and if it is
	 * higher than -4 (precision has different meaning
	 * here saying the max. number of all shown digits).
	 * We will simply use %f with fixed precision and 
	 * remove trailing zeros to reduce used space.
	 */
	snprintf(buf, sizeof(buf)-1, "%.5f", val);
	trim_trailing_zero(buf);
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

bool isBinaryString(const std::string& val)
{
	for(std::string::const_iterator i = val.begin(); i != val.end(); ++i) {
		unsigned char ch = *i;
		if(!isprint(ch))
			return true;
	}
	return false;
}

template<typename Iter>
std::string makeHexString(Iter it, Iter end)
{
	std::string tmp;
	for (; it != end; ++it)
	{
		char hexstr[4];
		snprintf(hexstr, sizeof(hexstr), "%02x", (unsigned)(*it)&0xff);
		tmp+=hexstr;
	}
	return tmp;
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
		{
			std::string validateStr = makeStringPdfValid (val);
			if (!isBinaryString(val))
				str = Specification::CSTRING_PREFIX + validateStr + Specification::CSTRING_SUFFIX;
			else
			{
				str = Specification::CHEXSTRING_PREFIX 
					+ makeHexString (validateStr.begin(), validateStr.end())
					+ Specification::CHEXSTRING_SUFFIX;
			}
			break;
		}

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
