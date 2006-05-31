#ifndef __PROPERTYFACTORY_H__
#define __PROPERTYFACTORY_H__

#include "property.h"
class QWidget;
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;
Property* propertyFactory(IProperty *prop,const QString &_name=0,QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);

} // namespace gui

#endif
