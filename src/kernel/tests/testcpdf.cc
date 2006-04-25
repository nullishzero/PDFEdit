// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.11  2006/04/25 02:28:05  misuj1am
 *
 * -- improved cpage, cstream, tests
 *
 * Revision 1.10  2006/04/23 23:20:40  misuj1am
 *
 * -- improved: zero page (incorrect) document handling
 *
 * Revision 1.9  2006/04/23 22:09:48  hockm0bm
 * * pageIterationTC finished
 *         - all tests are ok
 * * pageManipulationTC
 *         - almost finised (interNode removing is TODO)
 *         - all tests until now are ok
 * * revisionsTC
 *         - just skeleton
 *
 * Revision 1.8  2006/04/23 11:16:48  hockm0bm
 * pageManipulationTC added
 *         - insertPage, removePage test cases
 *
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
#include "../factories.h"
#include "../cobjecthelpers.h"

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
	using namespace utils;

		printf("%s\n", __FUNCTION__);
		
		// getPage and getPagePosition must match for all pages
		printf("TC01:\tPageCount, getPage, getPagePosition\n");
		size_t pageCount=pdf->getPageCount();
		if (0 == pageCount)
			return;
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
		
		printf("TC03:\tgetPage, getFirstPage and getLastPage test\n");
		if(pdf->getPageCount())
		{
			CPPUNIT_ASSERT(pdf->getPage(1)==pdf->getFirstPage());
			CPPUNIT_ASSERT(pdf->getPage(pdf->getPageCount())==pdf->getLastPage());
		}

		// out of range page positions must throw
		printf("TC04:\tgetPage, getNextPage, getPrevPage out of range test\n");

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
			CPPUNIT_FAIL("getPrevPage(getFirstPage()) should have failed");
		}catch(PageNotFoundException &e)
		{
			// ok, exception has been thrown
		}
		
		// getNextPage(getLastPage()) is out of range
		try
		{
			pdf->getNextPage(pdf->getLastPage());
			CPPUNIT_FAIL("getNextPage(getLastPage()) should have failed");
		}catch(PageNotFoundException &e)
		{
			// ok, exception has been thrown
		}

		printf("TC05:\thasNextPage, hasPrevPage test\n");

		if(pdf->getPageCount())
		{
			// first page hasPrevPage should return false
			CPPUNIT_ASSERT(!pdf->hasPrevPage(pdf->getFirstPage()));
			// last page hasNextPage should return false
			CPPUNIT_ASSERT(!pdf->hasNextPage(pdf->getLastPage()));
			// hasNextPage and hasPrevPage should return true for all other 
			// pages
			for(size_t i=2; i<pageCount; i++)
			{
				CPPUNIT_ASSERT(pdf->hasPrevPage(pdf->getPage(i)));
				CPPUNIT_ASSERT(pdf->hasNextPage(pdf->getPage(i)));
			}
		}

		printf("TC06:\tgetPagePosition, getNextPage, getPrevPage with fake page parameter test\n");
		// page from empty page dictionary - no pdf specified inside
		/* TODO uncoment when ready
		shared_ptr<CDict> fakeDict1(CDictFactory::getInstance());
		shared_ptr<CPage> fake1(new CPage(fakeDict1));
		Object fakeXpdfDict;
		XRef * fakeXref=pdf->getCXref();
		fakeXpdfDict.initDict(fakeXref);
		IndiRef fakeIndiRef={10, 0};
		shared_ptr<CDict> fakeDict2(CDictFactory::getInstance(*pdf, fakeIndiRef, fakeXpdfDict));
		shared_ptr<CPage> fake2(new CPage(fakeDict2));

		// getPagePosition should fail on both fakes
		try
		{
			pdf->getPagePosition(fake1);
			CPPUNIT_FAIL("getPagePosition(fake1) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}
		try
		{
			pdf->getPagePosition(fake2);
			CPPUNIT_FAIL("getPagePosition(fake2) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}

		// getPrevPage should fail on both fakes
		try
		{
			pdf->getPrevPage(fake1);
			CPPUNIT_FAIL("getPrevPage(fake1) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}
		try
		{
			pdf->getPrevPage(fake2);
			CPPUNIT_FAIL("getPrevPage(fake2) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}

		// getNextPage should fail on both fakes
		try
		{
			pdf->getNextPage(fake1);
			CPPUNIT_FAIL("getNextPage(fake1) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}
		try
		{
			pdf->getNextPage(fake2);
			CPPUNIT_FAIL("getNextPage(fake2) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}
		*/
	}

	void pageManipulationTC(CPdf * pdf)
	{
	using namespace boost;
	using namespace utils;

		printf("%s\n", __FUNCTION__);

		printf("TC01:\tremovePage, insertPage changes pageCount\n");
		size_t pageCount=pdf->getPageCount();
		if (0 == pageCount)
			return;
		shared_ptr<CPage> page=pdf->getPage(1);
		// remove page implies pageCount decrementation test
		pdf->removePage(1);
		//pdf->save();

		CPPUNIT_ASSERT(pageCount-1==pdf->getPageCount());
		// insert page implies pageCount incrementation test
		// and newPage must be defferent instance than original one
		shared_ptr<CPage> newPage=pdf->insertPage(page, 1);
		CPPUNIT_ASSERT(pageCount==pdf->getPageCount());

		printf("TC02:\tinsertPage returns different instance than parameter\n");
		CPPUNIT_ASSERT(page!=newPage);

		printf("TC03:\tremovePage out of range test\n");
		// remove from 0 page should fail
		try
		{
			pdf->removePage(0);
			CPPUNIT_FAIL("removePage should have failed");
		}catch(PageNotFoundException & e)
		{
			// everything ok
		}
		// remove from pageCount()+1 should fail
		try
		{
			pdf->removePage(pdf->getPageCount()+1);
			CPPUNIT_FAIL("removePage should have failed");
		}catch(PageNotFoundException & e)
		{
			// everything ok
		}

		printf("TC04:\tremoved page is no longer available test\n");
		// try to remove last page
		shared_ptr<CPage> removedPage=pdf->getLastPage();
		pdf->removePage(pdf->getPageCount());
		// FIXME uncoment when ready
		//CPPUNIT_ASSERT(! removePage->isValid());
		try
		{
			pdf->getPagePosition(removedPage);
			CPPUNIT_FAIL("getPagePosition on removed page should have failed.");
		}catch(PageNotFoundException & e)
		{
			/* ok, it should fail */
		}
		// restore to previous state and returns back last page
		pdf->insertPage(removedPage, pdf->getPageCount()+1);

		printf("TC05:\tremoving page's dictionary invalidates page\n");
		page=pdf->getPage(1);
		shared_ptr<CDict> pageDict=page->getDictionary();
		shared_ptr<CRef> pageRef(CRefFactory::getInstance(pageDict->getIndiRef()));
		// gets parent of page dictionary and removes reference of this page
		// from Kids array
		shared_ptr<CRef> parentRef=IProperty::getSmartCObjectPtr<CRef>(pageDict->getProperty("Parent"));
		shared_ptr<CDict> parentDict=IProperty::getSmartCObjectPtr<CDict>(
				pdf->getIndirectProperty(getValueFromSimple<CRef, pRef, IndiRef>(parentRef)));
		shared_ptr<CArray> kidsArray=IProperty::getSmartCObjectPtr<CArray>(
				parentDict->getProperty("Kids"));
		vector<CArray::PropertyId> positions;
		getPropertyId<CArray, vector<CArray::PropertyId> >(kidsArray, pageRef, positions);
		CPPUNIT_ASSERT(positions.size()>0);
		kidsArray->delProperty(positions[0]);
		// FIXME uncoment when ready
		//CPPUNIT_ASSERT(! page->isValid());
		try
		{
			pdf->getPagePosition(page);
			CPPUNIT_FAIL("getPagePosition on removed page should have failed");
		}catch(PageNotFoundException & e)
		{
			/* ok, it should fail */
		}
		
		printf("TC06:\tremovig inter node removes all pages under\n");
		// Uses 1st page parent from previous test case
		// paretDict has to be different from root of page tree
		PropertyEquals pe;
		if(pe(pdf->getDictionary()->getProperty("Pages"), parentDict))
			printf("\t\tnot suitable test input data\n");
		else
		{
			// collects all pages under this node
			// gets parent's parent and removes parent from kids array
			// all collected pages should be invalidated
			// TODO
		}
	}

/** Multiversion file name. */
#define MV_F "multiversion.pdf"

/** Multiversion revisin count. */
#define MV_RC  2

	void revisionsTC()
	{
		printf("%s\n", __FUNCTION__);

		// opens special test file
		CPdf *pdf=getTestCPdf(MV_F);

		// number of revision must match
		printf("TC01:\tRevisions count test\n");
		CPPUNIT_ASSERT(pdf->getRevisionsCount()==MV_RC);
		pdf->close();

		printf("TC02:\tchangeRevision tests\n");
		for(CPdf::revision_t i=1; i<pdf->getRevisionsCount(); i++)
			pdf->changeRevision(i);
		// TODO prepare test data with multiple revision each
		// specific by pagecount
		
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
			pageManipulationTC(*i);
			revisionsTC();
		}
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestCPdf);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCPdf, "TEST_CPDF");

pdfobjects::CPdf * getTestCPdf(const char* filename)
{
	return CPdf::getInstance(filename, CPdf::Advanced);
}
