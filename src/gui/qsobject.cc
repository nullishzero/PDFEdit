/** @file
 QSObject -> ancestor of all QObject wrappers used in scripting
 Provide garbage collection, etc ...
*/

#include "qsobject.h"

//todo: add garbage collection

/** Construct wrapper with given CObject */
template<typename T> QSObject<T>::QSObject<T>(T *_obj) {
 obj=_obj;
//todo: add to some sort of garbage collector if necessary
// addGC(this);
}

/** destructor */
template<typename T> QSObject<T>::~QSObject<T>() {
//todo: remove from  garbage collector if necessary. After returning from scripting, GC content is emptied ...
// removeGC(this);
}

/** get CObject held inside this class. Not exposed to scripting */
template<typename T> T *QSObject<T>::get() {
 return obj;
}
