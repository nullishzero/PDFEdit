// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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
#include "../pdfoperators.h"

// TFUJJ
using namespace std;
using namespace pdfobjects;


//
// Test pdf file
//
static const char* TESTPDFFILE= "../../doc/zadani.pdf";


// No output from KERNEL
#define NO_KERNEL_COUT_OUTPUT
// Quick compilation
#define __QUICK_TEST


//========= NASTY =============
#ifdef NO_KERNEL_COUT_OUTPUT
	#define OUTPUT		cout
	#define KOUTPUT		cerr
	#define OUTPUT_COND	(1==argc)
#else
	#define	OUTPUT		cerr
	#define KOUTPUT		cout
	#define OUTPUT_COND	(1!=argc)
#endif

#define	INIT_BUFS			ofstream redirect_file("/dev/null"); streambuf * strm_buffer = NULL;
#define	SWAP_BUFS			{strm_buffer = KOUTPUT.rdbuf(); KOUTPUT.rdbuf(redirect_file.rdbuf());}
#define SWAP_BUFS_BACK		{KOUTPUT.rdbuf(strm_buffer);}
#define KERNEL_OUTPUT		INIT_BUFS; if (OUTPUT_COND) {SWAP_BUFS;}
#define KERNEL_OUTPUT_BACK	if (OUTPUT_COND) {SWAP_BUFS_BACK;}

#define MEM_CHECK	{BasicMemChecker check;OUTPUT	<< "OBJECTS UNALLOCATED: " << check.getCount () \
													<< " OBJECTS ALLOCATED: " << check.getMaxCount () << endl;}

#define TEST(a)		OUTPUT << endl << endl << "//=================== " << (a) << endl << endl;
#define START_TEST	OUTPUT << endl << "Started testing..." << endl; KERNEL_OUTPUT;
#define END_TEST	OUTPUT << endl << "Ended testing..." << endl; KERNEL_OUTPUT_BACK; MEM_CHECK;
#define OK_TEST		OUTPUT << "TEST PASSED..." << endl; MEM_CHECK;
//==========================


//
//
//
template<PropertyType T, typename U>
IProperty*
createSimpleObj (U val)
{
		return new CObjectSimple<T> (val);
}

template<PropertyType T, typename U>
IProperty*
createComplexObj (U val)
{
		return new CObjectComplex<T> (val);
}

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

	os << "[CObject Type " << ip.getType () << "] " << str << endl;

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
		//os << "[Xpdf Type " << o->getTypeName () << "] " << str << endl;
		os << str;
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
		os << &obj;
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
		os << d->getKey(i) << "\t" << d->getValNF (i, &obj) << endl;
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

	os << "Stream:";
	int c;
	while (EOF != (c=s->getRawChar()))
		os << (char)c << " ";

	os << endl;
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
		OUTPUT << "*** Validation OK! <comparison of values>" << endl;
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
		OUTPUT << "*** Validation OK! <comparison of Indiref>" << endl;
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
ip_validate (Object* o, const string& expected, bool out = true)
{
	string str;
	assert (NULL != o);
	utils::xpdfObjToString (*o,str);

	if (str == expected)
	{
		if (out)
			OUTPUT << "*** Validation OK! " << expected << endl;
		else
			OUTPUT << "*** Validation OK! " << "<not shown, output probably too long>"<< endl;
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
ip_validate (Object& o, IProperty& ip, bool out = true)
{
	string str;
	utils::xpdfObjToString (o,str);

	string str1;
	ip.getStringRepresentation (str1);

	if (str == str1)
	{
		if (out)
			OUTPUT << "*** Validation OK! " << str << endl;
		else
			OUTPUT << "*** Validation OK! " << "<not shown, output probably too long>"<< endl;
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
ip_validate (const IProperty& ip, const string& expected, bool out = true)
{
	string str;
	ip.getStringRepresentation (str);
	
	if (str == expected)
	{
		if (out)
			OUTPUT << "*** Validation OK! " << expected << endl;
		else
			OUTPUT << "*** Validation OK! " << "<not shown, output probably too long>"<< endl;
				
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
		OUTPUT << "*** Validation OK! " << str1 << endl;
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
		OUTPUT << "*** validation ok! " << tp1 << endl;
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
		OUTPUT << "*** validation ok! size:" << n << endl;
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
		OUTPUT << "*** validation ok!" <<  endl;
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

#endif // __QUICK_TEST
