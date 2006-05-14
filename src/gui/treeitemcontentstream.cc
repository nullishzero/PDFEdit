/** @file
 TreeItemContentStream - class holding CContentStream object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include <ccontentstream.h>
#include "treeitemcontentstream.h"
#include "qscontentstream.h"
#include "treedata.h"
#include "util.h"

namespace gui {

class TreeData;

using namespace std;

/**
 constructor of TreeItemContentStream - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListView in which to put this item
 @param pdfObj ContentStream contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemContentStream::TreeItemContentStream(TreeData *_data,QListView *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 obj=pdfObj;
 init(name);
 reload(false);
}

/**
 constructor of TreeItemContentStream - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem under which to put this item
 @param pdfObj ContentStream contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemContentStream::TreeItemContentStream(TreeData *_data,QListViewItem *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 obj=pdfObj;
 init(name);
 reload(false);
}

/**
 initialize captions of this item from name
 @param name Caption of item
 */
void TreeItemContentStream::init(const QString &name) {
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,QObject::tr("Content Stream"));
 setText(2,"");
}

/** default destructor */
TreeItemContentStream::~TreeItemContentStream() {
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemContentStream::createChild(__attribute__((unused)) const QString &name,__attribute__((unused)) ChildType typ,__attribute__((unused)) QListViewItem *after/*=NULL*/) {
 //TODO
 assert(0);//no childs yet
 return NULL;
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemContentStream::getChildType(__attribute__((unused)) const QString &name) {
 //TODO
 assert(0);//no childs yet
 return 0;
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemContentStream::getChildNames() {
 //TODO: Stream children
 return QStringList(); 
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemContentStream::getQSObject() {
 return new QSContentStream(obj,data->base());
}

//See TreeItemAbstract for description of this virtual method
void TreeItemContentStream::remove() {
 //TODO: implement
 return;
}

//See TreeItemAbstract for description of this virtual method
void TreeItemContentStream::reloadSelf() {
 //TODO: implement
 return;
}

} // namespace gui
