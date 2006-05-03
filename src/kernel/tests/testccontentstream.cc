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
position (ostream& oss, const char* fileName, const Rectangle rc)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName));
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();

	// parse the content stream
	page->parseContentStream ();
	
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
	
	bool img (CStream& str)
	{
		  //
		  // Read from BI to ID
		  //
		  Object dict;
		  XRef* xref = (NULL != str.getPdf()) ? str.getPdf()->getCXref() : NULL;
		  dict.initDict (xref);
		  Object o;
		  str.getXpdfObject (o);
		 
		  while (!o.isCmd("ID") && !str.eof()) 
		  {
			if (!o.isName()) 
			{
			  std::cout << "No name" << flush;
			  
			}else if (o.isName())
			{
			  char* key = copyString(o.getName());
			  str.getXpdfObject (o);
			  if (o.isEOF() || o.isError()) 
			  {
				gfree (key);
				return false;
			  }
			  dict.dictAdd(key, &o);
			}
			
		  	str.getXpdfObject (o);
		  }
		 
		  //
		  // Read the image itself until EI
		  //
		  o.free();
		  Stream* xstr;	
		  // make stream
		  xstr = new EmbedStream(str.getXpdfStream(), &dict, gFalse, 0);
		  xstr = xstr->addFilters(&dict);
		  int c1, c2;
		  if (xstr) 
		  {
			c1 = xstr->getBaseStream()->getChar();
			c2 = xstr->getBaseStream()->getChar();
			while (!(c1 == 'E' && c2 == 'I') && c2 != EOF) 
			{
			  c1 = c2;
			  c2 = xstr->getBaseStream()->getChar();
			}
			dict.free ();
			delete xstr;
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
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName));
	if (1 > pdf->getPageCount())
		return true;

	/// Intermezzo
	boost::scoped_ptr<PDFDoc> doc (new PDFDoc (new GString(fileName), NULL, NULL));
	int pagesNum = 1;
	Object obj;
	XRef* xref = doc->getXRef();
	assert (xref);
	Catalog cat (xref);
	if (1 > cat.getNumPages())
		return true;
	cat.getPage(pagesNum)->getContents(&obj);
	////

	
	typedef vector<shared_ptr<CStream> > Streams;
	Streams streams;
	
	if (obj.isStream ())
	{
		streams.push_back ( shared_ptr<CStream> (new CStream (*pdf, obj, IndiRef()) ));
		
	}else if (obj.isArray())
	{
		Object o;
		for (int i = 0; i < obj.arrayGetLength(); ++i)
			streams.push_back ( shared_ptr<CStream> (new CStream (*pdf, 
																  *(obj.arrayGet (i, &o)),
																  IndiRef() 
																  ) 
													)
								);
	}
	obj.free ();

	int i = 0;
	for (Streams::iterator it = streams.begin (); it != streams.end(); ++it )
	{
		::Object o;
		CStream& str = *(*it);
		
		str.open ();
		testPrintDbg (debug::DBG_DBG, "");

		while (!str.eof())
		{
			// grab the next object
			o.free ();
			str.getXpdfObject (o);

			if (o.isCmd ("BI"))
			{
				if (!img (str))
					return false;
				testPrintDbg (debug::DBG_DBG, "end image...");
			}
			
			if (o.isCmd ())
			{
				i++;
				//oss << o.getCmd() << flush;
			}else
			{
				//oss << "(" << o.getType() << ")" << flush;
				//std::string tmp;
				//utils::xpdfObjToString (o, tmp);
				//oss << tmp << " " << flush;
			}

		}

		testPrintDbg (debug::DBG_DBG, "close stream");
		str.close ();
	}
	
	oss << "Operands count: " << i << flush;

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
		CPPUNIT_TEST(TestOpcount);
		CPPUNIT_TEST(TestPosition);
		CPPUNIT_TEST(TestPrint);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {OUTPUT << endl;}
	void tearDown() {}

public:
	//
	//
	//
	void TestOpcount ()
	{
		OUTPUT << "CContentStream..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" opcount");
			CPPUNIT_ASSERT (opcount (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestPosition ()
	{
		OUTPUT << "CContentStream..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" getPosition");
			CPPUNIT_ASSERT (position (OUTPUT, (*it).c_str(), Rectangle (100,100,300,300)));
			MEM_CHECK;
			CPPUNIT_ASSERT (position (OUTPUT, (*it).c_str(), Rectangle (10,10,50,50)));
			MEM_CHECK;
			CPPUNIT_ASSERT (position (OUTPUT, (*it).c_str(), Rectangle (400,400,450,450)));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestPrint ()
	{
		OUTPUT << "CContentStream..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

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








