/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 QSCObject -> base class of all QObject wrappers used in scripting<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions<br>
 @author Martin Petricek
*/

#include "qscobject.h"
#include "qtcompat.h"
#include <utils/debug.h>
#include <qstring.h>
#include "util.h"
#include "nullpointerexception.h"

namespace gui {

/**
 Construct wrapper with given CObject
 @param _typeName Name of this objects type
 @param _base scripting base for this object
 */
QSCObject::QSCObject(QString _typeName, BaseCore* _base) {
 typeName=_typeName;
 base=_base;
// guiPrintDbg(debug::DBG_DBG,"adding QSCObject "<< typeName);
 if (base) base->addGC(this);
}

/**
 Copy constructor
 @param source source object
*/
QSCObject::QSCObject(QSCObject &source) : QObject() {
 typeName=source.typeName;
 base=source.base;
// guiPrintDbg(debug::DBG_DBG,"adding QSCObject (copy) " << typeName);
 if (base) base->addGC(this);
}

/** destructor */
QSCObject::~QSCObject() {
 guiPrintDbg(debug::DBG_DBG,"removing QSCObject " << Q_OUT(typeName));
//TODO: implement QSWrapperFacrory for scripting as "garbage collector"
 if (base) base->removeGC(this);
}

/**
 Check if given pointer is NULL.<br>
 If it is, call base error handler for null pointers and return true, otherwise just return false;<br>
 @param ptr Pointer to check if it is not NULL
 @param methodName Name of method, to send to error handler if ptr is NULL
*/
bool QSCObject::nullPtr(const void* ptr,const QString &methodName) {
 if (ptr) return false;
 if (!base) throw NullPointerException(typeName,methodName);
 base->errorNullPointer(typeName,methodName);
 return true;
}

/**
 Return name of this object's type
 @return name of this object's type
*/
QString QSCObject::type() {
 return typeName;
}

} // namespace gui
