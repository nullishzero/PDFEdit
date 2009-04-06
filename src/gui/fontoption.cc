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
 FontOption - class for widget containing one editable string setting,
 while allowing to either type (allowing things like copy/paste) the font
 description (name), or use dialog to pick it. User's choice
 @author Martin Petricek
*/

#include "fontoption.h"
#include <qstring.h>
#include <qfontdialog.h>

namespace gui {

/**
 Default constructor of FontOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param defValue Default value if option not found in configuration
 */
FontOption::FontOption(const QString &_key/*=0*/,QWidget *parent/*=0*/,const QString &defValue/*=0*/)
 : DialogOption (_key,parent,defValue) {
}

//See DialogOption for description of this abstract method
void FontOption::dialog(QString &value) {
 QFont initial;
 initial.fromString(value);
 bool ok;
 QFont ret=QFontDialog::getFont(&ok,initial,this,"font_dialog");
 if (ok) { //Some font picked
  // font is set to the font the user selected
  value=ret.toString();
 }
}

/** default destructor */
FontOption::~FontOption() {
}

} // namespace gui
