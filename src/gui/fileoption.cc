/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
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
