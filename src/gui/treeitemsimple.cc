/** @file
 TreeItemDict - class holding one simple object in tree (not ref, dict or array), descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitemsimple.h"

namespace gui {

class TreeData;

using namespace std;

/**
 @copydoc TreeItem(const QString&,TreeData *,QListView *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemSimple::TreeItemSimple(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
}

/**
@copydoc TreeItem(const QString&,TreeData *,QListViewItem *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemSimple::TreeItemSimple(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
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
