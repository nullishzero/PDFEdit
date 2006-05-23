#ifndef __QSPDFOPERATOR_H__
#define __QSPDFOPERATOR_H__

#include <qstring.h>
#include <qobject.h>
#include <pdfoperators.h>
#include "qscobject.h"

namespace gui {

class Base;

using namespace pdfobjects;

/*= This type of object represents pdf operator in content stream */
class QSPdfOperator : public QSCObject {
 Q_OBJECT
public:
 QSPdfOperator(boost::shared_ptr<PdfOperator> _cs,Base *_base);
 virtual ~QSPdfOperator();
 boost::shared_ptr<PdfOperator> get();
public slots:
 /*- Return text representation of this pdf operator */
 QString getText();
 /*- Return name of this pdf operator */
 QString getName();
private:
 /** Object held in class*/
 boost::shared_ptr<PdfOperator> obj;
};

} // namespace gui

#endif
