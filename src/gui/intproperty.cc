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
 IntProperty - class for widget containing one editable property of type "Int"<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include "intproperty.h"
#include <qvalidator.h>
#include <kernel/cobject.h>
#include <qlineedit.h>

namespace gui {

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
IntProperty::IntProperty (const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=defaultPropertyMode*/)
 : StringProperty(_name,parent,_flags) {
 ed->setText("0");
 ed->setValidator(new QIntValidator(ed));
}

/** default destructor */
IntProperty::~IntProperty() {
}

/** \copydoc StringProperty::setValue */
void IntProperty::setValue(IProperty *pdfObject) {
 if (effectiveReadonly) return;//Honor readonly setting
 CInt* obj=dynamic_cast<CInt*>(pdfObject);
 assert(obj);
 int val=ed->text().toInt();
 obj->setValue(val);
 changed=false;
}

/** \copydoc StringProperty::readValue */
void IntProperty::readValue(IProperty *pdfObject) {
 CInt* obj=dynamic_cast<CInt*>(pdfObject);
 assert(obj);
 int val;
 obj->getValue(val);
 ed->setText(QString::number(val));
 changed=false;
}

} // namespace gui
