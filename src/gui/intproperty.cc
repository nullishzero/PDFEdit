/** @file
 IntProperty - class for widget containing one editable property of type "Int"<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include "intproperty.h"
#include <qvalidator.h>
#include <cobject.h>
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
