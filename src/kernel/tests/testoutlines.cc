// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  testioutlines.cc
 *         Created:  06/02/2006 02:30:27 AM CEST
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

#include "testmain.h"
#include "testcobject.h"
#include "testcpage.h"
#include "testcpdf.h"

#include "factories.h"

#include <PDFDoc.h>
#include "../cpage.h"


//=====================================================================================
namespace {
//=====================================================================================

bool
getout (__attribute__((unused)) ostream& __attribute__((unused)) oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());

		boost::shared_ptr<CDict> toplevel;
		try {
			toplevel = utils::getCDictFromDict (pdf->getDictionary(), "Outlines");
		}catch (ElementNotFoundException&)
			{oss << "No outlines" << flush; return true; }

		oss << "Outlines entry exists. " << flush;
		
		typedef vector<boost::shared_ptr<IProperty> > Outs;
		Outs outs;
		assert (toplevel);
		utils::getAllChildrenOfPdfObject (toplevel, outs);
		
		oss << "Outlines found: " << flush;

		assert (!outs.empty());
		// skip top level entry
		for (size_t i = 1; i < outs.size(); ++i)
		{
			std::string title = utils::getStringFromDict (outs[i], "Title");
			oss << "-" << title << flush;
		}
			

	return true;
}


//=========================================================================
// class TestOutline
//=========================================================================

class TestCOutline : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCOutline);
		CPPUNIT_TEST(TestGetOutline);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {OUTPUT << endl;}
	void tearDown() {}

public:
	//
	//
	//
	void TestGetOutline ()
	{
		OUTPUT << "Outlines..." << endl;

		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
		
			TEST(" get outlines");
			CPPUNIT_ASSERT (getout (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCOutline);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCOutline, "TEST_COUTLINE");

//=====================================================================================
} // namespace
//=====================================================================================








