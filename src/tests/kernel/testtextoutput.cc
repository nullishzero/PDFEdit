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
#include "tests/kernel/testcpage.h"
#include "tests/kernel/testcpdf.h"

#include "kernel/textoutput.h"
#include "kernel/textoutputengines.h"


//=====================================================================================
namespace {
//=====================================================================================
using namespace std;
using namespace boost;
using namespace textoutput;

//=====================================================================================
bool text_cpageout (UNUSED_PARAM std::ostream& oss, 
			   UNUSED_PARAM const char* file_name)
{

	boost::shared_ptr<CPdf> pdf = getTestCPdf (file_name);

	for (size_t i = 0; i < pdf->getPageCount() && i < 1/*TEST_MAX_PAGE_COUNT*/; ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);

		XmlOutputBuilder out;
		page->convert<SimpleWordEngine,
					  SimpleLineEngine,
					  SimpleColumnEngine> (out);
		ofstream of;
		#if TEMP_FILES_CREATE
		of.open ("1.xml");
		of << XmlOutputBuilder::xml(out);
		of.close();
		#endif
	}

	return true;
}


//=========================================================================
// class TestTextOutput
//=========================================================================

class TestTextOutput : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestTextOutput);
		CPPUNIT_TEST(test_cpageout);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {OUTPUT << endl;}
	void tearDown() {}

public:
	//
	//
	//
	void test_cpageout ()
	{
		for (TestParams::FileList::const_iterator it = TestParams::instance().files.begin (); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" text cpage output");
			CPPUNIT_ASSERT (text_cpageout (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTextOutput);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestTextOutput, "TEST_TEXTOUTPUT");

//=====================================================================================
} // namespace
//=====================================================================================








