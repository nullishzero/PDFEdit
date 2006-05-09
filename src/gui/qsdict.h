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
//ADDED functions begin
//ADDED functions end
 Q_OBJECT
public:
 QSDict(QSDict &source);
 QSDict(boost::shared_ptr<CDict> _dict,Base *_base);
 virtual ~QSDict();
 boost::shared_ptr<CDict> get();
public slots:
 /*- Add property with given name to this dictionary */
 void add(const QString &name,QSIProperty *ip);
 void add(const QString &name,QObject *ip);
 /*- Add string property with given name to this dictionary */
 void add(const QString &name,const QString &ip);
 /*- Return number of properties held in this dictionary */
 size_t count();
 /*- Delete property with given name from this dictionary */
 void delProperty(const QString &name);
 /*- Return string representation of this dictionary */
 QString getText();
 /*- Get property with given name from this dictionary */
 QSCObject* property(const QString &name);
};

} // namespace gui 

#endif
