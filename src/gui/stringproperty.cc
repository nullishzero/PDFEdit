/** @file
StringProperty - class for widget containing
 one editable property of type "String"
*/

#include "stringproperty.h"
#include <string>
#include <stdlib.h>

namespace gui {

using namespace std;

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
StringProperty::StringProperty(const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=0*/)
 : Property(_name,parent,_flags) {
 ed=new QLineEdit(this,"stringproperty_edit");
 connect(ed,SIGNAL(returnPressed())	,this,SLOT(emitChange()));
 connect(ed,SIGNAL(lostFocus())		,this,SLOT(emitChange()));
 connect(ed,SIGNAL(textChanged(const QString&)),this,SLOT(enableChange(const QString&)));
}

/** Called when text is accepted */
void StringProperty::emitChange() {
 if (!changed) return;
 emit propertyChanged(this);
}

/** Called when text changes */
void StringProperty::enableChange(const QString &newText) {
 changed=true;
}

/** return size hint of this property editing control */
QSize StringProperty::sizeHint() const {
 return ed->sizeHint();
}

/** Called on resizing of property editing control */
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
void StringProperty::writeValue(IProperty *pdfObject) {
 CString* obj=(CString*)pdfObject;
 string val=ed->text();
 obj->writeValue(val);
 changed=false;
}
/** read internal value from given PDF object
 @param pdfObject Object to read from
 */
void StringProperty::readValue(IProperty *pdfObject) {
 CString* obj=(CString*)pdfObject;
 string val;
 obj->getPropertyValue(val);
 ed->setText(val);
 changed=false;
}

} // namespace gui
