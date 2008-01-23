/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 RealOption - class for widget containing one editable setting of type float
 (internally represented as a double)<br>
 Used as one item in option window
 @author Martin Petricek
*/

#include "realoption.h"
#include <qvalidator.h>
#include <qlineedit.h>

namespace gui {

/** Default constructor of RealOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param _defValue Default value if option not found
 */
RealOption::RealOption(const QString &_key/*=0*/,QWidget *parent/*=0*/,double _defValue/*=0*/)
 : StringOption (_key,parent) {
 ed->setText(QString::number(_defValue));
 ed->setValidator(new QDoubleValidator(ed));
}

/** default destructor */
RealOption::~RealOption() {
}

} // namespace gui
