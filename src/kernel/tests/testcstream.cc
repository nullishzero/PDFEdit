// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  testcstream.cc
 *         Created:  04/24/2006 02:57:27 AM CEST
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

using namespace boost;

//=========================================================================
// class TestCStream
//=========================================================================

class TestCStream : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCStream);
		CPPUNIT_TEST(Test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {OUTPUT << endl;}
	void tearDown() {}

public:
	void Test ()
	{
		OUTPUT << "CStream methods..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" display");
			CPPUNIT_ASSERT (true);
			OK_TEST;
		}
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCStream, "TEST_CSTREAM");

//=====================================================================================
} // namespace
//=====================================================================================








