#ifndef __REALPROPERTY_H__
#define __REALPROPERTY_H__

#include "property.h"
#include "stringproperty.h"
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

class RealProperty : public StringProperty {
Q_OBJECT
public:
 RealProperty (const QString &_name, QWidget *parent=0, PropertyFlags _flags=0);
 virtual ~RealProperty();
 void writeValue(IProperty *pdfObject);
 void readValue(IProperty *pdfObject);
};

} // namespace gui

#endif
