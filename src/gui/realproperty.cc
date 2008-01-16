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
