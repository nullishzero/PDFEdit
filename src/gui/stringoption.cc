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
 StringOption - class for widget containing one editable setting of type string
 Also a base class for editing some more specific types.
 @author Martin Petricek
*/

#include "stringoption.h"
#include <utils/debug.h>
#include <qstring.h>
#include <qevent.h>
#include <qlineedit.h>
#include "settings.h"
#include "util.h"

namespace gui {

/** Default constructor of StringOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param defValue Default value if option not found in configuration
 */
StringOption::StringOption(const QString &_key/*=0*/,QWidget *parent/*=0*/,const QString &defValue/*=QString::null*/)
 : Option (_key,parent) {
 ed=new QLineEdit(this,"option_edit");
 if (!defValue.isNull()) ed->setText(defValue);
 connect(ed,SIGNAL(textChanged(const QString&)),this,SLOT(enableChange(const QString&)));
}

/** default destructor */
StringOption::~StringOption() {
 delete ed;
}

/**
 Called when text changes
 @param newText value of new text
 */
void StringOption::enableChange(__attribute__((unused)) const QString &newText) {
 changed=true;
}

/** write edited value to settings (using key specified in contructor) */
void StringOption::writeValue() {
 if (!changed) return; //Avoid writing unchanged values and cluttering user config
 globalSettings->write(key,ed->text());
}

/** read value from settings for editing (using key specified in contructor) */
void StringOption::readValue() {
 QString value=globalSettings->read(key);
 if (value.isNull()) return;
 ed->setText(value);
 changed=false; //Since we've just read the actual setting
}

/**
 return size hint of this option editing control
 @return size hint from inner editbox
  */
QSize StringOption::sizeHint() const {
 return ed->sizeHint();
}

/**
 Called on resizing of option editing control
 Will simply set the same fixed size to inner editbox
 @param e resize event
 */
void StringOption::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

} // namespace gui
