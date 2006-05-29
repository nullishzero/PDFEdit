// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  testcstream.cc
 *         Created:  04/24/2006 02:57:27 AM CEST
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

#include "testmain.h"
#include "testcobject.h"
#include "testcpage.h"
#include "testcpdf.h"

#include "factories.h"
#include "filters.h"

#include <PDFDoc.h>
#include "../cpage.h"


//=====================================================================================
namespace {
//=====================================================================================
using namespace boost;
using namespace std;

//=====================================================================================
bool setbuffer (__attribute__((unused))	std::ostream& oss, __attribute__((unused))	const char* fileName)
{
	typedef CStream::Buffer Buffer;
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getPage (1);
	boost::shared_ptr<CStream> stream = getTestStreamContent (page);

	//
	// Make a buffer
	//
	Buffer buf;
	buf.push_back ('h');
	buf.push_back ('a');
	buf.push_back ('l');
	buf.push_back ('o');
	
	shared_ptr<IProperty> ilen = utils::getReferencedObject (stream->getProperty ("Length"));
	assert (isInt(ilen));
	shared_ptr<CInt> clen = IProperty::getSmartCObjectPtr<CInt> (ilen);
	assert (isInt(ilen));
	int len = 0;
	clen->getPropertyValue (len);
	//oss << "Length: " << len << " " << flush;
	
	//
	// Set raw
	//
	stream->setRawBuffer (buf);
	oss << " If an xpdf ERROR occured, that is CORRECT!!!" << flush;
	string tmp;
	stream->getStringRepresentation (tmp);
	//oss << tmp << flush;
	clen->getPropertyValue (len);
	//oss << "Length: " << len << " " << flush;

	// Change buffer
	buf[2] = 'p';
	buf[3] = 'p';

	clen->getPropertyValue (len);
	//oss << "Length: " << len << " " << flush;
	
	//
	// Set buf
	//
	stream->setBuffer (buf);
	stream->getStringRepresentation (tmp);
	//oss << tmp << flush;
	clen->getPropertyValue (len);
	//oss << "Length: " << len << " " << flush;

	//
	// Test setBuffer
	// 
	for (int i = 0; i< 10; ++i)
	{
		stream->setBuffer (buf);
		CPPUNIT_ASSERT (buf.size() == stream->getBuffer().size());
	}
	
	//
	// Test setRawBuffer
	// 
	for (int i = 0; i< 10; ++i)
	{
		stream->setRawBuffer (buf);
		CPPUNIT_ASSERT (buf.size() == stream->getBuffer().size());
	}

	return true;
}


//=====================================================================================
bool buffer (__attribute__((unused))	std::ostream& oss, __attribute__((unused))	const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);

		//CStream::Buffer& buf = stream->buffer;
		//oss << "Buffer start: "<< std::flush;
		filters::Printable<CStream::Buffer::value_type> print;
		//for (CStream::Buffer::iterator it = buf.begin (); it != buf.end (); ++it)
		//	oss << print(*it) << std::flush;
		//oss << "\nBuffer end.."<< std::flush;
		
		std::string tmp;
		stream->getStringRepresentation (tmp);
		//oss << tmp << std::flush;
	}
	
	return true;
}

//=====================================================================================
bool createStream (std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);

		vector<string> names;
		stream->getAllPropertyNames (names);

		oss << " CStream dictionary: [" << flush;
		for (vector<string>::iterator it = names.begin(); it != names.end(); ++it)
			oss << (*it) << " " << flush;
		oss << "]" << flush;
	}
	return true;
}

//=========================================================================

bool getString (std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);
		boost::shared_ptr<Object> obj (stream->_makeXpdfObject (), xpdf::object_deleter());
		

		string tmp;
		stream->getStringRepresentation (tmp);
		//oss << tmp << endl;

		boost::shared_ptr<IProperty> ip = stream->getProperty ("Length");
		ip = utils::getReferencedObject (ip);

		tmp.clear ();
		ip->getStringRepresentation (tmp);
		oss << " Length: [" << tmp << "]" << flush;
	}

	return true;
}

//=========================================================================

bool getPdfString (std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);

		CharBuffer buf;
		size_t len = stream->getPdfRepresentation (buf);
		oss << " Length: [" << len << "]" << flush;
		//oss << std::endl << buf.get () << endl;
		filters::Printable<char> p;
		for (size_t i = 0; i < len; ++i)
			p (buf.get()[i]);
	}
	
	return true;
}

//=========================================================================

bool getFilter (std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);
		
		string tmp;
		vector<string> filters;
		stream->getFilters (filters);

		oss << " Filters: ";
		for (vector<string>::iterator it = filters.begin(); it != filters.end(); ++it)
			oss << *it << flush;
	}

	return true;
}


//=========================================================================
bool testdict (__attribute__((unused)) std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);

		//oss << " Dict item #: [" << stream->getPropertyCount () << "]" << flush;
	}

	return true;
}

//=========================================================================
bool testmakexpdf (__attribute__((unused)) std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName), pdf_deleter());

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);

		boost::shared_ptr<Object> str  (stream->_makeXpdfObject (),xpdf::object_deleter());
		assert (NULL != str);
		//oss << "object type " << str->getTypeName() << flush;
		assert (objStream == str->getType ());

		int c;
		//while (EOF != (c = str->getStream()->getChar())) 
		//	oss << (char)c << flush;
	}
	
	return true;
}


//=========================================================================

bool getSupportedF (std::ostream& oss)
{
	vector<string> supported;
	CStream::getSupportedStreams (supported);
	
	oss << " Supported: ";
	for (vector<string>::iterator it = supported.begin(); it != supported.end(); ++it)
		oss << *it << endl;
	
	return true;
}



//=========================================================================
// class TestCStream
//=========================================================================

class TestCStream : public CppUnit::TestFixture 
{
	CPPUNIT_TEST_SUITE(TestCStream);
		CPPUNIT_TEST(TestMakeXpdf);
		CPPUNIT_TEST(TestBuf);
		CPPUNIT_TEST(Test);
		CPPUNIT_TEST(TestString);
		CPPUNIT_TEST(TestFilter);
		CPPUNIT_TEST(TestSupFilter);
		CPPUNIT_TEST(TestDict);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {OUTPUT << endl;}
	void tearDown() {}

public:
	//
	//
	//
	void TestBuf ()
	{
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" buffer");
			CPPUNIT_ASSERT (buffer (OUTPUT, (*it).c_str()));
			OK_TEST;
			
			TEST(" set buffer");
			CPPUNIT_ASSERT (setbuffer (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void Test ()
	{
		OUTPUT << "CStream methods..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" create");
			CPPUNIT_ASSERT (createStream (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestString ()
	{
		OUTPUT << "CStream string methods..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" test string");
			CPPUNIT_ASSERT (getString (OUTPUT, (*it).c_str()));
			CPPUNIT_ASSERT (getPdfString (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
			
		TEST(" get supported filters");
		CPPUNIT_ASSERT (getSupportedF (OUTPUT));
		OK_TEST;
	}
	//
	//
	//
	void TestFilter ()
	{
		OUTPUT << "CStream string methods..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" filters");
			CPPUNIT_ASSERT (getFilter (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestSupFilter ()
	{
		TEST(" get supported filters");
		CPPUNIT_ASSERT (getSupportedF (OUTPUT));
		OK_TEST;
	}
	//
	//
	//
	void TestDict ()
	{
		OUTPUT << "CStream dict methods..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" dict");
			CPPUNIT_ASSERT (testdict (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}
	//
	//
	//
	void TestMakeXpdf ()
	{
		OUTPUT << "CStream dict methods..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" dict");
			CPPUNIT_ASSERT (testmakexpdf (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCStream, "TEST_CSTREAM");

//=====================================================================================
} // namespace
//=====================================================================================








