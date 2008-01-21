/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __FONTOPTION_H__
#define __FONTOPTION_H__

#include "dialogoption.h"
class QString;

namespace gui {

/**
 FontOption - class for widget containing one editable string setting,
 while allowing to either type (allowing things like copy/paste) the font
 description (name and parameters), or use dialog to pick it. User's choice<br>
 Used as one item type in option window<br>
 \brief Widget to edit option by typing or picking a font
*/
class FontOption : public DialogOption {
 Q_OBJECT
public:
 FontOption(const QString &_key=0,QWidget *parent=0,const QString &defValue=0);
 virtual void dialog(QString &value);
 virtual ~FontOption();
};

} // namespace gui

#endif
