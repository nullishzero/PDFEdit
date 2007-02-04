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
 BoolProperty - class for widget containing one editable property of type "Bool"
 (represented by checkbox)<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include "boolproperty.h"
#include <qcheckbox.h>
#include <cobject.h>

namespace gui {

using namespace std;

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
BoolProperty::BoolProperty(const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=defaultPropertyMode*/)
 : Property(_name,parent,_flags) {
 ed=new QCheckBox(this,"boolproperty_checkbox");
 connect(ed,SIGNAL(clicked()),this,SLOT(emitChange()));
}

/** \copydoc StringProperty::sizeHint */
QSize BoolProperty::sizeHint() const {
 return ed->sizeHint();
}

/** \copydoc StringProperty::resizeEvent */
void BoolProperty::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/** Called when clicked on the checkbox */
void BoolProperty::emitChange() {
 emit propertyChanged(this);
}

/** default destructor */
BoolProperty::~BoolProperty() {
 delete ed;
}

/** \copydoc StringProperty::setValue */
void BoolProperty::setValue(IProperty *pdfObject) {
 if (effectiveReadonly) return;//Honor readonly setting
 CBool* obj=dynamic_cast<CBool*>(pdfObject);
 assert(obj);
 bool val=ed->isChecked();
 obj->setValue(val);
 changed=false;
}

/** \copydoc StringProperty::readValue */
void BoolProperty::readValue(IProperty *pdfObject) {
 CBool* obj=dynamic_cast<CBool*>(pdfObject);
 assert(obj);
 bool val;
 obj->getValue(val);
 ed->setChecked(val);
 changed=false;
}

//See Property::setDisabled
void BoolProperty::setDisabled(bool disabled) {
 ed->setEnabled(!disabled);
}

//See Property::applyReadOnly
void BoolProperty::applyReadOnly(bool _readonly) {
 ed->setEnabled(!_readonly);
}

/** \copydoc Property::isValid() */
bool BoolProperty::isValid() {
 return true; //Checkbox is always valid :)
}

} // namespace gui
