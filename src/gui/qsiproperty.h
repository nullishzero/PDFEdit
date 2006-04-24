#ifndef __QSIPROPERTY_H__
#define __QSIPROPERTY_H__

#include <qobject.h>
#include <iproperty.h>
#include "qscobject.h"

class QString;

namespace gui {

using namespace pdfobjects;

/*= This object represent one property in dictionary or array */
class QSIProperty : public QSCObject {
 Q_OBJECT
public:
 QSIProperty(IProperty *_ip);
 virtual ~QSIProperty();
 IProperty* get();
public slots:
 /*- Return text representation of this property */
 QString getText();
protected:
 QSIProperty(IProperty *_ip, QString _className);
private:
 /** Object held in class*/
 IProperty *obj;
};

} // namespace gui

#endif
