#ifndef __NAMEPROPERTY_H__
#define __NAMEPROPERTY_H__

#include "property.h"
#include "stringproperty.h"
#include <qlineedit.h>
#include <cobject.h>

using namespace pdfobjects;

class NameProperty : public StringProperty {
Q_OBJECT
public:
 NameProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=0);
 virtual ~NameProperty();
};

#endif
