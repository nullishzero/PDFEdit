/** @file
Property - abstract class for widget containing one editable property
Should be subclassed for editing specific property types (integer, string, ...)
*/

#include "property.h"

//TODO: add functions for display & editing ... 

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param name name of this property
 @param flags flags of this property items (default 0)
 */
Property::Property(const QString &_name/*=0*/,QWidget *parent/*=0*/, PropertyFlags _flags/*=0*/)
 : QWidget (parent, "property",_flags){
 name=_name;
 flags=_flags;
}

/** return name of this property */
QString Property::getName() {
 return name;
}

/** return flags of this property */
PropertyFlags Property::getFlags() {
 return flags;
}

/** set flags of this property */
void Property::setFlags(PropertyFlags flag) {
 flags=flag;
}

/** returns readonly flag of this property */
bool Property::getReadOnly() {
 return readonly;
}

/** set readonly flag of this property */
void Property::setReadOnly(bool _readonly) {
 readonly=_readonly;
}

/** default destructor */
Property::~Property() {
// delete name;
}
