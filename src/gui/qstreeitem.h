#ifndef __QSTREEITEM_H__
#define __QSTREEITEM_H__

#include <qobject.h>
#include "qscobject.h"

namespace gui {

class Base;

class TreeItemAbstract;

/*=
 This type of object represents one item in treeview.
 One item in tree usually corespond to one object in PDF document
*/
class QSTreeItem : public QSCObject {
 Q_OBJECT
public slots:
 /*- Get object contained in this tree item */
 QSCObject* item();
 /*- Get type of object contained in this tree item (shortcut for .item().type()) */
 QString itemtype();
 /*- Return child of this tree item with given name, or NULL if this item have no child with such name */
 QSTreeItem* child(const QString &name);
 /*-
  Get names of all visible childs items of this treeitem 
  Childs that are not shown in tree (because of treeview settings, etc ...) are not returned.
  Childs that are "not yet known" (unopened references) are not returned too.
 */
 QStringList getChildNames();
 /*- Return parent of this tree item, or NULL if this item have no parent */
 QSTreeItem* parent();
 /*- Return name of this tree item */
 QString id();
 /*- Return caption of this tree item (sometimes can differ from name) */
 QString text();
 /*- Return path of this tree item (sequence of names from root item to this item, separated by slashes: "/" ) */
 QString path();
 /*- Explicitly reload contents of this item and its subtree from current state of PDF document */
 void reload();
 /*- Remove object in this tree item (and also any possible subitems) from document */
 void remove();
public:
 QSTreeItem(TreeItemAbstract *item,Base *_base);
 virtual ~QSTreeItem();
 TreeItemAbstract* get();
private:
 /** Object held in class*/
 TreeItemAbstract *obj;
};

} // namespace gui

#endif
