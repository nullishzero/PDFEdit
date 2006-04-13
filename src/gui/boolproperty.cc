/** @file
BoolProperty - class for widget containing
 one editable property of type "Bool" (checkbox)
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
BoolProperty::BoolProperty(const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=0*/)
 : Property(_name,parent,_flags) {
 ed=new QCheckBox(this,"boolproperty_checkbox");
 connect(ed,SIGNAL(clicked()),this,SLOT(emitChange()));
}

/** return size hint of this property editing control */
QSize BoolProperty::sizeHint() const {
 return ed->sizeHint();
}

/** Called on resizing of property editing control */
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

/** write internal value to given PDF object
 @param pdfObject Object to write to
 */
void BoolProperty::writeValue(IProperty *pdfObject) {
 CBool* obj=(CBool*)pdfObject;
 bool val=ed->isChecked();
 obj->writeValue(val);
 changed=false;
}
/** read internal value from given PDF object
 @param pdfObject Object to read from
 */
void BoolProperty::readValue(IProperty *pdfObject) {
 CBool* obj=(CBool*)pdfObject;
 bool val;
 obj->getPropertyValue(val);
 ed->setChecked(val);
 changed=false;
}

} // namespace gui
