/** @file
 TreeItemAbstract - common ancestor of TreeItems,
 with functions to support automatic reloading of tree or its parts
*/

#include "treeitemabstract.h"
#include <utils/debug.h>

namespace gui {

using namespace std;

/** constructor of TreeItemAbstract - create root item
 @param parent QListView in which to put item
 @param after Item after which this one will be inserted
 */
TreeItemAbstract::TreeItemAbstract(QListView *parent,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
}

/** constructor of TreeItemAbstract - create child item
 @param parent QListViewItem which is parent of this object
 @param after Item after which this one will be inserted
 */
TreeItemAbstract::TreeItemAbstract(QListViewItem *parent,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
}

/** Reload contents of this item's  subtree by calling
 \see reloadSelf, \see createChild, \see getChildNames and \see deleteChild
 */
void TreeItemAbstract::reload(bool reloadThis/*=true*/) {
 if (reloadThis) reloadSelf();
 QDict<QListViewItem> newItems;
 QStringList childs=getChildNames();
 QListViewItem *before=NULL;
 for (QStringList::Iterator it=childs.begin();it!=childs.end();++it) {
  QListViewItem *x=items.take(*it);
  if (x) { //already there -> move
   x->moveItem(before);
   before=x;
   TreeItemAbstract *xa=dynamic_cast<TreeItemAbstract*>(x);
   if (xa) {
    xa->reload();
   }
  } else { //not there -> add
   x=createChild(*it,before);
   if (!x) { //child item is turned off/does not exist
    continue;//TODFO: this is just ugly hack .... 
   }
   before=x;
  }
  newItems.replace(*it,x);
 }

 //Erase unused childs
 eraseItems();

 //Erase old list
 items=newItems;
}

/** Erase all items in current item dictionary */
void TreeItemAbstract::eraseItems() {
 QDictIterator<QListViewItem> it(items);
 for(;it.current();++it) deleteChild(it.current());
 items.clear();
}

/** Move all child items from other item to this item. If current item have any childs, they'll get deleted */
void TreeItemAbstract::moveAllChildsFrom(TreeItemAbstract* src) {
 eraseItems();
 QListViewItem *otherChild;
 while ((otherChild=src->firstChild())) {
  printDbg(debug::DBG_DBG,"Relocating child");
  src->takeItem(otherChild);
  insertItem(otherChild);
 }
 items=src->items;
 src->items.clear();
}

/** Delete subitem, given by its name
 @param name Name of subitem to delete
 */
void TreeItemAbstract::deleteChild(const QString &name) {
 QListViewItem *target=items.take(name);//Remove from list and return 
 if (!target) { 
  printDbg(debug::DBG_WARN,"Child to delete not found! -> " << name);
  return;//Item not found
 }
 deleteChild(target);
}

/** Delete subitem, given by its pointer
 @param target Pointer to subitem to delete
 */
void TreeItemAbstract::deleteChild(QListViewItem *target) {
 delete target;
}

/** default destructor */
TreeItemAbstract::~TreeItemAbstract() {
}

} // namespace gui
