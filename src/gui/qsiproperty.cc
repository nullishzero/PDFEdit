/** @file
 QObject wrapper around IProperty (one property in dictionary or array)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsiproperty.h"
#include <qstring.h>

namespace gui {

/**
 Construct wrapper with given IProperty
 @param _ip IProperty wrapped in this object
 @param _base Scripting base
*/
QSIProperty::QSIProperty(boost::shared_ptr<IProperty> _ip,Base *_base) : QSCObject ("IProperty",_base) {
 obj=_ip;
}

/** Copy constructor */
QSIProperty::QSIProperty(QSIProperty &source) : QSCObject ("IProperty",source.base) {
 obj=source.obj;
}

/**
 Construct wrapper with given IProperty and given type name
 @param _ip IProperty wrapped in this object
 @param _className type name of this object
 @param _base Scripting base
*/
QSIProperty::QSIProperty(boost::shared_ptr<IProperty> _ip, QString _className,Base *_base) : QSCObject (_className,_base) {
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
