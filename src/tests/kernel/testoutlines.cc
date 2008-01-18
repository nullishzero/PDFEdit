/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#include "kernel/static.h"
#include "tests/kernel/testmain.h"
#include "tests/kernel/testcobject.h"
#include "tests/kernel/testcpage.h"
#include "tests/kernel/testcpdf.h"

#include "kernel/factories.h"
#include "kernel/cpage.h"


//=====================================================================================
namespace {
//=====================================================================================
using namespace std;
using namespace boost;
	
bool
getout (__attribute__((unused)) ostream& __attribute__((unused)) oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());

	typedef vector<shared_ptr<IProperty> > Outs;
	Outs outs;
	pdf->getOutlines (outs);

	if (outs.empty())
		oss << " No outlines..." << flush;
	else
		for (Outs::iterator it = outs.begin(); it != outs.end(); ++it)
			oss << "-" << getOutlineText (*it) << flush;
			

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

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
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








