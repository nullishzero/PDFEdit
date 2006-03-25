/** @file
 QSCObject -> ancestor of all QObject wrappers used in scripting
 Provide garbage collection, etc ...
*/

#include "qscobject.h"

//todo: add garbage collection

/** Construct wrapper with given CObject */
QSCObject::QSCObject() {
//todo: add to some sort of garbage collector if necessary
// addGC(this);
}

/** destructor */
QSCObject::~QSCObject() {
//todo: remove from  garbage collector if necessary. After returning from scripting, GC content is emptied ...
// removeGC(this);
}

