/**
 * main_kernel.cc
 *
 * Kernel does not need QT 
 */

#include <stdlib.h>


#include "cobject.h"
#include "cpdf.h"


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














return 0;
}
