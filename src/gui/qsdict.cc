/** @file
 QObject wrapper around CDict
*/

#include "qsdict.h"

//TODO: this is for CObjectComplex, not only CDict ...

/** Construct wrapper with given CDict */
QSDict::QSDict(CDict *_dict) : QSCObject ("Dict") {
 obj=_dict;
}

/** destructor */
QSDict::~QSDict() {
}

/** get CDict held inside this class. Not exposed to scripting */
CDict* QSDict::get() {
 return obj;
}

/** call CDict::getStringRepresentation(ret); return ret */
QString QSDict::getString() {
 std::string text;
 obj->getStringRepresentation(text);
 return text;
}

//todo: incomplete
