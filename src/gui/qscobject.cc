/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
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
