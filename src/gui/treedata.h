#ifndef __TREEDATA_H__
#define __TREEDATA_H__

class QListView;
class QString;

namespace gui {

class TreeItem;
class TreeWindow;

class TreeData {
public:
 void add(TreeItem *it);
 TreeItem* find(const QString &ref);
 void clear();
 TreeData(TreeWindow *parent,QListView *tree);
 ~TreeData();
 TreeWindow* parent();
 QListView* tree();
private:
 /** Tree window holding these data */
 TreeWindow *_parent;
 /** List view from inside the tree window */
 QListView *_tree;
};

} // namespace gui 

#endif
