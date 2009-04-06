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
 StringProperty - class for widget containing one editable property of type "String"<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include "stringproperty.h"
#include <string>
#include <stdlib.h>
#include <qlineedit.h>
#include <kernel/cobject.h>
#include "util.h"
#include <qevent.h>

namespace gui {

using namespace std;

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
StringProperty::StringProperty(const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=defaultPropertyMode*/)
 : Property(_name,parent,_flags) {
 ed=new QLineEdit(this,"stringproperty_edit");
 setFocusProxy(ed);
 modifyColor(ed);
 connect(ed,SIGNAL(returnPressed())	,this,SLOT(emitChange()));
 connect(ed,SIGNAL(lostFocus())		,this,SLOT(emitChange()));
 connect(ed,SIGNAL(textChanged(const QString&)),this,SLOT(enableChange(const QString&)));
}

/** Called when text is accepted */
void StringProperty::emitChange() {
 if (!changed) return;
 emit propertyChanged(this);
}

/** Called when text changes
 @param newText value of new text
 */
void StringProperty::enableChange(__attribute__((unused)) const QString &newText) {
 changed=true;
}

/** return size hint of this property editing control
 @return preferred size for this widget
 */
QSize StringProperty::sizeHint() const {
 return ed->sizeHint();
}

/** Called on resizing of property editing control
 @param e resize event containing new widget size
 */
void StringProperty::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/** default destructor */
StringProperty::~StringProperty() {
 delete ed;
}

/** write internal value to given PDF object
 @param pdfObject Object to write to
 */
void StringProperty::setValue(IProperty *pdfObject) {
 if (effectiveReadonly) return;//Honor readonly setting
 CString* obj=dynamic_cast<CString*>(pdfObject);
 assert(obj);
 string val=util::convertFromUnicode(ed->text(),util::PDF);
 obj->setValue(val);
 changed=false;
}

/** read internal value from given PDF object
 @param pdfObject Object to read from
 */
void StringProperty::readValue(IProperty *pdfObject) {
 CString* obj=dynamic_cast<CString*>(pdfObject);
 assert(obj);
 string val;
 obj->getValue(val);
 ed->setText(util::convertToUnicode(val,util::PDF));
 changed=false;
}

/** \copydoc Property::isValid() */
bool StringProperty::isValid() {
 return ed->hasAcceptableInput();
}

//See Property::setDisabled
void StringProperty::setDisabled(bool disabled) {
 ed->setEnabled(!disabled);
}

//See Property::applyReadOnly
void StringProperty::applyReadOnly(bool _readonly) {
 ed->setReadOnly(_readonly);
}

} // namespace gui
