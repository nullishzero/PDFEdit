/*
 * main.cc
 *
 */

#include "static.h"

#include <iostream>
#include <fstream>

#include "cobject.h"
#include "cpdf.h"



// No output from KERNEL
#define NO_KERNEL_COUT_OUTPUT




//========= NASTY =============
#ifdef NO_KERNEL_COUT_OUTPUT
	#define OUTPUT		cout
	#define KOUTPUT		cerr
	#define OUTPUT_COND	(1==argc)
#else
	#define	OUTPUT 		cerr
	#define KOUTPUT		cout
	#define OUTPUT_COND	(1!=argc)
#endif

#define	INIT_BUFS			ofstream redirect_file("/dev/null"); streambuf * strm_buffer = NULL;
#define	SWAP_BUFS			{strm_buffer = KOUTPUT.rdbuf(); KOUTPUT.rdbuf(redirect_file.rdbuf());}
#define SWAP_BUFS_BACK		{KOUTPUT.rdbuf(strm_buffer);}
#define KERNEL_OUTPUT 		INIT_BUFS; if (OUTPUT_COND) {SWAP_BUFS;}
#define KERNEL_OUTPUT_BACK	if (OUTPUT_COND) {SWAP_BUFS_BACK;}

#define MEM_CHECK	{BasicMemChecker check;OUTPUT  << "OBJECTS UNALLOCATED: " << check.getCount () << endl;}

#define TEST(a) 	OUTPUT << endl << endl << "//=================== " << (a) << endl << endl;
#define START_TEST 	OUTPUT << endl << "Started testing..." << endl; KERNEL_OUTPUT;
#define END_TEST	OUTPUT << endl << "Ended testing..." << endl; KERNEL_OUTPUT_BACK;
#define OK_TEST		OUTPUT << "TEST PASSED..." << endl; MEM_CHECK;
//==========================


using namespace std;
using namespace pdfobjects;


namespace 
{

//
// examples
//
typedef struct
{
	string value;
	string expected;
	union {	
		GBool xb;		//   boolean
		int xi;			//   integer
		double xr;		//   real
		GString *xs;	//   string
		char* xn;		//   name
		Ref xref;		//   indirect reference
	};
	union {	
		bool b;			//   boolean
		int i;			//   integer
		double r;		//   real
		string* s;		//   string
		string* n;		//   name
		IndiRef* ref;	//   indirect reference
	};
	
} example_item;

typedef struct
{
	// simple
	example_item 	b;
	example_item 	i;
	example_item 	r;
	example_item 	s;
	example_item	n;
	example_item 	ref;

	// complex
	string			arTest1;
	string			arTest2;
	string			dcTest1;
	string			dcTest2;

} example;



//
//
//
namespace {

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

	ostream& 
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
	ostream& 
	operator << (ostream& os, Object* o)
	{
		string str;
		assert (NULL != o);
		utils::xpdfObjToString (*o,str);

		os << "[Xpdf Type " << o->getTypeName () << "] " << str << endl;

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

	bool
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

	bool
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

	bool
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

	bool
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
	
	bool
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

	bool
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

	bool
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

	bool
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

} // namespace


//=====================================================================================
// CObjectSimple
//



//====================================================

//
//
//
void 
s_clone ()
{
		CNull null;
		boost::shared_ptr<IProperty> ptr = null.clone ();

		if (ptr)
			OUTPUT << "null.getType(): " << null.getType () << endl;
		else 
			throw;
}

//====================================================

//
//
//
void 
s_ctors (const example& e)
{
		Object obj;
		CPdf pdf;
		IndiRef ref;
		
		// bool
		obj.initBool (e.b.xb);
		CBool bl (pdf, obj, ref);
		ip_validate (bl,e.b.expected);

		// int
		obj.initInt (e.i.xi);
		CInt i (pdf,obj,ref);
		ip_validate (i,e.i.expected);
		
		// Real
		obj.initReal (e.r.xr);
		CReal r (pdf,obj,ref);
		ip_validate (r,e.r.expected);
		
		// String
		obj.initString (e.s.xs);
		CString s (pdf,obj,ref);
		ip_validate (s,e.s.expected);

		
		// Name
		obj.initName (e.n.xn);
		CName n (pdf,obj,ref);
		ip_validate (n,e.n.expected);
		//
		// NAME IS COPIED !!!! fuck xpdf
		//		
		obj.free ();
		
		// Ref
		obj.initRef (e.ref.xref.num,e.ref.xref.gen);
		CRef f (pdf,obj,ref);
		ip_validate (f,e.ref.expected);
}

//====================================================

namespace
{
		template<typename T,typename U>
		void _s_ctors2 (const U var, const string& expected)
		{
				T cl (var);
				ip_validate (cl,expected);
		}

}

//
//
//
void 
s_ctors2 (const example& e)
{
		// bool
		_s_ctors2<CBool> (e.b.b,e.b.expected);
		// int
		_s_ctors2<CInt> (e.i.i,e.i.expected);
		// Real
		_s_ctors2<CReal> (e.r.r,e.r.expected);
		// String
		_s_ctors2<CString> (*(e.s.s),e.s.expected);
		// Name
		_s_ctors2<CName> (*(e.n.n),e.n.expected);
		// Ref
		_s_ctors2<CRef> (*(e.ref.ref),e.ref.expected);

		// Null
		CNull null;
		ip_validate (null, "null");
}

//====================================================

namespace
{
		template<typename T,typename U>
		void _s_setString (const U var, const string& expected)
		{
				T cl;
				cl.setStringRepresentation (var);
				ip_validate (cl,expected);
		}

}

//
//
//
void 
s_setString (const example& e)
{
		// bool
		_s_setString<CBool> (e.b.value,e.b.expected);
		// int
		_s_setString<CInt> (e.i.value,e.i.expected);
		// Real
		_s_setString<CReal> (e.r.value,e.r.expected);
		// String
		_s_setString<CString> (e.s.value,e.s.expected);
		// Name
		_s_setString<CName> (e.n.value,e.n.expected);
		// Ref
		_s_setString<CRef> (e.ref.value,e.ref.expected);

		// should get compile error
		// CNull null;
		// null.setStringRepresentation ("");
}

//====================================================

namespace
{
		template<typename T,PropertyType Tp>
		void _s_getTp ()
		{
				T cl;
				ip_validate (Tp,cl.getType ());
		}

}

//
//
//
void 
s_getTp ()
{
		// bool
		_s_getTp<CBool,pBool> ();
		// int
		_s_getTp<CInt,pInt> ();
		// Real
		_s_getTp<CReal,pReal> ();
		// String
		_s_getTp<CString,pString> ();
		// Name
		_s_getTp<CName,pName> ();
		// Ref
		_s_getTp<CRef,pRef> ();
		// Null
		_s_getTp<CNull,pNull> ();
}


//====================================================

namespace
{
		template<typename T,typename U>
		void _s_writeVal (const U var, const string& expected)
		{
				T cl;
				cl.writeValue (var);
				ip_validate (cl,expected);
		}

}

//
//
//
void 
s_writeVal (const example& e)
{
		// bool
		_s_writeVal<CBool> (e.b.b,e.b.expected);
		// int
		_s_writeVal<CInt> (e.i.i,e.i.expected);
		// Real
		_s_writeVal<CReal> (e.r.r,e.r.expected);
		// String
		_s_writeVal<CString> (*(e.s.s),e.s.expected);
		// Name
		_s_writeVal<CName> (*(e.n.n),e.n.expected);
		// Ref
		_s_writeVal<CRef> (*(e.ref.ref),e.ref.expected);

		// should get compile error
		// CNull null;
		// null.writeValue ("");

}

//====================================================

namespace
{
		template<typename T,typename U>
		void _s_getVal (const U expected)
		{
				T cl;
				cl.writeValue (expected);
				U val;
				cl.getPropertyValue (val);
				ip_validate (val,expected);
		}
}

void 
s_getVal (const example& e)
{
		// bool
		_s_getVal<CBool> (e.b.b);
		// int
		_s_getVal<CInt> (e.i.i);
		// Real
		_s_getVal<CReal> (e.r.r);
		// String
		_s_getVal<CString> (*(e.s.s));
		// Name
		_s_getVal<CName> (*(e.n.n));
		// Ref
		_s_getVal<CRef> (*(e.ref.ref));

		// should get compile error
		// CNull null;
		// null.getPropertyValue ();

}

//====================================================

namespace
{
		template<typename T,typename U>
		void _s_makeXpdf (const U var, const string& expected)
		{
				Object* obj;
				T cl;
				cl.writeValue (var);
				obj = cl._makeXpdfObject ();
				ip_validate (obj,expected);
				utils::freeXpdfObject (obj);
		}

}

//
//
//
void 
s_makeXpdf (const example& e)
{
		// bool
		_s_makeXpdf<CBool> (e.b.b,e.b.expected);
		// int
		_s_makeXpdf<CInt> (e.i.i,e.i.expected);
		// Real
		_s_makeXpdf<CReal> (e.r.r,e.r.expected);
		// String
		_s_makeXpdf<CString> (*(e.s.s),e.s.expected);
		// Name
		_s_makeXpdf<CName> (*(e.n.n),e.n.expected);
		// Ref
		_s_makeXpdf<CRef> (*(e.ref.ref),e.ref.expected);

		// should get compile error
		// CNull null;
		// null.getPropertyValue ();

}


//====================================================

void
s_rel ()
{
		CString cstr ("raz dva tri");
		boost::shared_ptr<IProperty> ip = cstr.clone (); // object UNALLOCATED
		OUTPUT << "unallocating " << endl;

}


//=====================================================================================
// CObjectComplex
//


//=====================================================================================

void
c_clone ()
{
		CDict dict;

		CString item1 ("fero");
		dict.addProperty ("jano", item1);
		
		boost::shared_ptr<IProperty> clone_ = dict.clone (); // 2x objects allocated
		ip_validate (dict, *clone_);
}


//=====================================================================================

void
c_smrt ()
{
		CDict dict; 
		CString val1 ("val1");
		CString val2 ("val2");
		dict.addProperty ("item1", val1);
		dict.addProperty ("item2", val2);

// BAD HABBIT
		CString* ptrStr1 = new CString ("val3");	// UNALLOCATED if not freed manually
		dict.addProperty ("item3", *ptrStr1);
		delete ptrStr1;
// GOOD HABBIT
		boost::shared_ptr<IProperty> ptrStr2 (new CString ("val4"));
		dict.addProperty ("item4", *ptrStr2);
// EVEN BETTER HABBIT
		CString str3 ("val5");
		dict.addProperty ("item5", str3);
// BEST HABBIT -- not functional (yet)
		//dict.addProperty ("item5", CString ("val6"));

		OUTPUT << dict;
		
		boost::shared_ptr<IProperty> ipp1 = dict.getPropertyValue ("item3");
		boost::shared_ptr<IProperty> ipp2 = dict.getPropertyValue ("item4");
		boost::shared_ptr<IProperty> ipp3 = dict.getPropertyValue ("item5");
}

//=====================================================================================

namespace
{
		template<typename U, PropertyType Tp>
		void
		_c_getTp ()
		{
			U obj;
			ip_validate (Tp, obj.getType ());
		}
}

void
c_getTp ()
{
		// Array
		_c_getTp<CArray,pArray> ();
		// Dict
		_c_getTp<CDict,pDict> ();
		// Stream
		//_c_getTp<CStream,pStream> ();


}

//=====================================================================================

void
mdctrl ()
{
	CPdf pdf;
	pdf.getModeController ();
}


//=====================================================================================



namespace
{
		void
		makeArTest1 (CArray& arTest1)
		{
			//
			// Make an array, simple values
			//
			CString it1 ("item1");
			CString it2 ("item2");
			CString it3 ("item3");
			arTest1.addProperty (it1);
			arTest1.addProperty (it2);
			arTest1.addProperty (it3);
			CInt i1 (1);
			CInt i2 (2);
			CInt i3 (3);
			CInt i4 (4);
			arTest1.addProperty (i1);
			arTest1.addProperty (i2);
			arTest1.addProperty (i3);
			arTest1.addProperty (i4);
			IndiRef ref = {3,4};
			CRef rf (ref);
			arTest1.addProperty (rf);
			CBool bl (false);
			arTest1.addProperty (bl);

		}
	
		void
		makeDcTest1 (CDict& dcTest1)
		{
			//
			// Make a dictionary, simple values
			//
			CString it1 ("item1");
			CString it2 ("item2");
			CString it3 ("item3");
			dcTest1.addProperty ("item1", it1);
			dcTest1.addProperty ("item2", it2);
			dcTest1.addProperty ("item3", it3);
			CInt i1 (1);
			CInt i2 (2);
			CInt i3 (3);
			CInt i4 (4);
			dcTest1.addProperty ("item4", i1);
			dcTest1.addProperty ("item5", i2);
			dcTest1.addProperty ("item6", i3);
			dcTest1.addProperty ("item7", i4);
			IndiRef ref = {3,4};
			CRef rf (ref);
			dcTest1.addProperty ("item8", rf);
			CBool bl (false);
			dcTest1.addProperty ("item9", bl);

		}

		void
		makeArTest2 (CArray& arTest2, CArray& arTest1, CDict& dcTest1)
		{
			arTest2.addProperty (arTest1);
			arTest2.addProperty (arTest1);
			arTest2.addProperty (arTest1);
			arTest2.addProperty (dcTest1);
			arTest2.addProperty (dcTest1);
			arTest2.addProperty (dcTest1);
		}

		void
		makeDcTest2 (CDict& dcTest2, CArray& arTest1, CDict& dcTest1)
		{
			dcTest2.addProperty ("1", arTest1);
			dcTest2.addProperty ("2", arTest1);
			dcTest2.addProperty ("3", arTest1);
			dcTest2.addProperty ("4", dcTest1);
			dcTest2.addProperty ("5", dcTest1);
			dcTest2.addProperty ("6", dcTest1);
		}
		
}

void
c_getString (const example& e)
{
		
	CArray arTest1;
	CArray arTest2;
	CDict dcTest1;
	CDict dcTest2;

	makeArTest1 (arTest1);
	ip_validate (arTest1, e.arTest1);

	makeDcTest1 (dcTest1);
	ip_validate (dcTest1, e.dcTest1, false);

	makeArTest2 (arTest2, arTest1, dcTest1);
	ip_validate (arTest2, e.arTest2, false); 

	makeDcTest2 (dcTest2, arTest1, dcTest1);
	ip_validate (dcTest2, e.dcTest2, false); 
}


//=====================================================================================

void
c_getCnt ()
{
		
	CArray arTest1;
	CArray arTest2;
	CDict dcTest1;
	CDict dcTest2;

	makeArTest1 (arTest1);
	ip_validate (arTest1.getPropertyCount (), 9);

	makeDcTest1 (dcTest1);
	ip_validate (dcTest1.getPropertyCount (), 9);

	
	makeArTest2 (arTest2, arTest1, dcTest1);
	ip_validate (arTest2.getPropertyCount (), 6); 

	makeDcTest2 (dcTest2, arTest1, dcTest1);
	ip_validate (dcTest2.getPropertyCount (), 6); 
}

//=====================================================================================

void
c_getNames ()
{
		
	CArray arTest1;
	CDict dcTest1;
	CDict dcTest2;

	// create Array 1
	makeArTest1 (arTest1);

	//
	// expected item1, ... ,item9
	//
	vector<string> expected;
	for (int i = 1; i <= 9; i++)
	{
		ostringstream oss;
		oss << i;
		expected.push_back (string ("item" + oss.str()));
	}
	
	// create dictionary 1
	makeDcTest1 (dcTest1);
	vector<string> names;
	dcTest1.getAllPropertyNames (names);
	ip_validate (names, expected);

	//
	// reset string
	//
	expected.clear ();
	names.clear ();
	
	//
	// other 1, ... ,9
	//
	for (int i = 1; i <= 9; i++)
	{
		ostringstream oss;
		oss << i;
		expected.push_back (string (oss.str()));
	}

	makeDcTest2 (dcTest2, arTest1, dcTest1);
	dcTest2.getAllPropertyNames (names);
	ip_validate (names, expected); 
}

		
//=====================================================================================
		
void
c_getType ()
{
		
	CArray arTest1;
	CDict dcTest1;

	// create Array 1
	makeArTest1 (arTest1);
	for (size_t i = 0; i < arTest1.getPropertyCount (); i++)
	{
		boost::shared_ptr<IProperty> ip = arTest1.getPropertyValue (i);
		ip_validate (arTest1.getPropertyType (i), ip->getType ());
	}
	
	
	// create dictionary 1
	makeDcTest1 (dcTest1);
	vector<string> names;
	dcTest1.getAllPropertyNames (names);
	
	for (vector<string>::iterator it = names.begin(); it != names.end(); ++it)
	{
		boost::shared_ptr<IProperty> ip = dcTest1.getPropertyValue (*it);
		ip_validate (dcTest1.getPropertyType (*it), ip->getType ());
	}

}

//=====================================================================================

void
c_xpdf (const example& e)
{

	CArray arTest1;
	CArray arTest2;
	CDict dcTest1;
	CDict dcTest2;
	string tmp;

	makeArTest1 (arTest1);
	Object* obj = arTest1._makeXpdfObject ();
	arTest1.getStringRepresentation (tmp);
	ip_validate (obj, e.arTest1);
	ip_validate (obj, tmp);
	utils::freeXpdfObject (obj);

	makeDcTest1 (dcTest1);
	obj = dcTest1._makeXpdfObject ();
	dcTest1.getStringRepresentation (tmp);
	ip_validate (obj, e.dcTest1, false);
	ip_validate (obj, tmp);
	utils::freeXpdfObject (obj);

	makeArTest2 (arTest2, arTest1, dcTest1);
	obj = arTest2._makeXpdfObject ();
	arTest2.getStringRepresentation (tmp);
	ip_validate (obj, e.arTest2, false);
	ip_validate (obj, tmp);
	utils::freeXpdfObject (obj);


	makeDcTest2 (dcTest2, arTest1, dcTest1);
	obj = dcTest2._makeXpdfObject ();
	dcTest2.getStringRepresentation (tmp);
	ip_validate (obj, e.dcTest2, false);
	ip_validate (obj, tmp);
	utils::freeXpdfObject (obj);
}

//=====================================================================================

void
c_del ()
{

	CArray arTest1;
	CArray arTest2;
	CDict dcTest1;
	CDict dcTest2;

	//
	//
	//
	makeArTest1 (arTest1);
	while (0 != arTest1.getPropertyCount())
		arTest1.delProperty (0);
	ip_validate (arTest1,"[ ]");

	//
	//
	//
	makeDcTest1 (dcTest1);
	list<string> names;
	dcTest1.getAllPropertyNames (names);
	for (list<string>::iterator it = names.begin (); it != names.end (); ++it)
		dcTest1.delProperty (*it);
	ip_validate (dcTest1,"<<\n>>");

	//
	//
	//
	makeArTest2 (arTest2, arTest1, dcTest1);
	while (0 != arTest2.getPropertyCount())
		arTest2.delProperty (0);
	ip_validate (arTest2,"[ ]");

	//
	//
	//
	makeDcTest2 (dcTest2, arTest1, dcTest1);
	names.clear ();
	dcTest2.getAllPropertyNames (names);
	for (list<string>::iterator it = names.begin (); it != names.end (); ++it)
		dcTest2.delProperty (*it);
	ip_validate (dcTest2,"<<\n>>");
}


//=====================================================================================

void
c_set ()
{

	CArray a;
	CDict d;

	CInt i1 (1);	CInt i2 (2);	CInt i3 (3);
	CString s1 ("s1");	CString s2 ("s2");	CString s3 ("s3");

	a.addProperty (i1);			a.addProperty (i2);			a.addProperty (i3);
	ip_validate (a,"[ 1 2 3 ]");
	
	a.setPropertyValue (0,s1);	a.setPropertyValue (1,s2);	a.setPropertyValue (2,s3);
	ip_validate (a,"[ (s1) (s2) (s3) ]");

	d.addProperty ("1", i1);		d.addProperty ("2", i2);		d.addProperty ("3", i3);
	ip_validate (d,"<<\n/1 1\n/2 2\n/3 3\n>>", false);
	
	d.setPropertyValue ("1",s1);	d.setPropertyValue ("2",s2);	d.setPropertyValue ("3",s3);
	ip_validate (d,"<<\n/1 (s1)\n/2 (s2)\n/3 (s3)\n>>", false);
}

//=====================================================================================

void
c_xpdfctor ()
{
	{
		//
		// Array
		//
		Object obj;
		obj.initArray (NULL);

		Object item1; item1.initBool 	(gFalse);
		Object item2; item2.initBool	(gTrue);
		Object item3; item3.initInt 	(-43);
		Object item4; item4.initString 	(new GString ("item4"));

		obj.arrayAdd (&item1);
		obj.arrayAdd (&item2);
		obj.arrayAdd (&item3);
		obj.arrayAdd (&item4);
		
		CPdf pdf;
		IndiRef ref = {12,11};
		CArray ar (pdf,obj,ref);

		ip_validate (obj,ar);
		
		obj.free ();
	}
	{
		//
		// Dict
		//
		Object obj;
		obj.initDict ((XRef*)NULL);

		Object item1; item1.initBool 	(gFalse);
		Object item2; item2.initBool	(gTrue);
		Object item3; item3.initInt 	(-43);
		Object item4; item4.initString 	(new GString ("item4"));

		char* it1 = strdup ("lajno1");
		char* it2 = strdup ("lajno2");
		char* it3 = strdup ("lajno3");
		char* it4 = strdup ("lajno4");
		obj.dictAdd (it1, &item1);
		obj.dictAdd (it2, &item2);
		obj.dictAdd (it3, &item3);
		obj.dictAdd (it4, &item4);

		CPdf pdf;
		IndiRef ref = {12,11};
		// CArray ar (pdf,obj,ref); // GOOOOOD -- assertation failed
		CDict dc (pdf,obj,ref); 
		
		ip_validate (obj,dc);
		ip_validate (dc, "<<\n/lajno1 false\n/lajno2 true\n/lajno3 -43\n/lajno4 (item4)\n>>");
		
		obj.free ();
	}
	
}

				
//=====================================================================================

} // namespace




/**
 *  test main - load settings and launches a main window 
 */
int 
main (int argc, char* [])
{

		static example e;
	
		//
		// TEST EXAMPLE
		//
		// Simple
		//
		e.b.xb 			= gFalse;
		e.b.b 			= false;
		e.b.value 		= "false";
		e.b.expected	= "false";
		
		e.i.xi = e.i.i 	= 321;							
		e.i.value 		= "321";
		e.i.expected 	= "321";
		
		e.r.xr = e.r.r 	= -3.2313; 						
		e.r.value 		= "-3.2313";
		e.r.expected 	= "-3.2313";
		
		e.n.xn = new char [100]; strncpy (e.n.xn, "name",10);
		e.n.n = new string ("name");
		e.n.value 		= "name";
		e.n.expected 	= "/name";
		
		e.s.xs = new GString("string");			
		e.s.s = new string ("string");
		e.s.value 		= "string";
		e.s.expected 	= "(string)";
		
		e.ref.ref = new IndiRef;
		e.ref.xref.num = e.ref.ref->num = 32;
		e.ref.xref.gen = e.ref.ref->gen = 10;
		e.ref.value 	= "32 10";
		e.ref.expected 	= "32 10 R";
		//
		// Complex
		//
		e.arTest1 = "[ (item1) (item2) (item3) 1 2 3 4 3 4 R false ]";
		e.dcTest1 = "<<\n/item1 (item1)\n/item2 (item2)\n/item3 (item3)\n/item4 1\n/item5 2\n/item6 "
				    "3\n/item7 4\n/item8 3 4 R\n/item9 false\n>>";
		const string ar2 ("[ " 	+ e.arTest1 + " " + e.arTest1 + " " 
						 	  	+ e.arTest1 + " " + e.dcTest1 + " " + e.dcTest1 + " " + e.dcTest1 + " ]");
		const string dc2 ("<<\n/1 "	+ e.arTest1 + "\n/2 " + e.arTest1 + "\n/3 " + e.arTest1 + "\n/4 " 
									+ e.dcTest1 + "\n/5 " + e.dcTest1 + "\n/6 " + e.dcTest1 + "\n>>");
		e.arTest2 = ar2;
		e.dcTest2 = dc2;

	
		START_TEST;
		
		TEST(" test 1.0 -- getType_");
		s_getTp ();
		OK_TEST;

		TEST(" test 1.1 -- clone");
		s_clone ();
		OK_TEST;
		
		TEST(" test 1.2 -- getString + constructors");
		s_ctors (e);
		OK_TEST;

		TEST(" test 1.3 -- getString + constructors 2");
		s_ctors2 (e);
		OK_TEST;
		
		TEST(" test 1.4 -- setString");
		s_setString (e);
		OK_TEST;

		TEST(" test 1.5 -- writeValue");
		s_writeVal (e);
		OK_TEST;

		TEST(" test 1.6 -- getPropertyValue");
		s_getVal (e);
		OK_TEST;

		TEST(" test 1.7 -- _makeXpdfObject");
		s_makeXpdf (e);
		OK_TEST;

		TEST(" test 1.8 -- _");
		s_rel ();
		OK_TEST;
	
		//======================= CObjectComplex
		
		TEST(" test 2.1.0 - getType")
		c_getTp ();
		OK_TEST;

		TEST(" test 2.1 - clone")
		c_clone ();
		OK_TEST;

		TEST(" test 2.2 - smart ptrs")
		c_smrt ();
		OK_TEST;
	
		TEST(" test 2.3 - mode controller")
		mdctrl ();
		OK_TEST;
	
		TEST(" test 2.4 - getStringRepre")
		c_getString (e);
		OK_TEST;

		TEST(" test 2.5 - getPropCount")
		c_getCnt ();
		OK_TEST;

		TEST(" test 2.6 - getAllPropNames")
		c_getNames ();
		OK_TEST;

		TEST(" test 2.7 - getType")
		c_getType ();
		OK_TEST;

		TEST(" test 2.8 - makeXpdf")
		c_xpdf (e);
		OK_TEST;

		TEST(" test 2.9 - delProp")
		c_del ();
		OK_TEST;

		TEST(" test 2.9 - setProp")
		c_set ();
		OK_TEST;

		TEST(" test 2.10 - xpdf ctors")
		c_xpdfctor ();
		OK_TEST;

		END_TEST;
		MEM_CHECK;

		return 0;
}






/*
		//======== test xx
		GString* fileName = new GString ("/home/jozo/_pdf/pdfedit/or_d0506.pdf");
		PDFDoc* doc = new PDFDoc (fileName,new GString(),new GString());
		cout << "Filename: "<< fileName->getCString() << endl;
		cout << "Number of pages: "<< doc->getNumPages () << endl;
		// Get an object from PDFDoc
		Object* obj; 
		

		//
		// Our stuff here
		//
		//CPdf pdf;
		//string str;
		XRef* xref = doc->getXRef();

		cout << "Xref: " << (unsigned int) xref << endl;

		obj = xref->getTrailerDict ();
		CPdf::objToString (obj,str);
		cout << str << endl;
		str = "";

		xref->getCatalog (obj);
		CPdf::objToString (obj,str);
		cout << str << endl;
		str = "";

		xref->getDocInfo (obj);
		CPdf::objToString (obj,str);
		cout << str << endl;
		str = "";
*/
