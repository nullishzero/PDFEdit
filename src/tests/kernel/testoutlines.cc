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
#include "kernel/cpdf.h"
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
getout (UNUSED_PARAM ostream& UNUSED_PARAM oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

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








