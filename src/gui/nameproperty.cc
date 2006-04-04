/** @file
NameProperty - class for widget containing
 one editable property of type "Name"

Basically, CName is CString, but we might want differentiation in case we'll handle names specially in future
*/

#include "nameproperty.h"
#include <string>
#include <stdlib.h>

using namespace std;

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
NameProperty::NameProperty(const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=0*/)
 : StringProperty(_name,parent,_flags) {
 //just use StringProperty
}

/** default destructor */
NameProperty::~NameProperty() {
}
