// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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
// cobject tests
#include "tests/testcobject.h"
// cpdf tests
#include "tests/testcpdf.h"
// cpage tests
#include "tests/testcpage.h"


const char* PDF_TEST_FILE_NAME = "../../doc/zadani.pdf";

/**
 *  Test main
 */
int 
main (int argc, char** argv)
{
	// uses default file name
	const char * fileName=PDF_TEST_FILE_NAME;
	
	// checks if first parameter is real file and if so, overwrites fileName
	if(argc>0)
	{
		const char * param=argv[1];
		struct stat info;
		if(!stat(param, &info))
		{
			// checks if it is regular file and if so, uses it
			if(S_ISREG(info.st_mode))
				fileName=param;
		}
	}
	
	START_TEST;

	// Test cobjects
	//cobject_tests (argc, argv);
	MEM_CHECK;
	
	// Test cpdf
	CPdf * testCPdf=getTestCPdf(fileName);
	cpdf_tests(testCPdf);
	testCPdf->close();
	MEM_CHECK;

	// Test cpage
	cpage_tests (argc, argv);
	END_TEST;
}
