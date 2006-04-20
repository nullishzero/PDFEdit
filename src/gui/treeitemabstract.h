#ifndef __TREEITEMABSTRACT_H__
#define __TREEITEMABSTRACT_H__

#include <qlistview.h>
#include <qdict.h>

class QString;
class QStringList;

namespace gui {

class TreeItemAbstract : public QListViewItem {
public:
 TreeItemAbstract(QListView *parent,QListViewItem *after=NULL);
 TreeItemAbstract(QListViewItem *parent,QListViewItem *after=NULL);
 virtual ~TreeItemAbstract();
 void reload(bool reloadThis=true);
 virtual void deleteChild(const QString &name);
 virtual void deleteChild(QListViewItem *target);
 void eraseItems();
 void moveAllChildsFrom(TreeItemAbstract* src);
 //Abstract functions
 /** Create one subchild
  @param name Name of child (the same as in list returned from \see getChildNames) 
  @param after LIstView itrem after which will be this item added
  @return Pointer to subchild. If returned NULL, the subchild is not available after all and is deleted
 */
 virtual TreeItemAbstract* createChild(const QString &name,QListViewItem *after=NULL)=0;
 /** Get list of all item's childs names
  @return list of child names. Names in the list must be unique and the list may be empty.
 */
 virtual QStringList getChildNames()=0;
 /** Reload contents of itself and only itself (excluding any children) */
 virtual void reloadSelf()=0;
protected:
 /** Mapping of all child names to child items for this tree item */
 QDict<QListViewItem> items;
};

} // namespace gui

#endif
