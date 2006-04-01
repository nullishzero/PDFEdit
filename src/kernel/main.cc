// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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


/**
 *  Test main
 */
int 
main (int argc, char** argv)
{
	
	START_TEST;

	// Test cobjects
	cobject_tests (argc, argv);
	
	MEM_CHECK;
	
	// Test cpdf
	cpdf_tests ();

	END_TEST;
}
