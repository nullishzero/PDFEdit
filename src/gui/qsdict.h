#ifndef __QSDICT_H__
#define __QSDICT_H__

#include "qsiproperty.h"
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

/*= This type represents a dictionary in PDF document.
It hold keys (String) and values. Each key is corresponding to one value.
Keys are strings, values can be of any type, either simple types (int, bool, float, string)
or complex types (Dict, Array)
*/
class QSDict : public QSIProperty {
 Q_OBJECT
public:
 QSDict(QSDict &source);
 QSDict(boost::shared_ptr<CDict> _dict);
 virtual ~QSDict();
 boost::shared_ptr<CDict> get();
public slots:
 /*- Return string representation of this dictionary */
 QString getText();
};

} // namespace gui 

#endif
