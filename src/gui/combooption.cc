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
 ComboOption - class for widget containing one editable setting of type string, selectable from combobox<br>
 If current setting specify item not in the list, the first item in list is shown instead<br>
 User is unable to specify string not in the list<br>
 Used as one item in option window<br>
 @author Martin Petricek
*/

#include "combooption.h"
#include "settings.h"
#include "util.h"
#include <qcombobox.h>
#include <qstring.h>
#include <qstringlist.h>
#include <utils/debug.h>
#include <qevent.h>

namespace gui {

/**
 Default constructor of ComboOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param _values List of accepted values
 */
ComboOption::ComboOption(const QStringList &_values,const QString &_key/*=0*/,QWidget *parent/*=0*/)
 : Option (_key,parent) {
 init(_values,_values);
}

/**
 Default constructor of ComboOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param _values List of accepted values
 @param _valueDesc List of value descriptions, must map 1:1 with values (same count and order of items)
 */
ComboOption::ComboOption(const QStringList &_values,const QStringList &_valueDesc,const QString &_key/*=0*/,QWidget *parent/*=0*/)
 : Option (_key,parent) {
 init(_values,_valueDesc);
}

/**
 Internal initialization function used in constructor
 @param _values list to use as item values
 @param _desc list to use as item descriptions
*/
void ComboOption::init(const QStringList &_values, const QStringList &_desc) {
 idx=-1;
 ed=new QComboBox(false,this);
 values=_values;
 caseSensitive=false;
 ed->setDuplicatesEnabled(true);
 ed->insertStringList(_desc);
 ed->setInsertionPolicy(QComboBox::NoInsertion);
 connect(ed,SIGNAL(activated(int)),this,SLOT(itemActivated(int)));
}

/** default destructor */
ComboOption::~ComboOption() {
 delete ed;
}

/** write edited value to settings */
void ComboOption::writeValue() {
 if (!changed) return;
 if (idx<0) {
  //No index, so just guess ... shouldn't happen
  globalSettings->write(key,ed->currentText());
 } else {
  globalSettings->write(key,values[idx]);
 }
}

/**
 Slot called when changing the item in combobox
 @param index Index of newly selected item
*/
void ComboOption::itemActivated(int index) {
 changed=true;
 idx=index;
}

/** read value from settings for editing */
void ComboOption::readValue() {
 QString value=globalSettings->read(key);
 if (value.isNull()) return;
 if (caseSensitive) value=value.lower();
 //Look for item in the list
 int valuesCount=values.count();
 idx=0;
 QString v;
 for(int i=0;i<valuesCount;i++) {
  v=values[i];
  if (caseSensitive) v=v.lower();
  if (v==value) {
   idx=i;
   break;
  }
 }
 //Use first match (Will "normalize" case in case of case-insensitive matching)
 ed->setCurrentItem(idx);
 changed=false; //Since we've just read the actual setting
}

/**
 Set case sensitivity of list items. Default is case insensitive
 @param value New value for case-sensitivity
 */
void ComboOption::setCaseSensitive(bool value) {
 caseSensitive=value;
}

/**
 return size hint of this option editing control
 @return Optimal size
 */
QSize ComboOption::sizeHint() const {
 return ed->sizeHint();
}

/**
 Called on resizing of option editing control
 @param e Resize event data (new size)
*/
void ComboOption::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

} // namespace gui
