/*
 * =====================================================================================
 *        Filename:  testcpage.cc
 *         Created:  04/02/2006 15:47:27 AM CEST
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

#include "testmain.h"
#include "testcobject.h"
#include "testcpdf.h"

#include <PDFDoc.h>
#include "../cpage.h"


//=====================================================================================
// CPage
//=====================================================================================

using namespace boost;

//=====================================================================================
namespace
//=====================================================================================
{
	boost::shared_ptr<CPage> 
	getPage (const char* fileName, boost::shared_ptr<CPdf> pdf, size_t pageNum = 1)
	{
		boost::scoped_ptr<PDFDoc> doc (new PDFDoc (new GString(fileName), NULL, NULL));
		
		//
		// Our stuff here
		//
		Object obj;
		XRef* xref = doc->getXRef();
		assert (xref);
		Catalog cat (xref);

		IndiRef ref;
		ref.num = cat.getPageRef(pageNum)->num;
		ref.gen = cat.getPageRef(pageNum)->gen;
		xref->fetch (ref.num, ref.gen, &obj);
		
		boost::shared_ptr<CDict> dict (new CDict (*pdf, obj, ref));
		obj.free ();
		
		return boost::shared_ptr<CPage> (new CPage(dict));
	}
//=====================================================================================
} // namespace
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
	obj.free ();
	oss << "Page:"			<< xref->fetch (cat.getPageRef(1)->num, cat.getPageRef(1)->gen, &obj) << endl;

	boost::scoped_ptr<CPdf> pdf (getTestCPdf (fileName));
	boost::shared_ptr<CDict> dict (new CDict (*pdf, obj, IndiRef ()));
	obj.free ();
	
	CPage page (dict);

	oss << page.getMediabox ();
	
	Rectangle rc;
	rc.xleft = 42;
	rc.xright = 12;
	rc.yleft = 62;
	rc.yright = 2342;
	page.setMediabox (rc);
	
	oss << page.getMediabox ();
}


//=====================================================================================
//
void
position (ostream& oss, const char* fileName)
{
	boost::shared_ptr<CPdf> pdf (getTestCPdf (fileName));
	boost::shared_ptr<CPage> page = getPage (fileName, pdf);

	
	std::vector<shared_ptr<PdfOperator> > ops;
	page->getObjectsAtPosition (ops, Rectangle (1,1,10,10));

	oss << "Found objects #" << ops.size() << std::endl;

	std::vector<shared_ptr<PdfOperator> >::iterator it = ops.begin ();
	for (; it != ops.end(); ++it)
	{
		std::string tmp;
		(*it)->getOperatorName (tmp);
		oss << tmp;
	}
	oss << std::endl;
	
}

//=====================================================================================

void
opcount (ostream& oss, const char* fileName)
{
	boost::scoped_ptr<PDFDoc> doc (new PDFDoc (new GString(fileName), NULL, NULL));
	int pagesNum = 1;
	
	//
	// Our stuff here
	//
	Object obj;
	XRef* xref = doc->getXRef();
	assert (xref);
	Catalog cat (xref);

	cat.getPage(pagesNum)->getContents(&obj);

	scoped_ptr<Parser> parser (new Parser (NULL, new Lexer(NULL, &obj)));
	
	Object o;
	parser->getObj (&o);
	int i = 0;
	
	while (!o.isEOF()) 
	{
		if (o.isCmd ())
			i++;

		// grab the next object
		parser->getObj(&o);
	}

	
	obj.free ();

	oss << "Operands count: " << i << endl;
}

//=====================================================================================

void
display (ostream& oss, const char* fileName)
{
	oss << "display" << endl;
	
	// Open pdf and get the first page	
	boost::scoped_ptr<CPdf> pdf (getTestCPdf (fileName));
	boost::shared_ptr<CPage> page = pdf->getFirstPage ();


  	//TextOutputDev textOut (NULL, gTrue, gFalse, gTrue);
  	TextOutputDev textOut ("1.txt", gFalse, gFalse, gFalse);
	if (!textOut.isOk ())
		throw;
	
	//
	// Output to file
	//
	oss << "Creating 1.txt which contains text from a pdf." << endl;
	page->displayPage (textOut);
	
	boost::scoped_ptr<GlobalParams> aGlobPar (new GlobalParams (""));
	GlobalParams* oldGlobPar = globalParams;
	globalParams = aGlobPar.get();

	oss << "Output from textoutputdevice." << endl;
	oss << textOut.getText(0, 0, 1000, 1000)->getCString() << endl;

	globalParams = oldGlobPar;
}

				

//=====================================================================================
void cpage_tests(int , char **, const char* fileName)
{
	TEST(" test 4.1 -- features");
	mediabox (OUTPUT, fileName);
	OK_TEST;

	TEST(" test 4.2-- opcount");
	opcount (OUTPUT, fileName);
	OK_TEST;

	TEST(" test 4.3-- getPosition");
	position (OUTPUT, fileName);
	OK_TEST;

	TEST(" test 4.4-- display");
	display (OUTPUT, fileName);
	OK_TEST;

}
