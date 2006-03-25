#ifndef __PROPERTYFACTORY_H__
#define __PROPERTYFACTORY_H__

#include <iproperty.h>
#include <qwidget.h>
#include "property.h"

using namespace pdfobjects;

Property* propertyFactory(IProperty *prop,const QString &_name=0,QWidget *parent=0, PropertyFlags _flags=0);

#endif
