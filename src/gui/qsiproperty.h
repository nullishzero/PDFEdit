#ifndef __QSIPROPERTY_H__
#define __QSIPROPERTY_H__

#include <qobject.h>
#include <iproperty.h>
#include "qscobject.h"

namespace gui {

using namespace pdfobjects;

class QSIProperty : public QSCObject {
 Q_OBJECT
public:
 QSIProperty(IProperty *_ip);
 virtual ~QSIProperty();
 IProperty* get();
protected:
 QSIProperty(IProperty *_ip, QString _className);
private:
 /** Object held in class*/
 IProperty *obj;
};

} // namespace gui

#endif
