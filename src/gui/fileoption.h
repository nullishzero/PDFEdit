/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __FILEOPTION_H__
#define __FILEOPTION_H__

#include "dialogoption.h"
class QString;

namespace gui {

/**
 FileOption - class for widget containing one editable string setting,
 while allowing to either type the name, or use dialog to pick the filename. User's choice<br>
 Used as one item type in option window<br>
 \brief Widget to edit option by typing or picking a file
*/

class FileOption : public DialogOption {
 Q_OBJECT
public:
 FileOption(const QString &_key=0,QWidget *parent=0,const QString &defValue=0);
 virtual void dialog(QString &value);
 virtual ~FileOption();
};

} // namespace gui

#endif
