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
QSTreeItem::QSTreeItem(TreeItemAbstract *item) : QSCObject ("TreeItem") {
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

/**
 Return list of all possibly visible childs names of this treeitem.
 Childs that are not shown in treeview (because of treeview settings, etc ...) are not returned.
 Childs that are "not yet known" (unopened references) are not returned too.
 @return list of childs
*/
QStringList QSTreeItem::getChildNames() {
 return obj->getChildNames();
}

/** destructor */
QSTreeItem::~QSTreeItem() {
}

/** get CGraphics held inside this class. Not exposed to scripting */
TreeItemAbstract* QSTreeItem::get() {
 return obj;
}

} // namespace gui
