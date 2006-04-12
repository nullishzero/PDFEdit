// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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

#include "static.h"

#include "tests/testmain.h"
// cobject tests
#include "tests/testcobject.h"
// cpdf tests
#include "tests/testcpdf.h"
// cpage tests
#include "tests/testcpage.h"

#define FILE_NAME       "test_file.pdf"

/**
 *  Test main
 */
int 
main (int argc, char** argv)
{
	
	START_TEST;

	// Test cobjects
	//cobject_tests (argc, argv);
	MEM_CHECK;
	
	CPdf * testCPdf=getTestCPdf(FILE_NAME);
	
	// Test cpdf
	cpdf_tests(testCPdf);
	testCPdf->close();

	MEM_CHECK;

	// Test cpage
	cpage_tests (argc, argv);
	END_TEST;
}
