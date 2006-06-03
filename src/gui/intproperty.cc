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

/** \copydoc StringProperty::writeValue */
void IntProperty::writeValue(IProperty *pdfObject) {
 if (readonly) return;//Honor readonly setting
 CInt* obj=(CInt*)pdfObject;
 int val=ed->text().toInt();
 obj->writeValue(val);
 changed=false;
}

/** \copydoc StringProperty::readValue */
void IntProperty::readValue(IProperty *pdfObject) {
 CInt* obj=(CInt*)pdfObject;
 int val;
 obj->getPropertyValue(val);
 ed->setText(QString::number(val));
 changed=false;
}

} // namespace gui
