/*
 * =====================================================================================
 *        Filename:  testcpage.cc
 *         Created:  04/02/2006 15:47:27 AM CEST
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

#include "testmain.h"
#include "testcobject.h"

#include <PDFDoc.h>
#include "../cpage.h"


//=====================================================================================
// CPage
//=====================================================================================

void
mediabox (ostream& oss, const char* fileName)
{
	boost::scoped_ptr<PDFDoc> doc (new PDFDoc (new GString(fileName), NULL, NULL));
	
	//
	// Our stuff here
	//
	Object obj;
	XRef* xref = doc->getXRef();
	Catalog cat (xref);

	oss << "Contents:"		<< cat.getPage(1)->getContents(&obj) << endl;
	oss << "Page:"			<< xref->fetch (cat.getPageRef(1)->num, cat.getPageRef(1)->gen, &obj) << endl;

	CPdf pdf;
	boost::shared_ptr<CDict> dict (new CDict (pdf, obj, IndiRef ()));
	
	CPage page (dict);

	oss << page.getMediabox ();
}

//=====================================================================================
void cpage_tests(int , char **)
{
	TEST(" test 4.1 -- features");
	mediabox (OUTPUT, TESTPDFFILE);
	OK_TEST;
}
