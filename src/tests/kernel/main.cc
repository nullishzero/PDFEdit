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

#include "kernel/static.h"

//#include <unistd.h>
#include "utils/debug.h"
#include "tests/kernel/testmain.h"

using namespace debug;

/**
 *  Test main
 *	-all - prints all output
 *	-dir dirname - input directory
 *	-debug_level level - level of verbosity 0 - minimum; 5 - maximum
 *	all other parameters are treated as file (if they stand for regular file)
 *	or test name otherwise
 * Note that -debug_level takes effect only if -all parameter was specified, 
 * otherwise all debug output is suppressed.
 */
int 
main (int argc, char* argv[])
{
	// Parse params
	if (!TestParams::init (argc, argv))
	{
		std::cout << "Invalid parameters." << std::endl;
		return -1;
	}
	changeDebugLevel (TestParams::instance().debugLevel);

	// If first parameter is "all" clear it
	CHECK_OUTPUT (TestParams::instance().all_output);
	
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
		if (0 < TestParams::instance().tests.size()) 
		{// Run only specified
			for (TestParams::FileList::const_iterator it = TestParams::instance().tests.begin();
					it != TestParams::instance().tests.end();
						++it) 
		   {
				CPPUNIT_NS::Test* suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry(*it).makeTest();
				runner.addTest(suite);
           }

	    } else 
		{// Get the top level suite from the registry
			
			CPPUNIT_NS::Test* suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
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
