#ifndef __QSSTREAM_H__
#define __QSSTREAM_H__

#include <qstring.h>
#include <qobject.h>
#include <cobject.h>
#include "qscobject.h"

namespace gui {

using namespace pdfobjects;

/*= This type of object represents stream in document (in page) */
class QSStream : public QSCObject {
 Q_OBJECT
public:
 QSStream(boost::shared_ptr<CStream> _cs);
 virtual ~QSStream();
 boost::shared_ptr<CStream> get();
public slots:
 /*- Return text representation of this stream */
 QString getText();
private:
 /** Object held in class*/
 boost::shared_ptr<CStream> obj;
};

} // namespace gui

#endif
