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

using namespace boost;


//=====================================================================================

bool
position (ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName));
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();

	
	std::vector<shared_ptr<PdfOperator> > ops;
	page->getObjectsAtPosition (ops, Rectangle (1,1,10,10));

	oss << "Found objects #" << ops.size();

	std::vector<shared_ptr<PdfOperator> >::iterator it = ops.begin ();
	for (; it != ops.end(); ++it)
	{
		std::string tmp;
		(*it)->getOperatorName (tmp);
		//oss << tmp;
	}
	//oss << std::endl;
	
	return true;
}

//=====================================================================================

bool
opcount (ostream& oss, const char* fileName)
{
	boost::scoped_ptr<PDFDoc> doc (new PDFDoc (new GString(fileName), NULL, NULL));
	int pagesNum = 1;
	
	//
	// Our stuff here
	//
	Object obj;
	XRef* xref = doc->getXRef();
	assert (xref);
	Catalog cat (xref);
	if (1 > cat.getNumPages())
		return true;

	cat.getPage(pagesNum)->getContents(&obj);

	scoped_ptr<Parser> parser (new Parser (NULL, new Lexer(NULL, &obj)));
	
	Object o;
	parser->getObj (&o);
	int i = 0;
	
	while (!o.isEOF()) 
	{
		if (o.isCmd ())
			i++;

		// grab the next object
		parser->getObj(&o);
	}

	
	obj.free ();

	oss << "Operands count: " << i;

	return true;
}


//=====================================================================================

bool
printContentStream (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::scoped_ptr<CPdf> pdf (getTestCPdf (fileName));
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();

	// Print content stream
	string str;
	page->getContentStream()->getStringRepresentation (str);
	//oss << "Content stream representation: " << str << endl;

	return true;
}



//=========================================================================
// class TestCContentStream
//=========================================================================

class TestCContentStream : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCContentStream);
		CPPUNIT_TEST(Test);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {OUTPUT << endl;}
	void tearDown() {}

public:
	void Test()
	{
		OUTPUT << "CContentStream..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" opcount");
			CPPUNIT_ASSERT (opcount (OUTPUT, (*it).c_str()));
			OK_TEST;

			TEST(" getPosition");
			CPPUNIT_ASSERT (position (OUTPUT, (*it).c_str()));
			OK_TEST;

			TEST(" print contentstream");
			CPPUNIT_ASSERT (printContentStream (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCContentStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCContentStream, "TEST_CCONTENTSTREAM");

//=====================================================================================
} // namespace
//=====================================================================================








