#ifndef __TREEDATA_H__
#define __TREEDATA_H__

#include <qdict.h>
class QListView;
class QString;

namespace gui {

class TreeItem;
class TreeWindow;

class TreeData {
public:
 TreeData(TreeWindow *parent,QListView *tree);
 ~TreeData();
 TreeWindow* parent();
 QListView* tree();
 void add(TreeItem *it);
 void remove(TreeItem *it);
 TreeItem* find(const QString &ref);
 void clear();
private:
 /** Tree window holding these data */
 TreeWindow *_parent;
 /** List view from inside the tree window */
 QListView *_tree;
 /** Dictionary holding tree items that are references */
 QDict<TreeItem> refs;
};

} // namespace gui 

#endif
