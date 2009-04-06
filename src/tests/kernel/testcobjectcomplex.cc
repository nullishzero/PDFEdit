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

//=====================================================================================
namespace {
//=====================================================================================


typedef struct
{
	// complex
	string			arTest1;
	string			arTest2;
	string			dcTest1;
	string			dcTest2;

} example;




//=====================================================================================
// CObjectComplex
//=====================================================================================



bool
c_clone ()
{
	CDict dict;

	CString item1 ("fero");
	dict.addProperty ("jano", item1);
	
	boost::shared_ptr<IProperty> clone_ = dict.clone (); // 2x objects allocated
	ip_validate (dict, *clone_);

	return true;
}


//=====================================================================================

bool
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

	//OUTPUT << dict;
	
	boost::shared_ptr<IProperty> ipp1 = dict.getProperty ("item3");
	boost::shared_ptr<IProperty> ipp2 = dict.getProperty ("item4");
	boost::shared_ptr<IProperty> ipp3 = dict.getProperty ("item5");

	ip_validate (*ipp1, "(val3)");
	ip_validate (*ipp2, "(val4)");
	ip_validate (*ipp3, "(val5)");

	return true;
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

bool
c_getTp ()
{
	// Array
	_c_getTp<CArray,pArray> ();
	// Dict
	_c_getTp<CDict,pDict> ();
	// Stream
	//_c_getTp<CStream,pStream> ();

	return true;
}

//=====================================================================================

bool
mdctrl (const char* filename)
{
	boost::shared_ptr<CPdf> _pdf = getTestCPdf (filename);
	_pdf->getModeController ();

	return true;
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
		IndiRef ref (3,4);
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
		IndiRef ref(3,4);
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

bool
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

	return true;
}


//=====================================================================================

bool
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

	return true;
}

//=====================================================================================

bool
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
	for (int i = 1; i <= 9; ++i)
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
	for (int i = 1; i <= 9; ++i)
	{
		ostringstream oss;
		oss << i;
		expected.push_back (string (oss.str()));
	}

	makeDcTest2 (dcTest2, arTest1, dcTest1);
	dcTest2.getAllPropertyNames (names);
	ip_validate (names, expected); 

	return true;
}

		
//=====================================================================================
		
bool
c_getType ()
{
		
	CArray arTest1;
	CDict dcTest1;

	// create Array 1
	makeArTest1 (arTest1);
	for (size_t i = 0; i < arTest1.getPropertyCount (); ++i)
	{
		boost::shared_ptr<IProperty> ip = arTest1.getProperty (i);
		ip_validate (arTest1.getPropertyType (i), ip->getType ());
	}
	
	
	// create dictionary 1
	makeDcTest1 (dcTest1);
	vector<string> names;
	dcTest1.getAllPropertyNames (names);
	
	for (vector<string>::iterator it = names.begin(); it != names.end(); ++it)
	{
		boost::shared_ptr<IProperty> ip = dcTest1.getProperty (*it);
		ip_validate (dcTest1.getPropertyType (*it), ip->getType ());
	}

	return true;
}

//=====================================================================================

bool
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
	xpdf::freeXpdfObject (obj);

	makeDcTest1 (dcTest1);
	obj = dcTest1._makeXpdfObject ();
	dcTest1.getStringRepresentation (tmp);
	ip_validate (obj, e.dcTest1, false);
	ip_validate (obj, tmp);
	xpdf::freeXpdfObject (obj);

	makeArTest2 (arTest2, arTest1, dcTest1);
	obj = arTest2._makeXpdfObject ();
	arTest2.getStringRepresentation (tmp);
	ip_validate (obj, e.arTest2, false);
	ip_validate (obj, tmp);
	xpdf::freeXpdfObject (obj);


	makeDcTest2 (dcTest2, arTest1, dcTest1);
	obj = dcTest2._makeXpdfObject ();
	dcTest2.getStringRepresentation (tmp);
	ip_validate (obj, e.dcTest2, false);
	ip_validate (obj, tmp);
	xpdf::freeXpdfObject (obj);

	return true;
}

//=====================================================================================

bool
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

	return true;
}


//=====================================================================================

bool
c_set ()
{

	CArray a;
	CDict d;

	CInt i1 (1);	CInt i2 (2);	CInt i3 (3);
	CString s1 ("s1");	CString s2 ("s2");	CString s3 ("s3");

	a.addProperty (i1);			a.addProperty (i2);			a.addProperty (i3);
	ip_validate (a,"[ 1 2 3 ]");
	
	a.setProperty (0,s1);	a.setProperty (1,s2);	a.setProperty (2,s3);
	ip_validate (a,"[ (s1) (s2) (s3) ]");

	d.addProperty ("1", i1);		d.addProperty ("2", i2);		d.addProperty ("3", i3);
	ip_validate (d,"<<\n/1 1\n/2 2\n/3 3\n>>", false);
	
	d.setProperty ("1",s1);	d.setProperty ("2",s2);	d.setProperty ("3",s3);
	ip_validate (d,"<<\n/1 (s1)\n/2 (s2)\n/3 (s3)\n>>", false);

	return true;
}

//=====================================================================================

bool
c_xpdfctor (const char* filename)
{
	{
		//
		// Array
		//
		Object obj;
		obj.initArray ((XRef*)NULL);

		Object item1; item1.initBool	(gFalse);
		Object item2; item2.initBool	(gTrue);
		Object item3; item3.initInt	(-43);
		Object item4; item4.initString	(new GString ("item4"));

		obj.arrayAdd (&item1);
		obj.arrayAdd (&item2);
		obj.arrayAdd (&item3);
		obj.arrayAdd (&item4);
		
		boost::shared_ptr<CPdf> pdf = getTestCPdf (filename);
		IndiRef ref (12,11);
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

		Object item1; item1.initBool	(gFalse);
		Object item2; item2.initBool	(gTrue);
		Object item3; item3.initInt	(-43);
		Object item4; item4.initString	(new GString ("item4"));

		char* it1 = strdup ("lajno1");
		char* it2 = strdup ("lajno2");
		char* it3 = strdup ("lajno3");
		char* it4 = strdup ("lajno4");
		obj.dictAdd (it1, &item1);
		obj.dictAdd (it2, &item2);
		obj.dictAdd (it3, &item3);
		obj.dictAdd (it4, &item4);

		boost::shared_ptr<CPdf> pdf = getTestCPdf (filename);

		IndiRef ref (12,11);
		// CArray ar (pdf,obj,ref); // GOOOOOD -- assertation failed
		CDict dc (pdf,obj,ref); 
		
		ip_validate (obj,dc);
		ip_validate (dc, "<<\n/lajno1 false\n/lajno2 true\n/lajno3 -43\n/lajno4 (item4)\n>>");
		
		obj.free ();
	}

	return true;
	
}

//=====================================================================================

bool
c_addprop2 ()
{
	CDict dict;
	CInt i (1);
	CInt ii (11);

	dict.addProperty ("1", i);
	boost::shared_ptr<IProperty> b = dict.addProperty ("2", ii);
	dict.addProperty ("3", i);
	dict.addProperty ("4", ii);

	
	CArray a;

	a.addProperty (i);
	boost::shared_ptr<IProperty> bb = a.addProperty (ii);
	a.addProperty (i);
	a.addProperty (ii);
	ip_validate (a,"[ 1 11 1 11 ]");


	CString s ("new");
	a.addProperty (3,s);
	a.addProperty (0,s);
	a.addProperty (6,s);
	ip_validate (a,"[ (new) 1 11 1 (new) 11 (new) ]");
	
	bool exc = false;
	try
	{
		a.addProperty (16,s);
	}catch(...)
	{
		exc = true;
	}
	if (!exc)
		return false;

	return true;
}

//=====================================================================================
namespace{
	struct printer {
		ostream* oss;
		printer (ostream* os) : oss (os) {};
		void operator() (std::pair<std::string, boost::shared_ptr<IProperty> > pair) 
		{
			assert (hasValidRef (pair.second));
			assert (hasValidPdf (pair.second));
			//*oss << "Valid pdf: " << endl;
			//*oss << (pair.second)->getIndiRef ();
		};
	};
}

bool
c_forEach (std::ostream& oss, const char* filename)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (filename);
	if (0 < pdf->getPageCount())
	{
		boost::shared_ptr<CPage> page = pdf->getPage (1);
		printer p (&oss);
		page->getDictionary()->forEach (p);
	} 
	
	return true;
}

//=========================================================================
// class TestCObjectComplex
//=========================================================================

class TestCObjectComplex : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCObjectComplex);
		CPPUNIT_TEST(TestGet);
		CPPUNIT_TEST(TestSet);
		CPPUNIT_TEST(TestForEach);
	CPPUNIT_TEST_SUITE_END();

private:
	example e;
	
public:
	void setUp() 
	{
		OUTPUT << endl;
		//
		// Complex
		//
		e.arTest1 = "[ (item1) (item2) (item3) 1 2 3 4 3 4 R false ]";
		e.dcTest1 = "<<\n/item1 (item1)\n/item2 (item2)\n/item3 (item3)"
				"\n/item4 1\n/item5 2\n/item6 "
				    "3\n/item7 4\n/item8 3 4 R\n/item9 false\n>>";
		const string ar2 ("[ "	+ e.arTest1 + " " + e.arTest1 + " " 
			+ e.arTest1 + " " + e.dcTest1 + " " + e.dcTest1 + " " + e.dcTest1 + " ]");
		const string dc2 ("<<\n/1 "	+ e.arTest1 + "\n/2 " + e.arTest1 + "\n/3 " + e.arTest1 + "\n/4 " 
			+ e.dcTest1 + "\n/5 " + e.dcTest1 + "\n/6 " + e.dcTest1 + "\n>>");
		e.arTest2 = ar2;
		e.dcTest2 = dc2;
	}
	
	void tearDown() {}

public:
	void TestGet()
	{
		OUTPUT << "CObjectComplex getter methods..." << endl;

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" getType")
			CPPUNIT_ASSERT (c_getTp ());
			OK_TEST;

			TEST(" clone")
			CPPUNIT_ASSERT (c_clone ());
			OK_TEST;

			TEST(" smart ptrs")
			CPPUNIT_ASSERT (c_smrt ());
			OK_TEST;

			TEST(" mode controller")
			CPPUNIT_ASSERT (mdctrl ((*it).c_str()));
			OK_TEST;

			TEST(" getStringRepre")
			CPPUNIT_ASSERT (c_getString (e));
			OK_TEST;

			TEST(" getPropCount")
			CPPUNIT_ASSERT (c_getCnt ());
			OK_TEST;

			TEST(" getAllPropNames")
			CPPUNIT_ASSERT (c_getNames ());
			OK_TEST;

			TEST(" getType")
			CPPUNIT_ASSERT (c_getType ());
			OK_TEST;

			TEST(" makeXpdf")
			CPPUNIT_ASSERT (c_xpdf (e));
			OK_TEST;

			TEST(" xpdf ctors")
			CPPUNIT_ASSERT (c_xpdfctor ((*it).c_str()));
			OK_TEST;
		}
	}

	void TestSet()
	{
		OUTPUT << "CObjectComplex setter methods..." << endl;

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			//======================= CObjectComplex
			
			TEST(" delProp")
			CPPUNIT_ASSERT (c_del ());
			OK_TEST;

			TEST(" setProp")
			CPPUNIT_ASSERT (c_set ());
			OK_TEST;

			TEST(" xpdf addProperty + getPosition")
			CPPUNIT_ASSERT (c_addprop2 ());
			OK_TEST;
		}
	}
	void TestForEach ()
	{
		OUTPUT << "CObjectComplex forEach method..." << endl;

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			//======================= CObjectComplex
			
			TEST(" forEach")
			CPPUNIT_ASSERT (c_forEach (OUTPUT, (*it).c_str()));
			OK_TEST;
		}

	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCObjectComplex);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCObjectComplex, "TEST_COBJECTCOMPLEX");

//=====================================================================================
} // namespace
//=====================================================================================
