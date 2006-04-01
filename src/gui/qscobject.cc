/** @file
 QSCObject -> ancestor of all QObject wrappers used in scripting
 Provide garbage collection, etc ...
*/

#include <utils/debug.h>
#include "qscobject.h"

//todo: add garbage collection if needed

/** Construct wrapper with given CObject */
QSCObject::QSCObject() {
 printDbg(debug::DBG_DBG,"adding QSCObject");
//todo: add to garbage collector. Garbage collector is one for each editor window.
// addGC(this);
}

/** destructor */
QSCObject::~QSCObject() {
 printDbg(debug::DBG_DBG,"removing QSCObject");
//todo: remove from  garbage collector. After returning from scripting, GC should be run ...
// removeGC(this);
}

/** Deletes itself */
void QSCObject::deleteSelf() {
 delete this;
}
