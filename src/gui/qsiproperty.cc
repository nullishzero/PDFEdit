/** @file
 QObject wrapper around CDict
*/

#include "qsiproperty.h"

/** Construct wrapper with given IProperty */
QSIProperty::QSIProperty(IProperty *_ip) : QSCObject ("IProperty") {
 obj=_ip;
}

/** Construct wrapper with given IProperty and given type name*/
QSIProperty::QSIProperty(IProperty *_ip, QString _className) : QSCObject (_className) {
 obj=_ip;
}

/** destructor */
QSIProperty::~QSIProperty() {
}

/** get IProperty held inside this class. Not exposed to scripting */
IProperty* QSIProperty::get() {
 return obj;
}

//todo: incomplete
