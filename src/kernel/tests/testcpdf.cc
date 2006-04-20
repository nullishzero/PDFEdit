// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.5  2006/04/20 18:56:04  hockm0bm
 * file saving is disabled now
 *
 * Revision 1.4  2006/04/13 18:12:57  hockm0bm
 * * changePageTree method added
 * * pageListing method updated
 *
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
#include "../factories.h"

/** Test for page listing methods.
 * @param pdf Instance of pdf.
 *
 * Checks following methods:
 * <ul>
 * <li>CPdf::getPageCount - must not fail and all pages from range must be found
 * <li>CPdf::getPage - returns correct page
 * <li>CPdf::getFirstPage, CPdf::getNextPage and getLastPage return correct page
 * <li>CPdf::getPagePosition - position must be same as returned from getPage
 * <li>CPdf::getIndirectProperty - indirect reference from page dictionary must
 * refer to same as returned from this method
 * <li>CPage::getDictionary - dictionary must be same as indirect object with
 * same indirect reference
 * </ul>
 */
void pageListing(pdfobjects::CPdf * pdf)
{
using namespace pdfobjects;
using namespace pdfobjects::utils;
using namespace boost;

	size_t count=pdf->getPageCount();

	// prints root of page tree
	IndiRef rootRef=getRefFromDict("Pages", pdf->getDictionary());
	shared_ptr<CDict> rootDict=getDictFromRef(*pdf, rootRef);
	printf("Page tree root dictionary: ref[%d, %d]\n", rootRef.num, rootRef.gen);
	printProperty(rootDict, std::cout);
	printf("==========================\n\n");
	
	shared_ptr<CPage> page;
	for(size_t i=1; i<=count; i++)
	{
		// gets page from iteration methods
		if(i==1)
			page=pdf->getFirstPage();
		else
			page=pdf->getNextPage(page);
			
		// gets page and its dictionary
		shared_ptr<CDict> pageDict=page->getDictionary();

		// gets reference of this dictionary and property with same reference.
		IndiRef pageRef=pageDict->getIndiRef();
		shared_ptr<IProperty> prop=pdf->getIndirectProperty(pageRef);

		// resolved property must be same as page dictionary
		assert(pageDict==prop);
		// page position must be same as i
		assert(i==pdf->getPagePosition(page));
		// getPage returns same page
		assert(pdf->getPage(i)==page);
		
		// prints page dictionary
		printf("Page #%d dictionary: ref=[%d,%d]\n", i, pageRef.num, pageRef.gen);
		utils::printProperty(pageDict, std::cout);
		printf("==========================\n\n");
	}

	// getLastPage must be same as last from loop
	assert(pdf->getLastPage()==page);
	assert(pdf->getLastPage()==pdf->getPage(pdf->getPageCount()));
}

void changePageTree(pdfobjects::CPdf * pdf)
{
using namespace pdfobjects;
using namespace pdfobjects::utils;
using namespace boost;
	
	// removes 1st page
	//pdf->removePage(1);
	
	// gets root dictionary and calls changeIndirectProperty to simulate change
	// of its value
	IndiRef rootRef=getRefFromDict("Pages", pdf->getDictionary());
	shared_ptr<CDict> rootDict_ptr=getDictFromRef(*pdf, rootRef);
	pdf->changeIndirectProperty(rootDict_ptr);
	
	
	// saves chnages temporarily to the file
	//pdf->save();
}

void cpdf_tests(pdfobjects::CPdf * pdf)
{
using namespace std;
using namespace boost;
using namespace pdfobjects::utils;

	// prints pdf dictionary - document catalog
	printf("Document Catalog:\n");
	printProperty(pdf->getDictionary(), std::cout);
	printf("==========================\n\n");
	
	// lists all pages from document
	pageListing(pdf);

	// changes page tree - all pages are thrown away
	changePageTree(pdf);
}

pdfobjects::CPdf * getTestCPdf(const char* filename)
{
	return pdfobjects::CPdf::getInstance(filename, CPdf::Advanced);
}
