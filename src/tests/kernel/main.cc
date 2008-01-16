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
 * main.cc
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kernel/static.h"

#include "testmain.h"
#include "testcobject.h"
#include "testcpdf.h"

// Default rest pdf file
const char* PDF_TEST_FILE_NAME = "../../doc/zadani.pdf";

// Filelist
FileList fileList;

/**
 *  Test main
 */
int 
main (int argc, char** argv)
{
	//
	// If first parameter is "all" clear it
	//
	CHECK_OUTPUT (argc,argv);
	
	// uses default file name
	const char * fileName = PDF_TEST_FILE_NAME;
	
	// checks if first parameter is real file and if so, overwrites fileName
	while (1 < argc)
	{
		struct stat info;
		if(!stat(argv[1], &info))
		{
			// checks if it is regular file and if so, uses it
			if(S_ISREG(info.st_mode))
			{
				// Push all files for testing into this conatiner	
				fileName = argv[1];
				fileList.push_back (fileName);
				
				--argc;++argv;
				continue;
			}
		}
		break;
		
	}// while (1 < argc)

	if (fileList.empty ())
		fileList.push_back (fileName);
	
	
	//
	// Initialization
	//
	GlobalParams::initGlobalParams("");
	globalParams->setupBaseFonts(NULL);

	//
	// Start testing
	//
	START_TEST;
	
	//
	// CPage test
	//
	CPPUNIT_NS::TextTestRunner runner;
	try 
	{
		if (1 < argc) 
		{// Run only specified
           for (int i = 1; i < argc; ++i) 
		   {
				CPPUNIT_NS::Test* suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry(argv[i]).makeTest();
                // Adds the test to the list of test to run
				runner.addTest(suite);
           }

	    } else 
		{// Get the top level suite from the registry
			
			CPPUNIT_NS::Test* suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
			// Adds the test to the list of test to run
			runner.addTest(suite);
		}
		
		// Change the default outputter to a compiler error format outputter
		//runner.setOutputter(new	CPPUNIT_NS::CompilerOutputter(&runner.result(),OUTPUT));
	
	}catch (...) 
	{
		OUTPUT << "Could not initialize tests..." << std::endl;
		return 1;
	}
	

	//
	// Run the tests.
	// 
	bool wasSucessful = false;
	try
	{
		std::cout << "Tests started." << std::endl;
		wasSucessful=runner.run();
		
	}catch (...) 
		{ OUTPUT << "Exception thrown..." << std::endl;	}

	END_TEST;

	std::cout << "Tests completed." << std::endl;
	// Return error code 1 if the one of test failed.
	return wasSucessful ? 0 : 1;

} // main
