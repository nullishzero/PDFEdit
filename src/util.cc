#include "util.h"
#include <iostream>

using namespace std;

/** Prints error message and terminates application
 @param message Error message to show
 */
void fatalError(const QString message){
 cout << "Fatal Error: " << message << endl;
 exit(-1);
}

/** splits QString containing elements separated with given character
    All whitespaces from beginning and end of elements are trimmed
@param separator Separator of elements
@param line String containing elements separated with separator
@return QStringlist with elements
 */
QStringList explode(char separator,const QString line) {
 QStringList qs=QStringList::split(separator,line,TRUE);
 for (unsigned int i=0;i<qs.count();i++) {
  qs[i]=qs[i].stripWhiteSpace();
 }
 return qs; 
}

