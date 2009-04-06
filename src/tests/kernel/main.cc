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


/*
 * main.cc
 *
 */

#include "kernel/static.h"
#include "kernel/pdfedit-core-dev.h"

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
	//
	// Initialization
	//
	if(pdfedit_core_dev_init(&argc, &argv))
	{
		std::cout << "Pdfedit-dev core couldn't be initialized" << std::endl;
		return 1;
	}

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

	delete globalParams;
	std::cout << "Tests completed." << std::endl;
	gMemReport(stdout);
	// Return error code 1 if the one of test failed.
	return wasSucessful ? 0 : 1;

} // main
