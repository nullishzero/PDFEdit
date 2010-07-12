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
#include "xpdf/PDFDoc.h"
#include "tests/kernel/testmain.h"
#include "tests/kernel/testcobject.h"
#include "tests/kernel/testcpage.h"
#include "tests/kernel/testcpdf.h"

#include "kernel/factories.h"
#include "kernel/cpage.h"
#include "kernel/cannotation.h"


//=====================================================================================

boost::shared_ptr<CPage> 
getPage (const char* fileName, boost::shared_ptr<CPdf> pdf, size_t pageNum)
{
	boost::scoped_ptr<PDFDoc> doc (new PDFDoc (new GString(fileName), NULL, NULL));
	
	//
	// Our stuff here
	//
	Object obj;
	XRef* xref = doc->getXRef();
	assert (xref);
	Catalog cat (xref);

	IndiRef ref;
	ref.num = cat.getPageRef(pageNum)->num;
	ref.gen = cat.getPageRef(pageNum)->gen;
	xref->fetch (ref.num, ref.gen, &obj);
	
	boost::shared_ptr<CDict> dict (new CDict (pdf, obj, ref));
	obj.free ();
	
	return boost::shared_ptr<CPage> (new CPage(dict));
}

//=====================================================================================
namespace {
//=====================================================================================

using namespace boost;

//=====================================================================================

bool
mediabox (UNUSED_PARAM ostream& UNUSED_PARAM oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);

		try {
		/* oss << */ page->getMediabox ();
		}catch (...)
		{
			std::cout << "NO MEDIA BOX FOUND.";
			return true;
		}

		libs::Rectangle rc;
		rc.xleft = 42;
		rc.yleft = 62;
		rc.xright = 12;
		rc.yright = 2342;
		page->setMediabox (rc);
		
		/* oss << */ page->getMediabox ();

		if (42 == page->getMediabox ().xleft &&
				62 == page->getMediabox ().yleft)
			_working (oss);
		else
			return false;

	}

	return true;
}


//=====================================================================================

bool
display (UNUSED_PARAM ostream& oss, const char* fileName)
{
	// Open pdf and get the first page	
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);

		#if TEMP_FILES_CREATE
			const char* FILE_OUT = "1.txt";
			TextOutputDev textOut (const_cast<char*>(FILE_OUT), gFalse, gFalse, gFalse);
		#else	
			TextOutputDev textOut (NULL, gTrue, gFalse, gTrue);
		#endif
		if (!textOut.isOk ())
			throw;
		
		//
		// Output to file
		//
		//oss << "Creating 1.txt which contains text from a pdf." << endl;
		page->displayPage (textOut);
		
		//oss << "Output from textoutputdevice." << endl;
		//oss << textOut.getText(0, 0, 1000, 1000)->getCString() << endl;
		delete textOut.getText(0, 0, 1000, 1000);

		_working (oss);
	}
	
	return true;
}

//=====================================================================================

bool
_export (UNUSED_PARAM ostream& oss, const char* fileName)
{
	// Open pdf and get the first page	
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);

		string tmp;
		page->getText (tmp);
		//oss << "Text: " << tmp << endl;
		_working (oss);
	}
	
	return true;
}

//=====================================================================================

bool
findtext (UNUSED_PARAM ostream& oss, const char* fileName)
{
	// Open pdf and get the first page	
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);

		string tmp;
		page->getText (tmp);
		//oss << "Text: " << tmp << endl;


		typedef std::vector<libs::Rectangle> Recs;
		Recs recs;
		string word;
		if (tmp.length() > 10)
		{
			// arbitrary word/string??
			word = tmp.substr (2,3);
			page->findText (word, recs); 
			if (recs.empty())
			{
					oss << "RECS ARE EMPTY !!!" << flush;
					//getchar ();
			}else
				oss << "Text: " << word << " at position: " << recs.front() << flush;
		}
	}
	
	return true;
}


//=====================================================================================

bool
getSetFonts (UNUSED_PARAM ostream& oss, const char* fileName)
{
	{
		// Open pdf and get the first page	
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

		for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
		{
			boost::shared_ptr<CPage> page = pdf->getPage (i+1);

			CPage::FontList fs;
			
			page->getFontIdsAndNames (fs);

			if (fs.empty())		
				oss << "FONTS ARE EMPTY !!!" << flush;
			else
			{
				#if REALLY_ALL_OUTPUT
					oss << "Fonts on " << (i + 1) << "-th page: " << flush;
					for (CPage::FontList::iterator it = fs.begin(); it != fs.end(); ++it)
						oss << "(" << (*it).first << ", " << (*it).second << ") " << flush;
				#endif
			}
			
		}
	}

	{
		// Open pdf and get the first page	
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

		for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
		{
			boost::shared_ptr<CPage> page = pdf->getPage (i+1);

			CPage::FontList fs;
			
			const string fn ("Jozov-font");
			page->addSystemType1Font (fn);
			page->getFontIdsAndNames (fs);

			CPPUNIT_ASSERT (!fs.empty());

			bool found = false;
			for (CPage::FontList::iterator it = fs.begin(); 
					it != fs.end(); ++it)
			{
				if (fn == (*it).second)
				{
					found = true;
					break;
				}
			}

			CPPUNIT_ASSERT (found);
			
			page->getFontIdsAndNames (fs);
			CPPUNIT_ASSERT (!fs.empty());
			//oss << "After change: " << (i + 1) << "-th page: " << flush;
			for (CPage::FontList::iterator it = fs.begin(); 
					it != fs.end(); ++it)
			{
			//	oss << "(" << (*it).first << ", " << (*it).second << ") " << flush;
			}
			
			{
				page->addSystemType1Font (fn);
				page->addSystemType1Font (fn);
				page->addSystemType1Font (fn);

				page->getFontIdsAndNames (fs);
				CPPUNIT_ASSERT (!fs.empty());
				#if REALLY_ALL_OUTPUT
					oss << "After change: " << (i + 1) << "-th page: " << flush;
					for (CPage::FontList::iterator it = fs.begin(); 
							it != fs.end(); ++it)
						oss << "(" << (*it).first << ", " << (*it).second << ") " << flush;
				#endif
			}

		}

	}

	
	return true;
}


//=====================================================================================
bool setattr(UNUSED_PARAM ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	size_t pageCount=pdf->getPageCount();
	for(size_t pos=1; pos<=pageCount; pos++)
	{
		shared_ptr<CPage> page=pdf->getPage(pos);

		page->setMediabox (libs::Rectangle (1,1,2,2));
		libs::Rectangle rc = page->getMediabox ();
		CPPUNIT_ASSERT (rc.xleft == 1 && rc.yleft == 1);
		CPPUNIT_ASSERT (rc.xright == 2 && rc.yright == 2);
		page->setRotation (10);
		CPPUNIT_ASSERT (10 == page->getRotation());

	}
	return true;
}

//=====================================================================================
bool change(UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	size_t pageCount=pdf->getPageCount();
	for(size_t pos=1; pos<=pageCount; pos++)
	{
		shared_ptr<CPage> page=pdf->getPage(pos);

		typedef vector<shared_ptr<CContentStream> > CCs;
		typedef vector<shared_ptr<PdfOperator> > Ops;
		
		CCs ccs;
		page->getContentStreams (ccs);
		if (ccs.empty())
			continue;
	
		size_t prevCh = 0;
		{
			CCs ccs1;
			page->getChanges (ccs1);
			prevCh = ccs1.size();
		}
	
		Ops ops;
		ccs.front()->getPdfOperators (ops);
		page->addContentStreamToBack (ops);
		page->addContentStreamToFront (ops);

		CPPUNIT_ASSERT (page->getChange (0));
		CPPUNIT_ASSERT (page->getChange (1));
		
		CCs ccs1;
		page->getChanges (ccs1);
		CPPUNIT_ASSERT ( (2 + prevCh) == ccs1.size());

		#if TEMP_FILES_CREATE
				const char* FILE_OUT = "2.txt";
				{
					TextOutputDev textOut (const_cast<char*>(FILE_OUT), gFalse, gFalse, gFalse);
					if (!textOut.isOk ())
						throw;

					// Display our change
					page->displayChange (textOut, ccs1);
				}

				string text;
				page->getText (text);

				ifstream in (FILE_OUT);
				string text1;
				while (!in.eof())
					in >> text1;

				CPPUNIT_ASSERT_EQUAL (text,text1);
		#endif
		_working (oss);
	}
	return true;
}

//=====================================================================================
bool move(UNUSED_PARAM ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	size_t pageCount=pdf->getPageCount();
	for(size_t pos=1; pos<=pageCount; pos++)
	{
		shared_ptr<CPage> page=pdf->getPage(pos);

		typedef vector<shared_ptr<CContentStream> > CCs;
		typedef vector<shared_ptr<PdfOperator> > Ops;
		
		CCs ccs;
		page->getContentStreams (ccs);
		if (ccs.empty())
			continue;
	
		Ops ops;
		ccs.front()->getPdfOperators (ops);
		page->addContentStreamToFront (ops);
		CPPUNIT_ASSERT (page->getChange (0));
		
		page->getContentStreams (ccs);
		page->moveAbove (ccs[0]);
		CCs ccs1;
		page->getContentStreams (ccs1);

		CPPUNIT_ASSERT_EQUAL (ccs[0],ccs1[1]);
		CPPUNIT_ASSERT_EQUAL (ccs[1],ccs1[0]);
		
		_working (oss);
	}
	return true;
}


//=====================================================================================
bool getcc (UNUSED_PARAM ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	size_t pageCount=pdf->getPageCount();
	for(size_t pos=1; pos<=pageCount; pos++)
	{
		shared_ptr<CPage> page=pdf->getPage(pos);

		typedef vector<shared_ptr<CContentStream> > CCs;
		typedef vector<shared_ptr<PdfOperator> > Ops;
		
		CCs ccs;
		page->getContentStreams (ccs);
		if (ccs.empty())
			continue;
	
		Ops ops;
		ccs.front()->getPdfOperators (ops);
		//assert (!ops.empty());
		if (ops.empty())
			continue;
		

		PdfOperator::Iterator it = PdfOperator::getIterator (ops.front());
		while (!it.isEnd())		
		{
			shared_ptr<CContentStream> cc = it.getCurrent()->getContentStream ();
			CPPUNIT_ASSERT (page->getContentStream(cc.get()) == cc);
			it.next();
		}
		
		_working (oss);
	}
	return true;
}




//=====================================================================================
bool creation (UNUSED_PARAM ostream& oss)
{
	shared_ptr<CDict> dict (CDictFactory::getInstance());
	CArray array;
	// Uuuuuhhhhhhhhhhh
	shared_ptr<CPage> page (new CPage(dict));
	//CPPUNIT_ASSERT (false == page->parseContentStream ());

	//dict->addProperty ("Contents", array);
	//CPPUNIT_ASSERT (true == page->parseContentStream ());

	return true;
}

bool annotsTests(ostream &, const char * fname)
{
	boost::shared_ptr<CPdf> pdf=getTestCPdf(fname);

	size_t pageCount=pdf->getPageCount();
	for(size_t pos=1; pos<=pageCount; pos++)
	{
		shared_ptr<CPage> page=pdf->getPage(pos);
		CPage::Annotations annots;
		page->getAllAnnotations(annots);
		if(annots.empty())
			continue;

	}
	return true;
}


//=========================================================================
// class TestCPage
//=========================================================================

class TestCPage : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCPage);
		CPPUNIT_TEST(TestOperators);
		CPPUNIT_TEST(TestFonts);
		CPPUNIT_TEST(Test);
		CPPUNIT_TEST(TestCreation);
		CPPUNIT_TEST(TestDisplay);
		CPPUNIT_TEST(TestExport);
		CPPUNIT_TEST(TestFind);
		//CPPUNIT_TEST(TestAnnotations);
		CPPUNIT_TEST(TestChanges);
		CPPUNIT_TEST(TestMoveUpDown);
		CPPUNIT_TEST(TestSet);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {OUTPUT << endl;}
	void tearDown() {}

public:
	//
	//
	//
	void Test ()
	{
		OUTPUT << "CPage methods..." << endl;

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
		
			BEGIN_CHECK_READONLY;
				TEST(" mediabox");
				CPPUNIT_ASSERT (mediabox (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}
	
	//
	//
	//
	void TestSet ()
	{
		OUTPUT << "CPage methods..." << endl;

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
		
			BEGIN_CHECK_READONLY;
				TEST(" set");
				CPPUNIT_ASSERT (setattr (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}

	//
	//
	//
	void TestOperators ()
	{
		OUTPUT << "CPage operator test..." << endl;

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
		
			TEST(" getcontentstream");
			CPPUNIT_ASSERT (getcc (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

	//
	//
	//
	void TestDisplay ()
	{
		OUTPUT << "CPage display methods..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" display");
			CPPUNIT_ASSERT (display (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestCreation ()
	{
		OUTPUT << "CPage creation methods..." << endl;
		
		TEST(" creation");
		CPPUNIT_ASSERT (creation (OUTPUT));
		OK_TEST;
	}
	//
	//
	//
	void TestExport ()
	{
		OUTPUT << "CPage export..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" export");
			CPPUNIT_ASSERT (_export (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestFind ()
	{
		OUTPUT << "CPage find..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" find text");
			CPPUNIT_ASSERT (findtext (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestFonts ()
	{
		OUTPUT << "CPage fonts..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			BEGIN_CHECK_READONLY;
				TEST(" get font names");
				CPPUNIT_ASSERT (getSetFonts (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}
	//
	//
	//
	void TestAnnotations()
	{
	using namespace boost;
	using namespace utils;

		OUTPUT << "CPage annotations..."<<endl;

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			boost::shared_ptr<CPdf> pdf=getTestCPdf((*it).c_str());
			if(pdf->getPageCount()==0)
				break;
			CPage::Annotations annotsList;
			size_t pos=1;
			shared_ptr<CPage> page;
			do
			{
				OUTPUT << "\tChecking "<<pos<<" page"<<endl;
				page=pdf->getPage(pos++);
				page->getAllAnnotations(annotsList);
				size_t annotsListSize=annotsList.size();
				OUTPUT <<"\t\tannotation count="<<annotsListSize<<endl;
				shared_ptr<CDict> pageDict=page->getDictionary();
				if(annotsListSize)
				{
					// annotations are presented
					shared_ptr<CArray> annotsArray = pageDict->getProperty<CArray>("Annots");
					if(annotsArray.get())
					{
						// collects all annotation referencies from array
						CPage::Annotations storage;
						for(size_t i=0; i<annotsArray->getPropertyCount(); i++)
						{
							try
							{
								shared_ptr<CDict> annotDict=annotsArray->getProperty<CDict>(i);
								storage.push_back(shared_ptr<CAnnotation>(new CAnnotation(annotDict)));	
							}catch(std::exception & e)
							{
								// target is not a dictionary or CAnnotation
								// has failed
							}
						}
						CPPUNIT_ASSERT(storage.size()==annotsListSize);
					}
				}

				// backup and removes Annots property - if it exists
				shared_ptr<IProperty> annotsProp;
				if(pageDict->containsProperty("Annots"))
				{
					annotsProp=pageDict->getProperty("Annots");
					pageDict->delProperty("Annots");
				}

				// no annotation available
				page->getAllAnnotations(annotsList);
				CPPUNIT_ASSERT(annotsList.size()==0);

				// creates new Annots property as reference to array with one
				// reference to some mess
				shared_ptr<CArray> annotsArray(CArrayFactory::getInstance());
				IndiRef messRef=pdf->addIndirectProperty(shared_ptr<IProperty>(CIntFactory::getInstance(1)));
				shared_ptr<CRef> annotCRef=shared_ptr<CRef>(CRefFactory::getInstance(messRef));
				annotsArray->addProperty(*annotCRef);
				// adds new annotsArray indirect object
				IndiRef annotsArrayRef=pdf->addIndirectProperty(annotsArray);
				shared_ptr<CRef> annotsArrayCRef(CRefFactory::getInstance(annotsArrayRef));
				pageDict->addProperty("Annots", *annotsArrayCRef);
				page->getAllAnnotations(annotsList);
				CPPUNIT_ASSERT(annotsList.size()==0);

				// adds text annotation to the array
				annotsArray=pageDict->getProperty<CArray>("Annots");
				libs::Rectangle rect(0,0,100,100);
				shared_ptr<CAnnotation> annot=CAnnotation::createAnnotation(rect, "Text");
				IndiRef annotRef=pdf->addIndirectProperty(annot->getDictionary());
				annotCRef=shared_ptr<CRef>(CRefFactory::getInstance(annotRef));
				annotsArray->addProperty(*annotCRef);
				page->getAllAnnotations(annotsList);
				CPPUNIT_ASSERT(annotsList.size()==1);
				
				// changes reference value of first reference (mess one) to text
				// annotation
				annotCRef=annotsArray->getProperty<CRef>(0);
				annotCRef->setValue(annotRef);
				page->getAllAnnotations(annotsList);
				CPPUNIT_ASSERT(annotsList.size()==2);

				// deletes element from array
				annotsArray->delProperty(0);
				page->getAllAnnotations(annotsList);
				CPPUNIT_ASSERT(annotsList.size()==1);
				
				// changes Annots property reference value to messRef
				annotsArrayCRef=pageDict->getProperty<CRef>("Annots");
				annotsArrayCRef->setValue(messRef);
				page->getAllAnnotations(annotsList);
				CPPUNIT_ASSERT(annotsList.size()==0);

				// restores to original state and number of annotation must
				// be same
				if(annotsProp.get())
				{
					pageDict->setProperty("Annots", *annotsProp);
					page->getAllAnnotations(annotsList);
					CPPUNIT_ASSERT(annotsList.size()==annotsListSize);
				}
			}while(pdf->hasNextPage(page));

		}
	}

	//
	//
	//
	void TestChanges ()
	{
		OUTPUT << "CPage methods..." << endl;

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
		
			BEGIN_CHECK_READONLY;
				TEST(" changes");
				CPPUNIT_ASSERT (change (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}
	//
	//
	//
	void TestMoveUpDown ()
	{
		OUTPUT << "CPage methods..." << endl;

		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
		
			BEGIN_CHECK_READONLY;
				TEST(" move up/down");
				CPPUNIT_ASSERT (move (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}


};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCPage);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCPage, "TEST_CPAGE");

//=====================================================================================
} // namespace
//=====================================================================================

