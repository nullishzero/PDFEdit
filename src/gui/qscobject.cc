/** @file
 QSCObject -> base class of all QObject wrappers used in scripting<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions<br>
 @author Martin Petricek
*/

#include <utils/debug.h>
#include "qscobject.h"
#include <qstring.h>
#include "util.h"

namespace gui {

//todo: add garbage collection if needed

/** Construct wrapper with given CObject */
QSCObject::QSCObject(QString _typeName) {
 typeName=_typeName;
 guiPrintDbg(debug::DBG_DBG,"adding QSCObject "<< typeName);
//todo: add to garbage collector. Garbage collector is one for each editor window.
// addGC(this);
}

/** Copy constructor */
QSCObject::QSCObject(QSCObject &source) {
 typeName=source.typeName;
 guiPrintDbg(debug::DBG_DBG,"adding QSCObject (copy) "<< typeName);
// addGC(this);
}

/** destructor */
QSCObject::~QSCObject() {
 guiPrintDbg(debug::DBG_DBG,"removing QSCObject" << typeName);
//todo: remove from  garbage collector. After returning from scripting, GC should be run ...
// removeGC(this);
}

void QSCObject::disconnectNotify(const char* signal) {
 guiPrintDbg(debug::DBG_DBG,"Disconnect notify" <<typeName << " - " << signal);
}

void QSCObject::connectNotify(const char* signal) {
 guiPrintDbg(debug::DBG_DBG,"connect notify" <<typeName << " - " << signal);
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
