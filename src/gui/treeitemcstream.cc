/** @file
 TreeItemDict - class holding CContentStream object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include <ccontentstream.h>
#include "treeitemcstream.h"
#include "qscontentstream.h"

namespace gui {

class TreeData;

using namespace std;

/**
 constructor of TreeItemCStream - create root item from given object
 @param parent QListView in which to put item
 @param pdfObj ContentStream contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemCStream::TreeItemCStream(QListView *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,parent,after) {
 obj=pdfObj;
 init(name);
 reload(false);
}

/**
 constructor of TreeItemCStream - create child item from given object
 @param parent QListView in which to put item
 @param pdfObj ContentStream contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemCStream::TreeItemCStream(QListViewItem *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,parent,after) {
 obj=pdfObj;
 init(name);
 reload(false);
}

/**
 initialize captions of this item from name
 @param name Caption of item
 */
void TreeItemCStream::init(const QString &name) {
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,QObject::tr("Stream"));
 setText(2,"");
}

/** default destructor */
TreeItemCStream::~TreeItemCStream() {
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemCStream::createChild(__attribute__((unused)) const QString &name,__attribute__((unused)) ChildType typ,__attribute__((unused)) QListViewItem *after/*=NULL*/) {
 assert(0);//no childs
 return NULL;
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemCStream::getChildType(__attribute__((unused)) const QString &name) {
 assert(0);//no childs
 return 0;
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemCStream::getChildNames() {
 //TODO: pStream children
 return QStringList(); 
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemCStream::getQSObject() {
 return new QSContentStream(obj);
}

//See TreeItemAbstract for description of this virtual method
void TreeItemCStream::remove() {
 //TODO: implement
 return;
}

} // namespace gui
