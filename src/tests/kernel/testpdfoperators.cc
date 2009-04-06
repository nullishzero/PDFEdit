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

#include "kernel/cpage.h"


//=====================================================================================
namespace {
//=====================================================================================

#define WANT_PDF_SAVE	0
	
using namespace pdfobjects;
using namespace boost;

//=====================================================================================

bool
textIter (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		vector<boost::shared_ptr<CContentStream> > ccs;
		page->getContentStreams (ccs);
		shared_ptr<CContentStream> cs = ccs.front();
		
		//typedef vector<boost::shared_ptr<PdfOperator> > Opers;
		//Opers opers;
		//cs->getPdfOperators (opers);
		//assert (!opers.empty());
		
		string str;
		//oss << "---- iterator ----" << flush;
		cs->getStringRepresentation<PdfOperator::Iterator> (str);
		//oss << str << flush;

		oss << "---- Text iterator ----" << flush;
		cs->getStringRepresentation<TextOperatorIterator> (str);
		oss << str << flush;
		
		oss << "---- Inline image iterator ----" << flush;
		cs->getStringRepresentation<InlineImageOperatorIterator> (str);
		oss << str << flush;
		
		_working (oss);
	}
	
	return true;
}

//=====================================================================================

bool
setCS (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t pagecount = ppdf->getPageCount ();
	ppdf.reset();
	for (size_t i = 0; i < pagecount && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);

		vector<boost::shared_ptr<CContentStream> > ccs;
		page->getContentStreams (ccs);
		shared_ptr<CContentStream> cs = ccs.front();
	
		// If size > 1, streams could have been very badly damaged (but we need
		// to be able to handle also this situation)
		// TOTO ... CONTENTSTREAM (fdjalds) TJ <-- font does not exist
	//	boost::shared_ptr<CDict> dict = page->getDictionary();
	//	assert (dict);
	//	boost::shared_ptr<IProperty> ccs = utils::getReferencedObject (dict->getProperty("Contents"));
	//	if (isStream(ccs))
		//	continue;


		
		// parse the content stream
		string tmp;
		cs->getStringRepresentation (tmp);
		//oss << tmp << std::endl;
		
		// parse the content stream after change
		shared_ptr<CStream> stream = getTestStreamContent (page);
		assert (isStream(stream));
		CStream::Buffer buf;
		string tmp1 = "TOTO URCITE NIE JE VALIDNY CONTENTSTREAM";
		std::copy (tmp1.begin(), tmp1.end(), back_inserter (buf));
		stream->setBuffer (buf);
		CPPUNIT_ASSERT (stream->getBuffer().size() == tmp1.size());
		cs->saveChange();
		
		_working (oss);
	}
	
	#if WANT_PDF_SAVE
	pdf->save (true);
	#endif

	return true;
}

//=====================================================================================

bool
delOper (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t pagecount = ppdf->getPageCount ();
	ppdf.reset();
	for (size_t i = 0; i < pagecount && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		vector<boost::shared_ptr<CContentStream> > ccs;
		page->getContentStreams (ccs);
		shared_ptr<CContentStream> cs = ccs.front();
		
		typedef vector<boost::shared_ptr<PdfOperator> > Opers;
		Opers opers;
		cs->getPdfOperators (opers);

		//
		// Delete some objects
		//
		if (opers.empty())
			continue;
		assert (!opers.empty());

		PdfOperator::Iterator it = PdfOperator::getIterator (opers.front());
		for (int i = 0; !it.isEnd() && i < 2; ++i)
			it.next();
		if (it.isEnd())
			return true;
		assert (!it.isEnd());
		for (int i = 0; i < 10; ++i)
		{
			if (!it.isEnd())
				break;
			assert (!it.isEnd());
			PdfOperator::Iterator itt = it;
			itt.prev();
			assert (!itt.isBegin());
			cs->deleteOperator (it, false);
			it = itt.next();
		}
		
		_working (oss);
		
		//
		// Delete text objects
		//
		assert (!opers.empty());
		TextOperatorIterator tit = PdfOperator::getIterator<TextOperatorIterator> (opers.front());
		while (!tit.isEnd())
		{
			TextOperatorIterator itt = tit;
			itt.prev();
			cs->deleteOperator (tit, false);
			if (itt.isBegin())
			{
				cs->getPdfOperators (opers);
				assert (!opers.empty());
				itt = PdfOperator::getIterator<TextOperatorIterator> (opers.front());
			}else
				itt.next();
			tit = itt;
		}
		
		_working (oss);

		assert (!opers.empty());
		cs->getPdfOperators (opers);
		PdfOperator::Iterator nit = PdfOperator::getIterator (opers.front());
		while (!nit.isEnd())
		{
			string str;
			nit.getCurrent()->getOperatorName (str);
	//		oss << str << "-" << flush;
			nit.next();
		}
		
		//
		// Save the contentstream
		//
		string tmp;
		cs->getStringRepresentation (tmp);
	//	oss << tmp << flush;

		cs->saveChange();

		_working (oss);
	}
	
	#if WANT_PDF_SAVE
	pdf->save (true);
	#endif

	return true;
}

//=====================================================================================

bool
delAllOper (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t pagecount = ppdf->getPageCount ();
	ppdf.reset();
	for (size_t i = 0; i < pagecount && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);

		vector<boost::shared_ptr<CContentStream> > ccs;
		page->getContentStreams (ccs);
		shared_ptr<CContentStream> cs = ccs.front();
		
		typedef vector<boost::shared_ptr<PdfOperator> > Opers;
		Opers opers;
		cs->getPdfOperators (opers);

		//
		// Delete all objects
		//
		if (opers.empty())
			continue;
		assert (!opers.empty());

		PdfOperator::Iterator it = PdfOperator::getIterator (opers.front());
		while (!it.isEnd())
		{
			PdfOperator::Iterator itt = it;
			itt.prev();
			cs->deleteOperator (it, false);
			if (itt.isBegin())
			{
				cs->getPdfOperators (opers);
				if (opers.empty())
					break;
				itt = PdfOperator::getIterator (opers.front());
			}else
				itt.next();
			it = itt;
		}

		// Should be empty
		cs->getPdfOperators (opers);
		assert (opers.empty());

		//
		// Insert
		//
		PdfOperator::Operands operands;
		shared_ptr<PdfOperator> oper;
		string strr;

		oper = createOperator ("BT", operands);
		cs->insertOperator (PdfOperator::Iterator (), oper, false);
		cs->getPdfOperators (opers);

		operands.clear ();
//		operands.push_back (shared_ptr<IProperty> (new CName ("R19")));
//		operands.push_back (shared_ptr<IProperty> (new CInt (42)));
//		oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("Tf", 2, operands));
//		cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
//		cs->getPdfOperators (opers);
//		cs->getStringRepresentation (strr);
//		_working (oss);

		operands.clear ();
		operands.push_back (shared_ptr<IProperty> (new CInt (200)));
		operands.push_back (shared_ptr<IProperty> (new CInt (400)));
		oper = createOperator ("Td", operands);
		cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
		cs->getPdfOperators (opers);
		cs->getStringRepresentation (strr);
		_working (oss);

	/*	operands.clear ();
		oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("q", 0, operands));
		cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
		cs->getPdfOperators (opers);
		cs->getStringRepresentation (strr);
		_working (oss);
	*/
		operands.clear ();
		operands.push_back (shared_ptr<IProperty> (new CReal (1.0)));
		operands.push_back (shared_ptr<IProperty> (new CReal (0.0)));
		operands.push_back (shared_ptr<IProperty> (new CReal (0.0)));
		oper = createOperator ("rg", operands);
		cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
		cs->getPdfOperators (opers);
		cs->getStringRepresentation (strr);
		_working (oss);

	/*	operands.clear ();
		oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("Q", 0, operands));
		cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
		cs->getPdfOperators (opers);
		cs->getStringRepresentation (strr);
		_working (oss);
	*/
		operands.clear ();
		operands.push_back (shared_ptr<IProperty> (new CString ("halooooooooooo")));
		oper = createOperator ("Tj", operands);
		cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
		cs->getPdfOperators (opers);
		cs->getStringRepresentation (strr);
		_working (oss);

		oper = createOperator ("ET", operands);
		cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
		cs->getPdfOperators (opers);
		cs->getStringRepresentation (strr);
		_working (oss);
		
		cs->saveChange();
	}

	#if WANT_PDF_SAVE
	pdf->save (true);
	#endif

	return true;
}


//=====================================================================================

bool
insertOper (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t pagecount = ppdf->getPageCount ();
	ppdf.reset();
	for (size_t i = 0; i < pagecount && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		vector<boost::shared_ptr<CContentStream> > ccs;
		page->getContentStreams (ccs);
		shared_ptr<CContentStream> cs = ccs.front();
		
		typedef vector<boost::shared_ptr<PdfOperator> > Opers;
		Opers opers;
		cs->getPdfOperators (opers);

		//
		// Create an operator
		//
		PdfOperator::Operands operands;
		operands.push_back (shared_ptr<IProperty> (new CString ("halooooooooooo")));
		shared_ptr<PdfOperator> oper = createOperator ("Tj", operands);
		
		string str;
		//cs->getStringRepresentation (str);
		_working (oss);
		// Insert after 10. element, hopefully around text somewhere
		if (opers.empty())
			continue;
		assert (!opers.empty());

		PdfOperator::Iterator it = PdfOperator::getIterator (opers.front());
		for (int i = 0; i < 10 && !it.isEnd(); it.next(), ++i)
			;
		if (it.isEnd())
			it = PdfOperator::getIterator (opers.front());
		assert (!it.isEnd());
		cs->insertOperator (it, oper);
		//cs->getStringRepresentation (str);
		oss << str << flush;

		cs->saveChange();
	}
	
	#if WANT_PDF_SAVE
	pdf->save (true);
	#endif

	return true;
}

//=====================================================================================

bool
cloneoper (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t pagecount = ppdf->getPageCount ();
	ppdf.reset();
	for (size_t i = 0; i < pagecount && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);

		vector<boost::shared_ptr<CContentStream> > ccs;
		page->getContentStreams (ccs);
		shared_ptr<CContentStream> cs = ccs.front();
		
		typedef vector<boost::shared_ptr<PdfOperator> > Opers;
		Opers opers;
		cs->getPdfOperators (opers);

		if (opers.empty())
			continue;
		
		assert (!opers.empty());
		PdfOperator::Iterator it = PdfOperator::getIterator (opers.front());
		while (!it.isEnd())
		{
			shared_ptr<PdfOperator> clone = it.getCurrent()->clone();
			std::string olds,news;
			it.getCurrent()->getStringRepresentation (olds);
			clone->getStringRepresentation (news);
			CPPUNIT_ASSERT (olds == news);

			_working (oss);
			
			it.next();
		}
		
	}

	return true;
}

//=====================================================================================

bool
changeColor (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t pagecount = ppdf->getPageCount ();
	ppdf.reset();
	for (size_t i = 0; i < pagecount && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);

		vector<boost::shared_ptr<CContentStream> > ccs;
		page->getContentStreams (ccs);
		shared_ptr<CContentStream> cs = ccs.front();
		
		typedef vector<boost::shared_ptr<PdfOperator> > Opers;
		Opers opers;
		cs->getPdfOperators (opers);

		//
		// Change text to red
		//
		if (opers.empty())
			continue;
		
		assert (!opers.empty());
		TextOperatorIterator it = PdfOperator::getIterator<TextOperatorIterator> (opers.front());
		boost::shared_ptr<PdfOperator> tmpop;
		if (!it.isEnd()) //while (!it.isEnd())
		{
			std::string tmp;
			it.getCurrent()->getStringRepresentation (tmp);
			
			// This will change iterator list of it.getCurrent() !!!
			//tmpop = operatorSetColor (it.getCurrent()->clone(), 1, 0, 0);
			//cs->replaceOperator (it, tmpop, false);
			//cs->replaceOperator (it, operatorSetColor (it.getCurrent(), 1, 0, 0), itPrv, itNxt, false);
			std::string tmp1;
			//tmpop->getStringRepresentation (tmp1);

			//oss << tmp << endl;
			//oss << "[" << tmp1 << "]" << endl;
			//oss << string("[q 1 0 0 rg ") + tmp + string(" Q ]") << flush;
			//CPPUNIT_ASSERT (string("q 1 0 0 rg ") + tmp + string(" Q ") == tmp1);
			
			//it = PdfOperator::getIterator<TextOperatorIterator> (getLastOperator(tmpop));
		}
		
		_working (oss);

		cs->saveChange();
	}

	#if WANT_PDF_SAVE
	pdf->save (true);
	#endif

	return true;
}


//=========================================================================
// class TestPdfOperators
//=========================================================================

class TestPdfOperators : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestPdfOperators);
		CPPUNIT_TEST(TestSetCS);
		CPPUNIT_TEST(TestChangeColor);
		CPPUNIT_TEST(TestDeleteOper);
		CPPUNIT_TEST(TestInsertOper);
		CPPUNIT_TEST(TestDeleteAllInsertOper);
		CPPUNIT_TEST(TestTextIterator);
		CPPUNIT_TEST(TestPdfOperClone);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {OUTPUT << endl;}
	void tearDown() {}

public:
	//
	//
	//
	void TestSetCS ()
	{
		OUTPUT << "PDfOperators..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			BEGIN_CHECK_READONLY;
				TEST(" set cs ");
				CPPUNIT_ASSERT (setCS (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}
	//
	//
	//
	void TestDeleteOper ()
	{
		OUTPUT << "Delete PDfOperator..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			BEGIN_CHECK_READONLY;
				TEST(" del oper");
				CPPUNIT_ASSERT (delOper (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}
	//
	//
	//
	void TestDeleteAllInsertOper ()
	{
		OUTPUT << "Delete PDfOperator..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			BEGIN_CHECK_READONLY;
				TEST(" del all oper");
				CPPUNIT_ASSERT (delAllOper (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}

	//
	//
	//
	void TestInsertOper ()
	{
		OUTPUT << "Insert PDfOperator..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			BEGIN_CHECK_READONLY;
				TEST(" insert oper");
				CPPUNIT_ASSERT (insertOper (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}
	//
	//
	//
	void TestTextIterator ()
	{
		OUTPUT << "Text iterator..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" text iterator");

			#if REALLY_ALL_OUTPUT
				CPPUNIT_ASSERT (cstreamsreader (OUTPUT, (*it).c_str()));
			#else
				std::ofstream o;
				o.open ("/dev/null");
				CPPUNIT_ASSERT (textIter (o, (*it).c_str()));
			#endif
			OK_TEST;
		}
	}
	//
	//
	//
	void TestChangeColor ()
	{
		OUTPUT << "Change color..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			BEGIN_CHECK_READONLY;
				TEST(" change color");
				CPPUNIT_ASSERT (changeColor (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}
	//
	//
	//
	void TestPdfOperClone ()
	{
		OUTPUT << "Clone PDfOperator..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" clone oper");
			CPPUNIT_ASSERT (cloneoper (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestPdfOperators);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestPdfOperators, "TEST_PDFOPERS");

//=====================================================================================
} // namespace
//=====================================================================================








