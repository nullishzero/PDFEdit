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
position (ostream& oss, const char* fileName, const Rectangle rc)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName));
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();

	// parse the content stream
	//page->parseContentStream ();
	
	std::vector<shared_ptr<PdfOperator> > ops;
	page->getObjectsAtPosition (ops, rc);

	oss << " Found objects #" << ops.size();

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

namespace  {
	
	bool img (Parser* parser, Object& o, XRef* xref)
	{
		  std::cout << "Inline image data (BI)" << flush;
		  
		  //
		  // Read from BI to ID
		  //
		  Object dict;
		  dict.initDict (xref);
		  o.free ();
		  
		  parser->getObj(&o);
		  while (!o.isCmd("ID") && !o.isEOF()) 
		  {
			if (!o.isName()) 
			{
			  std::cout << "No name" << flush;
			  o.free ();
			  
			}else if (o.isName())
			{
			  char* key = copyString(o.getName());
			  o.free();
			  parser->getObj(&o);
			  if (o.isEOF() || o.isError()) 
			  {
				gfree (key);
				return false;
			  }
			  dict.dictAdd(key, &o);
			}
			
			parser->getObj(&o);
		  }
		 
		  
		  //
		  // Read the image itself until EI
		  //
		  o.free();
		  Stream *str;	
		  // make stream
		  str = new EmbedStream(parser->getStream(), &dict, gFalse, 0);
		  str = str->addFilters(&dict);
		  int c1, c2;
		  if (str) 
		  {
			c1 = str->getBaseStream()->getChar();
			c2 = str->getBaseStream()->getChar();
			while (!(c1 == 'E' && c2 == 'I') && c2 != EOF) 
			{
			  c1 = c2;
			  c2 = str->getBaseStream()->getChar();
			}
			delete str;
			return true;

		  }else // if (str)
		  {
			  std::cout << "Bad str." << flush;
			  return false;
		  }
	}
} // namespace

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

	scoped_ptr<Parser> parser (new Parser (xref, new Lexer(xref, &obj)));
	
	Object o;
	parser->getObj (&o);
	int i = 0;
	
	while (!o.isEOF()) 
	{
		if (o.isCmd ("BI"))
		{
			if (!img (parser.get(),o,xref))
				return false;
	    	parser->getObj(&o);
		}
		
		if (o.isCmd ())
		{
			i++;
			//oss << o.getCmd() << flush;
		}
		else
		{
			//oss << "(" << o.getType() << ")" << flush;
			//std::string tmp;
			//utils::xpdfObjToString (o, tmp);
			//oss << tmp << " " << flush;
		}

		// grab the next object
		o.free ();
		parser->getObj(&o);
	}

	oss << "Operands count: " << i << flush;

	obj.free ();
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
	if (page)
	{
		page->getContentStream()->getStringRepresentation (str);
	}
	else 
		return false;
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
			CPPUNIT_ASSERT (position (OUTPUT, (*it).c_str(), Rectangle (100,100,300,300)));
			CPPUNIT_ASSERT (position (OUTPUT, (*it).c_str(), Rectangle (10,10,50,50)));
			CPPUNIT_ASSERT (position (OUTPUT, (*it).c_str(), Rectangle (400,400,450,450)));
			OK_TEST;

			TEST(" print contentstream");
			//CPPUNIT_ASSERT (printContentStream (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCContentStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCContentStream, "TEST_CCONTENTSTREAM");

//=====================================================================================
} // namespace
//=====================================================================================








