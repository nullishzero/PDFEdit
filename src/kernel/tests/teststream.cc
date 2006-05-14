// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.4  2006/05/14 21:10:19  hockm0bm
 * content stream to xpdf Object test
 *         - doesn't work properly
 *
 * Revision 1.3  2006/05/13 22:19:29  hockm0bm
 * isInValidPdf refactored to hasValidPdf or isPdfValid functions
 *
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

	void contentStreamTC(CPdf & pdf)
	{
	using namespace boost;
	using namespace std;
	using namespace pdfobjects;
	using namespace pdfobjects::utils;

		printf("%s\n", __FUNCTION__);

		printf("TC01:\tPage content stream can be read without any error\n");
		printf("\t\t%u pages found\n", pdf.getPageCount());
		for(size_t i=1; i<=pdf.getPageCount(); i++)
		{
			// gets page dictionary at position and gets Contents 
			// property from it
			shared_ptr<CDict> pageDict=pdf.getPage(i)->getDictionary();
			try
			{
				shared_ptr<IProperty> contentProp=pageDict->getProperty("Contents");
				if(! isRef(*contentProp))
				{
					printf("\t\tPage %u has uncorect Contents entry\n", i);
					continue;
				}
				IndiRef contentRef=getValueFromSimple<CRef, pRef, IndiRef>(contentProp);
				shared_ptr<CStream> contentStr=IProperty::getSmartCObjectPtr<CStream>(pdf.getIndirectProperty(contentRef));
				string str;
				contentStr->getStringRepresentation(str);
				printf("Content stream for %u page:\n%s\n\n", i, str.c_str());

				Object * xpdfContentStr=contentStr->_makeXpdfObject();
				int ch;
				xpdfContentStr->getStream()->reset();
				const CStream::Buffer & buffer=contentStr->getBuffer();
				size_t bytes=0;
				// xpdf content must be same as in CStream object
				printf("\t\tCStream::_makeXpdfObject provides correct Object instance\n");
				BaseStream * baseStream=xpdfContentStr->getStream()->getBaseStream();
				while((ch=baseStream->getChar())!=EOF)
				{
					CPPUNIT_ASSERT(ch==buffer[bytes]);
					bytes++;
				}
				
			}catch(ElementNotFoundException & e)
			{
				printf("\t\tPage %u has no content stream\n", i);
			}
		}
	}
	
	virtual ~TestStream()
	{
	}

	void setUp()
	{
	}


	void tearDown()
	{
	}

	void Test()
	{
		for(FileList::iterator i=fileList.begin(); i!=fileList.end(); i++)
		{
			fileStreamTC(*i);
			CPdf * pdf=CPdf::getInstance((*i).c_str(), CPdf::ReadOnly);
			contentStreamTC(*pdf);
			pdf->close();
		}
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestStream);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestStream, "TEST_STREAM");
