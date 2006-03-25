#ifndef __QSDICT_H__
#define __QSDICT_H__

#include <qobject.h>
#include <cobject.h>
#include "qsobject.h"

using namespace pdfobjects;

class QSDict : public QSObject<CDict> {
 Q_OBJECT
public:
 QSDict(CDict *_dict);
 virtual ~QSDict();
};

#endif
