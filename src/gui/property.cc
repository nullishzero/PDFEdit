/** @file
 Property - abstract class for widget containing one editable property<br>
 Should be subclassed for editing specific property types (integer, string, ...)
 @author Martin Petricek
*/

#include <utils/debug.h>
#include "property.h"
#include <iproperty.h>
#include <qstring.h>

namespace gui {

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
Property::Property(const QString &_name/*=0*/,QWidget *parent/*=0*/, PropertyFlags _flags/*=0*/)
 : QWidget (parent, "property",_flags){
 name=_name;
 flags=_flags;
 changed=false;
 //Default flags
 hidden=readonly=false; 
}

/** return name of this property
 @return name
 */
QString Property::getName() {
 return name;
}

/** return flags of this property
 @return flags
 */
PropertyFlags Property::getFlags() {
 return flags;
}

/** set flags of this property
 @param flag New value for property flags
 */
void Property::setFlags(PropertyFlags flag) {
 flags=flag;
}

/** returns readonly flag of this property
 @return readonly flag
 */
bool Property::getReadOnly() {
 return readonly;
}

/** set readonly flag of this property
 @param _readonly New value for readonly flag
 */
void Property::setReadOnly(bool _readonly) {
 readonly=_readonly;
}

/** default destructor */
Property::~Property() {

}

/** Emit signal indicating change of the property and set state
 of property to 'changed'*/
void Property::emitChanged() {
 emit propertyChanged(this);
 printDbg(debug::DBG_DBG,"Property was edited: " << name);
 changed=true;
}

} // namespace gui
