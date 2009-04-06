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
 DragListView class, ancestor of QListView.
 Enhances QListView by drag and drop capability
 @author Martin Petricek
*/

#include "draglistview.h"
#include <qdragobject.h>
#include "treeitemabstract.h"
#include <utils/debug.h>
#include <assert.h>
#include "util.h"

namespace gui {

/** Pseudo-mimetype for tree item */
const char *TREEITEM_TYPE="pdfedit/treeitem";

/** Dragobject constructor
 @param _item Item to store in the object
 @param dragSource Widget in which the drag originated
 @param name name of the object
 */
TreeItemDragObject::TreeItemDragObject(TreeItemAbstract *_item,QWidget *dragSource/*=0*/,const char *name/*=0*/) : QDragObject(dragSource,name) {
 guiPrintDbg(debug::DBG_DBG,"");
 item=_item;
}

/**
 Return supported mime types (for this class only one is supported, and it is equal to vlaue of TREEITEM_TYPE constant).
 @param i Number of mimetype to query
 @return Mimetype name
*/
const char* TreeItemDragObject::format (int i/*=0*/) const {
 if (i==0) return TREEITEM_TYPE;
 return NULL;
}

/**
 Return encoded data (which is pointer to TreeItemAbstract)
 @param m Mimetype. Must be equal to value of TREEITEM_TYPE constant for the item to be returned.
 @return Encoded data
 */
QByteArray TreeItemDragObject::encodedData (const char *m) const {
 if (strcmp(m,TREEITEM_TYPE)!=0) return QByteArray(0);
 //Copy the pointer
 QByteArray q(sizeof(item));
 q.duplicate((char*)(&item),sizeof(item));
 return q;
}

/** Destructor */
TreeItemDragObject::~TreeItemDragObject(){
 guiPrintDbg(debug::DBG_DBG,"");
 //Empty destructor
}

/**
 Same constructor as QListView
 @param parent Parent window
 @param name Widget name
 @param f Widget flags
 */
DragListView::DragListView(QWidget *parent,const char *name/*=0*/,WFlags f/*=0*/) : QListView(parent,name,f) {
 //Empty constructor
 setAcceptDrops(true);
 viewport()->setAcceptDrops(true); //Hmm ... why is THIS needed too?
}

/** Destructor */
DragListView::~DragListView() {
 //Empty Destructor
}

/**
 Called on starting a drag to create dragobject containing selected item
 @return QDragObject containing selected item
 */
QDragObject* DragListView::dragObject() {
 TreeItemAbstract *sel=dynamic_cast<TreeItemAbstract*>(selectedItem());
 if (!sel) return NULL;
 TreeItemDragObject *dr=new TreeItemDragObject(sel,this,"drag_n_drop");
 return dr;
}

/**
 Handler called when something is moved over the window.
 The event should be accepted if there is some change for drop to be accepted
 @param e Event
*/
void DragListView::contentsDragMoveEvent(QDragMoveEvent *e) {
 guiPrintDbg(debug::DBG_DBG,"DRop 1");
 if (!e->source()) {
  //Attempt to drop something from outside. Not supported
  e->ignore();
  return;
 }
 guiPrintDbg(debug::DBG_DBG,"DRop 2");
 TreeItemAbstract *target=dynamic_cast<TreeItemAbstract*>(itemAt(e->pos()));
 if (target) e->accept(); else e->ignore();
}

/**
 Handler called when something is dropped in the window
 If it is tree item from this or another another window
 @param e Event
*/
void DragListView::contentsDropEvent(QDropEvent *e) {
 QWidget *src=e->source();
 if (!src) {
  //Attempt to drop something from outside. Not supported
  e->ignore();
  return;
 }
 QByteArray a=e->encodedData(TREEITEM_TYPE);
 TreeItemAbstract *source=NULL;
 assert(a.size()==sizeof(source));
 memcpy(&source,a.data(),sizeof(source));
 TreeItemAbstract *target=dynamic_cast<TreeItemAbstract*>(itemAt(e->pos()));
 guiPrintDbg(debug::DBG_DBG,"DRAGDROP to item");
 if (source && target) {
  e->accept();
  if (src==this) emit dragDrop(source,target);
  else emit dragDropOther(source,target);
 } else e->ignore();
}

} // namespace gui
