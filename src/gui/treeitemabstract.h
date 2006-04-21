#ifndef __TREEITEMABSTRACT_H__
#define __TREEITEMABSTRACT_H__

#include <qlistview.h>
#include <qdict.h>

class QString;
class QStringList;

namespace gui {

/** Type of TreeItemAbstract's child.
 The type is arbitrary integer and must be unique only across single type
 (different types must have different values).
 Two distict TreeItemAbstract descendants can have collisions in types
 (same value meaning different type)
 Types of childs from different items are never mixed or compared together
*/
typedef int ChildType;

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
  @param name Name of child (the same as in list returned from getChildNames)
  @param after QListViewItem after which will be this item added. If unspecified (or  NULL), it is added before any other items
  @return Pointer to subchild. If returned NULL, the subchild can not be created
  \see getChildNames
 */
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL)=0;
 /** Get list of all item's childs names
  @return list of child names. Names in the list must be unique and the list may be empty.
 */
 virtual QStringList getChildNames()=0;
 /** Reload contents of itself and only itself (excluding any children) */
 virtual void reloadSelf()=0;
 /** Get type of this items child.
 @return type of specified child
 */
 virtual ChildType getChildType(const QString &name)=0;
protected:
 /** Mapping of all child names to child items for this tree item */
 QDict<QListViewItem> items;
 /** Mapping of all child names to types of child items for this tree item */
 QMap<QString,ChildType> types;
};

} // namespace gui

#endif
