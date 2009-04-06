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

#ifndef _TESTMAIN_H_
#define _TESTMAIN_H_

#include "kernel/static.h"
#include "tests/kernel/testparams.h"

// kernel
#include "kernel/cobject.h"
#include "kernel/cpdf.h"
#include "kernel/cpage.h"
#include "kernel/pdfoperators.h"
#include "kernel/coutline.h"
#include "kernel/exceptions.h"

//CPPUNITTEST
#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/CompilerOutputter.h>


// TFUJJ
using namespace std;
using namespace pdfobjects;


//=====================================================================================
// Settings
//=====================================================================================

// if set validation functions will output
#define REALLY_ALL_OUTPUT	0

// If set, some tests can create files with some special information
#define TEMP_FILES_CREATE	0

// MAX. of tested pages on 1 pdf.. if many, could take hours to complete
static const size_t TEST_MAX_PAGE_COUNT = 10000;


//========= NASTY =============
#define OUTPUT		cout
#define KOUTPUT		cerr

#define	INIT_BUFS			ofstream redirect_file("/dev/null"); streambuf * strm_buffer = NULL;bool swap = false;
#define	SWAP_BUFS			{swap=true;strm_buffer = KOUTPUT.rdbuf(); KOUTPUT.rdbuf(redirect_file.rdbuf());}
#define SWAP_BUFS_BACK		{KOUTPUT.rdbuf(strm_buffer);}
#define CHECK_OUTPUT(a)		INIT_BUFS; if (!a) {SWAP_BUFS;}
#define KERNEL_OUTPUT_BACK	if (swap) {SWAP_BUFS_BACK;}
#define TEST(a)				OUTPUT << "\t//== " << (a) << "  " << flush;
#define START_TEST			OUTPUT << endl << "Started testing..." << endl;
#define END_TEST			OUTPUT << "Ended testing..." << endl; KERNEL_OUTPUT_BACK;
#define OK_TEST				OUTPUT << "\t...TEST PASSED...\n" << flush;

//========= NASTY AGAIN ================
#define BEGIN_CHECK_READONLY	try {
#define END_CHECK_READONLY		} catch(ReadOnlyDocumentException& e) { std::cerr<< "Document is read only." << std::flush; }


//=====================================================================================
// Debug Helper functions
//=====================================================================================

//
// PrintDbg from tests
//
#define testPrintDbg(level, msg) _printDbg("TEST", level,  OUTPUT, msg);

//=====================================================================================
// Helper functions
//=====================================================================================

//
//
//
template<PropertyType T>
IProperty*
createSimpleObjFromXpdf (boost::shared_ptr<CPdf> pdf, Object& obj, IndiRef ref)
{
		return new CObjectSimple<T> (pdf, obj, ref);
}

inline ostream& 
operator << (ostream& os, IProperty& ip)
{
	string str;
	ip.getStringRepresentation (str);

	#if REALLY_ALL_OUTPUT
	os << "[CObject Type " << ip.getType () << "] " << str << endl;
	#endif
	
	return os;
}

//
//
//
inline ostream& 
operator << (ostream& os, Object* o)
{
	string str;
	assert (NULL != o);
	
	if (o->isCmd() || o->isEOF())
	{
		os << o->getCmd() << endl;

	}else
	{
		utils::xpdfObjToString (*o,str);
		#if REALLY_ALL_OUTPUT
		os << "[Xpdf Type " << o->getTypeName () << "] " << str << endl;
		os << str;
		#endif
	}

	return os;
}

//
//
//
inline void
print (UNUSED_PARAM ostream& os, Object* o, XRef* xref)
{
	if (!o->isStream())
		throw;
	
	Object obj;
	assert (NULL != xref);
	boost::scoped_ptr<Parser> parser (new Parser(xref, new Lexer(xref, o),
				gTrue  // TODO is gtrue ok here?
				)
			);
	parser->getObj(&obj); // FIXME handle return value
	while (!obj.isEOF()) 
	{
		//os << &obj;
		obj.free ();
		// grab the next object
		parser->getObj(&obj); // FIXME handle return value
	}
}


//
//
//
inline ostream& 
operator << (ostream& os, Dict* d)
{
	string str;
	Object obj;
	assert (NULL != d);

	for (int i = 0; i < d->getLength(); ++i)
	{
		#if REALLY_ALL_OUTPUT
		os << d->getKey(i) << "\t" << d->getValNF (i, &obj) << endl;
		#endif
	}

	return os;
}

//
//
//
//
inline std::ostream& 
operator << (ostream& os, const IndiRef& rf)
{
	os  << " Num: " << rf.num << "Gen: " << rf.gen;
	return os;
}


//
//
//
inline ostream& 
operator << (ostream& os, Stream* s)
{
	string str;
	assert (NULL != s);

	#if REALLY_ALL_OUTPUT
	os << "Stream:";
	#endif
	int c;
	while (EOF != (c=s->getRawChar()))
	{
		#if REALLY_ALL_OUTPUT
		os << (char)c << " ";
		#endif
	}

	#if REALLY_ALL_OUTPUT
	os << endl;
	#endif
	return os;
}

//
//
//
template<typename T>
bool
ip_validate (const T& val, const T& expected)
{
	
	if (val == expected)
	{
		#if REALLY_ALL_OUTPUT
		OUTPUT << "*** Validation OK! <comparison of values>" << endl;
		#endif
		return true;
	}
	else
	{
		OUTPUT << "DOES NOT MATCH: " << endl;
		throw;
	}
}

inline bool
ip_validate (const IndiRef& val, const IndiRef& expected)
{
	
	if ((val.gen == expected.gen) && (val.num == expected.num))
	{
		#if REALLY_ALL_OUTPUT
		OUTPUT << "*** Validation OK! <comparison of Indiref>" << endl;
		#endif
		return true;
	}
	else
	{
		OUTPUT << "DOES NOT MATCH: " << endl;
		throw;
	}
}

inline bool
ip_validate (Object* o, const string& expected,UNUSED_PARAM bool out = true)
{
	string str;
	assert (NULL != o);
	utils::xpdfObjToString (*o,str);

	if (str == expected)
	{
		#if REALLY_ALL_OUTPUT
		if (out)
			OUTPUT << "*** Validation OK! " << expected << endl;
		else
			OUTPUT << "*** Validation OK! " << "<not shown, output probably too long>"<< endl;
		#endif
		return true;
	}
	else
	{
		OUTPUT << "DOES NOT MATCH: " << str << " with " << expected << endl;
		throw;
	}
}

inline bool
ip_validate (Object& o, IProperty& ip,UNUSED_PARAM bool out = true)
{
	string str;
	utils::xpdfObjToString (o,str);

	string str1;
	ip.getStringRepresentation (str1);

	if (str == str1)
	{
		#if REALLY_ALL_OUTPUT
		if (out)
			OUTPUT << "*** Validation OK! " << str << endl;
		else
			OUTPUT << "*** Validation OK! " << "<not shown, output probably too long>"<< endl;
		#endif
		return true;
	}
	else
	{
		OUTPUT << "DOES NOT MATCH: " << str << " with " << str1 << endl;
		throw;
	}
}

inline bool
ip_validate (const IProperty& ip, const string& expected,UNUSED_PARAM bool out = true)
{
	string str;
	ip.getStringRepresentation (str);
	
	if (str == expected)
	{
		#if REALLY_ALL_OUTPUT
		if (out)
			OUTPUT << "*** Validation OK! " << expected << endl;
		else
			OUTPUT << "*** Validation OK! " << "<not shown, output probably too long>"<< endl;
		#endif		
		return true;
	}
	else
	{
		OUTPUT << "DOES NOT MATCH: " << str << " with " << expected << endl;
		throw;
	}
}

inline bool
ip_validate (const IProperty& ip1, const IProperty& ip2)
{
	string str1, str2;
	ip1.getStringRepresentation (str1);
	ip2.getStringRepresentation (str2);
	
	if (str1 == str2)
	{
		#if REALLY_ALL_OUTPUT
		OUTPUT << "*** Validation OK! " << str1 << endl;
		#endif
		return true;
	}
	else
	{
		OUTPUT << "DOES NOT MATCH: " << str1 << " with " << str2 << endl;
		throw;
	}
}

inline bool
ip_validate (PropertyType tp1, PropertyType tp2)
{
	if (tp1 == tp2)
	{
		#if REALLY_ALL_OUTPUT
		OUTPUT << "*** validation ok! " << tp1 << endl;
		#endif
		return true;
	}
	else
	{
		OUTPUT << "does not match: " << tp1 << " with " << tp2 << endl;
		throw;
	}
}

inline bool
ip_validate (size_t n, size_t m)
{
	if (n == m)
	{
		#if REALLY_ALL_OUTPUT
		OUTPUT << "*** validation ok! size:" << n << endl;
		#endif
		return true;
	}
	else
	{
		OUTPUT << "does not match: " << n << " with " << m << endl;
		throw;
	}
}

inline bool
ip_validate (vector<string>& n, vector<string>& m)
{
	if (equal (n.begin(), n.end(), m.begin()))
	{
		#if REALLY_ALL_OUTPUT
		OUTPUT << "*** validation ok!" <<  endl;
		#endif
		return true;
	}
	else
	{
		OUTPUT << "does not match: ";
		for (vector<string>::iterator it = n.begin (); it != n.end(); ++it)
				OUTPUT << *it << "|";
		OUTPUT << "... with ...";
		for (vector<string>::iterator it = m.begin (); it != m.end(); ++it)
				OUTPUT << *it << "|";
		OUTPUT << endl;
		throw;
	}
}

//
// Get test objects
//
inline boost::shared_ptr<CStream>
getTestStreamContent (boost::shared_ptr<CPage> page)
{
	boost::shared_ptr<CDict> dict = page->getDictionary();
	assert (dict);
	boost::shared_ptr<IProperty> ccs = utils::getReferencedObject (dict->getProperty(Specification::Page::CONTENTS));
	if (isStream(ccs))
		return IProperty::getSmartCObjectPtr<CStream> (ccs);
	else if (isArray(ccs))
		return utils::getCStreamFromArray (ccs, 0);
	else
	{
		//assert (!"Bad content stream type.");
		throw CObjInvalidObject ();
	}
}


//
// Indicate we are working
//
template<typename T>
void _working (T& oss)
{
	static int i = 0;
	static char chars[4] = { 'o', '/', '-', '\\' };
	
	if (0 == (i % 4))
		i = 0;
	
	oss << "\b" << flush;
	assert (4 > i);
	oss << chars[i] << flush;
	i++;
}


//=====================================================================================

#endif // _TESTMAIN_H_
