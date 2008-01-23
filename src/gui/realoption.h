/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __REALOPTION_H__
#define __REALOPTION_H__

#include "stringoption.h"
class QString;

namespace gui {

/**
 RealOption - class for widget containing one editable setting of type float
 (internally represented as a double)<br>
 Used as one item in option window
 \brief Widget to edit floating-point number option
*/
class RealOption : public StringOption {
 Q_OBJECT
public:
 RealOption(const QString &_key=0,QWidget *parent=0,double _defValue=0);
 virtual ~RealOption();
};

} // namespace gui

#endif
