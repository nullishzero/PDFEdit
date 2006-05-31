#ifndef __QSIPROPERTYARRAY_H__
#define __QSIPROPERTYARRAY_H__

#include "qscobject.h"
#include <cobject.h>
#include <stdlib.h>
#include <pdfoperators.h>

namespace gui {

class QSIProperty;
class Base;

using namespace pdfobjects;

/*=
 This type represents arbitrary array of IProperty items
 It hold zero or more values indexed by positive integer, starting from zero.
 Values can be of any type, either simple types (int, bool, float, string)
 or complex types (Dict, Array)
*/
class QSIPropertyArray : public QSCObject {
 Q_OBJECT
public:
 QSIPropertyArray(Base *_base);
 virtual ~QSIPropertyArray();
 boost::shared_ptr<IProperty> get(int index);
 void append(boost::shared_ptr<IProperty> prop);
 void copyTo(PdfOperator::Operands &oper);
public slots:
 /*- append one IProperty element to end of array. */
 void append(QSIProperty *prop);
 void append(QObject *obj);
 /*- Return size of this array (number of elements) */
 int count();
 /*- Get IProperty element with given index from this array */
 QSCObject* property(int index);
private:
 /** Vector holding parameters */
 std::vector<boost::shared_ptr<IProperty> > obj;
};

} // namespace gui 

#endif
