// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  testccontentsream.cc
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
	//pdf->save (true);
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

	string tmp;
	for (Opers::iterator it = opers.begin(); it != opers.end(); ++it)
	{
		string str;
		//(*it)->getStringRepresentation (str);
		//tmp += str;
		(*it)->getOperatorName (str);
		oss << "---" << str << flush;
	}
	
/*	ofstream of;
	of.open ("before.txt");
	of << tmp;
	of.close();
*/

	//
	// Delete som objects
	//
	PdfOperator::Iterator it = PdfOperator::getIterator (opers.front());
	for (int i = 0; i < 20; ++i)
		it.next();
	for (int i = 0; i < 10; ++i)
	{
		PdfOperator::Iterator itt = it;
		itt.next();
		cs->deleteOperator (it);
		it = itt;
	}
	
/*	{
		string str;
		cs->getStringRepresentation (str);
		ofstream of;
		of.open ("after.txt");
		of << str;
		of.close();
	}
*/
	
	return true;
}

//=========================================================================
// class TestPdfOperators
//=========================================================================

class TestPdfOperators : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestPdfOperators);
		CPPUNIT_TEST(TestSetCS);
		CPPUNIT_TEST(TestDeleteOper);
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestPdfOperators);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestPdfOperators, "TEST_PDFOPERS");

//=====================================================================================
} // namespace
//=====================================================================================








