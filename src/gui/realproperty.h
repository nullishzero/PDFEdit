/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __REALPROPERTY_H__
#define __REALPROPERTY_H__

#include "stringproperty.h"
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;

/**
 RealProperty - class for widget containing one editable property of type "Real"<br>
 (internally represented as a double)<br>
 Used as one item in property editor
 \brief Property editor widget for editing CReal
*/
class RealProperty : public StringProperty {
Q_OBJECT
public:
 RealProperty (const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
 virtual ~RealProperty();
 virtual void setValue(IProperty *pdfObject);
 virtual void readValue(IProperty *pdfObject);
};

} // namespace gui

#endif
