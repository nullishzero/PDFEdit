/** @file
IntProperty - class for widget containing
 one editable property of type "Int"
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
IntProperty::IntProperty (const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=0*/)
 : StringProperty(_name,parent,_flags) {
 ed->setText("0");
 ed->setValidator(new QIntValidator(ed));
}

/** default destructor */
IntProperty::~IntProperty() {
}

 /** write internal value to given PDF object */
void IntProperty::writeValue(IProperty *pdfObject) {
 CInt* obj=(CInt*)pdfObject;
 int val=ed->text().toInt();
 obj->writeValue(val);
 changed=false;
}
 /** read internal value from given PDF object */
void IntProperty::readValue(IProperty *pdfObject) {
 CInt* obj=(CInt*)pdfObject;
 int val;
 obj->getPropertyValue(val);
 ed->setText(QString::number(val));
 changed=false;
}

} // namespace gui
