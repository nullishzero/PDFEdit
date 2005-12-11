/** @file
IntProperty - class for widget containing
 one editable property of type "Int"
*/

#include "intproperty.h"
#include <qvalidator.h>

//TODO: add functions for display & editing ... 

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param name name of his property
 @param flags flags of this property items (default 0)
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
void IntProperty::writeValue(void *pdfObject) {
 //TODO
}
 /** read internal value from given PDF object */
void IntProperty::readValue(void *pdfObject) {
 //TODO
}
