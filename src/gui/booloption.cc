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
 BoolOption - class for widget containing one editable setting of type bool
 @author Martin Petricek
*/

#include "booloption.h"
#include "settings.h"
#include <qstring.h>
#include <qcheckbox.h>
#include <qevent.h>

namespace gui {

/**
 Default constructor of BoolOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param _defValue Default value if option not found
 */
BoolOption::BoolOption(const QString &_key,QWidget *parent/*=0*/,bool _defValue/*=false*/)
 : Option (_key,parent) {
 ed=new QCheckBox(this,"booloption_checkbox");
 connect(ed,SIGNAL(clicked()),this,SLOT(boolChange()));
 defValue=_defValue;
}

/** default destructor */
BoolOption::~BoolOption() {
 delete ed;
}

/** Called when clicked on the checkbox */
void BoolOption::boolChange() {
 changed=true;
}

/**
 return size hint of this control
 @return preferred size of this control
*/
QSize BoolOption::sizeHint() const {
 return ed->sizeHint();
}

/**
 Called on resizing of this option editing control
 @param e resize event parameters
*/
void BoolOption::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/** write edited value to settings */
void BoolOption::writeValue() {
 if (!changed) return;
 bool val=ed->isChecked();
 globalSettings->write(key,val?"1":"0");
}

/** read value from settings for editing */
void BoolOption::readValue() {
 bool val=globalSettings->readBool(key,defValue);
 ed->setChecked(val);
}

} // namespace gui

