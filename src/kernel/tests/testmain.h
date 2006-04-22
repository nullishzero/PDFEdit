// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.12  2006/04/22 02:36:50  misuj1am
 *
 * -- observer correction
 *
 * Revision 1.11  2006/04/21 20:39:50  misuj1am
 *
 * -- test output improved
 *
 * Revision 1.10  2006/04/21 19:06:21  misuj1am
 *
 * -- tests improved
 *
 * Revision 1.9  2006/04/20 21:53:20  misuj1am
 *
 * --minor improvments
 *
 * Revision 1.8  2006/04/20 19:06:28  misuj1am
 *
 * -- tests improved
 *
 * Revision 1.7  2006/04/20 18:27:57  misuj1am
 *
 * -- cppunit tests
 *
 * Revision 1.6  2006/04/20 13:22:23  misuj1am
 *
 * --ADD: cppunittests, improved other tests
 *
 * Revision 1.5  2006/04/20 12:17:51  misuj1am
 *
 * -- IMPORTANT: when kernel output in tests needed, write ./kernel [file] all
 *
 * Revision 1.4  2006/04/10 23:07:22  misuj1am
 *
 *
 * -- ADD support for getObjectsAddPosition
 * 	-- almost done
 *
 * Revision 1.3  2006/04/02 17:15:17  misuj1am
 *
 *
 * -- testing
 *
 * Revision 1.2  2006/04/01 17:55:44  misuj1am
 *
 *
 * --ADD: MEM_CHECK at the end of testing
 *
 * Revision 1.1  2006/04/01 00:39:51  misuj1am
 *
 *
 * -- tests
 *
 */
#ifndef _TESTMAIN_H_
#define _TESTMAIN_H_

#include "../static.h"
#include "../iproperty.h"
#include "../cobject.h"
#include "../cpage.h"
#include "../pdfoperators.h"

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

//
// Extern variable that will hold files that we should test
//
typedef std::vector<std::string> FileList;
extern FileList fileList;


//========= NASTY =============
#define OUTPUT		cout
#define KOUTPUT		cerr

#define	INIT_BUFS			ofstream redirect_file("/dev/null"); streambuf * strm_buffer = NULL;bool swap = false;
#define	SWAP_BUFS			{swap=true;strm_buffer = KOUTPUT.rdbuf(); KOUTPUT.rdbuf(redirect_file.rdbuf());}
#define SWAP_BUFS_BACK		{KOUTPUT.rdbuf(strm_buffer);}
#define OUTPUT_MAGIC_WORD 	"all"
#define CHECK_OUTPUT(a,b)	INIT_BUFS;\
							if (NULL != argv[1] && 0 == strcmp(argv[1],OUTPUT_MAGIC_WORD))\
							{--(a);++(b);}\
							else\
							{SWAP_BUFS;}
#define KERNEL_OUTPUT_BACK	if (swap) {SWAP_BUFS_BACK;}
#define MEM_CHECK	{BasicMemChecker check;OUTPUT	<< "UNALLOCATED: " << check.getCount () \
													<< " ALLOCATED: " << check.getMaxCount () << endl;}
#define TEST(a)		OUTPUT << "\t//== " << (a);
#define START_TEST	OUTPUT << endl << "Started testing..." << endl;
#define END_TEST	OUTPUT << endl << "Ended testing..." << endl; KERNEL_OUTPUT_BACK; MEM_CHECK;
#define OK_TEST		OUTPUT << "\t...TEST PASSED..."; MEM_CHECK;
//==========================

// if set validation functions will output
#define REALLY_ALL_OUTPUT	0

//
//
//
template<PropertyType T>
IProperty*
createSimpleObjFromXpdf (CPdf& pdf, Object& obj, IndiRef ref)
{
		return new CObjectSimple<T> (pdf, obj, ref);
}

template<PropertyType T>
IProperty*
createComplexObjFromXpdf (CPdf& pdf, Object& obj, IndiRef ref)
{
		return new CObjectComplex<T> (pdf,obj,ref);
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
print (ostream& os, Object* o, XRef* xref)
{
	if (!o->isStream())
		throw;
	
	Object obj;
	assert (NULL != xref);
	boost::scoped_ptr<Parser> parser (new Parser(xref, new Lexer(xref, o)));
	parser->getObj(&obj);
	while (!obj.isEOF()) 
	{
		//os << &obj;
		obj.free ();
		// grab the next object
		parser->getObj(&obj);
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
		return false;
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
		return false;
	}
}

inline bool
ip_validate (Object* o, const string& expected,__attribute__((unused)) bool out = true)
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
		return false;
	}
}

inline bool
ip_validate (Object& o, IProperty& ip,__attribute__((unused)) bool out = true)
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
		return false;
	}
}

inline bool
ip_validate (const IProperty& ip, const string& expected,__attribute__((unused)) bool out = true)
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
		return false;
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
		return false;
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
		return false;
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
		return false;
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
		return false;
	}
}

#endif // _TESTMAIN_H_
