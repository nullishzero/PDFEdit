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

#include "kernel/static.h"
#include "tests/kernel/testparams.h"



//
// Params
//
const char* TestParams::DEFAULT_DIR = "../../../testset/";
const char* TestParams::DEFAULT_PDF = "zadani.pdf";
const std::string TestParams::FILES	("-files");
const std::string TestParams::INPUT_DIR	("-dir");
const std::string TestParams::ALL_OUTPUT("-all");
const std::string TestParams::TESTS("-tests");


// Test parameters
bool 
TestParams::init (int argc, char* argv[])
{
	// init
	instance().all_output = false;
	instance().input_dir = DEFAULT_DIR;

	// 
	while (1 < argc)
	{
		char* param = argv[1];
		--argc;++argv;
		
		if (ALL_OUTPUT == param) 
		{
			instance().all_output = true;
		
		}else if (INPUT_DIR == param) 
		{
			param = argv[1];
			--argc;++argv;
			if (0 < argc)
				instance().input_dir = param;
			else
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
				}
			}
		}
	
	} // while

	if (0 == instance().files.size())
		instance().files.push_back (instance().input_dir+DEFAULT_PDF);

	return true;
}
