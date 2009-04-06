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
#include "tests/kernel/testmain.h"
#include "tests/kernel/testcobject.h"
#include "tests/kernel/testcpdf.h"

#include "kernel/ccontentstream.h"


//=====================================================================================
namespace {
//=====================================================================================

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

typedef struct example
{
	// simple
	example_item	b;
	example_item	i;
	example_item	r;
	example_item	s;
	example_item	n;
	example_item	ref;

} example;



//=====================================================================================
// CObjectSimple
//====================================================

//
//
//
bool 
s_clone ()
{
	CNull null;
	boost::shared_ptr<IProperty> ptr = null.clone ();

	if (ptr)
	{
		//OUTPUT << "null.getType(): " << null.getType () << endl;
	}
	else 
		return false;

	return true;
}

//====================================================

//
//
//
bool 
s_ctors (const char* filename, const example& e)
{
	Object obj;
	boost::shared_ptr<CPdf> pdf = getTestCPdf (filename);

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

	return true;
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
bool 
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

	return true;
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
bool 
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

	return true;
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
bool 
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

	return true;
}


//====================================================

namespace
{
	template<typename T,typename U>
	void _s_writeVal (const U var, const string& expected)
	{
			T cl;
			cl.setValue (var);
			ip_validate (cl,expected);
	}

}

//
//
//
bool 
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
	// null.setValue ("");
	 
	return true;
}

//====================================================

namespace
{
	template<typename T,typename U>
	void _s_getVal (const U expected)
	{
			T cl;
			cl.setValue (expected);
			U val;
			cl.getValue (val);
			ip_validate (val,expected);
	}
}

bool
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
	// null.getValue ();

	return true;
}

//====================================================

namespace
{
	template<typename T,typename U>
	void _s_makeXpdf (const U var, const string& expected)
	{
			Object* obj;
			T cl;
			cl.setValue (var);
			obj = cl._makeXpdfObject ();
			ip_validate (obj,expected);
			xpdf::freeXpdfObject (obj);
	}

}

//
//
//
bool 
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
	// null.getValue ();

	return true;
}


//====================================================

bool
s_rel ()
{
	CString cstr ("raz dva tri");
	boost::shared_ptr<IProperty> ip = cstr.clone (); // object UNALLOCATED
	return true;
}


//=========================================================================
// class TestCObjectSimple
//=========================================================================

class TestCObjectSimple : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCObjectSimple);
		CPPUNIT_TEST(TestGet);
		CPPUNIT_TEST(TestSet);
	CPPUNIT_TEST_SUITE_END();

private:
	example e;
	
public:
	void setUp() 
	{
		OUTPUT << endl;

		// TEST EXAMPLE
		e.b.xb			= gFalse;
		e.b.b			= false;
		e.b.value		= "false";
		e.b.expected	= "false";
		
		e.i.xi = e.i.i	= 321;							
		e.i.value		= "321";
		e.i.expected	= "321";
		
		e.r.xr = e.r.r	= -3.2313;						
		e.r.value		= "-3.2313";
		e.r.expected	= "-3.2313";
		
		e.n.xn = new char [100]; strncpy (e.n.xn, "name",10);
		e.n.n = new string ("name");
		e.n.value		= "name";
		e.n.expected	= "/name";
		
		e.s.xs = new GString("string");			
		e.s.s = new string ("string");
		e.s.value		= "string";
		e.s.expected	= "(string)";
		
		e.ref.ref = new IndiRef;
		e.ref.xref.num = e.ref.ref->num = 32;
		e.ref.xref.gen = e.ref.ref->gen = 10;
		e.ref.value	= "32 10";
		e.ref.expected	= "32 10 R";
	}
	void tearDown() 
	{
		delete[] e.n.xn;
		delete e.n.n;
		delete e.s.xs;
		delete e.s.s;
		delete e.ref.ref;
	
	}

public:
	void TestGet()
	{
		OUTPUT << "CObjectSimple getter methods..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" getType_");
			CPPUNIT_ASSERT (s_getTp ());
			OK_TEST;

			TEST(" clone");
			CPPUNIT_ASSERT (s_clone ());
			OK_TEST;
			
			TEST(" getString + constructors");
			CPPUNIT_ASSERT (s_ctors ((*it).c_str(), e));
			OK_TEST;

			TEST(" getString + constructors 2");
			CPPUNIT_ASSERT (s_ctors2 (e));
			OK_TEST;
			
			TEST(" getValue");
			CPPUNIT_ASSERT (s_getVal (e));
			OK_TEST;
		}
	}
	void TestSet()
	{
		OUTPUT << "CObjectSimple setter methods..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" setString");
			CPPUNIT_ASSERT (s_setString (e));
			OK_TEST;

			TEST(" setValue");
			CPPUNIT_ASSERT (s_writeVal (e));
			OK_TEST;

			TEST(" _makeXpdfObject");
			CPPUNIT_ASSERT (s_makeXpdf (e));
			OK_TEST;

			TEST(" __");
			CPPUNIT_ASSERT (s_rel ());
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCObjectSimple);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCObjectSimple, "TEST_COBJECTSIMPLE");

//=====================================================================================
} // namespace
//=====================================================================================
