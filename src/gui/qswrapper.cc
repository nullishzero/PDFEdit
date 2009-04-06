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
 Wrapper factory that will ensure proper object deallocation
 @author Martin Petricek
*/

#include "qswrapper.h"
#include "qtcompat.h"
#include "qstreeitem.h"
#include <utils/debug.h>

namespace gui {

/** Construct instance of wrapper factory */
QSWrapper::QSWrapper() {
 guiPrintDbg(debug::DBG_DBG,"Wrapper construct");
 registerWrapper("gui::QSTreeItem");
}

/** destructor */
QSWrapper::~QSWrapper() {
 guiPrintDbg(debug::DBG_DBG,"Wrapper destruct");
}

/**
 Create wrapper for given object.
 Just return the object and thus keep its deallocation to QSA
 @param className name of class
 @param ptr object pointer
*/
QObject* QSWrapper::create(const QString &className, void *ptr) {
 guiPrintDbg(debug::DBG_DBG,"Wrapper for: " << Q_OUT(className));
 QObject *ret=0;
 if (className=="gui::QSTreeItem") {
  ret=(QSTreeItem*)ptr;
  assert(dynamic_cast<QSTreeItem*>(ret));
 }
 assert(ret);
 return ret;
}

} // namespace gui
