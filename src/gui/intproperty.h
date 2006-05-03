#ifndef __INTPROPERTY_H__
#define __INTPROPERTY_H__

#include "stringproperty.h"
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;

class IntProperty : public StringProperty {
Q_OBJECT
public:
 IntProperty (const QString &_name, QWidget *parent=0, PropertyFlags _flags=0);
 virtual ~IntProperty();
 virtual void writeValue(IProperty *pdfObject);
 virtual void readValue(IProperty *pdfObject);
};

} // namespace gui

#endif
