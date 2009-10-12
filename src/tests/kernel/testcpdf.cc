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
#include "tests/kernel/testcpdf.h"
#include "kernel/factories.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/cpdf.h"
#include "kernel/pdfwriter.h"
#include "kernel/delinearizator.h"

using namespace pdfobjects;
using namespace utils;
using namespace observer;
using namespace boost;

class ProgressBar:public IProgressBar
{
	int maxStep;
	ostream & out;
	int displayStep;
	float lastPerc;
public:
	ProgressBar(ostream & str, int step=10):out(str), displayStep(step)
	{
	}
	
	virtual ~ProgressBar(){}

	virtual void start()
	{
		out << "Starting progress" << endl;
		lastPerc=0;
	}

	virtual void finish()
	{
		out << "Progress ended" << endl;
	}

	virtual void update(int step)
	{
		float perc=(float)(100*step)/(float)maxStep;
		if(perc>=lastPerc+displayStep)
		{
			out << perc << "% done" <<endl;
			lastPerc=perc;
		}
	}

	virtual void setMaxStep(int step)
	{
		maxStep=step;
	}
};

class TestCPdf: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestCPdf);
		CPPUNIT_TEST(Test);
	CPPUNIT_TEST_SUITE_END();

public:

	virtual ~TestCPdf()
	{
	}

	void cloneTC(boost::shared_ptr<CPdf> pdf, string & originalFile)
	{
		printf("%s\n", __FUNCTION__);

		if(pdf->isLinearized())
		{
			printf("%s is not suitable for this test, because file is linearized\n", originalFile.c_str());
			return;
		}
	
		char suffix[13];
		memset(suffix, '\0', sizeof(suffix));
		snprintf(suffix, sizeof(suffix)-1, "%u_clone.pdf", 0);
		string file=originalFile+suffix;
		printf("TC01:\tCloning of first (the newest one) revision. Output file=%s\n", file.c_str());
		FILE * cloneFile=fopen(file.c_str(), "wb");
		pdf->clone(cloneFile);
		fclose(cloneFile);
		#if TEMP_FILES_CREATE
		#else
			remove (file.c_str());
		#endif
		
		printf("TC02:\tCloning of later revisions.\n");
		int count;
		for(CPdf::revision_t rev=pdf->getRevisionsCount()-1, count=pdf->getRevisionsCount(); 
				count>0; rev--,count--)
		{
			try
			{
				printf("\t\t%d. revision clone. Output file=%s\n", rev, file.c_str());
				pdf->changeRevision(rev);
				snprintf(suffix, sizeof(suffix)-1, "%u_clone.pdf", rev);
				file=originalFile+suffix;
				cloneFile=fopen(file.c_str(), "wb");
				pdf->clone(cloneFile);
				fclose(cloneFile);
				#if TEMP_FILES_CREATE
				#else
					remove (file.c_str());
				#endif
			}catch(NotImplementedException & e)
			{
				printf("\t\tData not suitable for this test. Revision changing is not supported.\n");
			}
		}
		// gets back to the newest revision
		pdf->changeRevision(pdf->getRevisionsCount()-1);

		printf("TC03:\t cloning doesn't change content test\n");
		CPPUNIT_ASSERT(!pdf->isChanged());
	}
	
	void pageIterationTC(boost::shared_ptr<CPdf> pdf)
	{
	using namespace boost;
	using namespace utils;

		printf("%s\n", __FUNCTION__);
		
		CPPUNIT_ASSERT(!pdf->isChanged());
		// getPage and getPagePosition must match for all pages
		printf("TC01:\tPageCount, getPage, getPagePosition\n");
		size_t pageCount=pdf->getPageCount();
		CPPUNIT_ASSERT(!pdf->isChanged());
		if (0 == pageCount)
			return;
		for(size_t i=1; i<=pageCount; i++)
		{
			shared_ptr<CPage> page=pdf->getPage(i);
			CPPUNIT_ASSERT(!pdf->isChanged());
			size_t pos=pdf->getPagePosition(page);
			CPPUNIT_ASSERT(!pdf->isChanged());
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
			CPPUNIT_ASSERT(!pdf->isChanged());
		}
		
		printf("TC03:\tgetPage, getFirstPage and getLastPage test\n");
		if(pdf->getPageCount())
		{
			CPPUNIT_ASSERT(pdf->getPage(1)==pdf->getFirstPage());
			CPPUNIT_ASSERT(pdf->getPage(pdf->getPageCount())==pdf->getLastPage());
		}
		CPPUNIT_ASSERT(!pdf->isChanged());

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
		CPPUNIT_ASSERT(!pdf->isChanged());
		
		// pageCount + 1 is out of range
		try
		{
			pdf->getPage(pdf->getPageCount()+1);
			CPPUNIT_FAIL("getPage(%d) should have filed");
		}catch(PageNotFoundException &e)
		{
			// ok, exception has been thrown
		}
		CPPUNIT_ASSERT(!pdf->isChanged());
		// getPrevPage(getFirstPage()) is out of range
		try
		{
			pdf->getPrevPage(pdf->getFirstPage());
			CPPUNIT_FAIL("getPrevPage(getFirstPage()) should have failed");
		}catch(PageNotFoundException &e)
		{
			// ok, exception has been thrown
		}
		CPPUNIT_ASSERT(!pdf->isChanged());
		
		// getNextPage(getLastPage()) is out of range
		try
		{
			pdf->getNextPage(pdf->getLastPage());
			CPPUNIT_FAIL("getNextPage(getLastPage()) should have failed");
		}catch(PageNotFoundException &e)
		{
			// ok, exception has been thrown
		}
		CPPUNIT_ASSERT(!pdf->isChanged());

		printf("TC05:\thasNextPage, hasPrevPage test\n");

		if(pdf->getPageCount())
		{
			// first page hasPrevPage should return false
			CPPUNIT_ASSERT(!pdf->hasPrevPage(pdf->getFirstPage()));
			CPPUNIT_ASSERT(!pdf->isChanged());
			// last page hasNextPage should return false
			CPPUNIT_ASSERT(!pdf->hasNextPage(pdf->getLastPage()));
			CPPUNIT_ASSERT(!pdf->isChanged());
			// hasNextPage and hasPrevPage should return true for all other 
			// pages
			for(size_t i=2; i<pageCount; i++)
			{
				CPPUNIT_ASSERT(pdf->hasPrevPage(pdf->getPage(i)));
				CPPUNIT_ASSERT(!pdf->isChanged());
				CPPUNIT_ASSERT(pdf->hasNextPage(pdf->getPage(i)));
				CPPUNIT_ASSERT(!pdf->isChanged());
			}
		}

		printf("TC06:\tgetPagePosition, getNextPage, getPrevPage with fake page parameter test\n");
		// page from empty page dictionary - no pdf specified inside
		shared_ptr<CDict> fakeDict1(CDictFactory::getInstance());
		shared_ptr<CPage> fake1(new CPage(fakeDict1));
		Object fakeXpdfDict;
		XRef * fakeXref=pdf->getCXref();
		fakeXpdfDict.initDict(fakeXref);
		IndiRef fakeIndiRef(10, 0);
		shared_ptr<CDict> fakeDict2(CDictFactory::getInstance(pdf, fakeIndiRef, fakeXpdfDict));
		shared_ptr<CPage> fake2(new CPage(fakeDict2));
		fakeXpdfDict.free();

		// getPagePosition should fail on both fakes
		try
		{
			pdf->getPagePosition(fake1);
			CPPUNIT_FAIL("getPagePosition(fake1) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}
		CPPUNIT_ASSERT(!pdf->isChanged());
		try
		{
			pdf->getPagePosition(fake2);
			CPPUNIT_FAIL("getPagePosition(fake2) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}
		CPPUNIT_ASSERT(!pdf->isChanged());

		// getPrevPage should fail on both fakes
		try
		{
			pdf->getPrevPage(fake1);
			CPPUNIT_FAIL("getPrevPage(fake1) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}
		CPPUNIT_ASSERT(!pdf->isChanged());
		try
		{
			pdf->getPrevPage(fake2);
			CPPUNIT_FAIL("getPrevPage(fake2) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}
		CPPUNIT_ASSERT(!pdf->isChanged());

		// getNextPage should fail on both fakes
		try
		{
			pdf->getNextPage(fake1);
			CPPUNIT_FAIL("getNextPage(fake1) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}
		CPPUNIT_ASSERT(!pdf->isChanged());
		try
		{
			pdf->getNextPage(fake2);
			CPPUNIT_FAIL("getNextPage(fake2) should have failed");
		}catch(PageNotFoundException & e)
		{
			// ok, exception has been thrown
		}

		// no change made to document
		CPPUNIT_ASSERT(!pdf->isChanged());
	}

	void pageManipulationTC(boost::shared_ptr<CPdf> pdf)
	{
	using namespace boost;
	using namespace utils;

		printf("%s\n", __FUNCTION__);
		if(pdf->isLinearized())
		{
			printf("Usecase is not suitable becuase document is linearized\n");
			return;
		}

		size_t pageCount=pdf->getPageCount();
		if (0 == pageCount)
		{
			printf("TC01:\tinsertPage to an empty document results in 1 total pages\n");
			if (0 < TestParams::instance().files.size())
			{
				boost::shared_ptr<CPdf> test_doc = getTestCPdf(TestParams::instance().files.front().c_str());
				pdf->insertPage(test_doc->getFirstPage(), 1);
				CPPUNIT_ASSERT(pdf->getPageCount()==1);
			}
			return;
		}

		printf("TC01:\tremovePage, insertPage changes pageCount\n");
		shared_ptr<CPage> page=pdf->getPage(1);
		// remove page implies pageCount decrementation test
		pdf->removePage(1);
		CPPUNIT_ASSERT(pageCount-1==pdf->getPageCount());

		// insert page implies pageCount incrementation
		shared_ptr<CPage> newPage=pdf->insertPage(page, 1);
		CPPUNIT_ASSERT(pageCount==pdf->getPageCount());

		// page count is same as in original file now

		printf("TC02:\tremovePage out of range test\n");
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

		printf("TC03:\tinsert of already present page should fail\n");
		try
		{
			shared_ptr<CPage> addedPage=pdf->insertPage(pdf->getFirstPage(), 1);
			CPPUNIT_FAIL("Insert of already present page should have failed");
		}catch(AmbiguousPageTreeException &)
		{
			// everything ok
		}

		printf("TC04:\tremoved page is no longer available test\n");
		// try to remove last page
		shared_ptr<CPage> removedPage=pdf->getLastPage();
		IndiRef removedPageRef=removedPage->getDictionary()->getIndiRef();
		size_t pos=pdf->getPagePosition(removedPage);
		CPPUNIT_ASSERT(pos==pdf->getPageCount());
		pdf->removePage(pdf->getPageCount());
		CPPUNIT_ASSERT(! removedPage->isValid());
		try
		{
			pos=pdf->getPagePosition(removedPage);
			CPPUNIT_FAIL("getPagePosition on removed page should have failed.");
		}catch(PageNotFoundException & e)
		{
			/* ok, it should have fail */
		}
		// restore to previous state
		pdf->insertPage(removedPage, pos);
		
		printf("TC05:\tremoving page's dictionary invalidates page\n");
		pos=pdf->getPageCount()/2;
		// if we have just one page, use it
		if(!pos)
			pos=1;
		page=pdf->getPage(pos);
		shared_ptr<CDict> pageDict=page->getDictionary();
		shared_ptr<CRef> pageRef(CRefFactory::getInstance(pageDict->getIndiRef()));
		// gets parent of page dictionary and removes reference of this page
		// from Kids array
		shared_ptr<CRef> parentRef=IProperty::getSmartCObjectPtr<CRef>(pageDict->getProperty("Parent"));
		IndiRef ref = getValueFromSimple<CRef>(parentRef);
		shared_ptr<CDict> parentDict=IProperty::getSmartCObjectPtr<CDict>(
				pdf->getIndirectProperty(ref));
		shared_ptr<CArray> kidsArray=IProperty::getSmartCObjectPtr<CArray>(
				parentDict->getProperty("Kids"));
		vector<CArray::PropertyId> positions;
		getPropertyId<CArray, vector<CArray::PropertyId> >(kidsArray, pageRef, positions);
		CPPUNIT_ASSERT(positions.size()>0);
		// we know that position is not ambiguous so use 1st in positions array
		kidsArray->delProperty(positions[0]);
		CPPUNIT_ASSERT(! page->isValid());
		try
		{
			pdf->getPagePosition(page);
			CPPUNIT_FAIL("getPagePosition on removed page should have failed");
		}catch(PageNotFoundException & e)
		{
			/* ok, it should fail */
		}
		// inserts back removed page
		pdf->insertPage(page, pos);
		
		// uses 1st internode kid from page tree root dictionary
		shared_ptr<CDict> interNode;
		shared_ptr<CRef> interNodeCRef;
		shared_ptr<CDict> rootDict=getPageTreeRoot(pdf);
		shared_ptr<IProperty> rootKidsProp=rootDict->getProperty("Kids");
		shared_ptr<CArray> rootKids;
		if(isRef(rootKidsProp))
		{
			try
			{
				rootKids=getCObjectFromRef<CArray>(rootKidsProp);
			}catch(CObjectException & e)
			{
			}
		}
		else
			if(isArray(rootKidsProp))
				rootKids=IProperty::getSmartCObjectPtr<CArray>(rootKidsProp);

		size_t interPos=0;
		if(!rootKids.get())
		{
			printf("\t\tKids array has bad type, ignoring");
		}else
		{
			vector<shared_ptr<IProperty> > children;
			rootKids->_getAllChildObjects(children);
			size_t index=0;
			for(vector<shared_ptr<IProperty> >::iterator i=children.begin(); i!=children.end(); i++, index++)
			{
				shared_ptr<IProperty> child=*i;
				if(getNodeType(child)>=InterNode)
				{
					interNode=getCObjectFromRef<CDict>(child);
					interNodeCRef=IProperty::getSmartCObjectPtr<CRef>(child);
					interPos=index;
					break;
				}
			}
		}
		// creates new intermediate node with no children just for
		// simulation
		shared_ptr<CDict> fakeInterNode(CDictFactory::getInstance());
		shared_ptr<CName> type(CNameFactory::getInstance("Pages"));
		fakeInterNode->addProperty("Type", *type);
		IndiRef fakeInterRef=pdf->addIndirectProperty(fakeInterNode);
		size_t fakeInterLeafCount=0;

		if(!interNode)
			// no internode available
			printf("\t\tThis file is not suitable for this test\n");
		else
		{
			// collects all descendants from interNode
			vector<shared_ptr<CPage> > descendants;
			size_t pageCount=pdf->getPageCount();
			for(size_t i=1; i<=pageCount; i++)
			{
				shared_ptr<CPage> page=pdf->getPage(i);	
				if(isNodeDescendant(pdf, interNode->getIndiRef(), page->getDictionary()))
					descendants.push_back(page);
			}

			// all descendants are collected, we can remove interNode reference
			// from rootKids
			printf("TC06:\tremovig inter node removes all pages under\n");
			rootKids->delProperty(interPos);

			// page count has to be decreased by descendants.size()
			CPPUNIT_ASSERT(pdf->getPageCount()+descendants.size()==pageCount);

			// all pages from descendants are not available
			for(vector<shared_ptr<CPage> >::iterator i=descendants.begin();i!=descendants.end(); i++)
			{
				try
				{
					pos=pdf->getPagePosition(*i);
					CPPUNIT_FAIL("getPagePosition on removed page should have failed");
				}catch(PageNotFoundException &e)
				{
					/* ok */
				}
			}
			
			// inserts back interNode to 1st position in root kids array
			// currently first page should be at 1+getKidsCount (if there is any
			// page);
			printf("TC07:\tinserting intermediate node test.\n");
			shared_ptr<CPage> firstPage, lastPage;
			size_t currPageCount=pdf->getPageCount();
			if(currPageCount)
			{
				firstPage=pdf->getFirstPage();
				lastPage=pdf->getLastPage();
			}
	
			size_t interNodeLeafCount=getKidsCount(interNode,NULL);
			rootKids->addProperty(0, *interNodeCRef);
			CPPUNIT_ASSERT(currPageCount + interNodeLeafCount == pdf->getPageCount());
			if(firstPage.get())
				CPPUNIT_ASSERT(pdf->getPagePosition(firstPage) == interNodeLeafCount + 1);
			if(lastPage.get())
				CPPUNIT_ASSERT(pdf->getPagePosition(lastPage) == interNodeLeafCount + currPageCount);

			
			printf("TC08:\treplacing intermediate node by another intermediate node\n");
			// gets CRef of firts root's kid (one we have inserted)
			shared_ptr<CRef> replaceElement=IProperty::getSmartCObjectPtr<CRef>(rootKids->getProperty(0));
			// change value in replaceElement to contain reference to newly
			// created fake intermediate node
			currPageCount=pdf->getPageCount();
			replaceElement->setValue(fakeInterRef);
			CPPUNIT_ASSERT(pdf->getPageCount() == currPageCount - interNodeLeafCount + fakeInterLeafCount);
			if(firstPage.get())
				CPPUNIT_ASSERT(pdf->getPagePosition(firstPage) == 1 + fakeInterLeafCount);
			if(lastPage.get())
				CPPUNIT_ASSERT(pdf->getPagePosition(lastPage) == currPageCount - interNodeLeafCount + fakeInterLeafCount);
			// sets value back with setProperty
			rootKids->setProperty(0, *interNodeCRef);

			// TODO Kids array as reference to array/mess, 

			printf("TC09:\tambiguous page tree test - ambiguous intermediate node\n");
			// inserts reference to interNode to rootKids - this produces
			// ambiguous page tree for this intermediate node - despite that all
			// pages should be accessible
			CPPUNIT_ASSERT(getValueFromSimple<CRef>(interNodeCRef)==interNode->getIndiRef());
			currPageCount=pdf->getPageCount();
			rootKids->addProperty(*interNodeCRef);
			CPPUNIT_ASSERT(pdf->getPageCount()==currPageCount+getKidsCount(interNode,NULL));
			try
			{
				getNodePosition(pdf, interNode, NULL);
				CPPUNIT_FAIL("getNodePosition should have failed");
			}catch(AmbiguousPageTreeException & e)
			{
				/* ok */
			}

			try
			{
				shared_ptr<CPage> page=pdf->getFirstPage();
				while(pdf->hasNextPage(page))
					page=pdf->getNextPage(page);
			}catch(std::exception & e)
			{
				CPPUNIT_FAIL("page iteration methods shouldn't failed");
			}
			// removes last (ambiguous element)
			rootKids->delProperty(rootKids->getPropertyCount()-1);
			CPPUNIT_ASSERT(pdf->getPageCount()==currPageCount);
			
			printf("TC10:\tambiguous page tree test - ambiguous leaf node\n");
			// gets parent of first page
			firstPage=pdf->getFirstPage();
			shared_ptr<CDict> parentDict=getCObjectFromRef<CDict>(
					firstPage->getDictionary()->getProperty("Parent")
					);
			shared_ptr<CArray> parentKids=IProperty::getSmartCObjectPtr<CArray>(
					parentDict->getProperty("Kids")
					);
			shared_ptr<CRef> firstPageRef(CRefFactory::getInstance(
						firstPage->getDictionary()->getIndiRef())
					);	
			currPageCount=pdf->getPageCount();
			parentKids->addProperty(*firstPageRef);
			try
			{
				pdf->getPagePosition(firstPage);
				CPPUNIT_FAIL("getPagePosition should have failed on ambiguous page.");
			}catch(PageNotFoundException & e)
			{
				/* ok */
			}
			try
			{
				getNodePosition(pdf, firstPage->getDictionary(), NULL);
				CPPUNIT_FAIL("getNodePosition should have failed for ambiguous page dictionary.");
			}catch(AmbiguousPageTreeException & e)
			{
				/* ok */
			}
			try
			{
				for(size_t pos=1; pos<pdf->getPageCount(); pos++)
					page=pdf->getPage(pos);
			}catch(std::exception & e)
			{
				CPPUNIT_FAIL("page iteration methods shouldn't failed");
			}
			
			// page is ambiguous but total page count should have increased
			CPPUNIT_ASSERT(pdf->getPageCount()==currPageCount+1);
			// gets back to unambiguous state
			parentKids->delProperty(parentKids->getPropertyCount()-1);
			CPPUNIT_ASSERT(pdf->getPageCount()==currPageCount);
			getNodePosition(pdf, firstPage->getDictionary(), NULL);
		}

		// checks PageTreeRootObserver for all possible situations: Pages
		// property (add, delete or change and Pages reference change)
		printf("TC11:\tremoving page tree root test\n");
		shared_ptr<CDict> pageTreeRoot=getPageTreeRoot(pdf);
		size_t currPageCount=pdf->getPageCount();
		pdf->getDictionary()->delProperty("Pages");
		CPPUNIT_ASSERT(pdf->getPageCount()==0);

		printf("TC12:\tadding wrong page tree root element\n");
		shared_ptr<IProperty> fakeInterCRef(CRefFactory::getInstance(fakeInterRef));
		pdf->getDictionary()->addProperty("Pages", *fakeInterCRef);
		CPPUNIT_ASSERT(pdf->getPageCount()==0);
		try
		{
			pdf->getFirstPage();
			CPPUNIT_FAIL("getFirstPage should have failed on empty page tree.");
		}catch(PageNotFoundException & e)
		{
			/* ok */
		}
		try
		{
			pdf->getLastPage();
			CPPUNIT_FAIL("getLastPage should have failed on empty page tree.");
		}catch(PageNotFoundException & e)
		{
			/* ok */
		}

		printf("TC13:\treplacing page tree root test\n");
		// creates new reference which points to some nonsense and changes Pages
		// reference value
		shared_ptr<CRef> pagesProp=IProperty::getSmartCObjectPtr<CRef>(pdf->getDictionary()->getProperty("Pages"));
		pagesProp->setValue(pdf->addIndirectProperty(shared_ptr<CInt>(CIntFactory::getInstance(1))));
		CPPUNIT_ASSERT(pdf->getPageCount()==0);

		// sets new Pages property with correct reference to page tree root
		// (original one)
		scoped_ptr<CRef> pageTreeRootCRef(CRefFactory::getInstance(pageTreeRoot->getIndiRef()));
		pdf->getDictionary()->setProperty("Pages", *pageTreeRootCRef);
		CPPUNIT_ASSERT(pdf->getPageCount()==currPageCount);

		// change made to document
		CPPUNIT_ASSERT(pdf->isChanged());
	}

	void linearizedTC(boost::shared_ptr<CPdf> pdf)
	{
		printf("%s\n", __FUNCTION__);
		if(!pdf->isLinearized())
		{
			printf("Usecase is not suitable becuase document is not linearized\n");
			return;
		}
		printf("TC01:\tPdf mustn't be modified\n");
		CPPUNIT_ASSERT(!pdf->isChanged());
		printf("TC02:\tAll modification methods have to fail with exception\n");
		shared_ptr<CInt> prop(pdfobjects::CIntFactory::getInstance(1));
		try
		{
			pdf->addIndirectProperty(prop);
			CPPUNIT_FAIL("CPdf::addIndirectProperty should have failed");
		}catch(ReadOnlyDocumentException e)
		{
			// ok should have failed
		}
		try
		{
			pdf->changeIndirectProperty(prop);
			CPPUNIT_FAIL("CPdf::changeIndirectProperty should have failed");
		}catch(ReadOnlyDocumentException e)
		{
			// ok should have failed
		}
		shared_ptr<CPage> page = pdf->getFirstPage();
		try
		{
			pdf->insertPage(page, 1);
			CPPUNIT_FAIL("CPdf::insertPage should have failed");
		}catch(ReadOnlyDocumentException e)
		{
			// ok should have failed
		}
		try
		{
			pdf->removePage(1);
			CPPUNIT_FAIL("CPdf::removePage should have failed");
		}catch(ReadOnlyDocumentException e)
		{
			// ok should have failed
		}
		try
		{
			pdf->canChange();
			CPPUNIT_FAIL("CPdf::canChange should have failed");
		}catch(ReadOnlyDocumentException e)
		{
			// ok should have failed
		}
		
		printf("TC03:\tLinearized document has only one revision\n");
		CPPUNIT_ASSERT(pdf->getRevisionsCount() == 1);	
	}

/** Multiversion file name. */
#define MV_F "multiversion.pdf"

/** Multiversion revisin count. */
#define MV_RC  4

#define TRY_READONLY_OP(method, errorMsg)\
	{\
		try\
		{\
			method;\
			CPPUNIT_FAIL(errorMsg);\
		}catch(ReadOnlyDocumentException &e)\
		{\
			/* passed */\
		}\
	}
	void revisionsTC()
	{
	using namespace boost;

		printf("%s\n", __FUNCTION__);

		// opens special test file
		printf("Using file \"%s\"\n", TestParams::add_path(MV_F).c_str());
		boost::shared_ptr<CPdf> pdf=getTestCPdf(TestParams::add_path(MV_F).c_str());
		CPdf::OpenMode mode=pdf->getMode();

		// number of revision must match
		printf("TC01:\tRevisions count test\n");
		CPPUNIT_ASSERT(pdf->getRevisionsCount()==MV_RC);

		printf("TC02:\tchangeRevision, getActualRevision tests\n");
		for(CPdf::revision_t i=0; i<pdf->getRevisionsCount(); i++)
		{
			pdf->changeRevision(i);
			CPPUNIT_ASSERT(pdf->getActualRevision()==i);
		}

		printf("TC03:\tgetPageCount for revision test\n");
		// starts from the oldest one - each newer revision has one less
		// page count
		size_t page_count = pdf->getRevisionsCount();
		for(CPdf::revision_t i=0; i<pdf->getRevisionsCount(); i++, page_count--)
		{
			pdf->changeRevision(i);
			CPPUNIT_ASSERT(pdf->getPageCount()==page_count);
		}

		printf("TC04:\tgetCXref::getNumObjects is same for all revisions\n");
		// no changes has been done, so all revisions has to have same number
		// of objects
		int number=0;
		for(CPdf::revision_t i=0; i<pdf->getPageCount(); i++)
		{
			pdf->changeRevision(i);
			if(!number)
			{
				// first information is just stored to number
				number=pdf->getCXref()->getNumObjects();
				continue;
			}
			CPPUNIT_ASSERT(pdf->getCXref()->getNumObjects()==number);
		}

		printf("TC05:\tolder revisions has to be readOnly\n");
		for(CPdf::revision_t i=0; i<pdf->getRevisionsCount()-1; i++)
		{
			pdf->changeRevision(i);
			CPPUNIT_ASSERT(pdf->getMode()==CPdf::ReadOnly);
		}

		printf("TC06:\tThe newest revision has mode same as set in getInstace\n");
		pdf->changeRevision(pdf->getRevisionsCount()-1);
		CPPUNIT_ASSERT(pdf->getMode()==mode);
		
		printf("TC07:\tno changes can be done in older revisions\n");
		for(CPdf::revision_t i=0; i<pdf->getRevisionsCount()-1; i++)
		{
			printf("\trevision=%d\n", i);
			pdf->changeRevision(i);
			XRefWriter* xref = dynamic_cast<XRefWriter *>(pdf->getCXref());
			CPPUNIT_ASSERT(xref);
			CPPUNIT_ASSERT(!utils::isLatestRevision(*xref));
			
			// addIndirectProperty
			printf("\t\taddIndirectProperty\n");
			shared_ptr<IProperty> prop(CIntFactory::getInstance(1));
			TRY_READONLY_OP(pdf->addIndirectProperty(prop),"addIndirectProperty should have failed");

			// changeIndirectProperty
			printf("\t\tchangeIndirectProperty\n");
			TRY_READONLY_OP(pdf->changeIndirectProperty(prop),"changeIndirectProperty should have failed");
			
			// insertPage
			shared_ptr<CDict> pageDict(CDictFactory::getInstance());
			shared_ptr<CPage> page(CPageFactory::getInstance(pageDict));
			printf("\t\tinsertPage\n");
			TRY_READONLY_OP(pdf->insertPage(page,1), "insertPage should have failed");

			// removePage
			printf("\t\tremovePage\n");
			TRY_READONLY_OP(pdf->removePage(1), "removePage should have failed");
			
		}

		// isModified returns always false
		printf("TC08:\tisChanged is allways false on unchanged document\n");
		CPPUNIT_ASSERT(!pdf->isChanged());

		printf("TC09:\tLatest revision allows changes\n");
		pdf->changeRevision(pdf->getRevisionsCount()-1);
		shared_ptr<IProperty> newProp(CIntFactory::getInstance(1));
		IndiRef ref = pdf->addIndirectProperty(newProp);
		shared_ptr<IProperty> prop = pdf->getIndirectProperty(ref);
		CPPUNIT_ASSERT(prop->getType()==pInt);
		CPPUNIT_ASSERT(utils::getValueFromSimple<CInt>(prop)==1);

		printf("TC10:\tchange done to the latest revision is preserved\n");
		pdf->changeRevision(0);
		pdf->changeRevision(pdf->getRevisionsCount()-1);
		prop = pdf->getIndirectProperty(ref);
		CPPUNIT_ASSERT(prop->getType()==pInt);
		CPPUNIT_ASSERT(utils::getValueFromSimple<CInt>(prop)==1);
	}
#undef TRY_READONLY_OP
	
	void instancingTC()
	{
		printf("%s\n", __FUNCTION__);

		// checks whether getInstance works correctly
		// TODO figure out
		
	}


	void indirectPropertyTC(boost::shared_ptr<CPdf> pdf)
	{
	using namespace boost;
	using namespace utils;
	
		printf("%s\n", __FUNCTION__);
		if(pdf->isLinearized())
		{
			printf("Usecase is not suitable becuase document is linearized\n");
			return;
		}

		printf("TC01:\taddIndirectProperty with simple property with no pdf\n");
		// creates new property and adds it
		shared_ptr<IProperty> prop(CIntFactory::getInstance(1));
		IndiRef addedRef,intPropRef;
		addedRef=intPropRef=pdf->addIndirectProperty(prop);
		CPPUNIT_ASSERT(addedRef.num!=0);
		shared_ptr<IProperty> added=pdf->getIndirectProperty(addedRef);
		// type must be same
		CPPUNIT_ASSERT(added->getType()==prop->getType());
		// value must be same (we know that prop is CInt)
		PropertyEquals pe;
		CPPUNIT_ASSERT(pe(added, prop));
		CPPUNIT_ASSERT(added->getPdf().lock().get()==pdf.get());
		CPPUNIT_ASSERT(added->getIndiRef()==addedRef);
		printf("\t\tReference state is INITIALIZED_REF (according CXref::knowsRef)\n");
		CPPUNIT_ASSERT(pdf->getCXref()->knowsRef(addedRef)==INITIALIZED_REF);

		printf("TC02:\taddIndirectProperty with reference with no pdf\n");
		IndiRef ref(1,0);
		shared_ptr<IProperty> refProp(CRefFactory::getInstance(ref));
		try
		{
			addedRef=pdf->addIndirectProperty(refProp);
			CPPUNIT_FAIL("addIndirectProperty with reference should have failed");
		}catch(ElementBadTypeException & e)
		{
			/* ok, should have failed */
		}
		
		printf("TC03:\taddIndirectProperty dictionary with no pdf\n");
		// prepares dictionary with 3 entries
		// Elem1 Real
		// Elem2 Ref
		// Elem3 [Bool]
		shared_ptr<CDict> dictProp(CDictFactory::getInstance());
		shared_ptr<IProperty> dictElem1(CRealFactory::getInstance(1.5));
		dictProp->addProperty("Elem1", *dictElem1);
		shared_ptr<IProperty> dictElem2(CRefFactory::getInstance(ref));
		dictProp->addProperty("Elem2", *dictElem2);
		shared_ptr<CArray> dictElem3(new CArray());
		shared_ptr<IProperty> arrayElem1(CBoolFactory::getInstance(true));
		dictElem3->addProperty(*arrayElem1);
		dictProp->addProperty("Elem3", *dictElem3);
		// adds dictionary
		printf("\tno follow case\n");
		addedRef=pdf->addIndirectProperty(dictProp);
		added=pdf->getIndirectProperty(addedRef);
		CPPUNIT_ASSERT(isDict(*added));
		shared_ptr<CDict> addedDict=IProperty::getSmartCObjectPtr<CDict>(added);
		CPPUNIT_ASSERT(added->getPdf().lock().get()==pdf.get());
		CPPUNIT_ASSERT(added->getIndiRef()==addedRef);
		CPPUNIT_ASSERT(addedDict->getPropertyCount()==3);
		CPPUNIT_ASSERT(isReal(*(addedDict->getProperty("Elem1"))));
		CPPUNIT_ASSERT(isRef(*(addedDict->getProperty("Elem2"))));
		// target of Elem2 has to be CNull
		ref = getValueFromSimple<CRef>(addedDict->getProperty("Elem2"));
		CPPUNIT_ASSERT(isNull(*pdf->getIndirectProperty(ref)));
		CPPUNIT_ASSERT(isArray(*(addedDict->getProperty("Elem3"))));

		// adds the same dictionary, with followRef now!
		printf("\tfollow case\n");
		IndiRef followAddedRef = pdf->addIndirectProperty(dictProp, true);
		CPPUNIT_ASSERT(!(followAddedRef == addedRef));
		added=pdf->getIndirectProperty(followAddedRef);
		CPPUNIT_ASSERT(isDict(*added));
		shared_ptr<CDict> followAddedDict=IProperty::getSmartCObjectPtr<CDict>(added);
		CPPUNIT_ASSERT(followAddedDict->getIndiRef()==followAddedRef);
		CPPUNIT_ASSERT(followAddedDict->getPropertyCount()==3);
		CPPUNIT_ASSERT(isReal(*(followAddedDict->getProperty("Elem1"))));
		CPPUNIT_ASSERT(isRef(*(followAddedDict->getProperty("Elem2"))));
		// target of Elem2 has to be CNull - it makes no sense to 
		// dereference without document
		ref = getValueFromSimple<CRef>(followAddedDict->getProperty("Elem2"));
		CPPUNIT_ASSERT(isNull(*pdf->getIndirectProperty(ref)));
		CPPUNIT_ASSERT(isArray(*(followAddedDict->getProperty("Elem3"))));

		printf("TC04:\taddIndirectProperty with CRef from same pdf\n");
		// uses root of page tree, which has to be reference
		shared_ptr<IProperty> rootProp=pdf->getDictionary()->getProperty("Pages");
		if(isRef(rootProp))
		{
			shared_ptr<CRef> rootCRef=IProperty::getSmartCObjectPtr<CRef>(rootProp);
			try
			{
				addedRef=pdf->addIndirectProperty(rootCRef);
				CPPUNIT_FAIL("addIndirectProperty with reference should have failed");
			}catch(ElementBadTypeException & e)
			{
				/* ok should have failed */
			}
		}

		printf("TC05:\taddIndirectProperty from different pdf (followRefs==false)\n");
		// opens TEST_FILE
		if (0 == TestParams::instance().files.size())
			return;
		boost::shared_ptr<CPdf> differentPdf=getTestCPdf(TestParams::instance().files.front().c_str());
		// gets page dictionary which contains at least one reference (to its
		// parent). addIndirectProperty with followRefs==false
		shared_ptr<CDict> differentPageDict=differentPdf->getFirstPage()->getDictionary();
		addedRef=pdf->addIndirectProperty(differentPageDict, false);
		added=pdf->getIndirectProperty(addedRef);
		CPPUNIT_ASSERT(added->getIndiRef()==addedRef);
		CPPUNIT_ASSERT(added->getPdf().lock().get()==pdf.get());
		CPPUNIT_ASSERT(isDict(*added));
		printf("\t\tReference state is INITIALIZED_REF (according CXref::knowsRef)\n");
		CPPUNIT_ASSERT(pdf->getCXref()->knowsRef(addedRef)==INITIALIZED_REF);
		addedDict=IProperty::getSmartCObjectPtr<CDict>(added);
		CPPUNIT_ASSERT(addedDict->getPropertyCount()==differentPageDict->getPropertyCount());
		// gets parent dictionary and checks if its reference is known and value
		// is CNull (because deep copy was not done)
		shared_ptr<IProperty> parentProp=addedDict->getProperty("Parent");
		if(isRef(*parentProp))
		{
			ref=getValueFromSimple<CRef>(parentProp);
			shared_ptr<IProperty> parentPropValue=pdf->getIndirectProperty(ref);
			CPPUNIT_ASSERT(isNull(*parentPropValue));
			::Ref xpdfRef={ref.num, ref.gen};
			// has to know reference as RESERVED_REF
			printf("\t\tReference state of parentProp is RESERVED_REF (according CXref::knowsRef)\n");
			CPPUNIT_ASSERT(pdf->getCXref()->knowsRef(xpdfRef)==RESERVED_REF);
		}

		printf("TC06:\taddIndirectProperty from different pdf (followRefs==true)\n");
		// does the same as for tc05 with followRefs set to true and "Parent"
		// property from differentPageDict - insertion should reuse mapping from
		// firts insetion - without followRefs
		parentProp=differentPageDict->getProperty("Parent");
		IndiRef parentRef=getValueFromSimple<CRef>(parentProp);
		shared_ptr<IProperty> differentPageProp=differentPdf->getIndirectProperty(parentRef);
		CPPUNIT_ASSERT(isDict(*differentPageProp));
		differentPageDict=IProperty::getSmartCObjectPtr<CDict>(differentPageProp);
		addedRef=pdf->addIndirectProperty(differentPageDict, true);
		// addededRef must be same as reserved in first insertion
		CPPUNIT_ASSERT(addedRef==ref);
		added=pdf->getIndirectProperty(addedRef);
		CPPUNIT_ASSERT(added->getIndiRef()==addedRef);
		CPPUNIT_ASSERT(added->getPdf().lock().get()==pdf.get());
		CPPUNIT_ASSERT(isDict(*added));
		printf("\t\tReference state is INITIALIZED_REF (according CXref::knowsRef)\n");
		CPPUNIT_ASSERT(pdf->getCXref()->knowsRef(addedRef)==INITIALIZED_REF);
		addedDict=IProperty::getSmartCObjectPtr<CDict>(added);
		CPPUNIT_ASSERT(addedDict->getPropertyCount()==differentPageDict->getPropertyCount());
		differentPdf.reset();

		printf("TC07:\tgetIndirectProperty with uknown reference returns CNull\n");
		// creates unknown reference from existing by adding 1 to generation
		// number
		IndiRef unknownRef=pdf->getDictionary()->getIndiRef();
		unknownRef.gen++;
		CPPUNIT_ASSERT(isNull(*pdf->getIndirectProperty(unknownRef)));
		printf("\t\tReference state is UNUSED_REF (according CXref::knowsRef)\n");
		CPPUNIT_ASSERT(pdf->getCXref()->knowsRef(unknownRef)==UNUSED_REF);

		printf("TC08:\tchangeIndirectProperty with invalid parameter should fail\n");
		// creates fake and try to use changeIndirectProperty with unknownRef
		Object obj;
		obj.initInt(1);
		shared_ptr<CInt> fakeInt(CIntFactory::getInstance(pdf, unknownRef, obj));
		try
		{
			pdf->changeIndirectProperty(fakeInt);
			CPPUNIT_FAIL("changeIndirectProperty should have failed with faked object");
		}catch(CObjInvalidObject & e)
		{
			/* ok */
		}

		printf("TC09:\tchangeIndirectProperty with bad typed parameter should fail\n");
		// creates integer and tries to change value of Pages field which is
		// reference
		fakeInt=shared_ptr<CInt>(CIntFactory::getInstance(pdf, pdf->getDictionary()->getProperty("Pages")->getIndiRef(), obj));
		try
		{
			pdf->changeIndirectProperty(fakeInt);
			CPPUNIT_FAIL("changeIndirectProperty with bad typed value should have failed");
		}catch(ElementBadTypeException & e)
		{
			/* ok */
		}

		printf("TC08:\tChanging indirect property causes CPdf::changeIndirectProperty\n");
		// changes intPropRef added at the method beginning
		shared_ptr<CInt> originalIntProp=IProperty::getSmartCObjectPtr<CInt>(pdf->getIndirectProperty(intPropRef));
		// in first case change value directly in originalIntProp
		int originalIntValue=getIntFromIProperty(originalIntProp);
		// this should automatically call pdf->changeIndirectProperty
		originalIntProp->setValue(originalIntValue+1);
		shared_ptr<CInt> changedIntProp=IProperty::getSmartCObjectPtr<CInt>(pdf->getIndirectProperty(intPropRef));
		// we have changed just value, so instance must be same
		CPPUNIT_ASSERT(changedIntProp==originalIntProp);
		CPPUNIT_ASSERT(getIntFromIProperty(changedIntProp)-1==originalIntValue);

		printf("TC09:\tchangeIndirectProperty with proper (different) value\n");
		// sets changed property as original now and resets changedIntProp with
		// new value - so we have different instance of property with same type,
		// so type check doesn't fail
		originalIntProp=changedIntProp;
		changedIntProp.reset();
		Object intXpdfObj;
		intXpdfObj.initInt(0);
		changedIntProp=shared_ptr<CInt>(CIntFactory::getInstance(pdf, intPropRef, intXpdfObj));
		pdf->changeIndirectProperty(changedIntProp);
		// resets value and gets it again
		changedIntProp.reset();
		changedIntProp=IProperty::getSmartCObjectPtr<CInt>(pdf->getIndirectProperty(intPropRef));
		// instances must be different
		CPPUNIT_ASSERT(changedIntProp!=originalIntProp);
		CPPUNIT_ASSERT(getIntFromIProperty(changedIntProp)==0);
		// reference is kept
		CPPUNIT_ASSERT(changedIntProp->getIndiRef()==originalIntProp->getIndiRef());
	}

	void delinearizatorTC(string fileName)
	{
	using namespace pdfobjects::utils;

		printf("%s\n", __FUNCTION__);

		// creates delinearizator and delinearize file to the file
		// fileName-delinearized.pdf
		IPdfWriter * writer=new OldStylePdfWriter();
		ProgressBar * progressBar=new ProgressBar(cout);
		writer->registerObserver(shared_ptr<PdfWriterObserver>(new ProgressObserver(progressBar)));
		boost::shared_ptr<Delinearizator> delinearizator=Delinearizator::getInstance(fileName.c_str(), writer);
		if(!delinearizator)
		{
			printf("\t%s is not suitable because it is not linearized.\n", fileName.c_str());
			return;
		}
		string outputFile=fileName+"-delinearizator.pdf";
		printf("\tDelinearized output is in %s file\n", outputFile.c_str());
		delinearizator->delinearize(outputFile.c_str());
	}

#define staticArraySize(array) sizeof(array)/sizeof(*array)
	void changeTrailerTC(string& fname)
	{
		printf("%s\n", __FUNCTION__);
		struct TestField {
			const char *name;
			// NULL means that all values are valid
			::Object *validVal;
		};
		boost::shared_ptr<CPdf> pdf1 = getTestCPdf(fname.c_str());
		if (pdf1->getMode() == CPdf::ReadOnly)
		{
			printf("%s: Document is read only and it is not usable for this test\n", __FUNCTION__);
			return;
		}
		boost::shared_ptr<CPdf> pdf2 = getTestCPdf(fname.c_str(), CPdf::Advanced);
		XRefWriter *xref1 = dynamic_cast<XRefWriter *>(pdf1->getCXref());
		XRefWriter *xref2 = dynamic_cast<XRefWriter *>(pdf2->getCXref());

		Object oNull;
		oNull.initNull();
		Object oInt;
		oInt.initInt(1);
		Object oRef;
		oRef.initRef(1,0);
		Object oDict;
		oDict.initDict(xref1);
		Object oArray;
		oArray.initArray(xref1);
		Object oStream;
		Stream *str= new MemStream((char*)malloc(sizeof(char)*10), 0, 10, &oDict, true);
		oStream.initStream(str);

		printf("\tNULL value is not allowed\n");
		try
		{
			xref1->changeTrailer("Foo", NULL);
			CPPUNIT_FAIL("changeTrailer(\"Foo\", NULL) should fail");
		}catch(ElementBadTypeException )
		{
		}
		try
		{
			xref1->changeTrailer(NULL, NULL);
			CPPUNIT_FAIL("changeTrailer(NULL, NULL) should fail");
		}catch(ElementBadTypeException )
		{
		}
		try
		{
			xref1->changeTrailer(NULL, &oInt);
			CPPUNIT_FAIL("changeTrailer(null, obj) should fail");
		}catch(ElementBadTypeException )
		{
		}
		try
		{
			xref2->changeTrailer("Foo", NULL);
			CPPUNIT_FAIL("changeTrailer(\"Foo\", NULL) should fail also for easy mode");
		}catch(ElementBadTypeException )
		{
		}
		try
		{
			xref2->changeTrailer(NULL, NULL);
			CPPUNIT_FAIL("changeTrailer(NULL, NULL) should fail also for easy mode");
		}catch(ElementBadTypeException )
		{
		}
		try
		{
			xref2->changeTrailer(NULL, &oInt);
			CPPUNIT_FAIL("changeTrailer(NULL, obj) should fail also for easy mode");
		}catch(ElementBadTypeException )
		{
		}

		static const struct TestField fieldsToTest[] = {
			{"Prev", &oInt}, 
			{"Size", &oInt}, 
			{"Root", &oRef},
			{"Encrypt", &oRef},
			{"Info", &oRef},
			{"ID", &oArray},
			{"Foo", NULL}};
		Object *allValues[] = {&oNull, &oInt, &oRef, &oDict, &oArray, &oStream};

		for(size_t i=0; i<staticArraySize(fieldsToTest); ++i) 
		{
			const char *name = fieldsToTest[i].name;
			bool canChange = utils::canChangeTrailerEntry(name);
			printf("\t%s: canChange=%s\n", name, (canChange)?"yes":"no");
			
			// check for canChangeTrailerEntry and changeTrailer consistency
			try
			{
				// if the validVal==NULL then all values are valid
				if(fieldsToTest[i].validVal)
					xref1->changeTrailer(name, fieldsToTest[i].validVal);
				else
					for(size_t j=0; j<staticArraySize(allValues); ++j)
						xref1->changeTrailer(name, allValues[j]);

				CPPUNIT_ASSERT(canChange);
			}catch(ElementBadTypeException)
			{
				CPPUNIT_ASSERT(!canChange);
			}

			// invalid value must fail - only for fields which have some
			// well known invalid value (validVal!=NULL) - invalid values
			// are considered all but the valid one
			if (canChange && fieldsToTest[i].validVal)
				for(size_t j=0; j<staticArraySize(allValues); j++)
				{
					if (fieldsToTest[i].validVal != allValues[j])
					{
						Object *o = allValues[j];
						printf("\t%s: must fail with invalid value with type=%d\n", name, o->getType());
						try
						{
							xref1->changeTrailer(name, o);
							CPPUNIT_FAIL("Invalid value setting succedded for changeTrailer");
						}catch(ElementBadTypeException)
						{
						}
					}
				}

			// ADVANCED mode eats everything
			printf("\t%s: Advanced mode eats everything\n", name);
			for(size_t j=0; j<staticArraySize(allValues); j++)
			{
				try
				{
					Object *o = allValues[j];
					xref2->changeTrailer(name, o);
				}catch(ElementBadTypeException)
				{
					CPPUNIT_FAIL("changeTrailer failed in Advanced mode");
				}
			}
		}

		// play with CPdf now
		pdf2.reset();
		pdf2 = getTestCPdf(fname.c_str(), CPdf::Advanced);
		printf("\tCheck CPdf::changeTrailer\n");
		std::string name = "FooXXXXXXXXX";
		int initV = 100;
		shared_ptr<IProperty> vP(CIntFactory::getInstance(initV));
		pdf2->changeTrailer(name, vP);
		shared_ptr<const CDict> trailer = pdf2->getTrailer();
		shared_ptr<CInt> newVP = IProperty::getSmartCObjectPtr<CInt>(trailer->getProperty(name));
		int v = newVP->getValue();
		CPPUNIT_ASSERT(v == initV);

		if (pdf2->getRevisionsCount()>1)
		{
			pdf2->changeRevision(0);
			trailer = pdf2->getTrailer();
			printf("\tCPdf::changeTrailer mustn't influence other revisions\n");
			try
			{
				IProperty::getSmartCObjectPtr<CInt>(trailer->getProperty(name));
				CPPUNIT_FAIL("trailer shouldn't contain property added in a different revision");
			}catch(ElementNotFoundException )
			{
			}
			printf("\tCPdf::changeTrailer must survive revision change\n");
			pdf2->changeRevision(pdf2->getRevisionsCount()-1);
			trailer = pdf2->getTrailer();
			newVP = IProperty::getSmartCObjectPtr<CInt>(trailer->getProperty(name));
			v = newVP->getValue();
			CPPUNIT_ASSERT(v == initV);
		}
	}

	void setUp()
	{
	}

	void tearDown()
	{
	}

	void Test()
	{
		instancingTC();
		// creates pdf instances for all files
		for(TestParams::FileList::const_iterator i = TestParams::instance().files.begin(); 
				i != TestParams::instance().files.end(); 
					++i)
		{
			string fileName=*i;
			printf("\nTests for file:%s\n", fileName.c_str());
			boost::shared_ptr<CPdf> pdf=getTestCPdf(fileName.c_str());
			string filterName;
			if(pdfobjects::utils::isEncrypted(pdf))
			{
				printf("Test file is encrypted and so not supported.\n");
				continue;
			}
			// pageIterationTC is before indirectPropertyTC because it needs to
			// have no changes made before (checks isChanged on non change
			// producing operations)
			pageIterationTC(pdf);
			cloneTC(pdf, fileName);
			indirectPropertyTC(pdf);
			pageManipulationTC(pdf);
			linearizedTC(pdf);

			delinearizatorTC(fileName);
			changeTrailerTC(fileName);
		}
		revisionsTC();
		printf("TEST_CPDF testig finished\n");

	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestCPdf);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCPdf, "TEST_CPDF");

boost::shared_ptr<pdfobjects::CPdf> getTestCPdf(const char* filename, pdfobjects::CPdf::OpenMode mode)
{
	return CPdf::getInstance(filename, mode);
}
