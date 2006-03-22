/** @file
 Various utility functions
*/
#include "util.h"
#include <iostream>
#include <qfile.h>

using namespace std;

/** Prints error message and terminates application
 @param message Error message to show
 */
void fatalError(const QString message){
 cout << "Fatal Error: " << message << endl;
 exit(-1);
}

/** Convert < > and & characters to corresponding html entities (&gt; &lt; &amp;)
@param str String to convert
@return Converted string
 */
QString htmlEnt(const QString &str) {
 QString res=str;
 res.replace('&',"&amp;");
 res.replace('>',"&gt;");
 res.replace('<',"&lt;");
 return res;
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

/** Load content of file to string. Empty string is returned if file does not exist or is unreadable.
 @param name Filename of file to load
 @return file contents in string.
 */
QString loadFromFile(QString name) {
 QFile *f=new QFile(name);
 f->open(IO_ReadOnly);
 int size=f->size();
 char* buffer=(char *)malloc(size);
 size=f->readBlock(buffer,size);
 if (size==-1) return "";
 f->close();
 delete f;
 QByteArray qb;
 qb.assign(buffer,size);
 QString res=QString(qb);
 return res;
}
