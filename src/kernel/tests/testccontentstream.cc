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
setCS (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf (getTestCPdf (fileName), pdf_deleter());
	size_t pagecnt = ppdf->getPageCount ();
	ppdf.reset();
	
	for (size_t i = 0; i < pagecnt && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		vector<boost::shared_ptr<CContentStream> > ccs;
		page->getContentStreams (ccs);
		assert (!ccs.empty());
		shared_ptr<CContentStream> cs = ccs.front();

		// parse the content stream
		string tmp1, tmp2;
		cs->getStringRepresentation (tmp1);
		//oss << tmp << std::endl;
		ofstream of1;
		of1.open ("1.txt");
		of1 << tmp1 << flush;
		of1.close();
	
		// parse the content stream after change
		CStream::Buffer buf;
		std::copy (tmp1.begin(), tmp1.end(), back_inserter (buf));
		assert (tmp1.size() == buf.size());
		//
		// We CAN NOT do this directly, because contentcstream can be in more cstreams
		//
		boost::shared_ptr<CDict> dict = page->getDictionary();
		assert (dict);
		boost::shared_ptr<IProperty> cstream = utils::getReferencedObject (dict->getProperty("Contents"));
		if (isStream(cstream))
			IProperty::getSmartCObjectPtr<CStream> (cstream)->setBuffer (buf);
		cs->getStringRepresentation (tmp2);
		
		//
		// Compare string before and after change
		//
		if (tmp1 != tmp2)
		{
			ofstream of2;
			of2.open("2.txt");
			of2 << tmp2 << flush;
			of2.close();
		}
		CPPUNIT_ASSERT (tmp1 == tmp2);
		CPPUNIT_ASSERT (tmp1.size() == tmp2.size());
		//oss << tmp << std::endl;
		_working (oss);
	
		//pdf->save (true);
	}
	
	return true;
}


//=====================================================================================

bool
position (ostream& oss, const char* fileName, const Rectangle rc)
{
	boost::shared_ptr<CPdf> ppdf (getTestCPdf (fileName), pdf_deleter());
	size_t pagecnt = ppdf->getPageCount ();
	ppdf.reset();
	
	for (size_t i = 0; i < pagecnt && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
		boost::shared_ptr<CPage> page = pdf->getPage (i + 1);
		
		// parse the content stream
		page->parseContentStream ();
		
		std::vector<shared_ptr<PdfOperator> > ops;
		page->getObjectsAtPosition (ops, rc);

		oss << " Found objects #" << ops.size() << flush;

		std::vector<shared_ptr<PdfOperator> >::iterator it = ops.begin ();
		for (; it != ops.end(); ++it)
		{
			std::string tmp;
			(*it)->getOperatorName (tmp);
			//oss << tmp;
		}
		//oss << std::endl;
	}
	
	return true;
}

//=====================================================================================

namespace  {
	
	template <typename T>
	bool img (T& str, XRef* xref)
	{
		  //
		  // Read from BI to ID
		  //
		  Object dict;
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
opcount (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	/// Intermezzo
	boost::scoped_ptr<PDFDoc> doc  (new PDFDoc (new GString(fileName), NULL, NULL));

	for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		int pagesNum = i + 1;
		Object obj;
		XRef* xref = doc->getXRef();
		assert (xref);
		Catalog cat (xref);
		if (1 > cat.getNumPages())
			return true;
		cat.getPage(pagesNum)->getContents(&obj);
		IndiRef rf;
		rf.num = cat.getPageRef(pagesNum)->num;
		rf.gen = cat.getPageRef(pagesNum)->gen;
		////
		
		typedef vector<shared_ptr<CStream> > Streams;
		Streams streams;
		
		if (obj.isStream ())
		{
			streams.push_back ( shared_ptr<CStream> (new CStream (*pdf, obj, rf)));
			
		}else if (obj.isArray())
		{
			Object o;
			for (int i = 0; i < obj.arrayGetLength(); ++i)
				streams.push_back ( shared_ptr<CStream> (new CStream (*pdf,  *(obj.arrayGet (i, &o)), rf)));
		}
		obj.free ();

		CStreamXpdfReader<Streams> reader (streams);
		::Object o;		
		reader.open ();

		reader.getXpdfObject (o);
		while (!reader.eof())
		{

			if (o.isCmd ("BI"))
			{
				if (!img (reader,pdf->getCXref()))
					return false;
				//testPrintDbg (debug::DBG_DBG, "end image...");
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

			// grab the next object
			o.free ();
			reader.getXpdfObject (o);
		}

		reader.close ();
		
		oss << " Op#: " << i << flush;
	
	}
	
	return true;
}


//=====================================================================================

bool
printContentStream (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i + 1);

		// Print content stream
		string str;
		if (page)
		{
			vector<boost::shared_ptr<CContentStream> > ccs;
			page->getContentStreams (ccs);
			assert (!ccs.empty());
			shared_ptr<CContentStream> cs = ccs.front();
			cs->getStringRepresentation (str);
		}
		else 
			return false;
		
		//oss << "Content stream representation: " << str << endl;
		_working (oss);
	}
	
	return true;
}


//=========================================================================

bool
front (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i + 1);

		// Print content stream
		string str;
		if (page)
		{
			double tm[6] = {1,2,3,4,5,6};
			vector<boost::shared_ptr<CContentStream> > ccs;
			page->getContentStreams (ccs);
			page->setTransformMatrix (tm);
			
			assert (!ccs.empty());
			shared_ptr<CContentStream> cs = ccs.front();
			cs->getStringRepresentation (str);
			oss << str << flush;
		}
		else 
			return false;
		
		//oss << "Content stream representation: " << str << endl;
		_working (oss);
	}
	
	return true;
}


//=========================================================================


bool
primitiveprintContentStream (__attribute__((unused))	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	if (1 > pdf->getPageCount())
		return true;

	/// Intermezzo
	boost::scoped_ptr<PDFDoc> doc  (new PDFDoc (new GString(fileName), NULL, NULL));
	int pagesNum = 1;
	Object obj;
	XRef* xref = doc->getXRef();
	assert (xref);
	Catalog cat (xref);
	if (1 > cat.getNumPages())
		return true;
	cat.getPage(pagesNum)->getContents(&obj);
	IndiRef rf;
	rf.num = cat.getPageRef(pagesNum)->num;
	rf.gen = cat.getPageRef(pagesNum)->gen;
	////

	
	typedef vector<shared_ptr<CStream> > Streams;
	Streams streams;
	
	if (obj.isStream ())
	{
		streams.push_back ( shared_ptr<CStream> (new CStream (*pdf, obj, rf)));
		
	}else if (obj.isArray())
	{
		Object o;
		for (int i = 0; i < obj.arrayGetLength(); ++i)
			streams.push_back ( shared_ptr<CStream> (new CStream (*pdf,  *(obj.arrayGet (i, &o)), rf)));
	}
	obj.free ();

	CStreamXpdfReader<Streams> reader (streams);
		
	::Object o;
		
	reader.open ();

	// grab the next object
	reader.getXpdfObject (o);
	int i = 0;
	while (!reader.eof())
	{
		i++;
		if (410 == i)
		{
			oss << " " << flush;
			oss << " " << flush;
		}
		if (o.isCmd ("BI"))
		{
			if (!img (reader,xref))
				return false;
			oss << endl << " IMAGE DATA " << endl;
			//testPrintDbg (debug::DBG_DBG, "end image...");
	
			// grab the next object
			o.free ();
			reader.getXpdfObject (o);
			continue;
		}
		
		if (o.isCmd ())
		{
			oss << o.getCmd() << std::endl << flush;
		}else
		{
		//	oss << "(" << o.getType() << ")" << flush;
			std::string tmp;
			utils::xpdfObjToString (o, tmp);
			oss << tmp << " " << flush;
		}

		// grab the next object
		o.free ();
		reader.getXpdfObject (o);

	}
	reader.close ();
	
	return true;

}



//=========================================================================
// class TestCContentStream
//=========================================================================

class TestCContentStream : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCContentStream);
		CPPUNIT_TEST(TestPrimitivePrint);
		CPPUNIT_TEST(TestOpcount);
		CPPUNIT_TEST(TestPosition);
		CPPUNIT_TEST(TestPrint);
		CPPUNIT_TEST(TestSetCS);
		CPPUNIT_TEST(TestFront);
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
			OK_TEST;
		}
	}
	//
	//
	//
	void TestFront ()
	{
		OUTPUT << "CContentStream ..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" insert at front");
			CPPUNIT_ASSERT (front (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

	//
	//
	//
	void TestPrimitivePrint ()
	{
		OUTPUT << "CContentStream..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			std::ofstream o;
			o.open ("/dev/null");
			TEST("primitive  print contentstream");
			CPPUNIT_ASSERT (primitiveprintContentStream (/*o */OUTPUT, (*it).c_str()));
			o.close();
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
	//
	//
	//
	void TestSetCS ()
	{
		OUTPUT << "CContentStream..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" setCS");
			CPPUNIT_ASSERT (setCS (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCContentStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCContentStream, "TEST_CCONTENTSTREAM");

//=====================================================================================
} // namespace
//=====================================================================================








