/** @file
 RealProperty - class for widget containing one editable property of type "Real"<br>
 (internally represented as a double)<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include "realproperty.h"
#include <qvalidator.h>
#include <qlineedit.h>
#include <cobject.h>

namespace gui {

/** Default constructor of property item
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

/** @copydoc StringProperty::writeValue */
void RealProperty::writeValue(IProperty *pdfObject) {
 if (readonly) return;//Honor readonly setting
 CReal* obj=(CReal*)pdfObject;
 double val=ed->text().toDouble();
 obj->writeValue(val);
 changed=false;
}

/** @copydoc StringProperty::readValue */
void RealProperty::readValue(IProperty *pdfObject) {
 CReal* obj=(CReal*)pdfObject;
 double val;
 obj->getPropertyValue(val);
 ed->setText(QString::number(val));
 changed=false;
}

} // namespace gui
