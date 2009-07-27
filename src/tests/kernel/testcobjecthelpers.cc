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
#include "kernel/cobject.h"
#include "kernel/cobjecthelpers.h"
#include "tests/kernel/testmain.h"
#include "kernel/cobjecthelpers.h"

namespace {

bool
test_simple ()
{
//	std::string str;
//	pdfobjects::utils::simpleValueToString<pReal> (-0.012, str);
//	CPPUNIT_ASSERT_EQUAL(string("-0.012000"), str);
//	pdfobjects::utils::simpleValueToString<pReal> (-0.0004, str);
//	CPPUNIT_ASSERT_EQUAL(string("-0.000400"), str);
//	pdfobjects::utils::simpleValueToString<pReal> (-0.0000004, str);
//	CPPUNIT_ASSERT_EQUAL(string("-0.000000"), str);
	return true;
}

/** Checks functionality od IdCollector class.
 */
void idCollectorTC()
{
	typedef std::vector<boost::shared_ptr<IProperty> > Storage;
	typedef std::vector<size_t> Container;
	typedef std::pair<size_t, const boost::shared_ptr<IProperty> > Entry;
	
	Storage array;
	array.push_back(boost::shared_ptr<IProperty>(new CInt(1)));
	array.push_back(boost::shared_ptr<IProperty>(new CInt(2)));
	array.push_back(boost::shared_ptr<IProperty>(new CInt(1)));

	Container container;
	boost::shared_ptr<CInt> child(new CInt(1));
	//typedef IdCollector<Container, size_t> IdCollectorType;
	//IdCollectorType collector(container, child);
	utils::IdCollector<Container, size_t> collector(container, child);

	size_t pos=0;
	for(Storage::iterator i=array.begin(); i!=array.end(); i++, pos++)
	{
		collector(Entry(pos, *i));
	}

	printf("container size=%u", container.size());
	printf("Elements are:");
	for(Container::iterator i=container.begin(); i!=container.end(); i++)
		printf(" %u", *i);
	printf("\n");
}


} // end of annonym namespace

class TestCObjectHelpers: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestCObjectHelpers);
		//CPPUNIT_TEST(Test);
		CPPUNIT_TEST(TestConversionMethods);
	CPPUNIT_TEST_SUITE_END();
public:
	/*
	void setUp()
	{
	}

	void tearDown()
	{
	}

	*/

	void Test()
	{
		idCollectorTC();
	}
	void TestConversionMethods()
	{
		OUTPUT << "CObjectSimple simpleValueToString methods..." << endl;
		CPPUNIT_ASSERT (test_simple());
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCObjectHelpers);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCObjectHelpers, "TEST_COBJECTHELPERS");
