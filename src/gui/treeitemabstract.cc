/** @file
 TreeItemAbstract - common ancestor of TreeItems,
 with functions to support automatic loading and reloading of tree,
 or its subtrees
 @author Martin Petricek
*/

#include "treeitemabstract.h"
#include <assert.h>
#include <utils/debug.h>

namespace gui {


using namespace std;

/** constructor of TreeItemAbstract - create root item
 @param parent QListView in which to put item
 @param after Item after which this one will be inserted
 */
TreeItemAbstract::TreeItemAbstract(QListView *parent,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 //Empty constructor
}

/** constructor of TreeItemAbstract - create child item
 @param parent QListViewItem which is parent of this object
 @param after Item after which this one will be inserted
 */
TreeItemAbstract::TreeItemAbstract(QListViewItem *parent,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 //Empty constructor
}

/** Reload contents of this item's subtree recursively by calling
 reloadSelf, createChild, getChildNames and deleteChild
 \see reloadSelf
 \see createChild
 \see getChildNames
 \see deleteChild
 */
void TreeItemAbstract::reload(bool reloadThis/*=true*/) {
 if (reloadThis) reloadSelf();
 QDict<QListViewItem> newItems;
 QMap<QString,ChildType> newTypes;

 QStringList childs=getChildNames();
 QListViewItem *before=NULL;
 for (QStringList::Iterator it=childs.begin();it!=childs.end();++it) {
  QListViewItem *x=items.take(*it);	//Return and remove item from list of current
  ChildType typ=getChildType(*it);
  if (x) { // Check it type is the same for existing items
   assert(types.contains(*it));
   ChildType typOld=types[*it];	//Return type of old item from list of current (will be removed when items are cleared
   if (typ!=typOld) { //The type have changed
    deleteChild(x);	//Delete the old item
    x=NULL;		//The item will be treated as nonexistent and created
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
    assert(0);
    continue;
   }
  }
  before=x; //Place new items after this one
  newItems.replace(*it,x);
  newTypes.replace(*it,typ);
 }

 printDbg(debug::DBG_DBG,"Reload : 5");

 //Erase unused childs
 eraseItems();

 //Erase old list
 items=newItems;
 types=newTypes;
}

/** Erase all items in current item dictionary. After returning, the item dictionary is empty */
void TreeItemAbstract::eraseItems() {
 //Delete each item in "items"
 QDictIterator<QListViewItem> it(items);
 for(;it.current();++it) deleteChild(it.current());
 //Clear lists
 items.clear();
 types.clear();
}

/** Move all child items from other item to this item.
 If current item does have any childs, they will be deleted
 @param src TreeItemAbstract containing childs I want to move to this item
*/
void TreeItemAbstract::moveAllChildsFrom(TreeItemAbstract* src) {
 //Delete all local items
 eraseItems();
 QListViewItem *otherChild;
 while ((otherChild=src->firstChild())) {	 //For each child
  printDbg(debug::DBG_DBG,"Relocating child");
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

/** Delete subitem, given by its name
 @param name Name of subitem to delete
 */
void TreeItemAbstract::deleteChild(const QString &name) {
 QListViewItem *target=items.take(name);//Remove from list and return 
 if (!target) { //Item not found (should not happen usually)
  printDbg(debug::DBG_WARN,"Child to delete not found! -> " << name);
  return;
 }
 types.remove(name); //Item found - so we remove its type from dictionary too
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
 //Empty desctructor
}

} // namespace gui
