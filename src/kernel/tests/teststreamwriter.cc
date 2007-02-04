/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.2  2007/02/04 20:17:03  mstsxfx
 * Common Licence comment for all cc and h files available in doc/licence_header
 * file and its content to all cc and h files in src/{gui,kernel,utils}
 * directories.
 * Xpdf code, QSA and QOutputDevice code are not touched
 *
 * Revision 1.1  2006/05/08 14:20:41  hockm0bm
 * test cases for FileStreamWriter - everything seems to work correctly
 *
 *
 */

#include <errno.h>
#include "testmain.h"
#include "../streamwriter.h"

	
class TestStreamWriter: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestStreamWriter);
		CPPUNIT_TEST(Test);
	CPPUNIT_TEST_SUITE_END();

public:

	void fileStreamWriterTC(string test_file)
	{
		printf("%s with file %s\n", __FUNCTION__, test_file.c_str());
		
		FILE * file1=fopen(test_file.c_str(), "r+");
		// TODO ignore empty files
		if(!file1)
		{
			printf("file: %s open error (reason=%s)\n", test_file.c_str(), strerror(errno));
			return;
		}
		FILE * file2=fopen(test_file.c_str(), "r+");
		if(!file2)
		{
			printf("file: %s open error (reason=%s)\n", test_file.c_str(), strerror(errno));
			return;
		}

		Object dict;
		FileStreamWriter * streamWriter=new FileStreamWriter(file1, 0, false, 0, &dict);

		printf("TC01:\tData from FileStreamWriter are same as file content\n");
		int ch1,
			ch2;
		while((ch1=streamWriter->getChar())!=EOF)
		{
			ch2=fgetc(file2);
			CPPUNIT_ASSERT(ch1==ch2);
		}

		printf("TC02:\tData change is visible in original file\n");
		// sets position to the begining, reads data from this position
		// and puts different value there. Forces flush on stream to be
		// sure that data are really written
		streamWriter->setPos(0);
		int data1=streamWriter->getChar();
		streamWriter->setPos(0);
		streamWriter->putChar(++data1);
		streamWriter->flush();

		// seeks to the file2 begining and reads first byte
		fseek(file2, 0, SEEK_SET);
		int data2=fgetc(file2);
		
		// data must be same
		CPPUNIT_ASSERT(data1==data2);
		
		// returns to original state
		streamWriter->setPos(0);
		streamWriter->putChar(--data1);
		streamWriter->flush();

		printf("TC03:\tclone test");
		// clones stream from the begining to the file size half
		fseek(file2, 0, SEEK_END);
		size_t halfSize=ftell(file2) / 2;
		string cloneName=test_file+"_clone";
		FILE * file3=fopen(cloneName.c_str(), "w+");
		streamWriter->clone(file3, 0, halfSize);
		fflush(file3);
		
		// compares cloned file3 and stream content
		fseek(file3, 0, SEEK_SET);
		streamWriter->setPos(0);
		for(size_t i=0; i<halfSize; i++)
		{
			int ch1=streamWriter->getChar(),
				ch2=fgetc(file3);
			CPPUNIT_ASSERT(ch1==ch2);
		}

		delete streamWriter;
		fclose(file1);
		fclose(file2);
		fclose(file3);
		// removes clone file
		remove(cloneName.c_str());
	}
		
	virtual ~TestStreamWriter()
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
		// creates pdf instances for all files
		for(FileList::iterator i=fileList.begin(); i!=fileList.end(); i++)
		{
			fileStreamWriterTC(*i);
		}
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestStreamWriter);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestStreamWriter, "TEST_STREAMWRITER");

