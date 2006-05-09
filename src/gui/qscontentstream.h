#ifndef __QSCONTENTSTREAM_H__
#define __QSCONTENTSTREAM_H__

#include <qstring.h>
#include <qobject.h>
#include <ccontentstream.h>
#include "qscobject.h"

namespace gui {

class Base;

using namespace pdfobjects;

/*= This type of object represents content stream in document (in page) */
class QSContentStream : public QSCObject {
 Q_OBJECT
public:
 QSContentStream(boost::shared_ptr<CContentStream> _cs,Base *_base);
 virtual ~QSContentStream();
 boost::shared_ptr<CContentStream> get();
public slots:
 /*- Return text representation of this content stream */
 QString getText();
private:
 /** Object held in class*/
 boost::shared_ptr<CContentStream> obj;
};

} // namespace gui

#endif
