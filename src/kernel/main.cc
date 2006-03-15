/*
 * main.cc
 *
 * Kernel does not need QT 
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
	#define KERNEL_OUTPUT 	ofstream redirect_file("/dev/null"); streambuf * strm_buffer = cout.rdbuf();cout.rdbuf(redirect_file.rdbuf());
	#define KERNEL_OUTPUT_BACK	 cout.rdbuf(strm_buffer);
	#define OUTPUT	cerr
#else
	#define KERNEL_OUTPUT
	#define KERNEL_OUTPUT_BACK
	#define	OUTPUT 	cout
#endif

#define TEST(a) 	OUTPUT << endl << endl << "//=================== " << (a) << endl << endl;
#define START_TEST 	OUTPUT << endl << "Started testing..." << endl; KERNEL_OUTPUT;
#define END_TEST	OUTPUT << endl << "Ended testing..." << endl; KERNEL_OUTPUT_BACK;
#define OK_TEST		OUTPUT << "TEST PASSED..." << endl;

#define MEM_CHECK	BasicMemChecker check;OUTPUT  << "OBJECTS UNALLOCATED: " << check.getCount () << endl;
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
	example_item b;
	example_item i;
	example_item r;
	example_item s;
	example_item n;
	example_item ref;
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
	ip_validate (Object* o, const string& expected)
	{
		string str;
		assert (NULL != o);
		utils::xpdfObjToString (*o,str);

		if (str == expected)
		{
			OUTPUT << "*** Validation OK! " << expected << endl;
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
	ip_validate (const IProperty& ip, const string& expected)
	{
		string str;
		ip.getStringRepresentation (str);
		
		if (str == expected)
		{
			OUTPUT << "*** Validation OK! " << expected << endl;
			return true;
		}
		else
		{
			OUTPUT << "DOES NOT MATCH: " << str << " with " << expected << endl;
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
		null.clone (); // object UNALLOCATED
		OUTPUT << "null.getType(): " << null.getType () << endl;
}

//====================================================

//
//
//
void 
s_ctors (const example& e)
{
		static Object obj;
		static CPdf pdf;
		static IndiRef ref;
		
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
		
		boost::shared_ptr<IProperty> clone_ = dict.clone (); // 2x object UNALLOCATED
		OUTPUT << dict;
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
		CString* str = new CString ("val3");	// UNALLOCATED if not freed manually
		dict.addProperty ("item3", *str);

		OUTPUT << dict;
		
		boost::shared_ptr<IProperty> ipp = dict.getPropertyValue ("item3");

		delete str;
}


//=====================================================================================

void
mdctrl ()
{
	CPdf pdf;
	pdf.getModeController ();
}


//=====================================================================================

} // namespace




/**
 *  test main - load settings and launches a main window 
 */
int main ()
{

		static example e;
	
		//
		// TEST EXAMPLE
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
		//
		//

		START_TEST;

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

		TEST(" test 2.1 - clone")
		c_clone ();
		OK_TEST;

		TEST(" test 2.2 - smart ptrs")
		c_smrt ();
		OK_TEST;
	
		TEST(" test 2.3 - mode controller")
		mdctrl ();
		OK_TEST;
	

		END_TEST;
		MEM_CHECK;

		return 0;
}






		//======== test x
		/*CPdf pdf;
		CDict dc(&pdf);
		IProperty* ip = pdf.getExistingProperty(77,77);
		

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
