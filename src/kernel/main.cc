/**
 * main_kernel.cc
 *
 * Kernel does not need QT 
 */

#include <iostream>
#include <aconf.h>

#include "cobject.h"
#include "cpdf.h"


#include "lib.h"


using namespace std;
using namespace pdfobjects;


/**
 *  test main - load settings and launches a main window 
 */
int 
main ()
{
//======== test 1
/*using namespace pdfobjects;
map<IdPair,IProperty*,IdComparator> mapping;

mapping[make_pair(0,0)] = NULL;
mapping[make_pair(1,0)] = (IProperty*)1;
mapping[make_pair(2,1)] = (IProperty*)2;
mapping[make_pair(2,2)] = (IProperty*)3;
mapping[make_pair(1,0)] = (IProperty*)4;

map<IdPair,IProperty*,IdComparator>::iterator i;
for (i=mapping.begin(); i!=mapping.end(); i++)
{
		cout << (*i).first.first << "  " << (*i).first.second << "  ";
		cout << (int)((*i).second) << endl;
}*/

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

//Object* o = new Object ();
//o->initBool (gFalse);

//CBoolean cb (NULL);
//CRef cr (NULL);

//cr.release ();

//cb.writeValue (false);

// segmenation fault, Object is not defined


//======== test 5
//
pdfobjects::CPdf pdf;


CDict cmp;

CInt ii;

cmp.addProperty (ii, "jano");

CInt smpl;


//int j;
//smpl.getPropertyValue (j);
smpl.setStringRepresentation ("123");

//cout << j << "  ..." << endl;

f();
		
return 0;
}

