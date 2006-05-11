#ifndef __QSIPROPERTY_H__
#define __QSIPROPERTY_H__

#include <qobject.h>
#include <iproperty.h>
#include "qscobject.h"

class QString;

namespace gui {

using namespace pdfobjects;

class Base;

/*= This object represent one property in dictionary or array */
class QSIProperty : public QSCObject {
 Q_OBJECT
public:
 QSIProperty(boost::shared_ptr<IProperty> _ip,Base *_base);
 QSIProperty(QSIProperty &source);
 virtual ~QSIProperty();
 boost::shared_ptr<IProperty> get() const;
public slots:
 /*- Return text representation of this property */
 QString getText();
protected:
 QSIProperty(boost::shared_ptr<IProperty> _ip, QString _className,Base *_base);
protected:
 /** Object held in class*/
 boost::shared_ptr<IProperty> obj;
};

} // namespace gui

#endif
