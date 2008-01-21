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
 FileOption - class for widget containing one editable string setting,
 while allowing to either type the name, or use dialog to pick the filename. User's choice
 @author Martin Petricek
*/

#include "fileoption.h"
#include "dialog.h"
#include <qstring.h>

namespace gui {

/**
 Default constructor of FileOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param defValue Default value if option not found in configuration
 */
FileOption::FileOption(const QString &_key/*=0*/,QWidget *parent/*=0*/,const QString &defValue/*=0*/)
 : DialogOption (_key,parent,defValue) {
}

//See DialogOption for description of this abstract method
void FileOption::dialog(QString &value) {
 QString ret;
 //spawn instance of generic "save file" dialog
 ret=saveFileDialog(this,value,false,QObject::tr("Pick a file ..."),"file_option");
 if (ret.isNull()) return; //No file picked
 value=ret;
}

/** default destructor */
FileOption::~FileOption() {
}

} // namespace gui
