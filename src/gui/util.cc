/** @file
 Various utility functions (string processing, file loading, debugging, etc ...)
 @author Martin Petricek
 \brief Utility functions
*/
#include "util.h"
#include <iostream>
#include <qcolor.h>
#include <qfile.h>
#include <qstring.h>
#include <qwidget.h>
#include <qobject.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qstringlist.h>
#include <utils/debug.h>
#include <assert.h>

namespace util {

using namespace std;

/**
 Prints error message and terminates application
 @param message Error message to show
*/
void fatalError(const QString &message){
 cerr << endl << QObject::tr("Fatal Error") << "!" << endl << message << endl;
 exit(-1);
}

/**
 Convert < > and & characters to corresponding html entities (&gt; &lt; &amp;)
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

/**
 Find first occurence of separator in string and return its position, ignoring backslash-escaped separators
 If separator is not found, -1 is returned
 @param start Start searching from this position in string
 @param separator Separator of string elements
 @param line String to look for separator
 @return position of first separator from start position, ignoring backslash-escaped separators
*/
int findSep(int start,const QString &line,char separator) {
 int pos=line.find(separator,start);
 while(true) {
  if (pos==-1) {
   return -1;
  } else { //Found
   if (pos>start) { //Want escaped character and this is not first character
    if (line.at(pos-1)=='\\') { //Preceding character is a backslash -> need to track back
     int count=0;
     while(pos>count && line.at(pos-count-1)=="\\") {
      count++;
     }
     if (!(count&1)) {
      // Even number of backslashes -> not escaped character, only escaped backslash
      return pos;
     }
     pos=line.find(separator,pos+1);//Look for next character
     continue;//Restart loop
    }
   }
   return pos;
  }
 }
}

/**
 splits QString containing elements separated with given character
 All whitespaces from beginning and end of elements are trimmed
 @param separator Separator of elements
 @param line String containing elements separated with separator
 @param escape If true, "\\" will be converted to "\" and \(separator) will be converted to (separator) and not treated as separator
 @return QStringlist with elements
*/
QStringList explode(char separator,const QString &line,bool escape/*=false*/) {
 QStringList qs;
 if (escape) {
  int pos=0;
  QString add;
  while(true) {
   int nPos=findSep(pos,line,separator);
   if (nPos<0) {
    add=line.mid(pos);//rest of string
   } else {
    add=line.mid(pos,nPos-pos);//Part of string
   }
   escapeSlash(add);
   qs+=add;
   if (nPos<0) break;
   pos=nPos+1;//Skip separatr and look for next
  }  
 } else {
  qs=QStringList::split(separator,line,TRUE);
 }
 for (unsigned int i=0;i<qs.count();i++) {
  qs[i]=qs[i].stripWhiteSpace();
 }
 return qs;
}

/**
 Load content of file to string.
 NULL string is returned if file does not exist or is unreadable.
 It is assumed that the file is in utf8 encoding
 @param name Filename of file to load
 @return file contents in string.
*/
QString loadFromFile(const QString &name) {
 QFile f(name);
 if (!f.open(IO_ReadOnly)) {
  //Failure
  return QString::null;
 } 
 QByteArray qb=f.readAll();;
 f.close();
 QTextCodec *codec=QTextCodec::codecForName("utf8");
 assert(codec);
 QString res=codec->toUnicode(qb);
 return res;
}

/**
 Print stringlist to stdout
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
 Replace two backslashes followed by any character (escaped character) with that character
 @param line String in which to remove extra backslashes (in,out)
*/
void escapeSlash(QString &line) {
 static QRegExp slash("\\\\(.)");
 line.replace(slash,"\\1");
}

/**
 Return string from line up to first separator character and remove that string and the separator from the line.
 If separator is not found, entire string is returned and the line is set to empty string
 @param separator Separator of string elements
 @param line line to get from and remove first element
 @param escape If true, "\\" will be converted to "\" and \(separator) will be converted to (separator) and not treated as separator
 @return first string element (contents of string until separator)
*/
QString getUntil(char separator,QString &line,bool escape/*=false*/) {
 int pos=line.find(separator);
 while(true) {
  if (pos==-1) { //Not found
   QString _line=line;
   line="";
   if (escape) escapeSlash(_line);
   return _line;
  } else { //Found
   if (escape && pos>0) { //Want escaped character and this is not first character
    if (line.at(pos-1)=='\\') { //Preceding character is a backslash
     pos=line.find(separator,pos+1);//Look for next character
     continue;//Restart loop
    }
   }
   QString first=line.left(pos);
   line=line.mid(pos+1);
   if (escape) escapeSlash(first);
   return first;
  }
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

/**
 Return correctly localized string telling count of some items.
 Some languages (for example Czech) have more plural forms (2-4 items / 5 or more items), which is handled here
 @param count Count of items
 @param singular English signular form of the noun, without space before the word
 @param plural English plural form of the noun
 @return Localized string
*/
QString countString(int count,QString singular,QString plural) {
 QString str=QString::number(count)+" ";
 if (count==1) str+=QObject::tr(singular,"1");
 else if (count>=2 && count<=4) str+=QObject::tr(plural,"2-4");
 else str+=QObject::tr(plural,"5+");
 return str;
}

/**
 Mix colors from oldColor and newColor according to weight
 Weight should be between 0 and 1,
 1 meaning the color will be "new",
 0 meaning the color will be "old",
 everything else mean linear interpolation between new color (with given weight) and old color (with weight of (1-weight))
 @param weight Weight of new color.
 @param oldColor Old color
 @param newColor New color
*/
QColor mixColor(const QColor &oldColor,double weight,const QColor &newColor) {
 QColor retCol(
  (int)(oldColor.red()*(1-weight)+newColor.red()*weight),
  (int)(oldColor.green()*(1-weight)+newColor.green()*weight),
  (int)(oldColor.blue()*(1-weight)+newColor.blue()*weight)
 );
 return retCol;
}

/**
 Modify widget foreground and background color by blending
 with new colors using given weight
 @param widget Widget to modify
 @param fg new foreground color
 @param weight_fg weight of new foreground color
 @param bg new background color
 @param weight_bg weight of new background color
*/
void colorMod(QWidget* widget,QColor fg,double weight_fg,QColor bg,double weight_bg) {
 if (weight_bg>0) {
  QColor oldColorBack=widget->paletteBackgroundColor();
  QColor newColorBack=mixColor(oldColorBack,weight_bg,bg);
  widget->setPaletteBackgroundColor(newColorBack);
 }
 if (weight_fg>0) {
  QColor oldColorFore=widget->paletteForegroundColor();
  QColor newColorFore=mixColor(oldColorFore,weight_fg,fg);
  widget->setPaletteForegroundColor(newColorFore);
 }
 //TODO: check contrast of new color and modify them if necessary
}

/**
 Return string list containing 'count' items, numbered from 'start' to 'start+count-1'
 @param count number of items in list
 @param start 
*/
QStringList countList(unsigned int count,unsigned int start/*=0*/) {
 QStringList items;
 for(unsigned int i=0;i<count;i++) {
  items+=QString::number(i+start);
 }
 return items;
}

/**
 Return list of supported text encodings
 (Based on support from QTextCodec in Qt)
*/
QStringList supportedEncodings() {
 return QStringList::split(",",",Apple Roman,Big5,Big5-HKSCS,\
CP874,CP1250,CP1251,CP1252,CP1253,CP1254,CP1255,CP1256,CP1257,CP1258,\
GB2312,GB18030,GBK,IBM 850,IBM 866,\
ISO8859-1,ISO8859-2,ISO8859-3,ISO8859-4,ISO8859-5,ISO8859-6,ISO8859-7,ISO8859-8,ISO8859-8-i,ISO8859-9,\
ISO8859-10,ISO8859-13,ISO8859-14,ISO8859-15,\
JIS7,KOI8-R,KOI8-U,Latin1Shift-JIS,TIS-620TSCII,eucJP,eucKR,utf8,utf16",true);
}

/** Codec for default charset */
QTextCodec *defCodec=NULL;

/**
 set default charset used when reading (and writing) property values
 @param charsetName charset name used for all 8bit to/from unicode conversions
*/
void setDefaultCharset(const QString &charsetName) {
 //Codecs should not be deleted, as Qt is maniaging them
 if (defCodec) defCodec=NULL;
 if (charsetName.isNull() || charsetName=="") return;//no codec
 defCodec=QTextCodec::codecForName(charsetName); 
}

/**
 Return QString from std::string with optional character conversion according to settings
 @param str String to convert
 @return converted string
 */
QString convertToUnicode(const std::string &str) {
 if (defCodec) {
  return defCodec->toUnicode(str.c_str());
 }
 return QString(str);
}

/**
 Return std::string from QString with optional character conversion according to settings
 @param str String to convert
 @return converted string
 */
std::string convertFromUnicode(const QString &str) {
 if (defCodec) {
  const char *cstr=defCodec->fromUnicode(str);
  return cstr;
 }
 return str;
}

} //namespace util
