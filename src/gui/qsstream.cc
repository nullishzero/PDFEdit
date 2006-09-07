/** @file
 QObject wrapper around CStream<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsstream.h"
#include "util.h"
#include <qfile.h>
#include <cobject.h>
#include <qstring.h>

namespace gui {

/**
 Construct wrapper with given CStream 
 @param _cs CStream shared pointer
 @param _base scripting base
*/
QSStream::QSStream(boost::shared_ptr<CStream> _cs,BaseCore *_base) : QSIProperty (_cs,"Stream",_base) {
}

/** destructor */
QSStream::~QSStream() {
}

/**
 Convert QString (unicode string) to CStream::Buffer (basically vector of chars)
 unicode characters 0-255 are mapped to characters with code 0-255
 Mapping of characters outside of range 0-255 is undefined
 \see stringFromBuffer
 @param s QString used as input
 @return Buffer with characters from string
*/
const CStream::Buffer QSStream::stringToBuffer(const QString &s) {
 CStream::Buffer b(s.length());
 if (s.isNull()) return b;
 for (unsigned int i=0;i<s.length();i++) {
  //Assumption: b[i] accepts char
  b[i]=(char)(s[i].latin1());
 }
 assert(s.length()==b.size());
 return b;
}

/** Call CStream::getDecodedStringRepresentation(ret); return ret */
QString QSStream::getDecoded() {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 std::string text;
 st->getDecodedStringRepresentation(text);
 return text;
}

/**
 Convert CStream::Buffer (basically vector of chars) to QString (unicode string)
 characters 0-255 are mapped to unicode characters with code 0-255
 \see stringToBuffer
 @param b Buffer used as input
 @return QString with characters from buffer
*/
QString QSStream::stringFromBuffer(const CStream::Buffer &b) {
 QString s;
 s.reserve(b.size());
 for (unsigned int i=0;i<b.size();i++) {
  //Assumption: b[i] returns char
  s.append((char)(b[i]));
 }
 assert(s.length()==b.size());
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
 assert(a.count()==b.size());
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
 assert(a.count()==b.size());
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
 try {
  st->setBuffer(stringToBuffer(s));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Stream","setBuffer",QObject::tr("Document is read-only"));
 }
}

/**
 Set buffer from given byte array
 @param a Data to set into buffer
*/
void QSStream::setBuffer(const QByteArray &a) {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 try {
  st->setBuffer(arrayToBuffer(a));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Stream","setBuffer",QObject::tr("Document is read-only"));
 }
}

/**
 Set raw buffer from given string
 @param s Data to set into buffer
*/
void QSStream::setRawBuffer(const QString &s) {
 if (s.isNull()) return;
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 try {
  st->setRawBuffer(stringToBuffer(s));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Stream","setRawBuffer",QObject::tr("Document is read-only"));
 }
}

/**
 Set raw buffer from given byte array
 @param a Data to set into buffer
*/
void QSStream::setRawBuffer(const QByteArray &a) {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 assert(st);
 try {
  st->setRawBuffer(arrayToBuffer(a));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Stream","setRawBuffer",QObject::tr("Document is read-only"));
 }
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
