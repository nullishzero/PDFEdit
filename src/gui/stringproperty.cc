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
 StringProperty - class for widget containing one editable property of type "String"<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include "stringproperty.h"
#include <string>
#include <stdlib.h>
#include <qlineedit.h>
#include <cobject.h>
#include "util.h"

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
