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
 DialogOption - Abstract class for widget containing one editable string setting,
 with nearby "..." button allowing to invoke dialog to change the setting
 in some possibly  more user-friently way
 Subclasses should reimplement the dialog function to invoke some dialog and change the string value accordingly
 @author Martin Petricek
*/

#include "dialogoption.h"
#include <utils/debug.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qevent.h>
#include "settings.h"
#include "util.h"

namespace gui {

/**
 Default constructor of DialogOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param defValue Default value if option not found in configuration
 */
DialogOption::DialogOption(const QString &_key/*=0*/,QWidget *parent/*=0*/,const QString &defValue/*=QString::null*/)
 : Option (_key,parent) {
 ed=new QLineEdit(this,"option_edit");
 setFocusProxy(ed);
 pb=new QPushButton("..",this,"option_pickbutton");
 if (!defValue.isNull()) ed->setText(defValue);
 connect(pb,SIGNAL(clicked()),this,SLOT(invokeDialog()));
 connect(ed,SIGNAL(textChanged(const QString&)),this,SLOT(enableChange(const QString&)));

}

/**
 Called when text changes
 @param newText value of new text
 */
void DialogOption::enableChange(__attribute__((unused)) const QString &newText) {
 changed=true;
}

/**
 Slot called when user pressed the "..." (invoke dialog) button.
 Basically, the dialog function is executed and the text is updated
*/
void DialogOption::invokeDialog() {
 QString value=ed->text();
 dialog(value);
 ed->setText(value);
 changed=true;
}

/** default destructor */
DialogOption::~DialogOption() {
 delete ed;
}

/** write edited value to settings (using key specified in contructor) */
void DialogOption::writeValue() {
 globalSettings->write(key,ed->text());
}

/** read value from settings for editing (using key specified in contructor) */
void DialogOption::readValue() {
 QString value=globalSettings->read(key);
 if (value.isNull()) return;
 ed->setText(value);
 changed=false; //Since we've just read the actual setting
}

/**
 return size hint of this option editing control
 @return size hint from inner editbox
  */
QSize DialogOption::sizeHint() const {
 return ed->sizeHint();
}

/**
 Called on resizing of option editing control
 Will simply set the same fixed size to inner editbox
 @param e resize event
 */
void DialogOption::resizeEvent (QResizeEvent *e) {
 int w=e->size().width();
 int h=e->size().height();
 if (w<=0 || h<=0) return;//Some invalid size anyway. No need to move the subwidgets
 pb->move(w-h,0);
 pb->setFixedSize(QSize(h,h));
 ed->move(0,0);
 ed->setFixedSize(QSize(MAX(0,w-h),h));
}

} // namespace gui
