/** @file
 TreeItem - class holding one PDF object in tree, descendant of QListViewItem
*/

#include "treeitem.h"
#include "treewindow.h"
#include "util.h"
#include <cobject.h>

namespace gui {

using namespace std;

//todo: observers?

/** constructor of TreeItem - create root item from given object
 @param parent QListView in which to put item
 @param pdfObj Object contained in this item
 @param name Name of this item - will be shown in treeview
 */
TreeItem::TreeItem(TreeWindow *_tree,QListView *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 tree=_tree;
 init(pdfObj,name);
}

/** constructor of TreeItem - create child item from given object
 @param parent QListViewItem which is parent of this object
 @param pdfObj Object contained in this item
 @param name Name of this item - will be shown in treeview
 */
TreeItem::TreeItem(TreeWindow *_tree,QListViewItem *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 tree=_tree;
 init(pdfObj,name);
}

/** Initialize item from given PDF object
 @param pdfObj Object used to initialize this item
 @param name Name of this item - will be shown in treeview
 */
void TreeItem::init(IProperty *pdfObj,const QString name) {
 complete=false;
 obj=pdfObj;
 typ=obj->getType();
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,getTypeName(typ));
 addData();
}

/** Add some info into third column of treeview (mostly value) */
void TreeItem::addData() {
 if (typ==pRef) { // referenced object generation and number
  QString s;
  CRef* ref=(CRef*)obj;
  IndiRef iref;
  ref->getPropertyValue(iref);//=obj->getIndiRef();  
  setText(2,s.sprintf("-> %d,%d",iref.num,iref.gen));
 }
}

void TreeItem::setOpen(bool open) {
 printDbg(debug::DBG_DBG," Opening referenced property");
 if (!complete) { //not expanded
  printDbg(debug::DBG_DBG," Opening referenced property incomplete");
  if (open) {
   printDbg(debug::DBG_DBG," Opening referenced property open");
   if (typ==pRef) { // referenced object generation and number
     printDbg(debug::DBG_DBG," Opening referenced property pREF");
    tree->addChilds(this,true);
    complete=true;
   }
  }
 }
 //TODO: if closing, delete childs?

 //Call original method
 QListViewItem::setOpen(open);
}

/** return CObject stored inside this item
 @return stored object (IProperty) */
IProperty* TreeItem::getObject() {
 return obj;
}

/** default destructor */
TreeItem::~TreeItem() {
}

} // namespace gui
