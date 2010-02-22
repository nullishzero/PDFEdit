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
 QObject wrapper around CStream<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsstream.h"
#include "qsimporter.h"
#include "pdfutil.h"
#include "util.h"
#include <qfile.h>
#include <kernel/cobject.h>
#include <qstring.h>
#include <qcstring.h>

namespace gui {

using namespace std;
using namespace pdfobjects;

//Theoretically, this could be done better with templates, but unfortunately, it is not possible due to some C++ limitations
/** Standard error handling - code added before the operation */
#define OP_BEGIN \
 CStream *st=dynamic_cast<CStream*>(obj.get());\
 assert(st);\
 try {
/** Standard error handling - code added after the operation */
#define OP_END(func) \
 } catch (ReadOnlyDocumentException &e) { \
  base->errorException("Stream",func,QObject::tr("Document is read-only")); \
 } catch (NotImplementedException &e) { \
  base->errorException("Stream",func,QObject::tr("Operation not implemented: %1").arg(e.what())); \
 }

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

/**
 Call CStream::getDecodedStringRepresentation(ret); 
 return ret (as unicode)
*/
QString QSStream::getDecoded() {
 OP_BEGIN
  std::string text;
  st->getDecodedStringRepresentation(text);
  return util::convertToUnicode(text,util::PDF);
 OP_END("getDecoded")
 return QString::null;
}

/** 
 Call CStream::getDecodedStringRepresentation(ret);
 return ret (as raw bytes)
*/
QByteArray QSStream::getRawDecoded() {
 OP_BEGIN
  std::string text;
  st->getDecodedStringRepresentation(text);
  QByteArray res(text.size());
  memcpy(res.data(), text.c_str(), text.size());
  return res;
 OP_END("getRawDecoded")
 return QByteArray();
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
 OP_BEGIN
  QByteArray qb;
  if (!loadFile(fileName,qb)) return false;
  st->setBuffer(arrayToBuffer(qb));
  return true;
 OP_END("loadBuffer")
 return false;
}

/**
 Load raw buffer from contents of given file
 @param fileName name of the file
 @return true on success, false on failure while reading from file
*/
bool QSStream::loadRawBuffer(const QString &fileName) {
 OP_BEGIN
  QByteArray qb;
  if (!loadFile(fileName,qb)) return false;
  st->setBuffer(arrayToBuffer(qb));
  return true;
 OP_END("loadRawBuffer")
 return false;
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
 OP_BEGIN
  st->setBuffer(stringToBuffer(s));
 OP_END("setBuffer")
}

/**
 Set buffer from given byte array
 @param a Data to set into buffer
*/
void QSStream::setBuffer(const QByteArray &a) {
 OP_BEGIN
  st->setBuffer(arrayToBuffer(a));
 OP_END("setBuffer")
}

/**
 Set raw buffer from given string
 @param s Data to set into buffer
*/
void QSStream::setRawBuffer(const QString &s) {
 OP_BEGIN
  if (s.isNull()) return;
  st->setRawBuffer(stringToBuffer(s));
 OP_END("setRawBuffer")
}

/**
 Set raw buffer from given byte array
 @param a Data to set into buffer
*/
void QSStream::setRawBuffer(const QByteArray &a) {
 OP_BEGIN
  st->setRawBuffer(arrayToBuffer(a));
 OP_END("setRawBuffer")
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

/**
 Delete property from stream dictionary
 @param name Property name
*/
void QSStream::delProperty(const QString &name) {
 OP_BEGIN
  string pName=util::convertFromUnicode(name,util::PDF);
  st->delProperty(pName);
 OP_END("delProperty")
}

/**
 Add property to stream dictionary
 @param name Property name
 @param ip Property to add
*/
void QSStream::add(const QString &name,QSIProperty *ip) {
 OP_BEGIN
  string pName=util::convertFromUnicode(name,util::PDF);
  st->addProperty(pName,*(ip->get().get()));
 OP_END("add")
}

/** \copydoc add(const QString&,QSIProperty*) */
void QSStream::add(const QString &name,QObject *ip) {
 //QSA-bugfix variant of this method
 QSIProperty *ipr=dynamic_cast<QSIProperty*>(ip);
 if (ipr) add(name,ipr);
}

/**
 Add string to stream dictionary as property
 @param name Property name
 @param ip string to add
*/
void QSStream::add(const QString &name,const QString &ip) {
 OP_BEGIN
  string pName=util::convertFromUnicode(name,util::PDF);
  CString property(util::convertFromUnicode(ip,util::PDF));
  st->addProperty(pName,property);
 OP_END("add")
}

/**
 Add integer to stream dictionary as property
 @param name Property name
 @param ip integer to add
*/
void QSStream::add(const QString &name,int ip) {
 OP_BEGIN
  string pName=util::convertFromUnicode(name,util::PDF);
  CInt property(ip);
  st->addProperty(pName,property);
 OP_END("add")
}

/**
 Return list of all property names in stream dictionary
 @return List of all property names
*/
QStringList QSStream::propertyNames() {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 QStringList names;
 vector<string> list;
 st->getAllPropertyNames(list);
 vector<string>::iterator it;
 for( it=list.begin();it!=list.end();++it) { // for each property
  names+=util::convertToUnicode(*it,util::PDF);
 }
 return names;
}

/** call CDict::getPropertyCount() for stream dictionary */
int QSStream::count() {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 return st->getPropertyCount();
}

/**
 Get property by its name
 @param name Name of property
 @return the property, or NULL if not found
*/
QSCObject* QSStream::property(const QString &name) {
 try {
  CStream *st=dynamic_cast<CStream*>(obj.get());
  boost::shared_ptr<IProperty> property=st->getProperty(util::convertFromUnicode(name,util::PDF));
  return QSImporter::createQSObject(property,base);
 } catch (...) {
  //Some error, probably the property does not exist
  return NULL;
 }
}

/**
 Check if given property exists in stream dictionary
 @param name Name of property
 @return True, if property exists in stream dictionary, false if not
*/
bool QSStream::exist(const QString &name) {
 CStream *st=dynamic_cast<CStream*>(obj.get());
 return st->containsProperty(util::convertFromUnicode(name,util::PDF));
}

//TODO: add exist(..) once support in kernel

} // namespace gui
