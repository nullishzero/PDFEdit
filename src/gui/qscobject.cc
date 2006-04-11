/** @file
 QSCObject -> ancestor of all QObject wrappers used in scripting
 Provide garbage collection, etc ...
*/

#include <utils/debug.h>
#include "qscobject.h"

namespace gui {

//todo: add garbage collection if needed

/** Construct wrapper with given CObject */
QSCObject::QSCObject(QString _typeName) {
 typeName=_typeName;
 printDbg(debug::DBG_DBG,"adding QSCObject "<< typeName);
//todo: add to garbage collector. Garbage collector is one for each editor window.
// addGC(this);
}

/** destructor */
QSCObject::~QSCObject() {
 printDbg(debug::DBG_DBG,"removing QSCObject" << typeName);
//todo: remove from  garbage collector. After returning from scripting, GC should be run ...
// removeGC(this);
}

/** Return name of this object's type
  @return name of this object's type
*/
QString QSCObject::type() {
 return typeName;
}
/** Deletes itself */
void QSCObject::deleteSelf() {
 delete this;
}

} // namespace gui
