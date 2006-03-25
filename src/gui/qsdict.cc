/** @file
 QObject wrapper around CDict
*/

#include "qsdict.h"

/** Construct wrapper with given CDict */
QSDict::QSDict(CDict *_dict) : QSCObject () {
 obj=_dict;
}

/** destructor */
QSDict::~QSDict() {
}

/** get CDict held inside this class. Not exposed to scripting */
CDict* QSDict::get() {
 return obj;
}

//todo: incomplete
