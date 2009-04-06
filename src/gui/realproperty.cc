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
 RealProperty - class for widget containing one editable property of type "Real"<br>
 (internally represented as a double)<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include "realproperty.h"
#include <qvalidator.h>
#include <qlineedit.h>
#include <kernel/cobject.h>

namespace gui {

/**
 Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
*/
RealProperty::RealProperty (const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=defaultPropertyMode*/)
 : StringProperty(_name,parent,_flags) {
 ed->setText("0");
 ed->setValidator(new QDoubleValidator(ed));
}

/** default destructor */
RealProperty::~RealProperty() {
}

/** \copydoc StringProperty::setValue */
void RealProperty::setValue(IProperty *pdfObject) {
 if (effectiveReadonly) return;//Honor readonly setting
 CReal* obj=dynamic_cast<CReal*>(pdfObject);
 assert(obj);
 double val=ed->text().toDouble();
 obj->setValue(val);
 changed=false;
}

/** \copydoc StringProperty::readValue */
void RealProperty::readValue(IProperty *pdfObject) {
 CReal* obj=dynamic_cast<CReal*>(pdfObject);
 assert(obj);
 double val;
 obj->getValue(val);
 ed->setText(QString::number(val));
 changed=false;
}

} // namespace gui
