#ifndef __QSDICT_H__
#define __QSDICT_H__

#include "qscobject.h"
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

class QSDict : public QSCObject {
 Q_OBJECT
public:
 QSDict(CDict *_dict);
 virtual ~QSDict();
 CDict* get();
public slots:
 QString getString();
private:
 /** Object held in class*/
 CDict *obj;
};

} // namespace gui 

#endif
