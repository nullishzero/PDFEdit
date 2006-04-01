/** @file
 QObject wrapper around CDict
*/

#include "qsiproperty.h"

/** Construct wrapper with given IProperty */
QSIProperty::QSIProperty(IProperty *_ip) : QSCObject () {
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
