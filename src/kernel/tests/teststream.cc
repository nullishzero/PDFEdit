// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.2  2006/05/08 14:47:46  hockm0bm
 * * clone for FileStream test
 *         - seems to work
 * * clone for FileStream substream test
 *         - seems to work
 *
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

	bool compareStreams(Stream * str1, Stream * str2, size_t pos=0, int count=-1)
	{
		int ch1, ch2;
		// starts from givem position
		str1->setPos(pos);
		str2->setPos(pos);
		do
		{
			// decrement only if count parameter is used
			if(count!=-1)
				count--;

			ch1=str1->getChar();
			ch2=str2->getChar();
			if(ch1!=ch2)
				return false;
			
			// if count bytes read, ends loop
			if(!count)
				break;

		}while(ch1!=EOF);

		return true;
	}
	
	void fileStreamTC(string fileName)
	{

		printf("%s fileName %s\n", __FUNCTION__, fileName.c_str());
		FILE * f1=fopen(fileName.c_str(), "r+");
		if(!f1)
		{
			printf("File open failed: %s\n", strerror(errno));
			return;
		}

		// creates unlimited stream from file
		Object dict;
		dict.initNull();
		FileStream * unlimitedStream=new FileStream(f1, 0, false, 0, &dict);

		printf("TC01:\tcontent of FileStream is same as file's content\n");
		// opens same file 
		FILE * f2=fopen(fileName.c_str(), "r+");
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
		CPPUNIT_ASSERT(compareStreams(unlimitedStream, clonedStream));

		printf("TC03:\tsubstream clone test");
		Stream * subStream=unlimitedStream->makeSubStream(0, true, 1, &dict);
		Stream * cloneSubStream=subStream->clone();
		CPPUNIT_ASSERT(compareStreams(subStream, cloneSubStream, 0 ,1));
		
		delete cloneSubStream;
		delete subStream;
		delete unlimitedStream;
		fclose(f1);
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
		for(vector<string>::iterator i=files.begin(); i!=files.end(); i++)
		{
			fileStreamTC(*i);
		}
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestStream, "TEST_STREAM");
