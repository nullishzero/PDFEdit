/*
 * main.cc
 *
 * Kernel does not need QT 
 */

#include "static.h"

//#include <iostream>

#include "cobject.h"
#include "cpdf.h"


using namespace std;
using namespace pdfobjects;


static	Object obj;
static	IndiRef ref;
static	CPdf pdf;



//=====================================================================================
// CObjectSimple
//

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
	
	
	ostream& 
	operator << (ostream& os, Object* o)
	{
		string str;
		assert (NULL != o);
		utils::xpdfObjToString (*o,str);

		os << "[Xpdf Type " << o->getTypeName () << "] " << str << endl;

		return os;
	}

	bool
	ip_validate (IProperty& ip, string expected)
	{
		string str;
		ip.getStringRepresentation (str);
		
		if (str == expected)
		{
			cout << "Validation OK! " << expected << endl;
			return true;
		}
		else
		{
			cout << "DOES NOT MATCH: " << str << " with " << expected << endl;
			throw;
			return false;
		}
	}
	
} // namespace

//
//
//
void 
s_clone ()
{
		CNull null;
		null.clone (); // object UNALLOCATED
		cout << "null.getType(): " << null.getType () << endl;
}

//
//
//
void 
s_ctors (example& e)
{
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

//
//
//
void 
s_ctors2 (example& e)
{
		// bool
		CBool bl (e.b.b);
		ip_validate (bl,e.b.expected);
		
		// int
		CInt i (e.i.i);
		ip_validate (i,e.i.expected);

		// Real
		CReal r (e.r.r);
		ip_validate (r,e.r.expected);
		
		// String
		CString s (*(e.s.s));
		ip_validate (s,e.s.expected);
		
		// Name
		CName n (*(e.n.n));
		ip_validate (n,e.n.expected);
		
		// Ref
		IndiRef rf = *(e.ref.ref);
		CRef f (rf);
		ip_validate (f, e.ref.expected);
		
		// Null
		CNull null;
		ip_validate (null, "null");
}

//
//
//
void 
s_setString (example& e)
{
		// bool
		CBool bl;
		bl.setStringRepresentation (e.b.value);
		ip_validate (bl,e.b.expected);
		
		// int
		CInt i;
		i.setStringRepresentation (e.i.value);
		ip_validate (i,e.i.expected);
		
		// Real
		CReal r;
		r.setStringRepresentation (e.r.value);
		ip_validate (r,e.r.expected);
		
		// String
		CString s;
		s.setStringRepresentation (e.s.value);
		ip_validate (s,e.s.expected);
		
		// Name
		CName n;
		n.setStringRepresentation (e.n.value);
		ip_validate (n,e.n.expected);
		
		// Ref
		CRef f;
		f.setStringRepresentation (e.ref.value);
		ip_validate (f,e.ref.expected);

		// should get compile error
		// CNull null;
		// null.setStringRepresentation ("");
}

//
//
//
void 
s_writeVal (example& e)
{
		string str;
		// bool
		CBool bl;
		bl.writeValue (e.b.b);
		cout << bl;
		ip_validate (bl, e.b.expected);
		
		// int
		CInt i;
		i.writeValue (e.i.i);
		cout << i;
		ip_validate (i, e.i.expected);
		
		// Real
		CReal r;
		r.writeValue (e.r.r);
		cout << r;
		ip_validate (r,e.r.expected);
		
		// String
		CString s;
		s.writeValue (*(e.s.s));
		cout << s;
		ip_validate (s, e.s.expected);
		
		// Name
		CName n;
		n.writeValue (*(e.n.n));
		cout << n;
		ip_validate (n, e.n.expected);
		
		// Ref
		CRef f;
		IndiRef rf = *(e.ref.ref);
		f.writeValue (rf);
		cout << f;
		ip_validate (f, e.ref.expected);

		// should get compile error
		// CNull null;
		// null.writeValue ("");

}

void 
s_getVal ()
{
		string str;
		// bool
		CBool bl;
		bl.writeValue (false);
		bool blVal;
		bl.getPropertyValue (blVal);
		cout << "Bool: " << blVal << endl;
		// int
		CInt i;
		i.writeValue (-7);
		int ii;
		i.getPropertyValue (ii);
		cout << "Int: " << ii << endl;
		// Real
		CReal r;
		r.writeValue (-0.12321);
		double db;
		r.getPropertyValue (db);
		cout << "Real: " << db << endl;
		// String
		CString s;
		s.writeValue ("1232123");
		s.getPropertyValue (str);
		cout << "String: " << str << endl;
		// Name
		CName n;
		n.writeValue ("namenamename");
		n.getPropertyValue (str);
		cout << "Name: " << str << endl;
		// Ref
		CRef f;
		IndiRef rf = {7,9};
		f.writeValue (rf);
		rf.gen = ref.num = 0;
		f.getPropertyValue (rf);
		cout << "Ref: " << rf.num << " " << rf.gen << endl;

		// should get compile error
		// CNull null;
		// null.getPropertyValue ();

}

//
//
//
void 
s_makeXpdf ()
{
		string str;
		Object* obj;
		// bool
		CBool bl;
		bl.writeValue (false);
		obj = bl._makeXpdfObject ();
		cout << obj;
		utils::freeXpdfObject (obj);
		
		// int
		CInt i;
		i.writeValue (-7);
		obj = i._makeXpdfObject ();
		cout << obj;
		utils::freeXpdfObject (obj);
		
		// Real
		CReal r;
		r.writeValue (-0.12321);
		obj = r._makeXpdfObject ();
		cout << obj;
		utils::freeXpdfObject (obj);
		
		// String
		CString s;
		s.writeValue ("1232123");
		obj = s._makeXpdfObject ();
		cout << obj;
		utils::freeXpdfObject (obj);
		
		// Name
		CName n;
		n.writeValue ("namenamename");
		n._makeXpdfObject ();
		cout << obj;
		utils::freeXpdfObject (obj);
		
		// Ref
		CRef f;
		IndiRef rf = {7,9};
		f.writeValue (rf);
		obj = f._makeXpdfObject ();
		cout << obj;
		utils::freeXpdfObject (obj);

		// should get compile error
		// CNull null;
		// null.getPropertyValue ();

}


void
s_rel ()
{
		CString cstr ("raz dva tri");
		boost::shared_ptr<IProperty> ip = cstr.clone (); // object UNALLOCATED
		cout << "unallocating " << endl;

}


//=====================================================================================
// CObjectComplex
//


void
c_clone ()
{
		CDict dict;

		CString item1 ("fero");
		dict.addProperty ("jano", item1);
		
		boost::shared_ptr<IProperty> clone_ = dict.clone (); // 2x object UNALLOCATED
		cout << dict;
}


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

		cout << dict;
		
		boost::shared_ptr<IProperty> ipp = dict.getPropertyValue ("item3");

		delete str;
}

void
mdctrl ()
{
	CPdf pdf;
	pdf.getModeController ();
}



#define TEST(a) 	cout << endl << endl << "//=================== " << (a) << endl << endl;
#define START_TEST 	cout << endl << "Started testing..." << endl;
#define END_TEST	cout << endl << "Ended testing..." << endl;
#define OK_TEST		cout << "TEST PASSED..." << endl;

#define MEM_CHECK	BasicMemChecker check;cout  << "OBJECTS UNALLOCATED: " << check.getCount () << endl;

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

//		TEST(" test 1.1 -- clone");
//		s_clone ();
//		OK_TEST;
		
/*		TEST(" test 1.2 -- getString + constructors");
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
*/
//		TEST(" test 1.6 -- getPropertyValue");
//		s_getVal ();
//		OK_TEST;

//		TEST(" test 1.7 -- _makeXpdfObject");
//		s_makeXpdf ();
//		OK_TEST;

//		TEST(" test 1.8 -- _");
//		s_rel ();
//		OK_TEST;

//		TEST(" test 2.1 - clone")
//		c_clone ();
//		OK_TEST;

//		TEST(" test 2.2 - smart ptrs")
//		c_smrt ();
//		OK_TEST;
	
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
