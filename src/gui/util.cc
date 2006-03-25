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

/** Return human-readable (and possibly localized) name of given type
 @param typ Type of PDF Object
 @return Human readable type name
 */
QString getTypeName(PropertyType typ) {
 switch (typ) {
  case pNull:   return QObject::tr("Null",	"Type name"); 
  case pBool:   return QObject::tr("Bool",	"Type name"); 
  case pInt:    return QObject::tr("Int",	"Type name"); 
  case pReal:   return QObject::tr("Real",	"Type name"); 
  case pString: return QObject::tr("String",	"Type name");
  case pName:   return QObject::tr("Name",	"Type name");
  case pRef:    return QObject::tr("Ref",	"Type name");
  case pArray:  return QObject::tr("Array",	"Type name"); 
  case pDict:   return QObject::tr("Dict",	"Type name"); 
  case pStream: return QObject::tr("Stream",	"Type name"); 
// Debug types
  case pOther:    return QObject::tr("DebugCmd",	"Type name");
  case pOther1:  return QObject::tr("DebugError",	"Type name");
  case pOther2:    return QObject::tr("DebugEOF",	"Type name");
  case pOther3:   return QObject::tr("DebugNone",	"Type name");
 }
 //Unknown type. Should not happen
 assert(0);
 return "?";
}
