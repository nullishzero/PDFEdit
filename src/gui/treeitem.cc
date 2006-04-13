/** @file
 TreeItem - class holding one PDF object in tree, descendant of QListViewItem
*/

#include <cobject.h>
#include "treeitem.h"
#include "treedata.h"
#include "treewindow.h"
#include "util.h"
#include "pdfutil.h"

namespace gui {

using namespace std;
using namespace util;

//todo: observers?

/** constructor of TreeItem - create root item from given object
 @param parent QListView in which to put item
 @param pdfObj Object contained in this item
 @param name Name of this item - will be shown in treeview
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param after Item after which this one will be inserted
 */
TreeItem::TreeItem(TreeData *_data,QListView *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 _parent=NULL; //If not TreeItem, _parent will be NULL
 data=_data;
 init(pdfObj,name);
}

/** constructor of TreeItem - create child item from given object
 @param parent QListViewItem which is parent of this object
 @param pdfObj Object contained in this item
 @param name Name of this item - will be shown in treeview
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param after Item after which this one will be inserted
 */
TreeItem::TreeItem(TreeData *_data,QListViewItem *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 _parent=dynamic_cast<TreeItem*>(parent); //If not TreeItem, _parent will be NULL
 data=_data;
 init(pdfObj,name);
}

/** Return parent of this Tree Item
 @return parent TreeItem */
TreeItem* TreeItem::parent() {
 return _parent;
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

/** Checks if any parent is not CRef referencing the same CObject as this item
 @return TreeItem with same reference as this TreeItem or NULL if no such item found */
TreeItem* TreeItem::parentCheck() {
 if (typ!=pRef) return NULL;
 TreeItem* par=_parent;
 while (par) {
  if (par->getRef()==selfRef) return par;
  par=par->parent();
 }
 return NULL;
}

/** Add some info into third column of treeview (mostly value) */
void TreeItem::addData() {
 if (typ==pRef) { // referenced object generation and number
  QString s;
  IndiRef iref=util::getRef(obj);
  selfRef=s.sprintf("%d,%d",iref.num,iref.gen);
  setText(2,QString("-> ")+selfRef);
 }
}

/** 
 If this item contains a CRef, return reference target in text form.
 Otherwise return QString::null;
@return Rference target
*/
QString TreeItem::getRef() {
 return selfRef;
}

/** Slot that will be called when item si opened/closed
 @param open True if item is being opened, false if closed
*/
void TreeItem::setOpen(bool open) {
 printDbg(debug::DBG_DBG," Opening referenced property");
 if (!complete) { //not expanded
  printDbg(debug::DBG_DBG," Opening referenced property incomplete");
  if (open) {
   printDbg(debug::DBG_DBG," Opening referenced property open");
   if (typ==pRef) { // referenced object generation and number
    printDbg(debug::DBG_DBG," Opening referenced property pREF");
    TreeItem *up=parentCheck();
    if (up) { //some parent of this object is referencing the same object
     printDbg(debug::DBG_DBG,"Found ref in parent - not expanding item (-> infinite recursion)");
     data->tree()->setCurrentItem(up);

     //TODO: nefunguje pri otevreni pomoci sipky.
//     tree->ensureItemVisible(up);
     return;//Do not expand references
    }
    data->parent()->addChilds(this,true);
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
