/** @file
 QObject wrapper around CDict
*/

#include "qsdict.h"

/** Construct wrapper with given CDict */
QSDict::QSDict(CDict *_dict) : QSObject<CDict> (_dict) {
}

/** destructor */
QSDict::~QSDict() {
}

//todo: incomplete
