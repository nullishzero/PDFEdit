// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  testpdfoperators.cc
 *         Created:  04/20/2006 23:47:27 AM CEST
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

#include "testmain.h"
#include "testcobject.h"
#include "testcpage.h"
#include "testcpdf.h"

#include <PDFDoc.h>
#include "../cpage.h"


//=====================================================================================
namespace {
//=====================================================================================

#define WANT_PDF_SAVE	0
	
using namespace pdfobjects;
using namespace boost;

//=====================================================================================

bool
textIter (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();
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
	oss << "-----------" << flush;

	return true;
}

//=====================================================================================

bool
setCS (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();
	vector<boost::shared_ptr<CContentStream> > ccs;
	page->getContentStreams (ccs);
	shared_ptr<CContentStream> cs = ccs.front();
	

	// parse the content stream
	string tmp;
	cs->getStringRepresentation (tmp);
	//oss << tmp << std::endl;
	
	// parse the content stream after change
	shared_ptr<CDict> dict = page->getDictionary ();
	shared_ptr<CStream> stream = utils::getCStreamFromDict (dict, "Contents");
	assert (isStream(stream));
	//CStream::Buffer buf;
	//std::copy (tmp.begin(), tmp.end(), back_inserter (buf));
	//stream->setBuffer (buf);
	//cs->getStringRepresentation (tmp);
	//oss << tmp << std::endl;
	cs->saveChange();
	#if WANT_PDF_SAVE
	pdf->save (true);
	#endif
	return true;
}

//=====================================================================================

bool
delOper (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();
	vector<boost::shared_ptr<CContentStream> > ccs;
	page->getContentStreams (ccs);
	shared_ptr<CContentStream> cs = ccs.front();
	
	typedef vector<boost::shared_ptr<PdfOperator> > Opers;
	Opers opers;
	cs->getPdfOperators (opers);

	//
	// Delete some objects
	//
	assert (!opers.empty());
	PdfOperator::Iterator it = PdfOperator::getIterator (opers.front());
	for (int i = 0; !it.isEnd() && i < 2; ++i)
		it.next();
	if (!it.isEnd())
		return true;
	assert (!it.isEnd());
	for (int i = 0; i < 10; ++i)
	{
		if (!it.isEnd())
			break;
		assert (!it.isEnd());
		PdfOperator::Iterator itt = it;
		itt.prev();
		assert (!itt.isEnd());
		cs->deleteOperator (it, false);
		it = itt.next();
	}
	oss << ".." << flush;
	
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
		if (itt.isEnd())
		{
			cs->getPdfOperators (opers);
			assert (!opers.empty());
			itt = PdfOperator::getIterator<TextOperatorIterator> (opers.front());
		}else
			itt.next();
		tit = itt;
	}
	oss << ".." << flush;

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
	oss << ".." << flush;

	//
	// Save the contentstream
	//
	string tmp;
	cs->getStringRepresentation (tmp);
//	oss << tmp << flush;

	cs->saveChange();
	#if WANT_PDF_SAVE
	pdf->save (true);
	#endif
	return true;
}

//=====================================================================================

bool
delAllOper (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();
	vector<boost::shared_ptr<CContentStream> > ccs;
	page->getContentStreams (ccs);
	shared_ptr<CContentStream> cs = ccs.front();
	
	typedef vector<boost::shared_ptr<PdfOperator> > Opers;
	Opers opers;
	cs->getPdfOperators (opers);

	//
	// Delete all objects
	//
	assert (!opers.empty());
	PdfOperator::Iterator it = PdfOperator::getIterator (opers.front());
	while (!it.isEnd())
	{
		PdfOperator::Iterator itt = it;
		itt.prev();
		cs->deleteOperator (it, false);
		if (itt.isEnd())
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

	oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("BT", 0, operands));
	cs->insertOperator (PdfOperator::Iterator (), oper, false);
	cs->getPdfOperators (opers);

	operands.clear ();
	operands.push_back (shared_ptr<IProperty> (new CName ("R19")));
	operands.push_back (shared_ptr<IProperty> (new CInt (42)));
	oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("Tf", 2, operands));
	cs->insertOperator (PdfOperator::getIterator(opers.back()), oper);
	//cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
	cs->getPdfOperators (opers);
	cs->getStringRepresentation (strr);
	oss << strr << "--" << flush;

	operands.clear ();
	operands.push_back (shared_ptr<IProperty> (new CInt (200)));
	operands.push_back (shared_ptr<IProperty> (new CInt (400)));
	oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("Td", 2, operands));
	cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
	cs->getPdfOperators (opers);
	cs->getStringRepresentation (strr);
	//oss << strr << "--" << flush;

/*	operands.clear ();
	oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("q", 0, operands));
	cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
	cs->getPdfOperators (opers);
	cs->getStringRepresentation (strr);
	//oss << strr << "--" << flush;
*/
	operands.clear ();
	operands.push_back (shared_ptr<IProperty> (new CReal (1.0)));
	operands.push_back (shared_ptr<IProperty> (new CReal (0.0)));
	operands.push_back (shared_ptr<IProperty> (new CReal (0.0)));
	oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("rg", 3, operands));
	cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
	cs->getPdfOperators (opers);
	cs->getStringRepresentation (strr);
	//oss << strr << "--" << flush;

/*	operands.clear ();
	oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("Q", 0, operands));
	cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
	cs->getPdfOperators (opers);
	cs->getStringRepresentation (strr);
	//oss << strr << "--" << flush;
*/
	operands.clear ();
	operands.push_back (shared_ptr<IProperty> (new CString ("halooooooooooo")));
	oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("Tj", 1, operands));
	cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
	cs->getPdfOperators (opers);
	cs->getStringRepresentation (strr);
	//oss << strr << "--" << flush;

	oper = shared_ptr<PdfOperator> (new SimpleGenericOperator ("ET", 0, operands));
	cs->insertOperator (PdfOperator::getIterator(opers.back()), oper, false);
	cs->getPdfOperators (opers);
	cs->getStringRepresentation (strr);
	//oss << strr << "--" << flush;
	
	cs->saveChange();
	#if WANT_PDF_SAVE
	pdf->save (true);
	#endif
	return true;
}


//=====================================================================================

bool
insertOper (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();
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
	shared_ptr<PdfOperator> oper (new SimpleGenericOperator ("Tj", 1, operands));
	
	string str;
	//cs->getStringRepresentation (str);
	oss << str << "-----" << flush;
	// Insert after 10. element, hopefully around text somewhere
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
	#if WANT_PDF_SAVE
	pdf->save (true);
	#endif
	return true;
}


//=====================================================================================

bool
changeColor (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();
	vector<boost::shared_ptr<CContentStream> > ccs;
	page->getContentStreams (ccs);
	shared_ptr<CContentStream> cs = ccs.front();
	
	typedef vector<boost::shared_ptr<PdfOperator> > Opers;
	Opers opers;
	cs->getPdfOperators (opers);

	//
	// Change text to red
	//
	assert (!opers.empty());
	TextOperatorIterator it = PdfOperator::getIterator<TextOperatorIterator> (opers.front());
	while (!it.isEnd())
	{
		PdfOperator::Iterator itPrv = it; itPrv.prev();
		PdfOperator::Iterator itNxt = it; itNxt.next();
		// This will change iterator list of it.getCurrent() !!!
		boost::shared_ptr<PdfOperator> op = operatorSetColor (it.getCurrent(), 1, 0, 0);
		cs->replaceOperator (it, op, itPrv, itNxt, false);
		//cs->replaceOperator (it, operatorSetColor (it.getCurrent(), 1, 0, 0), itPrv, itNxt, false);
		
		it.next();
	}
	oss << ".." << flush;


	cs->saveChange();
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
		CPPUNIT_TEST(TestChangeColor);
		CPPUNIT_TEST(TestSetCS);
		CPPUNIT_TEST(TestDeleteOper);
		CPPUNIT_TEST(TestInsertOper);
		CPPUNIT_TEST(TestDeleteAllInsertOper);
		CPPUNIT_TEST(TestTextIterator);
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
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" set cs");
			CPPUNIT_ASSERT (setCS (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestDeleteOper ()
	{
		OUTPUT << "Delete PDfOperator..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" del oper");
			CPPUNIT_ASSERT (delOper (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestDeleteAllInsertOper ()
	{
		OUTPUT << "Delete PDfOperator..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" del all oper");
			CPPUNIT_ASSERT (delAllOper (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

	//
	//
	//
	void TestInsertOper ()
	{
		OUTPUT << "Insert PDfOperator..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" insert oper");
			CPPUNIT_ASSERT (insertOper (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestTextIterator ()
	{
		OUTPUT << "Text iterator..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" text iterator");
			CPPUNIT_ASSERT (textIter (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestChangeColor ()
	{
		OUTPUT << "Change color..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" change color");
			CPPUNIT_ASSERT (changeColor (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestPdfOperators);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestPdfOperators, "TEST_PDFOPERS");

//=====================================================================================
} // namespace
//=====================================================================================








