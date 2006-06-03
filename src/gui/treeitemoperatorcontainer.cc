/** @file
 TreeItemOperatorContainer - class holding vector with pdf operators
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitemoperatorcontainer.h"
#include "treeitempdfoperator.h"
#include "multitreewindow.h"
#include "treedata.h"
#include "util.h"

namespace gui {

class TreeData;

using namespace std;

/**
 constructor of TreeItemOperatorContainer - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListView in which to put this item
 @param _op Vector with operators
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemOperatorContainer::TreeItemOperatorContainer(TreeData *_data,QListView *parent,const OperatorVector &_op,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 op=_op;
 init(name);
 reload();
}

/**
 constructor of TreeItemOperatorContainer - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem under which to put this item
 @param _op Vector with operators
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemOperatorContainer::TreeItemOperatorContainer(TreeData *_data,QListViewItem *parent,const OperatorVector &_op,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 op=_op;
 init(name);
 reload();
}

/**
 initialize captions of this item from name
 @param name Caption of item
 */
void TreeItemOperatorContainer::init(const QString &name) {
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,QObject::tr("Operators"));
 setText(2,"");
}

/** default destructor */
TreeItemOperatorContainer::~TreeItemOperatorContainer() {
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemOperatorContainer::createChild(const QString &name,__attribute__((unused)) ChildType typ,QListViewItem *after/*=NULL*/) {
 size_t position=name.toUInt();
 return new TreeItemPdfOperator(data,this,op[position],name,after);
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemOperatorContainer::getChildType(__attribute__((unused)) const QString &name) {
 return 1;//Just one type : PDF Operator
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemOperatorContainer::getChildNames() {
 return util::countList(op.size());
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemOperatorContainer::getQSObject() {
 return NULL;//TODO: return something reasonable
}

//See TreeItemAbstract for description of this virtual method
void TreeItemOperatorContainer::remove() {
 // Do nothing, it doesn't make sense to delete this container
 return;
}

//See TreeItemAbstract for description of this virtual method
void TreeItemOperatorContainer::reloadSelf() {
 //We have operators already inside, so do nothing
 return;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemOperatorContainer::haveChild() {
 return op.size()>0;
}

} // namespace gui
