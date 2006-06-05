/** @file
 TreeItemAnnotation - class holding one annotation in tree
 @author Martin Petricek
*/

#include "treeitemannotation.h"
#include "treeitem.h"
#include "treeitemdict.h"
#include "treedata.h"
#include "util.h"
#include "pdfutil.h"
#include "qsannotation.h"
#include <cannotation.h>
#include <cpage.h>

namespace gui {

using namespace std;
using namespace util;

TreeItemAnnotation::TreeItemAnnotation(TreeData *_data,QListView *parent,boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/)
 : TreeItemAbstract(nameId,_data,parent,after) {
 assert(data);
 title=name;
 obj=pdfObj;
 page=_page;
 init();
 reload(false);
}

TreeItemAnnotation::TreeItemAnnotation(TreeData *_data,QListViewItem *parent,boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/)
 : TreeItemAbstract(nameId,_data,parent,after) {
 assert(data);
 title=name;
 obj=pdfObj;
 page=_page;
 init();
 reload(false);
}

/** Initialize the item */
void TreeItemAnnotation::init() {
 QString type=annotTypeName(obj);
 setText(0,title);
 setText(2,type);
}

//See TreeItemAbstract for description of this virtual method
void TreeItemAnnotation::reloadSelf() {
 init();
}

/** default destructor */
TreeItemAnnotation::~TreeItemAnnotation() {
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemAnnotation::getChildType(__attribute__((unused)) const QString &name) {
 return 1;//Just one type : Annotation dictinary
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemAnnotation::createChild(__attribute__((unused)) const QString &name,ChildType typ,QListViewItem *after/*=NULL*/) {
 if (typ==1) { //Return object dictionary
  return TreeItem::create(data,this,obj->getDictionary(),QObject::tr("Dictionary"),after);
 }
 assert(0);
 return NULL;
}

/**
 Return CAnnotation inside this tree items
 @return stored annotation
*/
boost::shared_ptr<CAnnotation> TreeItemAnnotation::getObject() {
 return obj;
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemAnnotation::getChildNames() {
 QStringList childs;
 if (data->showODict()) childs+="Dict";
 return childs;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemAnnotation::haveChild() {
 return data->showODict();
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemAnnotation::getQSObject() {
 return new QSAnnotation(obj,page,data->base());
}

//See TreeItemAbstract for description of this virtual method
void TreeItemAnnotation::remove() {
 //Remove from CPage 
 page->delAnnotation(obj);
 return;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemAnnotation::validChild(__attribute__((unused)) const QString &name,QListViewItem *oldChild) {
 TreeItemDict *itp=dynamic_cast<TreeItemDict*>(oldChild);
 if (itp) { //Is a dictionary
  return obj->getDictionary().get()==itp->getObject().get();
 }
 return false;
}

} // namespace gui
