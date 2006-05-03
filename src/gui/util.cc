/** @file
 Various utility functions (string processing, file loading, debugging, etc ...)
 @author Martin Petricek
*/
#include "util.h"
#include <iostream>
#include <qfile.h>
#include <qstring.h>
#include <qobject.h>
#include <qstringlist.h>
#include <utils/debug.h>

namespace util {

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
  cout << x <<endl;
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

/**
 Return string from line up to first separator character and remove that string and the separator from the line.
 If separator is not found, entire string is returned and the line is set to empty string
 @param separator Separator of string elements
 @param line line to get from and remove first element
 @return first string element (contents of string until separator)
*/
QString getUntil(char separator,QString &line) {
 int pos=line.find(separator);
 if (pos==-1) { //Not found
  QString _line=line;
  line="";
  return _line;
 } else { //Found
  QString first=line.left(pos);
  line=line.mid(pos+1);
  return first;
 }
}

/**
 Set debugging verbosity level. Accept debugging level either as number, or as one of symbolic constants:<br>
 PANIC, CRITICAL, ERROR, WARNING, INFO, DEBUG, DBG (=DEBUG)<br>
 Constants are case insensitive, starting part of constant is sufficient<br>
 Only messages with priority equal or higher to specified level are shown
 @param param new debugging level
*/
void setDebugLevel(const QString &param){
 QString cns=param.upper();
 if (cns.length()) { //Check for symbolic constants
  if (QString("PANIC").startsWith(cns))		{ debug::debugLevel=debug::DBG_PANIC;	return; }
  if (QString("CRITICAL").startsWith(cns))	{ debug::debugLevel=debug::DBG_CRIT;	return; }
  if (QString("ERROR").startsWith(cns))		{ debug::debugLevel=debug::DBG_ERR;	return; }
  if (QString("WARNING").startsWith(cns))	{ debug::debugLevel=debug::DBG_WARN;	return; }
  if (QString("INFO").startsWith(cns))		{ debug::debugLevel=debug::DBG_INFO;	return; }
  if (QString("DEBUG").startsWith(cns))		{ debug::debugLevel=debug::DBG_DBG;	return; }
  if (QString("DBG").startsWith(cns))		{ debug::debugLevel=debug::DBG_DBG;	return; }
 }
 //If debuglevel is set outside of limits - no problem, nearest "in limits" value is defacto used
 debug::debugLevel=atoi(param);
 //If non-number is given, default 0 is silently used ... :)
}

} //namespace util
