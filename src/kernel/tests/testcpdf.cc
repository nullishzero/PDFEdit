// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.3  2006/04/12 21:41:38  hockm0bm
 * pageListing function added
 *
 * Revision 1.2  2006/04/12 20:40:44  hockm0bm
 * getTestCPdf is not inline now - because of linking problem for kernel binary
 *
 * Revision 1.4  2006/04/12 20:19:33  hockm0bm
 * methods reorganization
 *
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
#include "testcpdf.h"
#include "../cobjecthelpers.h"

void pageListing(pdfobjects::CPdf * pdf)
{
using namespace pdfobjects;
using namespace boost;

	size_t count=pdf->getPageCount();
	for(size_t i=1; i<=count; i++)
	{
		shared_ptr<CPage> page=pdf->getPage(i);
		shared_ptr<CDict> pageDict=page->getDictionary();
		IndiRef pageRef=pageDict->getIndiRef();
		printf("Page #%d dictionary: ref=[%d,%d]\n", i, pageRef.num, pageRef.gen);
		utils::printProperty(pageDict, std::cout);
		printf("==========================\n\n");
	}
}

void cpdf_tests(pdfobjects::CPdf * pdf)
{
	// lists all pages from document
	pageListing(pdf);
}

pdfobjects::CPdf * getTestCPdf(const char* filename)
{
	return pdfobjects::CPdf::getInstance(filename, CPdf::Advanced);
}
