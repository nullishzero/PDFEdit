#ifndef __QSDICT_H__
#define __QSDICT_H__

#include <qobject.h>
#include <cobject.h>
#include "qscobject.h"

using namespace pdfobjects;

class QSDict : public QSCObject {
 Q_OBJECT
public:
 QSDict(CDict *_dict);
 virtual ~QSDict();
 CDict* get();
private:
 /** Object held in class*/
 CDict *obj;
};

#endif
