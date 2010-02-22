/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 Various utility functions (string processing, file loading, debugging, etc ...)
 @author Martin Petricek
 \brief Utility functions
*/
#include "util.h"
#include "qtcompat.h"
#include <iostream>
#include <assert.h>
#include QBYTEARRAY
#include <qcolor.h>
#include <qfile.h>
#include <qobject.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qvariant.h>
#include <qwidget.h>
#include <kernel/static.h>
#include <utils/debug.h>
#include QLIST

namespace util {

using namespace std;

namespace {

	//
	// Pdfedit encoding class
	//
	class PdfeditEncoding; //forward declaration
	/** Single instance of PdfeditEncoding class */
	PdfeditEncoding *encoding_instance=0;
	/**
	 * This class is used because of special pdf accented character encoding.
	 */
	class PdfeditEncoding : public QTextCodec
	{
		typedef std::map<unsigned short, string> UMap;

		// Display this char when mapping is not known
		const static char UNKNOWN_CHAR = '?';

		private:
			UMap umap;	// Mapping from unicode to windows character encoding

			//
			// Ctor - singleton
			//
		private:
			PdfeditEncoding ()
			{
				umap[271] = string (1,'d') + string (1,(char)39);	// daccent
				umap[318] = string (1,'l') + string (1,(char)39);	// laccent
				umap[352] = string (1,(char)138);					// Scaron
				umap[353] = string (1,(char)154);					// scaron
				umap[357] = string (1,'t') + string (1,(char)39);	// tcaron
				umap[381] = string (1,(char)142);					// Zcaron
				umap[382] = string (1,(char)158);					// zcaron
			
			};

			//
			// Instance
			//
		public:
			static PdfeditEncoding* get ()
			{
				if (!encoding_instance) encoding_instance=new PdfeditEncoding();
				return encoding_instance;
			}

			//
			// Interface
			//
		public:
#ifdef QT4
			virtual QByteArray name () const
#else
			virtual const char* name () const
#endif
			{
				static const char* _name = "Pdfedit accented windows encoding";
				return _name;
			}
			int mibEnum () const
				{ return 2252; /* windows 1252 */};
			int heuristicContentMatch (const char* ,int) const
				{ return 0; }


			QString
			toUnicode (const char* chars, int len ) const
			{
				return QString::fromLatin1(string(chars,len).c_str());
			}
#ifdef QT4
			QString convertToUnicode (const char* chars, int len, QTextCodec::ConverterState* ) const
			{
				return toUnicode(chars,len);
			}
			QByteArray convertFromUnicode (const QChar* chars, int len, QTextCodec::ConverterState* ) const
			{
				string tmp;
				for (int i = 0; i < len; ++i)
				{
					QChar c =chars[i];
					UMap::const_iterator it = umap.find (c.unicode());
					if (it != umap.end())
						tmp += (*it).second;
					else
						tmp += c.latin1() ? c.latin1() : UNKNOWN_CHAR ;
				}
				assert (tmp.size() >= len);
				return QCString (tmp.c_str());
			}
#else
			QCString
			fromUnicode (const QString& uc, int& lenInOut) const
			{
				string tmp;
				for (size_t i = 0; i < uc.length(); ++i)
				{
					QChar c (uc[i]);
					UMap::const_iterator it = umap.find (c.unicode());
					if (it != umap.end())
						tmp += (*it).second;
					else
						tmp += c.latin1() ? c.latin1() : UNKNOWN_CHAR ;
				}
				lenInOut = tmp.size();
				assert (tmp.size() >= uc.length());
				return QCString (tmp.c_str());
			}
#endif
	
	}; // class PdfeditEncoding

} // namespace

/**
 Convert array of double values to variant
 @param d Pointer to array fo doubles
 @param count number of elements in the array
*/
QVariant varFromDoubleArray(double *d,int count) {
 Q_List<QVariant> rPos;
 for (int i=0;i<count;i++) {
  rPos.append(d[i]);
 }
 return QVariant(rPos);
}

/**
 Convert variant to array of doubles. If size of variant array is larger than out_size,
 only part will be converted. If size is smaller, output array will be zero-padded
 @param v Variant to convert
 @param out pointer to array of doubles
 @param out_size number of elements in output array.
 @return number of elements converted (between 0 and out_size)
*/
int varToDoubleArray(const QVariant &v,double *out,int out_size) {
 Q_List<QVariant> list=v.toList();
 Q_List<QVariant>::const_iterator it=list.constBegin();
 int al=0;
 while(it!=list.constEnd()) {
  if (al>=out_size) break;//We filled all values
  out[al]=(*it).toDouble();
  it++;
  al++;
 }
 //Pad with zeroes
 int alx=al;
 while (alx<out_size) {
  out[alx]=0;
  alx++;
 }
 return al;
}

/**
 Prints error message and terminates application
 @param message Error message to show
*/
void fatalError(const QString &message){
 cerr << endl
      << convertFromUnicode(QObject::tr("Fatal Error"),CON) << "!" << endl
      << convertFromUnicode(message,CON) << endl;
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
     while(pos>count && line.at(pos-count-1)=='\\') {
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
 QByteArray qb=f.readAll();
 f.close();
 QTextCodec *codec=QTextCodec::codecForName("utf8");
 assert(codec);
 QString res=codec->toUnicode(qb);
 return res;
}

/**
 Save string into file with utf8 encoding.
 @param name Filename of file to write to. It will be overwritten.
 @param content String to write.
 @return true in case of success, false in case of failure
*/
bool saveToFile(const QString &name,const QString &content) {
 QFile f(name);
 if (!f.open(IO_WriteOnly)) {
  //Failure
  return false;
 }
 QTextCodec *codec=QTextCodec::codecForName("utf8");
 assert(codec);
 QCString qb=codec->fromUnicode(content);
 f.writeBlock(qb.data(),qb.length());
 f.close();

 return true;
}

/**
 Save raw bytes into file.
 @param name Filename of file to write to. It will be overwritten.
 @param content Buffer to write.
 @return true in case of success, false in case of failure
*/
bool saveRawToFile(const QString &name, const QByteArray &content) {
 QFile f(name);
 if (!f.open(IO_WriteOnly)) {
  //Failure
  return false;
 }

 f.writeBlock(content.data(),content.size());

 f.close();
 return true;
}

/**
 Print stringlist to stdout
 @param l String list to print
*/
void printList(const QStringList &l) {
 QStringList::ConstIterator it=l.begin();
 for (;it!=l.end();++it) { //load all subitems
  QString x=*it;
  cout << Q_OUT(x) <<endl;
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
  if (QString("PANIC").startsWith(cns))		{ debug::changeDebugLevel(debug::DBG_PANIC); return; }
  if (QString("CRITICAL").startsWith(cns))	{ debug::changeDebugLevel(debug::DBG_CRIT); return; }
  if (QString("ERROR").startsWith(cns))		{ debug::changeDebugLevel(debug::DBG_ERR); return; }
  if (QString("WARNING").startsWith(cns))	{ debug::changeDebugLevel(debug::DBG_WARN); return; }
  if (QString("INFO").startsWith(cns))		{ debug::changeDebugLevel(debug::DBG_INFO); return; }
  if (QString("DEBUG").startsWith(cns))		{ debug::changeDebugLevel(debug::DBG_DBG); return; }
  if (QString("DBG").startsWith(cns))		{ debug::changeDebugLevel(debug::DBG_DBG); return; }
 }
 //If debuglevel is set outside of limits - no problem, nearest "in limits" value is defacto used
 debug::changeDebugLevel(atoi(param));
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
 @param start first item in the list
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
JIS7,KOI8-R,KOI8-U,Latin1,Shift-JIS,TIS-620TSCII,eucJP,eucKR,utf8,utf16," + QString(PdfeditEncoding::get()->name()),true);
}

/** Codec for default charset */
QTextCodec *defCodec=NULL;

/**
 set default charset used when reading (and writing) property values
 @param charsetName charset name used for all 8bit to/from unicode conversions
*/
void setDefaultCharset(const QString &charsetName) {
 //Codecs should not be deleted, as Qt is managing them
 if (defCodec) defCodec=NULL;
 // Is it our codec?
 if (PdfeditEncoding::get()->name() == charsetName)
 {	
 	defCodec = PdfeditEncoding::get();
	return;
 }
 // Default codecs
 if (charsetName.isNull() || charsetName=="") return;//no codec
 defCodec=QTextCodec::codecForName(charsetName);
}

/**
 Return QString from std::string with optional character conversion according to settings
 @param str String to convert
 @param ctx Context of conversion (PDF, console, filename ...)
 @return converted string
 */
QString convertToUnicode(const std::string &str, CharsetContext ctx) {
 /** \todo: handle context, different settings for different contexts */
 if (ctx==UTF8) {
  return QString::fromUtf8(str.c_str());
 }
 if (ctx==CON) {
  return QString::fromLocal8Bit(str.c_str());
 }
 if (ctx==NAME) {
  return QFile::decodeName(str.c_str());
 }
 //PDF
 if (defCodec) {
  return defCodec->toUnicode(str.c_str());
 }
 return QString::fromLatin1(str.c_str());
}

/**
 Return std::string from QString with optional character conversion according to settings
 @param str String to convert
 @param ctx Context of conversion (PDF, console, filename ...)
 @return converted string
 */
std::string convertFromUnicode(const QString &str, CharsetContext ctx) {
 /** \todo: handle context, different settings for different contexts */
 if (ctx==UTF8) {
  return std::string(str.utf8());
 }
 if (ctx==CON) {
  return std::string(str.local8Bit());
 }
 if (ctx==NAME) {
  return std::string(QFile::encodeName(str).data());
 }
 //PDF
 if (defCodec) {
  QCString cstr=defCodec->fromUnicode(str);
  return std::string(cstr);
 }
 //Fallback to Latin1 if no codec is set
 return std::string(str.latin1());
}

} //namespace util
