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

#include "kernel/factories.h"


//=====================================================================================
namespace {
//=====================================================================================
using namespace boost;
using namespace std;

/** Functor replacing non printable characters with printable. */
template<typename T>
struct Printable
{
	typedef char Char;
	Char operator () (T _c) const
	{
		Char c = _c;
		if ('!' < c && c < '~')
			return c;
		else
			// FIXME is this OK?
			return '+';
	}
};

//=====================================================================================
bool setbuffer (UNUSED_PARAM	std::ostream& oss, UNUSED_PARAM	const char* fileName)
{
	typedef CStream::Buffer Buffer;
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);
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
	
	shared_ptr<CInt> clen = stream->getProperty<CInt> ("Length");
	int len = 0;
	clen->getValue (len);
	//oss << "Length: " << len << " " << flush;
	
	//
	// Set raw
	//
	stream->setRawBuffer (buf);
	oss << " If an xpdf ERROR occured, that is CORRECT!!!" << flush;
	string tmp;
	stream->getStringRepresentation (tmp);
	//oss << tmp << flush;
	clen->getValue (len);
	//oss << "Length: " << len << " " << flush;

	// Change buffer
	buf[2] = 'p';
	buf[3] = 'p';

	clen->getValue (len);
	//oss << "Length: " << len << " " << flush;
	
	//
	// Set buf
	//
	stream->setBuffer (buf);
	stream->getStringRepresentation (tmp);
	//oss << tmp << flush;
	clen->getValue (len);
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
bool buffer (UNUSED_PARAM	std::ostream& oss, UNUSED_PARAM	const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);

		//CStream::Buffer& buf = stream->buffer;
		//oss << "Buffer start: "<< std::flush;
		//filters::Printable<CStream::Buffer::value_type> print;
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
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

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
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

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

bool getPdfString (UNUSED_PARAM std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);

		string tmp;
		stream->getDecodedStringRepresentation (tmp);
		Printable<char> p;
		for (size_t i = 0; i < tmp.length(); ++i)
			p (tmp[i]);
	}
	
	return true;
}

//=========================================================================

bool getFilter (std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

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
bool testdict (UNUSED_PARAM std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);

		//oss << " Dict item #: [" << stream->getPropertyCount () << "]" << flush;
	}

	return true;
}

//=========================================================================
bool testmakexpdf (UNUSED_PARAM std::ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf = getTestCPdf (fileName);

	for (size_t i = 0; i < pdf->getPageCount(); ++i)
	{
		boost::shared_ptr<CPage> page = pdf->getPage (i+1);
		boost::shared_ptr<CStream> stream = getTestStreamContent (page);

		boost::shared_ptr<Object> str  (stream->_makeXpdfObject (),xpdf::object_deleter());
		assert (NULL != str);
		//oss << "object type " << str->getTypeName() << flush;
		assert (objStream == str->getType ());

		//int c;
		//while (EOF != (c = str->getStream()->getChar())) 
		//	oss << (char)c << flush;
	}
	
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
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			BEGIN_CHECK_READONLY;
				TEST(" buffer");
				CPPUNIT_ASSERT (buffer (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
			
			BEGIN_CHECK_READONLY;
				TEST(" set buffer");
				CPPUNIT_ASSERT (setbuffer (OUTPUT, (*it).c_str()));
				OK_TEST;
			END_CHECK_READONLY;
		}
	}
	//
	//
	//
	void Test ()
	{
		OUTPUT << "CStream methods..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
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
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
		{
			OUTPUT << "Testing filename: " << *it << endl;
			
			TEST(" test string");
			CPPUNIT_ASSERT (getString (OUTPUT, (*it).c_str()));
			CPPUNIT_ASSERT (getPdfString (OUTPUT, (*it).c_str()));
			OK_TEST;
		}
			
		OK_TEST;
	}
	//
	//
	//
	void TestFilter ()
	{
		OUTPUT << "CStream string methods..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
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
	void TestDict ()
	{
		OUTPUT << "CStream dict methods..." << endl;
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
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
		
		for(TestParams::FileList::const_iterator it = TestParams::instance().files.begin(); 
				it != TestParams::instance().files.end(); 
					++it)
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








