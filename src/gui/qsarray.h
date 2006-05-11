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
 //TODO: is it really insert?
 /*- Insert element at given index in array */
 void add(int index,QSIProperty *ip);
 void add(int index,QObject *ip);
 /*- Insert string element at given index in array */
 void add(int index,const QString &ip);
 /*- Append element at end of array */
 void add(QSIProperty *ip);
 void add(QObject *ip);
 /*- Append string element at end of array */
 void add(const QString &ip);
 /*- Return size of this array (number of elements) */
 int count();
 /*- Delete element with given index from this array. Elements with higher index (if any) are shifted to occupy the space */
 void delProperty(int index);
 /*- Return string representation of this array */
 QString getText();
 /*- Get element with given index from this array */
 QSCObject* property(int index);
};

} // namespace gui 

#endif
