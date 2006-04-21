/** @file
 TreeItemDict - class holding one CDict object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitem.h"
#include "treeitemsimple.h"
#include "treedata.h"
#include "treewindow.h"
#include "util.h"
#include "pdfutil.h"

namespace gui {

using namespace std;
using namespace util;

/**
 @copydoc TreeItem(TreeData *,QListView *,IProperty *,const QString,QListViewItem *)
 */
TreeItemSimple::TreeItemSimple(TreeData *_data,QListView *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItem(_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
}

/**
@copydoc TreeItem(TreeData *,QListViewItem *,IProperty *,const QString,QListViewItem *)
 */
TreeItemSimple::TreeItemSimple(TreeData *_data,QListViewItem *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItem(_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
}

/** default destructor */
TreeItemSimple::~TreeItemSimple() {
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemSimple::createChild(const QString &name,ChildType typ,QListViewItem *after/*=NULL*/) {
 assert(typ!=pRef);//Must not be called on complex childs
 assert(typ!=pDict);
 assert(typ!=pArray);
 assert(0);//Simple tree items have no childs
 return NULL;
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemSimple::getChildType(const QString &name) {
 assert(typ!=pRef);//Must not be called on complex childs
 assert(typ!=pDict);
 assert(typ!=pArray);
 assert(0);//Simple tree items have no childs
 return 0;
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemSimple::getChildNames() {
 assert(typ!=pRef);//Must not be called on complex childs
 assert(typ!=pDict);
 assert(typ!=pArray);
 //TODO: pStream -> have children?

 //Null, Bool, Int, Real, Name, String -> These are simple types without any children
 return QStringList(); 
}

} // namespace gui
