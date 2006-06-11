/** @file
 NameProperty - class for widget containing one editable property of type "Name"<br>
 Basically, CName is CString, but we might want differentiation in case we'll handle names specially in future<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include "nameproperty.h"
#include <string>
#include <stdlib.h>
#include <qlineedit.h>
#include <cobject.h>
#include "util.h"

namespace gui {

using namespace std;
using namespace pdfobjects;

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
NameProperty::NameProperty(const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=defaultPropertyMode*/)
 : StringProperty(_name,parent,_flags) {
 //just use StringProperty
}

//These two are same as of string, but need to be here because of the typecast ...

/**
 write internal value to given PDF object
 @param pdfObject Object to write to
 */
void NameProperty::setValue(pdfobjects::IProperty *pdfObject) {
 if (effectiveReadonly) return;//Honor readonly setting
 CName* obj=dynamic_cast<CName*>(pdfObject);
 assert(obj);
 string val=ed->text();
 obj->setValue(val);
 changed=false;
}

/** read internal value from given PDF object
 @param pdfObject Object to read from
 */
void NameProperty::readValue(pdfobjects::IProperty *pdfObject) {
 CName* obj=dynamic_cast<CName*>(pdfObject);
 assert(obj);
 string val;
 obj->getValue(val);
 ed->setText(val);
 changed=false;
}

/** default destructor */
NameProperty::~NameProperty() {
}

} // namespace gui
