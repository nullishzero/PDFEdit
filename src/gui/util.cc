/** @file
 Various utility functions
*/
#include "util.h"
#include <utils/debug.h>
#include <iostream>
#include <qfile.h>
#include <qstring.h>
#include <qobject.h>
#include <qstringlist.h>

using namespace std;

/** Prints error message and terminates application
 @param message Error message to show
 */
void fatalError(const QString &message){
 cout << endl << QObject::tr("Fatal Error") << "!" << endl << message << endl;
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
QStringList explode(char separator,const QString &line) {
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
QString loadFromFile(const QString &name) {
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

/** Print stringlist to stdout
 @param l String list to print
 */
void printList(const QStringList &l) {
 QStringList::ConstIterator it=l.begin();
 for (;it!=l.end();++it) { //load all subitems
  QString x=*it;
  printDbg(debug::DBG_DBG,x);
 }
}

/** 
 Write line to specified logfile
 @param message Line to write to logfile
 @param fileName Name of log file. If this is null or empty string, nothing is done
*/
void consoleLog(const QString &message,const QString &fileName) {
 if (fileName.isNull()) return;
 if (fileName=="") return;
 QFile con(fileName);
 con.open(IO_WriteOnly | IO_Append);
 QTextStream conOut(&con);
 conOut << message << "\n";
 con.close();
}
