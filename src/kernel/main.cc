/*
 * main.cc
 *
 * Kernel does not need QT 
 */

#include "static.h"

//#include <iostream>

#include "cobject.h"
#include "cpdf.h"


using namespace std;
using namespace pdfobjects;


/**
 *  test main - load settings and launches a main window 
 */
int main ()
{
		cout << endl << "Started testing..." << endl;
		//======== test 1
		//======== test 2
		/*CPdf pdf;
		CDict dc(&pdf);
		IProperty* ip = pdf.getExistingProperty(77,77);
		*/

		//======== test 3
		/*
		GString* fileName = new GString ("/home/jozo/_pdf/pdfedit/or_d0506.pdf");
		PDFDoc* doc = new PDFDoc (fileName,new GString(),new GString());
		cout << "Filename: "<< fileName->getCString() << endl;
		cout << "Number of pages: "<< doc->getNumPages () << endl;
		// Get an object from PDFDoc
		Object* obj; 
		*/

		//
		// Our stuff here
		//
		//CPdf pdf;
		//string str;
		/*XRef* xref = doc->getXRef();

		cout << "Xref: " << (unsigned int) xref << endl;

		obj = xref->getTrailerDict ();
		CPdf::objToString (obj,str);
		cout << str << endl;
		str = "";

		xref->getCatalog (obj);
		CPdf::objToString (obj,str);
		cout << str << endl;
		str = "";

		xref->getDocInfo (obj);
		CPdf::objToString (obj,str);
		cout << str << endl;
		str = "";
		*/

		//======== test 4
		//======== test 5
		//
		pdfobjects::CPdf pdf;
		cout << "1st created" << endl;
		pdfobjects::CNull nl;
		cout << "2nd created" << endl;


		cout << endl << "Ended testing..." << endl;
		return 0;
}

