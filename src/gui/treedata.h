#ifndef __TREEDATA_H__
#define __TREEDATA_H__

#include <qdict.h>
class QListView;
class QString;

namespace gui {

class TreeItem;
class TreeItemRef;
class TreeWindow;

class TreeData {
//ADDED functions begin
//ADDED functions end
public:
 TreeData(TreeWindow *parent,QListView *tree);
 ~TreeData();
 TreeWindow* parent();
 QListView* tree();
 void add(TreeItemRef *it);
 void remove(TreeItemRef *it);
 void remove(const QString &ref);
 TreeItemRef* find(const QString &ref);
 void clear();
 //ShowData
 void checkSetting(bool &target,const QString &key);
 void update();
 bool showSimple();
 bool isDirty();
 void resetDirty();
 bool needReload();
 void resetReload();
private:
 /** Tree window holding these data */
 TreeWindow *_parent;
 /** List view from inside the tree window */
 QListView *_tree;
 /** Dictionary holding tree items that are references */
 QDict<TreeItemRef> refs;
 //ShowData
 /** Show simple objects (int,bool,string,name,real) in object tree? */
 bool show_simple;
 /** True, if any change since last time this was reset to false. Initial value is true */
 bool dirty;
 /** True if the tree needs reloading */
 bool needreload;
};

} // namespace gui 

#endif
