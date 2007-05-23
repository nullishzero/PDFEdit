/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __TREEDATA_H__
#define __TREEDATA_H__

#include "qtcompat.h"
#include QDICT
class Q_ListView;
class QString;

namespace gui {

class TreeItem;
class TreeItemRef;
class TreeWindow;
class MultiTreeWindow;
class BaseCore;

/**
 Storage class holding Tree Data that need to be shared between tree window and its items
 and also holding setting related to what items to show in tree
 \brief Tree parameters shared between tree and tree items
*/
class TreeData {
public:
 TreeData(TreeWindow *parent,Q_ListView *tree,BaseCore *base,MultiTreeWindow *multi);
 ~TreeData();
 TreeWindow* parent();
 Q_ListView* tree();
 MultiTreeWindow* multi();
 BaseCore* base();
 void add(TreeItemRef *it);
 void remove(TreeItemRef *it);
 void remove(const QString &ref);
 TreeItemRef* find(const QString &ref);
 void clear();
 //ShowData
 void checkSetting(bool &target,const QString &key,bool defaultValue=true);
 void update();
 bool sortDict();
 bool showDict();
 bool showODict();
 bool showOutline();
 bool showAnnot();
 bool showGraphic();
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
 Q_ListView *_tree;
 /** Scripting base */
 BaseCore *_base;
 /** Dictionary holding tree items that are references */
 Q_Dict<TreeItemRef> refs;
 //ShowData
 /** Show simple objects (int,bool,string,name,real) in object tree? */
 bool show_simple;
 /** Show document dictionary? */
 bool show_dict;
 /** Sort dictionaries? */
 bool show_dict_sort;
 /** Show graphic objects? */
 bool show_graphic;
 /** Show annotations? */
 bool show_annot;
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
