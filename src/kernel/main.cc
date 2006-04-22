// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.34  2006/04/22 19:33:30  hockm0bm
 * test_cpdf removed - only CPP Unit test style remained
 *
 * Revision 1.33  2006/04/20 21:53:20  misuj1am
 *
 * --minor improvments
 *
 * Revision 1.32  2006/04/20 19:06:47  misuj1am
 *
 * -- default test added
 *
 * Revision 1.31  2006/04/20 18:27:57  misuj1am
 *
 * -- cppunit tests
 *
 * Revision 1.30  2006/04/20 15:00:57  misuj1am
 *
 * -- cobject tests splitted to testcobjectsimple and testcobjectcomplex
 *
 * Revision 1.29  2006/04/20 13:21:41  misuj1am
 *
 * -- ADD: cppunit tests
 *
 * Revision 1.28  2006/04/18 14:18:14  misuj1am
 *
 *
 * -- improved main
 *
 * Revision 1.27  2006/04/16 23:09:34  misuj1am
 *
 *
 * -- observers implemented
 * -- changes delegated to pdf
 *
 * Revision 1.26  2006/04/13 18:07:19  hockm0bm
 * if first parameter is regular file, uses it instead of default one
 *
 * Revision 1.25  2006/04/12 22:46:48  misuj1am
 *
 *
 * -- main.cc made usable
 * 	-- REMARK it is good practise to rely only on those things that are in cvs tree
 *
 * Revision 1.24  2006/04/12 20:32:21  hockm0bm
 * code for cpdf testing reorganized
 *
 * Revision 1.23  2006/04/02 17:12:59  misuj1am
 *
 *
 * -- ADD: media box
 * -- improved interface a bit
 * -- testing
 *
 * Revision 1.22  2006/04/01 00:39:28  misuj1am
 *
 *
 * -- main separated to tests directory
 *
 * Revision 1.21  2006/03/31 21:14:40  hockm0bm
 * * file format changed (tabulators - same style)
 * * CVS header
 * * main changed to contain only test functions
 *         - all code from main moved to cobjects_test function
 *         - added cpdf_tests function for CPdf testing
 *
 *
 */

/*
 * main.cc
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "static.h"

#include "tests/testmain.h"
#include "tests/testcobject.h"
#include "tests/testcpdf.h"

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
	bool wasSucessful;
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
