/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
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
#include "tests/kernel/testparams.h"



//
// Params
//
const char* TestParams::DEFAULT_DIR = "../../../testset/";
const char* TestParams::DEFAULT_PDF = "zadani.pdf";
const std::string TestParams::INPUT_DIR	("-dir");
const std::string TestParams::ALL_OUTPUT("-all");
const std::string TestParams::TESTS("-tests");
const std::string TestParams::DEBUG_LEVEL("-debug_level");
const unsigned int TestParams::DEFAULT_DEBUG_LEVEL = debug::DBG_WARN;


// Test parameters
bool 
TestParams::init (int argc, char* argv[])
{
	// init
	instance().all_output = false;
	instance().input_dir = DEFAULT_DIR;
	instance().debugLevel = TestParams::DEFAULT_DEBUG_LEVEL;

	// 
	while (1 < argc)
	{
		char* param = argv[1];
		--argc;++argv;
		
		if (ALL_OUTPUT == param) 
		{
			instance().all_output = true;
			continue;
		
		}else if (INPUT_DIR == param) 
		{
			param = argv[1];
			--argc;++argv;
			if (0 < argc) {
				instance().input_dir = param;
				continue;
			}
			else
				return false;
		
		}else if (DEBUG_LEVEL == param)
		{
			param = argv[1];
			--argc;++argv;
			if(0 < argc) {
				int val = atoi(param);
				if(val < 0 || val > (int)debug::DBG_DBG)
					return false;
				instance().debugLevel = (unsigned int)val;
				continue;
			}else
				return false;
		}else {

			struct stat info;
			if (!stat(param, &info))
			{
				// checks if it is regular file and if so, uses it
				if(S_ISREG(info.st_mode))
				{
					// Push all files for testing into this conatiner	
					instance().files.push_back (param);
					continue;
				}
			}
		}

		// none of special parameters, so fallback to the testname
		instance().tests.push_back(param);
	
	} // while

	if (0 == instance().files.size())
		instance().files.push_back (instance().input_dir+DEFAULT_PDF);

	return true;
}
