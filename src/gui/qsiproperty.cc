/** @file
 QObject wrapper around IProperty (one property in dictionary or array)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsiproperty.h"
#include <qstring.h>

namespace gui {

/** Construct wrapper with given IProperty */
QSIProperty::QSIProperty(boost::shared_ptr<IProperty> _ip) : QSCObject ("IProperty") {
 obj=_ip;
}

/** Construct wrapper with given IProperty and given type name*/
QSIProperty::QSIProperty(boost::shared_ptr<IProperty> _ip, QString _className) : QSCObject (_className) {
 obj=_ip;
}

/** Call IProperty::getStringRepresentation(ret); return ret */
QString QSIProperty::getText() {
 std::string text;
 obj->getStringRepresentation(text);
 return text;
}

/** destructor */
QSIProperty::~QSIProperty() {
}

/** get IProperty held inside this class. Not exposed to scripting */
boost::shared_ptr<IProperty> QSIProperty::get() {
 return obj;
}

} // namespace gui

//todo: incomplete
