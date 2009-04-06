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
 TreeItemAbstract - common ancestor of TreeItems,
 with functions to support automatic loading and reloading of tree,
 or its subtrees
 @author Martin Petricek
*/

#include "treeitemabstract.h"
#include "multitreewindow.h"
#include "treedata.h"
#include "treewindow.h"
#include "util.h"
#include <assert.h>
#include <utils/debug.h>
#include <qstring.h>

namespace gui {

using namespace std;

/**
 constructor of TreeItemAbstract - create root item
 @param itemName Name of this item
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListView in which to put item
 @param after Item after which this one will be inserted
 */
TreeItemAbstract::TreeItemAbstract(const QString &itemName,TreeData *_data,Q_ListView *parent,Q_ListViewItem *after/*=NULL*/):Q_ListViewItem(parent,after) {
 nameId=itemName;
 data=_data;
 initAbs();
}


/**
 constructor of TreeItemAbstract - create child item
 @param itemName Name of this item
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListViewItem which is parent of this object
 @param after Item after which this one will be inserted
 */
TreeItemAbstract::TreeItemAbstract(const QString &itemName,TreeData *_data,Q_ListViewItem *parent,Q_ListViewItem *after/*=NULL*/):Q_ListViewItem(parent,after) {
 nameId=itemName;
 data=_data;
 initAbs();
}

/**
 Initialize the class - set root window, check data ...
*/
void TreeItemAbstract::initAbs() {
 parsed=false;
 assert(data);
 Q_ListView *lv=listView();
 assert(lv);
 assert(lv->parentWidget());
 rootWindow=dynamic_cast<TreeWindow*>(lv->parentWidget());
 assert(rootWindow);
}

/**
 Create and return object for scripting representing this tree item, relocated in different base
 If the object cannot be copied into new base, return NULL
 @param _base BaseCore for the new object.
 @return object for scripting
*/
QSCObject* TreeItemAbstract::getQSObject(__attribute__((unused)) BaseCore *_base) {
 return NULL;
}

/**
 Slot that will be called when item is opened/closed
 @param open True if item is being opened, false if closed
*/
void TreeItemAbstract::setOpen(bool open) {
 if (open && !parsed) { //Trying to open unparsed item
  parsed=true;
  reload(false);//And now get the subitems!
 }
 Q_ListViewItem::setOpen(open);
}

/**
 Attempt to "deep reload", i.e. exchange the item inside the tree for an actual (different) one,
 without destroying the object
 (destroying have unfortunate effect of losing information about which child were opened ...)
 oldChild should be modified to refrlect the new object
 If deep reload is supported and was sucessful, return true, otherwise false.
 <br><br>

  Note: oldItem should not check or reload its children in this method
        (at it might lead to multiple reloading of same part of tree)
        Also, it is guaranteed that reloadSelf will be called on that item,
        so explicitly reloading the item might be unnecessary too

 @param childName name of (old and new) treeitem
 @param oldItem reference to old child tree item
 @return true if successful, false otherwise
*/
bool TreeItemAbstract::deepReload(__attribute__((unused)) const QString &childName,__attribute__((unused)) Q_ListViewItem *oldItem) {
 //By default this feature is not supported. Reimplement this method to support it.
 return false;
}

/** Reload contents of this item's subtree recursively by calling
 reloadSelf, createChild, getChildNames and deleteChild
 \see reloadSelf
 \see createChild
 \see getChildNames
 \see deleteChild
 @param reloadThis reload also itself, or only child? Default reload also itself
 @param forceReload force reload? Just delete all childs and re-create them, effectively bypassing any optimizations
 */
void TreeItemAbstract::reload(bool reloadThis/*=true*/,bool forceReload/*=false*/) {
 if (reloadThis) reloadSelf();
 if (!parsed) { //Not yet parsed, just check for presence of any childs
  setExpandable(haveChild());
  return;
 }
 Q_Dict<Q_ListViewItem> newItems;
 QMap<QString,ChildType> newTypes;

 QStringList childs=getChildNames();
 Q_ListViewItem *before=NULL;
 for (QStringList::Iterator it=childs.begin();it!=childs.end();++it) {
  Q_ListViewItem *x=items.take(*it);	//Return and remove item from list of current
  ChildType typ=getChildType(*it);
  if (forceReload) { //Just create that child again
   deleteChild(x);	//Delete the old item
   x=NULL;		//The item will be treated as nonexistent and created
  }
  if (x) { // Check it type is the same for existing items, check if the item is the same
   assert(types.contains(*it));
   ChildType typOld=types[*it];	//Return type of old item from list of current (will be removed when items are cleared
   if (typ!=typOld) { //The type have changed
    deleteChild(x);	//Delete the old item
    x=NULL;		//The item will be treated as nonexistent and created
   } else if (!validChild(*it,x)) {//Child item is no longer the same item
    if (!deepReload(*it,x)) { //Try to reload the child "in place"
     //This have unfortunate effect of closing the children. But should not happen often
     deleteChild(x);	//Delete the old item
     x=NULL;		//The item will be treated as nonexistent and created
    }
   }
  }
  if (x) { //Item is already there -> move it to right place
   x->moveItem(before);
   TreeItemAbstract *xa=dynamic_cast<TreeItemAbstract*>(x);
   if (xa) { //It is TreeItemAbstract -> reload it (recursively)
    xa->reload();
   }
  } else { //not there -> add
   x=createChild(*it,typ,before);
   if (!x) { //child item does not exist after all -> indication of major bug somewhere
    //This can happen for example if someone does really horrible to edited PDF like deleting "kids" in page dictionary
    //That way, page count reports 1, but no pages in fact exist. We try to handle this simply omitting that tree item
    //It is rare, but it can happen.
    //assert(0);
    continue;
   }
  }
  before=x; //Place new items after this one
  newItems.replace(*it,x);
  newTypes.replace(*it,typ);
 }

 //Erase unused childs
 eraseItems();

 //Erase old list, replace by new list
 items=newItems;
 types=newTypes;
}

/** Erase all items in current item dictionary. After returning, the item dictionary is empty */
void TreeItemAbstract::eraseItems() {
 //Delete each item in "items"
 Q_DictIterator<Q_ListViewItem> it(items);
 for(;it.current();++it) deleteChild(it.current());
 //Clear lists
 items.clear();
 types.clear();
}

/**
 Look for child with given name and return it.
 Return NULL if child not found
 @param name Name of child to look for
 @return Specified child of this tree item
*/
Q_ListViewItem* TreeItemAbstract::child(const QString &name) {
 if (!parsed) {
  //Tree not expanded here.
  //We need to expand it, so we can return the items
  parsed=true;
  reload(false);
  //Now all direct subitems are loaded
 }
 return items[name];//QDict will return NULL if item is not there
}
/**
 Move selection away from this item to nearest sensible item<br>
 If this item is not selected, task is done, does nothing<br>
 If it is selected, firt attempt is to move selection on next sibling, then on item above, then on item below.
 @param tree Q_ListView in which this item resides
*/
void TreeItemAbstract::unSelect(Q_ListView *tree) {
 if (!tree->isSelected(this)) return;
 //It is selected
 Q_ListViewItem* it=nextSibling();
 if (it) { //Next in same level
  tree->setSelected(it,true);
  return;
 }
 it=itemAbove();
 if (it) { //Previous in same level or parent
  tree->setSelected(it,true);
  return;
 }
 //Next in other level
 it=itemBelow();
 if (it) { //Previous in same level or parent
  tree->setSelected(it,true);
  return;
 }
 guiPrintDbg(debug::DBG_INFO,"Removed last item from tree");
 //This was last item. Can't select anything else
}

/**
 Move all child items from other item to this item.
 If current item does have any childs, they will be deleted
 @param src TreeItemAbstract containing childs I want to move to this item
*/
void TreeItemAbstract::moveAllChildsFrom(TreeItemAbstract* src) {
 //Delete all local items
 eraseItems();
 Q_ListViewItem *otherChild;
 while ((otherChild=src->firstChild())) {	 //For each child
  guiPrintDbg(debug::DBG_DBG,"Relocating child");
  src->takeItem(otherChild);
  insertItem(otherChild);
 }
 //Copy dictionaries to this items
 items=src->items;
 types=src->types;
 //Clear dictionaries on source item
 src->items.clear();
 src->types.clear();
}

/**
 Delete subitem, given by its name
 @param name Name of subitem to delete
 */
void TreeItemAbstract::deleteChild(const QString &name) {
 Q_ListViewItem *target=items.take(name);//Remove from list and return
 if (!target) { //Item not found (should not happen usually)
  guiPrintDbg(debug::DBG_WARN,"Child to delete not found! -> " << Q_OUT(name));
  return;
 }
 types.remove(name); //Item found - so we remove its type from dictionary too
 deleteChild(target);
}

/**
 Delete subitem, given by its pointer
 @param target Pointer to subitem to delete
 */
void TreeItemAbstract::deleteChild(Q_ListViewItem *target) {
 delete target;
}

/**
 Return name (id) of this item. Name is unique only for each item's children.
 Children of different tree items can (and often will) have same name
 @return name of itself
*/
QString TreeItemAbstract::name() {
 return nameId;
}

/**
 Set name (id) of this item. Name is unique only for each item's children.
 Children of different tree items can (and often will) have same name
 Can be used if changing array elements (moving, etc ...)
 @param newNameId New name for this element
*/
void TreeItemAbstract::setName(const QString &newNameId) {
 nameId=newNameId;
}

/** default destructor */
TreeItemAbstract::~TreeItemAbstract() {
 if (rootWindow) {
  //Notify root window - if that item is selected, unselect it
  rootWindow->deleteNotify(this);
  //unselect self
  data->tree()->setSelected(this,false);
 }
 //Notify MultiTreeWindow about deletion of itself
// guiPrintDbg(debug::DBG_DBG,"Item deleting" << intptr_t(this));
 data->multi()->notifyDelete(this);
}

/**
 Set this item as selected/unselected and repaint the tree
 @param selected true to select, false to unselect
*/
void TreeItemAbstract::setSelect(bool selected) {
 Q_ListView *lv=listView();
 assert(lv);
 lv->setSelected(this,selected);
}

/**
 Return path of this item
 (sequence of names from root to this item, separated by slash "/").
 @return path of this item
*/
QString TreeItemAbstract::path() {
 QString path=name();
 TreeItemAbstract* theParent=dynamic_cast<TreeItemAbstract*>(parent());
 while (theParent) { //Traverse to root, prepending path elements
  path=theParent->name()+"/"+path;
  theParent=dynamic_cast<TreeItemAbstract*>(theParent->parent());
 }
 return path;
}

/**
 Return hint (description) of this item
 @return hint for given item
*/
QString TreeItemAbstract::itemHint() {
 return path();
}

} // namespace gui
