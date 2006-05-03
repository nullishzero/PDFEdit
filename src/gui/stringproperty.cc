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
 setFocusProxy(ed);
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
void StringProperty::writeValue(IProperty *pdfObject) {
 if (readonly) return;//Honor readonly setting
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

/** @copydoc Property::isValid() */
bool StringProperty::isValid() {
 return ed->hasAcceptableInput();
}

/* @copydoc Property:setReadOnly */
void StringProperty::setReadOnly(bool _readonly) {
 //Widget is enabled if it is not read-only
 ed->setEnabled(!_readonly);
 Property::setReadOnly(_readonly);
}

} // namespace gui
