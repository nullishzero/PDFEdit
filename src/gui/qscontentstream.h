#ifndef __QSCONTENTSTREAM_H__
#define __QSCONTENTSTREAM_H__

#include <qstring.h>
#include <qobject.h>
#include <ccontentstream.h>
#include "qscobject.h"

namespace gui {

class QSPdfOperator;
class QSPdfOperatorIterator;

using namespace pdfobjects;

/*= This type of object represents content stream in document (in page) */
/** QObject wrapper around CContentStream */
class QSContentStream : public QSCObject {
 Q_OBJECT
public:
 QSContentStream(boost::shared_ptr<CContentStream> _cs,BaseCore *_base);
 virtual ~QSContentStream();
 boost::shared_ptr<CContentStream> get();
public slots:
 /*-
  Delete specified operator from this content stream.
  If parameter indicateChange is true (which is default), changes are immediately written to underlying stream.
 */
 void deleteOperator(QSPdfOperator *op,bool indicateChange=true);
 void deleteOperator(QObject *op,bool indicateChange=true);
 /*- Return text representation of this content stream */
 QString getText();
 /*-
  Insert specified operator newOp in this content stream, after operator op.
  If parameter indicateChange is true (which is default), changes are immediately written to underlying stream.
 */
 void insertOperator(QSPdfOperator *op,QSPdfOperator *newOp,bool indicateChange=true);
 void insertOperator(QObject *op,QObject *newOp,bool indicateChange=true);
 /*-
  Replace old operator oldOp with new operator newOp in this stream.
  If parameter indicateChange is true (which is default), changes are immediately written to underlying stream.
 */
 void replace(QSPdfOperator* oldOp,QSPdfOperator* newOp,bool indicateChange=true);
 void replace(QObject* oldOp,QObject* newOp,bool indicateChange=true);
 /*- Write any unwritten changes to operators to underlying stream. */
 void saveChange();
private:
 bool opValid(QSPdfOperator *op,bool checkThis=false);
private:
 /** Object held in class*/
 boost::shared_ptr<CContentStream> obj;
};

} // namespace gui

#endif
