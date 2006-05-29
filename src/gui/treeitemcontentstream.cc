/** @file
 TreeItemContentStream - class holding CContentStream object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include <ccontentstream.h>
#include "treeitempage.h"
#include "treeitemcontentstream.h"
#include "treeitempdfoperator.h"
#include "multitreewindow.h"
#include "qscontentstream.h"
#include "treedata.h"
#include "util.h"
#include "treeitemcontentstreamobserver.h"

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
 reload();
 initObserver();
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
 reload();
 initObserver();
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
 uninitObserver();
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemContentStream::createChild(const QString &name,__attribute__((unused)) ChildType typ,QListViewItem *after/*=NULL*/) {
 size_t position=name.toUInt();
 return new TreeItemPdfOperator(data,this,op[position],obj,name,after);
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemContentStream::getChildType(__attribute__((unused)) const QString &name) {
 return 1;//Just one type : PDF Operator
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemContentStream::getChildNames() {
 QStringList childs;
 for (size_t i=0;i<op.size();i++) {
  childs+=QString::number(i);
 }
 return childs; 
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemContentStream::getQSObject() {
 return new QSContentStream(obj,data->base());
}

//See TreeItemAbstract for description of this virtual method
void TreeItemContentStream::remove() {
 // Do nothing
 // Contentstream can't just delete itself.
 return;
}

//See TreeItemAbstract for description of this virtual method
void TreeItemContentStream::reloadSelf() {
 //Reload list of pdf operators
 obj->getPdfOperators(op);
 return;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemContentStream::haveChild() {
 return op.size()>0;
}

/**
 Slot that will be called when item is opened/closed
 @param open True if item is being opened, false if closed
*/
void TreeItemContentStream::setOpen(bool open) {
 if (parent() && open) { //Activate/open new tree
  //Check for parent if it is a page and try to "invent" some nice text for the tab and the item
  TreeItemPage* parentPage=dynamic_cast<TreeItemPage*>(parent());
  QString pName=text(0);
  if (parentPage) { //Parent tree item is a page
   pName+=" (";
   pName+=parentPage->text(0);
   pName+=")";
  }
  data->multi()->activate(obj,pName);
  return;//Do not open
 }
 TreeItemAbstract::setOpen(open);
}

/**
 Return content stream inside this tree items
 @return Content stream in this tree item
*/
boost::shared_ptr<CContentStream> TreeItemContentStream::getObject() {
 return obj;
}

/** Sets observer for this item */
void TreeItemContentStream::initObserver() {
 guiPrintDbg(debug::DBG_DBG,"Set Observer");
 observer=boost::shared_ptr<TreeItemContentStreamObserver>(new TreeItemContentStreamObserver(this));
 obj->registerObserver(observer);
}

/** Unsets observer for this item */
void TreeItemContentStream::uninitObserver() {
 observer->deactivate();
 obj->unregisterObserver(observer);
 observer.reset();
 guiPrintDbg(debug::DBG_DBG,"UnSet Observer");
}

} // namespace gui
