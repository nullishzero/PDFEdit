#ifndef __QSTREEITEM_H__
#define __QSTREEITEM_H__

#include <qobject.h>
#include "qscobject.h"

namespace gui {

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
 /*-
  Get names of all visible childs items of this treeitem 
  Childs that are not shown in tree (because of treeview settings, etc ...) are not returned.
  Childs that are "not yet known" (unopened references) are not returned too.
 */
 QStringList getChildNames();
public:
 QSTreeItem(TreeItemAbstract *item);
 virtual ~QSTreeItem();
 TreeItemAbstract* get();
private:
 /** Object held in class*/
 TreeItemAbstract *obj;
};

} // namespace gui

#endif
