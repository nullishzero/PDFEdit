/** @file
 QSObject -> ancestor of all QObject wrappers used in scripting
 Provide garbage collection, etc ...
*/

#include "qsobject.h"

//todo: add garbage collection

/** Construct wrapper with given CObject */
QSObject::QSObject() {
//todo: add to some sort of garbage collector if necessary
// addGC(this);
}

/** destructor */
QSObject::~QSObject() {
//todo: remove from  garbage collector if necessary. After returning from scripting, GC content is emptied ...
// removeGC(this);
}

