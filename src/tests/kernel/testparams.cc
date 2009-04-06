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
#include "tests/kernel/testparams.h"



//
// Params
//
const char* TestParams::DEFAULT_DIR = "../../../testset/";
const char* TestParams::DEFAULT_PDF = "zadani.pdf";
const char* TestParams::DEFAULT_ENCRYPT_PASSWD_RC = "encrypt_passwd.rc";
const std::string TestParams::PASSWD_RC("-passwd_rc");
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
	TestParams &params = instance();
	params.all_output = false;
	params.input_dir = DEFAULT_DIR;
	params.debugLevel = TestParams::DEFAULT_DEBUG_LEVEL;
	params.passwd_rc = TestParams::DEFAULT_ENCRYPT_PASSWD_RC;

	// 
	while (1 < argc)
	{
		char* param = argv[1];
		--argc;++argv;
		
		if (ALL_OUTPUT == param) 
		{
			params.all_output = true;
			continue;
		
		}else if (INPUT_DIR == param) 
		{
			param = argv[1];
			--argc;++argv;
			if (0 < argc) {
				params.input_dir = param;
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
				params.debugLevel = (unsigned int)val;
				continue;
			}else
				return false;
		}else if (PASSWD_RC == param)
		{
			param = argv[1];
			--argc;++argv;
			struct stat info;
			if(0 < argc || !stat(param, &info)) {
				if(S_ISREG(info.st_mode))
				{
					params.passwd_rc = param;
					continue;
				}
			}
			return false;
		}else
		{
			struct stat info;
			if (!stat(param, &info))
			{
				// checks if it is regular file and if so, uses it
				if(S_ISREG(info.st_mode))
				{
					// Push all files for testing into this conatiner	
					params.files.push_back (param);
					continue;
				}
			}
		}

		// none of special parameters, so fallback to the testname
		params.tests.push_back(param);
	
	} // while

	if (0 == params.files.size())
		params.files.push_back (params.input_dir+DEFAULT_PDF);

	return true;
}
