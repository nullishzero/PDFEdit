/** @file
 QSTreeItem - QObject wrapper around TreeItemAbstract (single tree item in tree view)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qstreeitem.h"
#include "qsiproperty.h"
#include "treeitemabstract.h"
#include "qsimporter.h"

namespace gui {

/**
 Construct wrapper with given TreeItemAbstract<br>
 This constructor is available only to subclasses - they can set their own class name here
 @param className Item type to use.
 @param item Tree item
 @param _base scripting base
*/
QSTreeItem::QSTreeItem(const QString &className,TreeItemAbstract *item,Base *_base) : QSCObject (className,_base) {
 obj=item;
 assert(obj);
}

/**
 Construct wrapper with given TreeItemAbstract
 @param item Tree item
 @param _base scripting base
*/
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
 Return item inside this tree item, as item() does, but with one exception:
 If the item is reference, return reference target instead of the reference
 Caller is responsible for freeing the object
 \see item
 @return QObject wrapper around data inside treeitem
*/
QSCObject* QSTreeItem::itemref() {
 QSCObject* rItem=obj->getQSObject();
 QSIProperty* ip=dynamic_cast<QSIProperty*>(rItem);
 if (!ip) return rItem;//Not IProperty
 QSCObject* refItem=ip->ref();
 if (refItem==rItem) return rItem;//Wasn't a reference
 delete rItem;//Delete original
 return refItem;//Return reference 
}

/**
 Return type of item inside this tree item
 @return Type of item
*/
QString QSTreeItem::itemtype() {
 assert(obj);
 QSCObject* it=obj->getQSObject();

 //Some tree item does not have actually any "item" in them
 if (!it) return QString::null;

 QString type=it->type();
 delete it;
 return type;
}

/** Explicitly reload contents of this item and its subtree from current state of PDF document */
void QSTreeItem::reload() {
 obj->reload();
}

/**
 Explicitly force reload contents of this item and its subtree from current state of PDF document
 Add "force reload" flag, to test for possible tree item reloading-related bugs
 Should only be used for debugging, reload() should normally work well
 */
void QSTreeItem::reload_force() {
 obj->reload(true,true);
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
 return dynamic_cast<QSTreeItem*>(QSImporter::createQSObject(parent,base));
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
 return dynamic_cast<QSTreeItem*>(QSImporter::createQSObject(child,base));
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
TreeItemAbstract* QSTreeItem::get() const {
 return obj;
}

/** destructor */
QSTreeItem::~QSTreeItem() {
}

} // namespace gui
