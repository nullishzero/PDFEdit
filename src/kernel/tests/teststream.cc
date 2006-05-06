// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.1  2006/05/06 21:16:22  hockm0bm
 * test class for streams
 *
 *
 */

#include <errno.h>
#include "testmain.h"
#include "../xpdf.h"

class TestStream: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestStream);
		CPPUNIT_TEST(Test);
	CPPUNIT_TEST_SUITE_END();

	vector<string> files;
public:

	void fileStreamTC()
	{
		printf("%s\n", __FUNCTION__);

		for(vector<string>::iterator i=files.begin(); i!=files.end(); i++)
		{
			printf("Opening file: %s\n", (*i).c_str());
			FILE * f1=fopen((*i).c_str(), "r+");
			if(!f1)
			{
				printf("File open failed: %s\n", strerror(errno));
				continue;
			}

			// creates unlimited stream from file
			Object dict;
			dict.initNull();
			FileStream * unlimitedStream=new FileStream(f1, 0, false, 0, &dict);

			printf("TC01:\tcontent of FileStream is same as file's content\n");
			// opens same file 
			FILE * f2=fopen((*i).c_str(), "r+");
			int ch1, ch2;
			while((ch1=unlimitedStream->getChar())!=EOF)
			{
				ch2=fgetc(f2);
				CPPUNIT_ASSERT(ch1==ch2);
			}
			fclose(f2);

			printf("TC02:\tcloned stream's content is same as original\n");
			Stream * clonedStream=unlimitedStream->clone();
			unlimitedStream->reset();
			while((ch1=unlimitedStream->getChar())!=EOF)
			{
				ch2=clonedStream->getChar();
				CPPUNIT_ASSERT(ch1==ch2);
			}
			

			delete unlimitedStream;
			fclose(f1);
		}
	}
	
	virtual ~TestStream()
	{
	}

	void setUp()
	{
		files.push_back("file1");
	}


	void tearDown()
	{
	}

	void Test()
	{
		fileStreamTC();
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestStream, "TEST_STREAM");
