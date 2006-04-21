// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  testcpage.cc
 *         Created:  04/02/2006 15:47:27 AM CEST
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

#include "testmain.h"
#include "testcobject.h"
#include "testcpage.h"
#include "testcpdf.h"

#include <PDFDoc.h>
#include "../cpage.h"


//=====================================================================================

boost::shared_ptr<CPage> 
getPage (const char* fileName, boost::shared_ptr<CPdf> pdf, size_t pageNum)
{
	boost::scoped_ptr<PDFDoc> doc (new PDFDoc (new GString(fileName), NULL, NULL));
	
	//
	// Our stuff here
	//
	Object obj;
	XRef* xref = doc->getXRef();
	assert (xref);
	Catalog cat (xref);

	IndiRef ref;
	ref.num = cat.getPageRef(pageNum)->num;
	ref.gen = cat.getPageRef(pageNum)->gen;
	xref->fetch (ref.num, ref.gen, &obj);
	
	boost::shared_ptr<CDict> dict (new CDict (*pdf, obj, ref));
	obj.free ();
	
	return boost::shared_ptr<CPage> (new CPage(dict));
}

//=====================================================================================
namespace {
//=====================================================================================

using namespace boost;

//=====================================================================================

bool
mediabox (ostream& /*oss*/, const char* fileName)
{
	boost::scoped_ptr<PDFDoc> doc (new PDFDoc (new GString(fileName), NULL, NULL));
	
	//
	// Our stuff here
	//
	Object obj;
	XRef* xref = doc->getXRef();
	Catalog cat (xref);

	/*oss << "Contents:"		<< */cat.getPage(1)->getContents(&obj) /*<< endl*/;
	obj.free ();
	/*oss << "Page:"			<< */xref->fetch (cat.getPageRef(1)->num, cat.getPageRef(1)->gen, &obj) /*<< endl*/;

	boost::scoped_ptr<CPdf> pdf (getTestCPdf (fileName));
	boost::shared_ptr<CDict> dict (new CDict (*pdf, obj, IndiRef ()));
	obj.free ();
	
	CPage page (dict);

	/*oss <<*/ page.getMediabox ();
	
	Rectangle rc;
	rc.xleft = 42;
	rc.xright = 12;
	rc.yleft = 62;
	rc.yright = 2342;
	page.setMediabox (rc);
	
	if (42 == page.getMediabox ().xleft &&
			62 == page.getMediabox ().yleft)
		return true;
	else
		return false;
}


//=====================================================================================

bool
display (ostream& /*oss*/, const char* fileName)
{
	// Open pdf and get the first page	
	boost::scoped_ptr<CPdf> pdf (getTestCPdf (fileName));
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();

  	//TextOutputDev textOut (NULL, gTrue, gFalse, gTrue);
  	TextOutputDev textOut ("1.txt", gFalse, gFalse, gFalse);
	if (!textOut.isOk ())
		throw;
	
	//
	// Output to file
	//
	//oss << "Creating 1.txt which contains text from a pdf." << endl;
	page->displayPage (textOut);
	
	boost::scoped_ptr<GlobalParams> aGlobPar (new GlobalParams (""));
	GlobalParams* oldGlobPar = globalParams;
	globalParams = aGlobPar.get();

	//oss << "Output from textoutputdevice." << endl;
	//oss << textOut.getText(0, 0, 1000, 1000)->getCString() << endl;
	textOut.getText(0, 0, 1000, 1000)->getCString();

	globalParams = oldGlobPar;

	return true;
}


//=========================================================================
// class TestCPage
//=========================================================================

class TestCPage : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCPage);
		CPPUNIT_TEST(Test);
		CPPUNIT_TEST(TestDisplay);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {}
	void tearDown() {}

public:
	void Test()
	{
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			
			TEST(" test CPage -- features");
			CPPUNIT_ASSERT (mediabox (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	void TestDisplay()
	{
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			
			TEST(" test CPage -- display");
			CPPUNIT_ASSERT (display (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCPage);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCPage, "TEST_CPAGE");

//=====================================================================================
} // namespace
//=====================================================================================








