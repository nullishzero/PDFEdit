#ifndef __QSSTREAM_H__
#define __QSSTREAM_H__

#include <qstring.h>
#include <qobject.h>
#include <cobject.h>
#include "qsiproperty.h"

namespace gui {

using namespace pdfobjects;

/*= This type of object represents stream in document (in page) */
class QSStream : public QSIProperty {
 Q_OBJECT
public slots:
 /*- Sets buffer of this stream from given string */
 void setBuffer(const QString &s);
 /*- Loads buffer of this stream from given file. Return true on success, false on failure while loading */
 bool loadBuffer(const QString &fileName);
 /*- Sets raw buffer of this stream from given string */
 void setRawBuffer(const QString &s);
 /*- Loads raw buffer of this stream from given file. Return true on success, false on failure while loading */
 bool loadRawBuffer(const QString &fileName);
 /*- Gets buffer of this stream */
 QString getBuffer();
 /*- Saves buffer of this stream to given file. Return true on success, false on failure while saving */
 bool saveBuffer(const QString &fileName);
public:
 static bool loadFile(const QString &fileName,QByteArray &qb);
 static QByteArray arrayFromBuffer(const CStream::Buffer &b);
 static const CStream::Buffer arrayToBuffer(const QByteArray &a);
 static const CStream::Buffer stringToBuffer(const QString &s);
 static QString stringFromBuffer(const CStream::Buffer &b);
 QSStream(boost::shared_ptr<CStream> _cs);
 virtual ~QSStream();
};

} // namespace gui

#endif
