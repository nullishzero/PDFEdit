// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.1  2006/04/01 00:39:50  misuj1am
 *
 *
 * -- tests
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
#ifndef _TESTCPDF_H_
#define _TESTCPDF_H_

#include "testmain.h"

#define FILE_NAME       "test_file.pdf"

inline 
void cpdf_tests()
{
	using namespace pdfobjects;
	/* CPdf testing */
	CPdf * cpdf=CPdf::getInstance(FILE_NAME, CPdf::Advanced);
	boost::shared_ptr<CDict> dict=cpdf->getDictionary();

	cpdf->close();

}

#endif // _TESTCPDF_H_
