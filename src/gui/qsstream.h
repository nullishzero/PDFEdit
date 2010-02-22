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
#ifndef __QSSTREAM_H__
#define __QSSTREAM_H__

#include <kernel/cobject.h>
#include <qstringlist.h>
#include "qsiproperty.h"
class QString;

namespace gui {

using namespace pdfobjects;

/*= This type of object represents stream in document (in page) */
/** \brief QObject wrapper around CStream */
class QSStream : public QSIProperty {
 Q_OBJECT
public:
 static bool loadFile(const QString &fileName,QByteArray &qb);
 static QByteArray arrayFromBuffer(const CStream::Buffer &b);
 static const CStream::Buffer arrayToBuffer(const QByteArray &a);
 static const CStream::Buffer stringToBuffer(const QString &s);
 static QString stringFromBuffer(const CStream::Buffer &b);
 QSStream(boost::shared_ptr<CStream> _cs,BaseCore *_base);
 virtual ~QSStream();
public slots:
 /*- Sets buffer of this stream from given byte array */
 void setBuffer(const QByteArray &a);
 /*- Sets buffer of this stream from given string (overloaded method) */
 void setBuffer(const QString &s);
 /*- Sets raw buffer of this stream from given byte array */
 void setRawBuffer(const QByteArray &a);
 /*- Sets raw buffer of this stream from given string (overloaded method) */
 void setRawBuffer(const QString &s);
 /*- Loads buffer of this stream from given file. Return true on success, false on failure while loading */
 bool loadBuffer(const QString &fileName);
 /*- Loads raw buffer of this stream from given file. Return true on success, false on failure while loading */
 bool loadRawBuffer(const QString &fileName);
 /*- Gets buffer of this stream as string */
 QString getBufferString();
 /*- Gets buffer of this stream */
 QByteArray getBuffer();
 /*- Saves buffer of this stream to given file. Return true on success, false on failure while saving */
 bool saveBuffer(const QString &fileName);
 /*- Return decoded text representation of this property */
 QString getDecoded();
 /*- Return decoded raw bytes representation of this property */
 QByteArray getRawDecoded();
 /*- Add property with given name to stream dictionary */
 void add(const QString &name,QSIProperty *ip);
 void add(const QString &name,QObject *ip);
 /*- Add string property with given name to stream dictionary */
 void add(const QString &name,const QString &ip);
 /*- Add integer property with given name to stream dictionary */
 void add(const QString &name,int ip);
 /*- Return number of properties held in stream dictionary */
 int count();
 /*- Delete property with given name from stream dictionary */
 void delProperty(const QString &name);
 /*- Check for existence of property with given name in stream dictionary. If it exists, returns true */
 bool exist(const QString &name);
 /*- Get property with given name from stream dictionary */
 QSCObject* property(const QString &name);
 /*- Return array containing names of all properties in stream dictionary */
 QStringList propertyNames();
};

} // namespace gui

#endif
