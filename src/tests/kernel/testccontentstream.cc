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
#include "kernel/cstreamsxpdfreader.h"
#include "tests/kernel/testmain.h"
#include "tests/kernel/testcobject.h"
#include "tests/kernel/testcpage.h"
#include "tests/kernel/testcpdf.h"


//=====================================================================================
namespace {
//=====================================================================================
using namespace pdfobjects;
using namespace boost;


//=====================================================================================

bool
setCS (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t pagecnt = ppdf->getPageCount ();
	ppdf.reset();
	
	for (size_t i = 0; i < pagecnt && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
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
		dict->getProperty<CStream>("Contents")->setBuffer(buf);
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
frontinsert (ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t pagecnt = ppdf->getPageCount ();
	ppdf.reset();
	
	for (size_t i = 0; i < pagecnt && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i + 1);
		
		vector<boost::shared_ptr<CContentStream> > ccs;
		page->getContentStreams (ccs);
		assert (!ccs.empty());
		shared_ptr<CContentStream> cs = ccs.front();

		string tmp;
		cs->getStringRepresentation (tmp);

		shared_ptr<PdfOperator> op (new UnknownCompositePdfOperator ("halo","kto tam"));
		shared_ptr<PdfOperator> opp (new UnknownCompositePdfOperator ("tu","fun, tam?"));
		op->push_back (opp,op);
		cs->frontInsertOperator (op);
		string tmp1;
		cs->getStringRepresentation (tmp1);

		//oss << endl << "[" << tmp1.substr(0,100) << "]" << endl; // Only first name is used
		//oss << string ("[halo tu   ") << tmp.substr(0,90) << endl;
		CPPUNIT_ASSERT (tmp1 == string ("halo tu   ") + tmp);

		_working (oss);
	}
	
	return true;
}

//=====================================================================================

bool
position (ostream& oss, const char* fileName, const libs::Rectangle rc)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t pagecnt = ppdf->getPageCount ();
	ppdf.reset();
	
	for (size_t i = 0; i < pagecnt && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i + 1);
		
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
	
	bool img (Parser* parser, Object& o, XRef* xref)
	{
		  std::cout << "Inline image data (BI)" << flush;
		  
		  //
		  // Read from BI to ID
		  //
		  Object dict;
		  dict.initDict (xref);
		  o.free ();
		  
		  parser->getObj(&o); // FIXME handle return value
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
			  parser->getObj(&o); // FIXME handle return value
			  if (o.isEOF() || o.isError()) 
			  {
				gfree (key);
				return false;
			  }
			  dict.dictAdd(key, &o);
			}
			
			parser->getObj(&o); // FIXME handle return value
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
opcount (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
	/// Intermezzo
	boost::scoped_ptr<PDFDoc> doc  (new PDFDoc (new GString(fileName), NULL, NULL));
	if(!doc->isOk())
	{
		std::cout << "Bad PDF document: " << fileName << flush;
		return false;
	}

	for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i + 1);

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
		
		//
		// The problem is that we can not easily get indiref from an xpdf object
		//
		shared_ptr<CDict> dict = page->getDictionary ();
		if (dict->containsProperty ("Contents"))
		{
			shared_ptr<IProperty> tmp = utils::getReferencedObject (dict->getProperty ("Contents"));
			if (isStream (tmp) && obj.isStream())
			{
				
				streams.push_back ( shared_ptr<CStream> (new CStream (pdf, obj, tmp->getIndiRef())));

			}else if (isArray (tmp) && obj.isArray())
			{
				Object o;
				for (int i = 0; i < obj.arrayGetLength(); ++i)
				{
					shared_ptr<IProperty> ent = utils::getReferencedObject(IProperty::getSmartCObjectPtr<CArray> (tmp)->getProperty (i));
					streams.push_back ( shared_ptr<CStream> (new CStream (pdf,  *(obj.arrayGet (i, &o)), ent->getIndiRef())));
					o.free ();
				}
			}else
			{
				assert (!"Invalid contents entry.");
			}
		}

		obj.free ();

		CStreamsXpdfReader<Streams> reader (streams);
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
printContentStream (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
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
addcc (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
	for (size_t i = 0; i < pdf->getPageCount() && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i + 1);

		// Print content stream
		string str;
		if (page)
		{
			vector<boost::shared_ptr<CContentStream> > ccs;
			page->getContentStreams (ccs);

			typedef vector<boost::shared_ptr<PdfOperator> > Opers;
			PdfOperator::Operands operands;
				
			Opers ops;
			ops.push_back (createOperator("lala",operands));
			page->addContentStreamToFront (ops);

			vector<boost::shared_ptr<CContentStream> > cccs;
			boost::shared_ptr<CContentStream> newcc;
			page->getContentStreams (cccs);
			CPPUNIT_ASSERT (cccs.size() == (ccs.size() + 1));
			
			for (size_t i = 0; i < ccs.size(); ++i)
			{
				CPPUNIT_ASSERT(ccs[i] = cccs[i]);
				
			}
			
			std::string str;
			cccs[ccs.size()]->getStringRepresentation (str);
			//oss << str << flush;
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
settm (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t num = ppdf->getPageCount ();
	for (size_t i = 0; i < num && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
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
			//oss << str << flush;
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
primitiveprintContentStream (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	{// what if file is corrupted etc..
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
	}
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

	scoped_ptr<Parser> parser (new Parser (xref, new Lexer(xref, &obj), 
				gFalse  // TODO check if gFalse is ok here
					// it should be, because content stream
					// mustn't contain stream objects
				)
			);
	
	Object o;
	parser->getObj (&o); // FIXME handle return value
	int i = 0;
	
	while (!o.isEOF()) 
	{
		i++;
		if (o.isCmd ("BI"))
		{
			if (!img (parser.get(),o,xref))
				return false;
	    	parser->getObj(&o); // FIXME handle return value
		}
		
		if (o.isCmd ())
		{
			oss << o.getCmd() << "\n#" << i << ": " << flush;
		}
		else
		{
			//oss << "(" << o.getType() << ")" << flush;
			std::string tmp;
			utils::xpdfObjToString (o, tmp);
			oss << tmp << " " << flush;
		}

		// grab the next object
		o.free ();
		parser->getObj(&o); // FIXME handle return value
	}

	obj.free ();
	return true;

}

//=========================================================================

bool
cstreamsreader (UNUSED_PARAM	ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
	if (1 > pdf->getPageCount())
		return true;

	
	boost::shared_ptr<CPdf> ppdf = getTestCPdf (fileName);
	size_t num = ppdf->getPageCount ();
	for (size_t i = 0; i < num && i < TEST_MAX_PAGE_COUNT; ++i)
	{
		//
		// - init our reader then xpdf reader
		// - read one object and compare it
		//
		boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
		boost::shared_ptr<CPage> page = pdf->getPage (i + 1);

		/// Intermezzo cstreamsreader
		boost::scoped_ptr<PDFDoc> doc  (new PDFDoc (new GString(fileName), NULL, NULL));
		if (doc->isEncrypted())
			continue;
		int pagesNum = i + 1;
		Object obj;
		XRef* xref = doc->getXRef();
		assert (xref);
		Catalog cat (xref);
		cat.getPage(pagesNum)->getContents(&obj);
		IndiRef rf;
		rf.num = cat.getPageRef(pagesNum)->num;
		rf.gen = cat.getPageRef(pagesNum)->gen;
		typedef vector<shared_ptr<CStream> > Streams;
		Streams streams;

		//
		// The problem is that we can not easily get indiref from an xpdf object
		//
		shared_ptr<CDict> dict = page->getDictionary ();
		if (dict->containsProperty ("Contents"))
		{
			shared_ptr<IProperty> tmp = utils::getReferencedObject (dict->getProperty ("Contents"));
			if (isStream (tmp) && obj.isStream())
			{
				
				streams.push_back ( shared_ptr<CStream> (new CStream (pdf, obj, tmp->getIndiRef())));

			}else if (isArray (tmp) && obj.isArray())
			{
				Object o;
				for (int i = 0; i < obj.arrayGetLength(); ++i)
				{
					shared_ptr<IProperty> ent = utils::getReferencedObject (IProperty::getSmartCObjectPtr<CArray> (tmp)->getProperty (i));
					streams.push_back ( shared_ptr<CStream> (new CStream (pdf,  *(obj.arrayGet (i, &o)), ent->getIndiRef())));
					o.free ();
				}
			}else
			{
				assert (!"Invalid contents entry.");
			}
		}
		CStreamsXpdfReader<Streams> reader (streams);
		::Object o;
		reader.open ();
		reader.getXpdfObject (o);
		///

		/// Intermezzo for xpdf
		scoped_ptr<Parser> parser (new Parser (xref, new Lexer(xref, &obj),
					gFalse  // TODO check if gFalse is ok here
						// it should be, because content stream
						// mustn't contain stream objects
					)
				);
		Object o1;
		parser->getObj (&o1); // FIXME handle return value
		///
		obj.free ();

		while (!o1.isEOF())
		{
			if (o.getType() != o1.getType())
			{
				oss << "Cstreams are not read equally. [" << o.getTypeName() << "] [" << o1.getTypeName() << "]" << flush;
			
			}else
			{
				oss << "[" << o.getTypeName() << "] " << flush;
			}
			
			CPPUNIT_ASSERT_EQUAL (o.getType(), o1.getType());
			
			// grab the next object
			o.free ();
			reader.getXpdfObject (o);
		
			o1.free();
			parser->getObj (&o1); // FIXME handle return value

		}
		reader.close ();
		
		_working (oss);
	}
	
	return true;
}



//=========================================================================
// class TestCContentStream
//=========================================================================

class TestCContentStream : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCContentStream);
		CPPUNIT_TEST(TestTm);
		CPPUNIT_TEST(TestPrimitivePrint);
		CPPUNIT_TEST(TestOpcount);
		CPPUNIT_TEST(TestPosition);
		CPPUNIT_TEST(TestPrint);
		CPPUNIT_TEST(TestSetCS);
		CPPUNIT_TEST(TestFront);
		CPPUNIT_TEST(TestCStreams);
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
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
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
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" getPosition");
			CPPUNIT_ASSERT (position (OUTPUT, (*it).c_str(), libs::Rectangle (100,100,300,300)));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestTm ()
	{
		OUTPUT << "CContentStream ..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			BEGIN_CHECK_READONLY;
				TEST(" test matrix");
				CPPUNIT_ASSERT (settm (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;

			BEGIN_CHECK_READONLY;
				TEST(" add content stream");
				CPPUNIT_ASSERT (addcc (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}
	//
	//
	//
	void TestFront ()
	{
		OUTPUT << "CContentStream ..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			BEGIN_CHECK_READONLY;
				TEST(" insert at front");
				CPPUNIT_ASSERT (frontinsert (OUTPUT, (*it).c_str()));
				OK_TEST;

				TEST(" add content stream");
				CPPUNIT_ASSERT (addcc (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}


	//
	//
	//
	void TestPrimitivePrint ()
	{
		OUTPUT << "CContentStream..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			std::ofstream o;
			#if TEMP_FILES_CREATE
			o.open ("_primitive");
			#else
			o.open ("/dev/null");
			#endif
			TEST("primitive print contentstream");
			CPPUNIT_ASSERT (primitiveprintContentStream (o /*OUTPUT*/, (*it).c_str()));
			o.close();
			OK_TEST;
		}
	}

	//
	//
	//
	void TestCStreams ()
	{
		OUTPUT << "CContentStream..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			TEST(" cstreams");

			#if REALLY_ALL_OUTPUT
				CPPUNIT_ASSERT (cstreamsreader (OUTPUT, (*it).c_str()));
			#else
				std::ofstream o;
				o.open ("/dev/null");
				CPPUNIT_ASSERT (cstreamsreader (o, (*it).c_str()));
			#endif
			OK_TEST;
		}
	}

	//
	//
	//
	void TestPrint ()
	{
		OUTPUT << "CContentStream..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
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
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;

			BEGIN_CHECK_READONLY;
				TEST(" setCS");
				CPPUNIT_ASSERT (setCS (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCContentStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCContentStream, "TEST_CCONTENTSTREAM");

//=====================================================================================
} // namespace
//=====================================================================================








