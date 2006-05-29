#ifndef __TREEDATA_H__
#define __TREEDATA_H__

#include <qdict.h>
class QListView;
class QString;

namespace gui {

class TreeItem;
class TreeItemRef;
class TreeWindow;
class MultiTreeWindow;
class Base;

class TreeData {
public:
 TreeData(TreeWindow *parent,QListView *tree,Base *base,MultiTreeWindow *multi);
 TreeData(TreeWindow *parent,QListView *tree,Base *base);
 ~TreeData();
 TreeWindow* parent();
 QListView* tree();
 MultiTreeWindow* multi();
 Base* base();
 void add(TreeItemRef *it);
 void remove(TreeItemRef *it);
 void remove(const QString &ref);
 TreeItemRef* find(const QString &ref);
 void clear();
 //ShowData
 void checkSetting(bool &target,const QString &key);
 void update();
 bool showDict();
 bool showODict();
 bool showOutline();
 bool showPage();
 bool showSimple();
 bool showStream();
 bool isDirty();
 void resetDirty();
 bool needReload();
 void resetReload();
private:
 /** Tree window holding these data */
 TreeWindow *_parent;
 /** MultiTreeWindow holding specified window */
 MultiTreeWindow *_multi;
 /** List view from inside the tree window */
 QListView *_tree;
 /** Scripting base */
 Base *_base;
 /** Dictionary holding tree items that are references */
 QDict<TreeItemRef> refs;
 //ShowData
 /** Show simple objects (int,bool,string,name,real) in object tree? */
 bool show_simple;
 /** Show document dictionary? */
 bool show_dict;
 /** Show dictionary of separate CObjects? */
 bool show_odict;
 /** Show Outlines under PDF document? */
 bool show_outline;
 /** Show Pages under PDF document? */
 bool show_page;
 /** Show ContentStreams in Pages */
 bool show_stream;
 /** True, if any change since last time this was reset to false. Initial value is true */
 bool dirty;
 /** True if the tree needs reloading */
 bool needreload;
};

} // namespace gui 

#endif
