/** @file
RealProperty - class for widget containing
 one editable property of type "Real" (internally a double)
*/

#include "realproperty.h"
#include <qvalidator.h>

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
RealProperty::RealProperty (const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=0*/)
 : StringProperty(_name,parent,_flags) {
 ed->setText("0");
 ed->setValidator(new QDoubleValidator(ed));
}

/** default destructor */
RealProperty::~RealProperty() {
}

 /** write internal value to given PDF object */
void RealProperty::writeValue(IProperty *pdfObject) {
 CReal* obj=(CReal*)pdfObject;
 double val=ed->text().toDouble();
 obj->writeValue(val);
}
 /** read internal value from given PDF object */
void RealProperty::readValue(IProperty *pdfObject) {
 CReal* obj=(CReal*)pdfObject;
 double val;
 obj->getPropertyValue(val);
 ed->setText(QString::number(val));
}
