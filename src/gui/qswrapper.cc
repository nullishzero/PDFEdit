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
