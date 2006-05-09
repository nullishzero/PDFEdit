/** @file
 QSTreeItem - QObject wrapper around TreeItemAbstract (single tree item in tree view)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qstreeitem.h"
#include "treeitemabstract.h"

namespace gui {

/** Construct wrapper with given CGraphic */
QSTreeItem::QSTreeItem(TreeItemAbstract *item,Base *_base) : QSCObject ("TreeItem",_base) {
 obj=item;
}

/**
 Return item inside this tree item, wrapped by QObject wrapper.
 Caller is responsible for freeing the object
 @return QObject wrapper around data inside treeitem
*/
QSCObject* QSTreeItem::item() {
 return obj->getQSObject();
}

/**
 Return type of item inside this tree item
 @return Type of item
*/
QString QSTreeItem::itemtype() {
 QSCObject* it=obj->getQSObject();
 QString type=it->type();
 delete it;
 return type;
}

/** Explicitly reload contents of this item and its subtree from current state of PDF document */
void QSTreeItem::reload() {
 obj->reload();
}

/** Remove itself from Dict/Array where this property is held (and from document) */
void QSTreeItem::remove() {
 obj->remove();
}

/**
 Return list of all possibly visible childs names of this treeitem.
 Childs that are not shown in treeview (because of treeview settings, etc ...) are not returned.
 Childs that are "not yet known" (unopened references) are not returned too.
 @return list of childs
*/
QStringList QSTreeItem::getChildNames() {
 return obj->getChildNames();
}

/**
 Return parent of this Tree Item, or NULL if this item have not parent,
 or parent is not subclass of TreeItemAbstract
 @return parent of this TreeItem
*/
QSTreeItem* QSTreeItem::parent() {
 TreeItemAbstract* parent=dynamic_cast<TreeItemAbstract*>(obj->parent());
 if (!parent) return NULL;
 return new QSTreeItem(parent,base);
}

/**
 Return child of this Tree Item with given name, or NULL if such child does not exist,
 or is not subclass of TreeItemAbstract
 @param name Name of child to get
 @return child of this TreeItem
*/
QSTreeItem* QSTreeItem::child(const QString &name) {
 TreeItemAbstract* child=dynamic_cast<TreeItemAbstract*>(obj->child(name));
 if (!child) return NULL;
 return new QSTreeItem(child,base);
}

/**
 Return name of this item.
 @return name of this item
*/
QString QSTreeItem::id() {
 return obj->name();
}

/**
 Return caption of this item in tree window. Most times it is identical to name, or at least similar.
 @return caption of this item
*/
QString QSTreeItem::text() {
 return obj->text(0);
}

/**
 Return path of this item (sequence of names from root to this item, separated by slash "/").
 @return path of this item
*/
QString QSTreeItem::path() {
 QString path=obj->name();
 TreeItemAbstract* parent=dynamic_cast<TreeItemAbstract*>(obj->parent());
 while (parent) { //Traverse to root, prepending path elements
  path=parent->name()+"/"+path;
  parent=dynamic_cast<TreeItemAbstract*>(parent->parent());
 }
 return path;
}

/** get TreeItemAbstract held inside this class. Not exposed to scripting */
TreeItemAbstract* QSTreeItem::get() {
 return obj;
}

/** destructor */
QSTreeItem::~QSTreeItem() {
}

} // namespace gui
