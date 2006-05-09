/** @file
 QObject wrapper around CStream<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsstream.h"
#include "util.h"
#include <qfile.h>

namespace gui {

/** Construct wrapper with given CStream */
QSStream::QSStream(boost::shared_ptr<CStream> _cs,Base *_base) : QSIProperty (_cs,"Stream",_base) {
}

/** destructor */
QSStream::~QSStream() {
}

/**
 Convert QString (unicode string) to CStream::Buffer (basically vector of chars)
 @param s QString used as input
 @return Buffer with characters from string
*/
const CStream::Buffer QSStream::stringToBuffer(const QString &s) {
 //TODO: convert to/from unicode more intelligently
 CStream::Buffer b(s.length());
 if (s.isNull()) return b;
 for (unsigned int i=0;i<s.length();i++) {
  b[i]=s[i];
 }
 //TODO: debug, remove
 for (unsigned int i=0;i<MIN(4,s.length());i++) {
  unsigned int bb=b[i];
  unsigned int ss=s[i];
  guiPrintDbg(debug::DBG_DBG,"cmp [" << i << "] " << bb << " <- " << ss);
 }
 return b;
}

/**
 Convert CStream::Buffer (basically vector of chars) to QString (unicode string)
 @param b Buffer used as input
 @return QString with characters from buffer
*/
QString QSStream::stringFromBuffer(const CStream::Buffer &b) {
 //TODO: convert to/from unicode more intelligently
 QString s;
 s.setLength(b.size());
 for (unsigned int i=0;i<b.size();i++) {
  s[i]=b[i];
 }
 //TODO: debug, remove
 for (unsigned int i=0;i<MIN(4,s.length());i++) {
  unsigned int bb=b[i];
  unsigned int ss=QChar(s[i]);
  guiPrintDbg(debug::DBG_DBG,"cmp [" << i << "] " << bb << " -> " << ss);
 }
 return s;
}

/**
 Convert CStream::Buffer (basically vector of chars) to QByteArray
 @param b Buffer used as input
 @return Array with characters from buffer
*/
QByteArray QSStream::arrayFromBuffer(const CStream::Buffer &b) {
 QByteArray a(b.size());
 for (unsigned int i=0;i<b.size();i++) {
  a[i]=b[i];
 }
 //TODO: debug, remove
 for (unsigned int i=0;i<MIN(4,b.size());i++) {
  unsigned int bb=a[i];
  unsigned int ss=b[i];
  guiPrintDbg(debug::DBG_DBG,"cmp [" << i << "] " << bb << " <- " << ss);
 }
 return a;
}

/**
 Convert QByteArray to CStream::Buffer (basically vector of chars)
 @param a QByteArray used as input
 @return Buffer with characters from array
*/
const CStream::Buffer QSStream::arrayToBuffer(const QByteArray &a) {
 CStream::Buffer b(a.count());
 for (unsigned int i=0;i<a.count();i++) {
  b[i]=a[i];
 }
 //TODO: debug, remove
 for (unsigned int i=0;i<MIN(4,a.count());i++) {
  unsigned int bb=a[i];
  unsigned int ss=b[i];
  guiPrintDbg(debug::DBG_DBG,"cmp [" << i << "] " << bb << " -> " << ss);
 }
 return b;
}

/**
 Save contents of buffer to given file
 @param fileName name of the file
 @return true on success, false on failure while writing to file
*/
bool QSStream::saveBuffer(const QString &fileName) {
 if (fileName.isNull()) return false;
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 QByteArray qb(arrayFromBuffer(st->getBuffer()));
 QFile f(fileName);
 f.open(IO_WriteOnly | IO_Truncate);
 size_t len=f.writeBlock(qb);
 f.close();
 return len==qb.size();
}

/**
 Load buffer from contents of given file
 @param fileName name of the file
 @return true on success, false on failure while reading from file
*/
bool QSStream::loadBuffer(const QString &fileName) {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 QByteArray qb;
 if (!loadFile(fileName,qb)) return false;
 st->setBuffer(arrayToBuffer(qb));
 return true;
}

/**
 Load raw buffer from contents of given file
 @param fileName name of the file
 @return true on success, false on failure while reading from file
*/
bool QSStream::loadRawBuffer(const QString &fileName) {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 QByteArray qb;
 if (!loadFile(fileName,qb)) return false;
 st->setBuffer(arrayToBuffer(qb));
 return true;
}

/** 
 Load contents of given file into specified QByteArray
 @param fileName name of the file
 @param qb [out] QByteArray into which the file will be loaded
 @return true on success, false on failure while reading from file 
*/
bool QSStream::loadFile(const QString &fileName,QByteArray &qb) {
 if (fileName.isNull()) return false;
 QFile f(fileName);
 if (!f.open(IO_ReadOnly)) return false;
 qb=f.readAll();
 f.close();
 return true;
}

/**
 Set buffer from given string
 @param s Data to set into buffer
*/
void QSStream::setBuffer(const QString &s) {
 if (s.isNull()) return;
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 st->setBuffer(stringToBuffer(s));
}

/**
 Set buffer from given byte array
 @param a Data to set into buffer
*/
void QSStream::setBuffer(const QByteArray &a) {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 st->setBuffer(arrayToBuffer(a));
}

/**
 Set raw buffer from given string
 @param s Data to set into buffer
*/
void QSStream::setRawBuffer(const QString &s) {
 if (s.isNull()) return;
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 st->setRawBuffer(stringToBuffer(s));
}

/**
 Set raw buffer from given byte array
 @param a Data to set into buffer
*/
void QSStream::setRawBuffer(const QByteArray &a) {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 st->setRawBuffer(arrayToBuffer(a));
}

/** 
 Return data from buffer
 @return buffer as QString
*/
QString QSStream::getBufferString() {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 return stringFromBuffer(st->getBuffer());
}

/** 
 Return data from buffer
 @return buffer as QByteArray
*/
QByteArray QSStream::getBuffer() {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 return arrayFromBuffer(st->getBuffer());
}

} // namespace gui
