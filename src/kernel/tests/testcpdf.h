// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.3  2006/04/09 21:29:33  misuj1am
 *
 *
 * -- changed the method that creates CPdf to match my views
 *
 * Revision 1.2  2006/04/03 14:38:21  misuj1am
 *
 *
 * -- ADD: getTestPdf function that returns valid pdf object
 *
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
#include "../cpage.h"

#define FILE_NAME       "test_file.pdf"

inline 
void cpdf_tests()
{
	using namespace pdfobjects;
	/* CPdf testing */
	CPdf * cpdf=CPdf::getInstance(TESTPDFFILE, CPdf::Advanced);
	boost::shared_ptr<CDict> dict=cpdf->getDictionary();

	cpdf->close();

}

inline 
pdfobjects::CPdf * getTestCPdf(const char* filename)
{
using namespace pdfobjects;
using namespace utils;
using namespace std;

	/* CPdf testing */
	CPdf * cpdf=CPdf::getInstance(filename, CPdf::Advanced);
	boost::shared_ptr<CDict> dict=cpdf->getDictionary();
	
	// gets all pages
	//size_t pageCount=cpdf->getPageCount();
	//for(size_t i=1;i<=pageCount; i++)
	{
	//	boost::shared_ptr<CPage> page=cpdf->getPage(i);	
	//	printf("Page #%u\n", i);
	//	vector<string> names;
	//	boost::shared_ptr<CDict> pageDict_ptr=page->getDictionary();
	//	//printProperty(pageDict_ptr);
	//	printf("\n");
	}

	return cpdf;
}

#endif // _TESTCPDF_H_
