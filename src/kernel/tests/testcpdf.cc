// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.7  2006/04/22 19:46:12  hockm0bm
 * test case Test added to CPPUnit tests of CPdf suite
 *
 * Revision 1.6  2006/04/22 19:32:20  hockm0bm
 * * old test style replaced by CPPUINT TestCPdf class
 * * TestCPdf::pageIterationTC method implemented
 *
 *
 *
 */

#include "testmain.h"
#include "testcpdf.h"

namespace {



} // end of annonym namespace

class TestCPdf: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestCPdf);
		CPPUNIT_TEST(Test);
	CPPUNIT_TEST_SUITE_END();

	typedef std::vector<CPdf *> PdfList;
	PdfList	pdfs;
public:

	virtual ~TestCPdf()
	{
	}

	void pageIterationTC(CPdf * pdf)
	{
	using namespace boost;

		printf("%s\n", __FUNCTION__);
		
		// getPage and getPagePosition must match for all pages
		printf("TC01:\tPageCount, getPage, getPagePosition\n");
		size_t pageCount=pdf->getPageCount();
		for(size_t i=1; i<=pageCount; i++)
		{
			shared_ptr<CPage> page=pdf->getPage(i);
			size_t pos=pdf->getPagePosition(page);
			CPPUNIT_ASSERT(i==pos);
		}

		// getPage, getNextPage, getPrevPage must match for all pages
		printf("TC02:\tgetPage, getNextPage, getPrevPage\n");
		for(size_t i=1; i<=pageCount; i++)
		{
			shared_ptr<CPage> pos=pdf->getPage(i);
			if(i>1)
			{
				shared_ptr<CPage> prev=pdf->getPage(i-1);
				CPPUNIT_ASSERT(pdf->getPrevPage(pos)==prev);
			}
			if(i<pageCount)
			{
				shared_ptr<CPage> next=pdf->getPage(i+1);
				CPPUNIT_ASSERT(pdf->getNextPage(pos)==next);
			}
		}
		
		printf("TC03\tgetPage, getFirstPage and getLastPage test\n");
		CPPUNIT_ASSERT(pdf->getPage(1)==pdf->getFirstPage());
		CPPUNIT_ASSERT(pdf->getPage(pdf->getPageCount())==pdf->getLastPage());

		// out of range page positions must throw
		printf("TC04\tgetPage, getNextPage, getPrevPage out of range test\n");

		// 0 page is out of range
		try
		{
			pdf->getPage(0);
			CPPUNIT_FAIL("getPage(0) should have failed");
		}catch(PageNotFoundException &e)
		{
			// ok, exception has been thrown
		}
		
		// pageCount + 1 is out of range
		try
		{
			pdf->getPage(pdf->getPageCount()+1);
			CPPUNIT_FAIL("getPage(%d) should have filed");
		}catch(PageNotFoundException &e)
		{
			// ok, exception has been thrown
		}
		// getPrevPage(getFirstPage()) is out of range
		try
		{
			pdf->getPrevPage(pdf->getFirstPage());
			CPPUNIT_FAIL("getPrevPage(getFirstPage()) should have filed");
		}catch(PageNotFoundException &e)
		{
			// ok, exception has been thrown
		}
		
		// getNextPage(getLastPage()) is out of range
		try
		{
			pdf->getNextPage(pdf->getLastPage());
			CPPUNIT_FAIL("getNextPage(getLastPage()) should have filed");
		}catch(PageNotFoundException &e)
		{
			// ok, exception has been thrown
		}

		printf("TC05\thasNextPage, hasPrevPage test\n");

		// first page hasPrevPage should return false
		CPPUNIT_ASSERT(!pdf->hasPrevPage(pdf->getFirstPage()));
		// last page hasNextPage should return false
		CPPUNIT_ASSERT(!pdf->hasNextPage(pdf->getLastPage()));
		// hasNextPage and hasPrevPage should return true for all other pages
		for(size_t i=2; i<pageCount; i++)
		{
			CPPUNIT_ASSERT(pdf->hasPrevPage(pdf->getPage(i)));
			CPPUNIT_ASSERT(pdf->hasNextPage(pdf->getPage(i)));
		}
	}
	
	void setUp()
	{
		// creates pdf instances for all files
		for(FileList::iterator i=fileList.begin(); i!=fileList.end(); i++)
		{
			try
			{
				CPdf * pdf=getTestCPdf(i->c_str());
				pdfs.push_back(pdf);
			}catch(PdfOpenException &e)
			{
				printf("file \"%s\" open failed. msg=", e.what());
			}
		}

	}

	void tearDown()
	{
		// closes all opened pdfs and removes them all from pdfs list
		// to prepare this test for reuse
		for(PdfList::iterator i=pdfs.begin(); i!=pdfs.end(); i++)
			(*i)->close();
		pdfs.clear();
	}

	void Test()
	{
		for(PdfList::iterator i=pdfs.begin(); i!=pdfs.end(); i++)
		{
			// all tests for page itaration
			pageIterationTC(*i);
		}
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestCPdf);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCPdf, "TEST_CPDF");

pdfobjects::CPdf * getTestCPdf(const char* filename)
{
	return CPdf::getInstance(filename, CPdf::Advanced);
}
