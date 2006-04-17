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

/** Return parent of this Tree Item, if it is also TreeItem.
 If not, return NULL
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
 setText(1,getTypeName(obj));
 addData();
 initObserver();
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
 } else {
  setText(2,"");
 }
}

/** 
 If this item contains a CRef, return reference target in text form.
 Otherwise return QString::null;
 @return Rerence target
*/
QString TreeItem::getRef() {
 return selfRef;
}

/** Will remove itself from list of opened references and close itself*/
void TreeItem::unOpen() {
 assert(typ==pRef); //only usable on pRef
 data->remove(this);
 complete=false;
 QListViewItem::setOpen(false);
}

/** Slot that will be called when item si opened/closed
 @param open True if item is being opened, false if closed
*/
void TreeItem::setOpen(bool open) {
 if (!complete) { //not expanded
  printDbg(debug::DBG_DBG," Opening referenced property -> expanding ");
  if (open) {
   printDbg(debug::DBG_DBG," Opening referenced property open");
   if (typ==pRef) { // referenced object generation and number
    printDbg(debug::DBG_DBG," Opening referenced property pREF : " << selfRef);
    TreeItem *up=parentCheck();
    if (up) { //some parent of this object is referencing the same object
     printDbg(debug::DBG_DBG,"Found ref in parent - not expanding item (-> infinite recursion)");
     data->tree()->setCurrentItem(up);
     data->tree()->ensureItemVisible(up);
     //TODO: nefunguje pri otevreni pomoci sipky.
     //QListViewItem::setOpen(false);//Keep closed
     return;//Do not expand references
    }
    TreeItem *other=data->find(selfRef);
    if (other && other!=this) { //subtree already found elsewhere -> reparent
     QListViewItem *otherChild;
     printDbg(debug::DBG_DBG,"Will relocate child, counts: "<< this->childCount() << " , " << other->childCount());
     while ((otherChild=other->firstChild())) {
      printDbg(debug::DBG_DBG,"Relocating child");
      other->takeItem(otherChild);
      insertItem(otherChild);
     }
     printDbg(debug::DBG_DBG,"Done relocate child, counts: "<< this->childCount() << " , " << other->childCount());
     other->unOpen();
     data->add(this);//re-add itself
     complete=true;
     QListViewItem::setOpen(open);
     return;//Do not expand references
    } else { // subtree not found -> add to list
     printDbg(debug::DBG_DBG,"Subtree not found");
     data->add(this);
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

/** Reload itself - update self and all childs from state in kernel */
void TreeItem::reloadSelf() {
 printDbg(debug::DBG_DBG,"This item will now reload itself: " << getTypeName(obj));
 data->parent()->reloadFrom(this);
}

/** Reload itself - update only data in itself, not any children */
void TreeItem::reloadData() {
 printDbg(debug::DBG_DBG,"This item will now reload data " << getTypeName(obj));
 if (typ==pRef) { // reference
  QString old=selfRef;
  //Update reference target
  addData();
  if (old!=selfRef) {
   //Remove old reference from list of opened and available items
   data->remove(old);
   printDbg(debug::DBG_DBG,"Reference target changed: " << old << " -> " << selfRef);
   //Close itself
   this->setOpen(false);
   QListViewItem *item;
   //Remove all child items
   while ((item=this->firstChild())) {
    printDbg(debug::DBG_DBG,"Deleting child (deletelater)");
    delete item;
    //TODO: "Warning: Do not delete any QListViewItem objects in slots connected to this signal."
    //      in QT documentation -> why?
   }
   //Set as incomplete
   complete=false;
  }
 } else {
  addData();
 }
}

/** Internal class providing observer */
class TreeItemObserver: public IProperty::Observer {
public:
 /** Constructor
 @param _parent Object to be reloaded on any change to monitored IProperty
 */
 TreeItemObserver(TreeItem* _parent){
  parent=_parent;
 };
 /** Deactivate observer */
 void deactivate() {
  parent=0;
 }
 /** Notification function called by changing property */
 virtual void notify (boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const IProperty::ObserverContext> context) const {
  if (!parent) {
   //Should never happen
   printDbg(debug::DBG_ERR,"BUG: Kernel is holding observer for item already destroyed");
   assert(parent);
   return;
  }
  //Reload contents of parent
  parent->reloadSelf();
 }
 /** Priority of this observer */
 virtual priority_t getPriority()const {
  return 0;//TODO: what priority?
 }
 /** Descructor */
 virtual ~TreeItemObserver(){
 };
private:
 /** Parent object holding observer property*/
 TreeItem *parent;
};


/** Sets observer for this item */
void TreeItem::initObserver() {
 //TODO: Do not set on simple types
 printDbg(debug::DBG_DBG,"Set Observer");
 observer=boost::shared_ptr<TreeItemObserver>(new TreeItemObserver(this));
 obj->registerObserver(observer);
}

/** Unsets observer for this item */
void TreeItem::uninitObserver() {
 //TODO: Do not set on simple types
 observer->deactivate();
 obj->unregisterObserver(observer);
 printDbg(debug::DBG_DBG,"UnSet Observer");
}

/** Check if childs of this items are yet unknown and to be parsed/added
 @return true if item is expanded, false if it is incomplete */
bool TreeItem::isComplete() {
 return complete;
}

/** default destructor */
TreeItem::~TreeItem() {
 uninitObserver();
 data->remove(this);
}

} // namespace gui
