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
TreeItem::TreeItem(TreeData *_data,QListView *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(parent,after) {
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
TreeItem::TreeItem(TreeData *_data,QListViewItem *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(parent,after) {
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

/** Set new parent of this object
 @param parent New parent of this object if it is TreeItem (or NULL otherwise) */
void TreeItem::setParent(TreeItem *parent) {
 _parent=parent;
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
 reload(false);
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
  //TODO: better dynamic generation, using reload() and TreeItemAbstract
  setExpandable(true);
 } else {
  complete=true;
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
     return;//Do not expand references and keep closed
    }
    TreeItem *other=data->find(selfRef);
    if (other && other!=this) { //subtree already found elsewhere -> reparent
     QListViewItem *otherChild;
     printDbg(debug::DBG_DBG,"Will relocate child, counts: "<< this->childCount() << " , " << other->childCount());
     moveAllChildsFrom(other);
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
    //Reload as "complete and opened item"
    complete=true;
    reload();
   }
  }
 }
 //TODO: if closing, delete childs?

 //Call original method
 QListViewItem::setOpen(open);
}

void TreeItem::insertItem( QListViewItem * newChild) {
 printDbg(debug::DBG_DBG,"Reparenting item");
 QListViewItem::insertItem(newChild);
 TreeItem *oChild=dynamic_cast<TreeItem*> (newChild);
 if (oChild) oChild->setParent(this);
}


/** return CObject stored inside this item
 @return stored object (IProperty) */
IProperty* TreeItem::getObject() {
 return obj;
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
 virtual void notify (boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const IProperty::ObserverContext> context) const throw() {
  if (!parent) {
   //Should never happen
   printDbg(debug::DBG_ERR,"BUG: Kernel is holding observer for item already destroyed");
   assert(parent);
   return;
  }
  //Reload contents of parent
  parent->reload();
 }
 /** Priority of this observer */
 virtual priority_t getPriority() const throw(){
  return 0;//TODO: what priority?
 }
 /** Descructor */
 virtual ~TreeItemObserver() throw(){
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

/** Reload itself - update only data in itself, not any children */
void TreeItem::reloadSelf() {
 printDbg(debug::DBG_DBG,"This item will now reload data " << getTypeName(obj));
 if (typ==pRef) { // reference
  QString old=selfRef;
  //Update reference target
  addData();
  if (old!=selfRef) {
   //Remove old reference from list of opened and available items
   if (complete) data->remove(old);//Was complete -> remove data
   printDbg(debug::DBG_DBG,"Reference target changed: " << old << " -> " << selfRef);
   //Close itself
   this->setOpen(false);
   //Set as incomplete
   complete=false;
   //Childs will go away in reload() later
   //TODO: delete them all now
  }
 } else {
  addData();
 }
}

/** Create subchild */
TreeItemAbstract* TreeItem::createChild(const QString &name,QListViewItem *after/*=NULL*/) {
 if (obj->getType()==pDict) {	//Object is CDict
  CDict *dict=(CDict*)obj;
  boost::shared_ptr<IProperty> property=dict->getProperty(name);
  if (!data->showSimple() && isSimple(property)) return NULL; //simple item -> skip it
  return new TreeItem(data,this,property.get(),name,after);//TODO: factory
 }
 if (obj->getType()==pArray) {	//Object is CArray
  CArray *ar=(CArray*)obj;
  unsigned int i=name.toUInt();
  boost::shared_ptr<IProperty> property=ar->getProperty(i);
  if (!data->showSimple() && isSimple(property)) return NULL; //simple item -> skip it
  QString oname;
  oname.sprintf("[%d]",i);
  return new TreeItem(data,this, property.get(),oname,after);//TODO: factory
 }
 if (obj->getType()==pRef) {	//Object is CRef
  assert(name=="Target");
  QString s;
  CPdf* pdf=obj->getPdf();
  if (!pdf) return NULL; //No document opened -> cannot parse references
                    //Should happen only while testing
  CRef* cref=(CRef*)obj;
  IndiRef ref;
  cref->getPropertyValue(ref);
  printDbg(debug::DBG_DBG," LOADING referenced property: " << ref.num << "," << ref.gen);
  boost::shared_ptr<IProperty> rp=pdf->getIndirectProperty(ref);
  return new TreeItem(data,this, rp.get(),s.sprintf("<%d,%d>",ref.num,ref.gen),after);//TODO: factory
 }
 assert(0);
 return NULL;
}
/** Return list of child names */
QStringList TreeItem::getChildNames() {
 if (!complete) return QStringList(); //Childs not loaded yet
 if (obj->getType()==pDict) {	//Object is CDict
  QStringList itemList;
  CDict *dict=(CDict*)obj;
  vector<string> list;
  dict->getAllPropertyNames(list);
  vector<string>::iterator it;
  for( it=list.begin();it!=list.end();++it) { // for each property
   boost::shared_ptr<IProperty> property=dict->getProperty(*it);
   if (!data->showSimple() && isSimple(property)) continue; //simple item -> skip it
   itemList += *it;
  }
  return itemList;
 }
 if (obj->getType()==pArray) {	//Object is CArray
  QStringList itemList;
  CArray *ar=(CArray*)obj;
  size_t n=ar->getPropertyCount();
  QString name;
  for(size_t i=0;i<n;i++) { //for each property
   boost::shared_ptr<IProperty> property=ar->getProperty(i);
   if (!data->showSimple() && isSimple(property)) continue; //simple item -> skip it
   itemList += QString::number(i);
  }
  return itemList;
 }

 if (obj->getType()==pRef) {	//Object is CRef
  return QStringList("Target");
 }

 //Null, Bool, Int, Real, Name, String -> These are simple types without any children
 //TODO: pStream -> have they children?

 return QStringList(); 
}

//TODO: break into TreeItem(Simple|Ref|Dict|Array)

} // namespace gui
