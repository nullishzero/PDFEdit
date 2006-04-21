// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.1  2006/04/21 20:42:17  hockm0bm
 * start of cobjecthelpers tests
 *
 *
 */

#include "testmain.h"
#include "../cobjecthelpers.h"

namespace {

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
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCObjectHelpers);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCObjectHelpers, "TEST_COBJECTHELPERS");
