/** @file
 TreeItemAbstract - common ancestor of TreeItems,
 with functions to support automatic loading and reloading of tree,
 or its subtrees
 @author Martin Petricek
*/

#include "treeitemabstract.h"
#include <assert.h>
#include "util.h"
#include <utils/debug.h>
#include <qlistview.h>
#include "util.h"

namespace gui {


using namespace std;

/**
 constructor of TreeItemAbstract - create root item
 @param itemName Name of this item
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListView in which to put item
 @param after Item after which this one will be inserted
 */
TreeItemAbstract::TreeItemAbstract(const QString &itemName,TreeData *_data,QListView *parent,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 nameId=itemName;
 data=_data;
 parsed=false;
 assert(data);
}

/**
 constructor of TreeItemAbstract - create child item
 @param itemName Name of this item
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem which is parent of this object
 @param after Item after which this one will be inserted
 */
TreeItemAbstract::TreeItemAbstract(const QString &itemName,TreeData *_data,QListViewItem *parent,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 nameId=itemName;
 data=_data;
 parsed=false;
 assert(data);
}

/** 
 Create and return object for scripting representing this tree item, relocated in different base
 If the object cannot be copied into new base, return NULL
 @param _base Base for the new object.
 @return object for scripting
*/
QSCObject* TreeItemAbstract::getQSObject(__attribute__((unused)) Base *_base) {
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
 QListViewItem::setOpen(open);
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
 @param oldChild reference to old child tree item
 @return true if successful, false otherwise
*/
bool TreeItemAbstract::deepReload(__attribute__((unused)) const QString &childName,__attribute__((unused)) QListViewItem *oldChild) {
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
 QDict<QListViewItem> newItems;
 QMap<QString,ChildType> newTypes;

 QStringList childs=getChildNames();
 QListViewItem *before=NULL;
 for (QStringList::Iterator it=childs.begin();it!=childs.end();++it) {
  QListViewItem *x=items.take(*it);	//Return and remove item from list of current
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
    assert(0);
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
 QDictIterator<QListViewItem> it(items);
 for(;it.current();++it) deleteChild(it.current());
 //Clear lists
 items.clear();
 types.clear();
}

/** 
 Validate child, given its name and reference to old child
 Usually it is valid, since item with same name refer to same subitem (key-based items),
 but for value-based items (array) same keys can correspond to different values after reloading.
 In such cases, false should be returned.
 Also, similar problems can happen when switching revisions (with almost all types)
 <br><br>

 Note: false negative it not a problem (only unnecessary reloading),
       while false positive mean the tree is invalid/outdated

 @param name name of (old and new) treeitem
 @param oldChild reference to old child tree item
 @return True, if the old child is pointing to same item as item "name", false, if the child item "name" no longer point to same item as oldChild.
*/
bool TreeItemAbstract::validChild(__attribute__((unused)) const QString &name,__attribute__((unused)) QListViewItem *oldChild) {
 //True by default
 return true;
}

/**
 Look for child with given name and return it.
 Return NULL if child not found
 @param name Name of child to look for
 @return Specified child of this tree item
*/
QListViewItem* TreeItemAbstract::child(const QString &name) {
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
 @param tree QLIstView in which this item resides
*/
void TreeItemAbstract::unSelect(QListView *tree) {
 if (!tree->isSelected(this)) return;
 //It is selected
 QListViewItem* it=nextSibling();
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

/** Move all child items from other item to this item.
 If current item does have any childs, they will be deleted
 @param src TreeItemAbstract containing childs I want to move to this item
*/
void TreeItemAbstract::moveAllChildsFrom(TreeItemAbstract* src) {
 //Delete all local items
 eraseItems();
 QListViewItem *otherChild;
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
 QListViewItem *target=items.take(name);//Remove from list and return 
 if (!target) { //Item not found (should not happen usually)
  guiPrintDbg(debug::DBG_WARN,"Child to delete not found! -> " << name);
  return;
 }
 types.remove(name); //Item found - so we remove its type from dictionary too
 deleteChild(target);
}

/**
 Delete subitem, given by its pointer
 @param target Pointer to subitem to delete
 */
void TreeItemAbstract::deleteChild(QListViewItem *target) {
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
 //Empty desctructor
}

} // namespace gui
