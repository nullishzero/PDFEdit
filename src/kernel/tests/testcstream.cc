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

#include <PDFDoc.h>
#include "../cpage.h"


//=====================================================================================
namespace {
//=====================================================================================
using namespace boost;
using namespace std;

bool createStream (std::ostream& oss, const char* fileName)
{
	boost::scoped_ptr<CPdf> pdf (getTestCPdf (fileName));
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getPage (1);

	shared_ptr<CDict> dict = page->getDictionary ();

	boost::shared_ptr<IProperty> contents = utils::getReferencedObject (dict->getProperty ("Contents"));
	boost::shared_ptr<CStream> stream;
	if (contents)
	{
		if (isArray (contents))
			stream = utils::getCStreamFromArray (contents, 0);
		else if (isStream (contents))
			stream = IProperty::getSmartCObjectPtr<CStream> (contents);
		else
		{
			string tmp;
			contents->getStringRepresentation (tmp);
			testPrintDbg (debug::DBG_CRIT, "Bad contents entry type: " << contents->getType() << " repre: " << tmp);
			assert (!"Bac contents entry type.");
		}
	}else
	{
		assert (!"No content stream");
		throw CObjInvalidObject ();
	}

	vector<string> names;
	stream->getAllPropertyNames (names);

	oss << "CStream dictionary: " << endl;
	for (vector<string>::iterator it = names.begin(); it != names.end(); ++it)
		oss << (*it) << endl;

	return true;
}

//=========================================================================

bool getString (std::ostream& oss, const char* fileName)
{
	boost::scoped_ptr<CPdf> pdf (getTestCPdf (fileName));
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getPage (1);

	shared_ptr<CDict> dict = page->getDictionary ();

	boost::shared_ptr<IProperty> contents = utils::getReferencedObject (dict->getProperty ("Contents"));
	boost::shared_ptr<CStream> stream;
	if (contents)
	{
		if (isArray (contents))
			stream = utils::getCStreamFromArray (contents, 0);
		else if (isStream (contents))
			stream = IProperty::getSmartCObjectPtr<CStream> (contents);
		else
		{
			string tmp;
			contents->getStringRepresentation (tmp);
			testPrintDbg (debug::DBG_CRIT, "Bad contents entry type: " << contents->getType() << " repre: " << tmp);
			assert (!"Bac contents entry type.");
		}
	}else
	{
		assert (!"No content stream");
		throw CObjInvalidObject ();
	}

	assert (isStream (stream));
	boost::scoped_ptr<Object> obj (stream->_makeXpdfObject ());
	obj->free ();
	

	string tmp;
	stream->getStringRepresentation (tmp);

	oss << tmp << endl;

	boost::shared_ptr<IProperty> ip = stream->getProperty ("Length");
	ip = utils::getReferencedObject (ip);

	tmp.clear ();
	ip->getStringRepresentation (tmp);
	oss << "Length: " << tmp << endl;

	return true;
}

//=========================================================================

bool getFilter (std::ostream& oss, const char* fileName)
{
	boost::scoped_ptr<CPdf> pdf (getTestCPdf (fileName));
	if (1 > pdf->getPageCount())
		return true;
	boost::shared_ptr<CPage> page = pdf->getPage (1);

	shared_ptr<CDict> dict = page->getDictionary ();
	boost::shared_ptr<IProperty> contents = utils::getReferencedObject (dict->getProperty ("Contents"));
	boost::shared_ptr<CStream> stream;
	if (contents)
	{
		if (isArray (contents))
			stream = utils::getCStreamFromArray (contents, 0);
		else if (isStream (contents))
			stream = IProperty::getSmartCObjectPtr<CStream> (contents);
		else
		{
			string tmp;
			contents->getStringRepresentation (tmp);
			testPrintDbg (debug::DBG_CRIT, "Bad contents entry type: " << contents->getType() << " repre: " << tmp);
			assert (!"Bac contents entry type.");
		}
	}else
	{
		assert (!"No content stream");
		throw CObjInvalidObject ();
	}

	assert (isStream (stream));

	string tmp;
	vector<string> filters;
	stream->getFilters (filters);

	oss << " Filters: ";
	for (vector<string>::iterator it = filters.begin(); it != filters.end(); ++it)
		oss << *it << endl;

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
		CPPUNIT_TEST(Test);
		CPPUNIT_TEST(TestString);
		CPPUNIT_TEST(TestFilter);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {OUTPUT << endl;}
	void tearDown() {}

public:
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
	void TestString ()
	{
		OUTPUT << "CStream string methods..." << endl;
		
		for (FileList::const_iterator it = fileList.begin (); it != fileList.end(); ++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" create");
			CPPUNIT_ASSERT (getString (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
			
		TEST(" get supported filters");
		CPPUNIT_ASSERT (getSupportedF (OUTPUT));
		OK_TEST;
	}
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
			
		TEST(" get supported filters");
		CPPUNIT_ASSERT (getSupportedF (OUTPUT));
		OK_TEST;
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCStream, "TEST_CSTREAM");

//=====================================================================================
} // namespace
//=====================================================================================








