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
public:
 QSStream(boost::shared_ptr<CStream> _cs);
 virtual ~QSStream();
};

} // namespace gui

#endif
