#ifndef __QSARRAY_H__
#define __QSARRAY_H__

#include "qsiproperty.h"
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

/*= This type represents an array in PDF document.
It hold zero or more values indexed by positive integer, starting from zero.
Values can be of any type, either simple types (int, bool, float, string)
or complex types (Dict, Array)
*/
class QSArray : public QSIProperty {
 Q_OBJECT
public:
 QSArray(QSArray &source);
 QSArray(boost::shared_ptr<CArray> _array,Base *_base);
 virtual ~QSArray();
 boost::shared_ptr<CArray> get();
public slots:
 /*- Return string representation of this array */
 QString getText();
};

} // namespace gui 

#endif
