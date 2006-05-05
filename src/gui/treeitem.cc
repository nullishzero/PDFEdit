/** @file
 TreeItem - class holding one PDF object (IProperty) in tree, descendant of TreeItemAbstract
 @author Martin Petricek
*/

#include <cobject.h>
#include "treedata.h"
#include "treeitem.h"
#include "treeitemref.h"
#include "treeitemsimple.h"
#include "treeitemdict.h"
#include "treeitemarray.h"
#include "treeitemcstream.h"
#include "pdfutil.h"
#include "util.h"
#include "qsiproperty.h"

namespace gui {

using namespace std;
using namespace util;

/** constructor of TreeItem - create root item from given object
 @param parent QListView in which to put item
 @param pdfObj Object contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param after Item after which this one will be inserted
 */
TreeItem::TreeItem(const QString &nameId,TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(nameId,parent,after) {
 _parent=NULL; //If not TreeItem, _parent will be NULL
 data=_data;
 init(pdfObj,name);
}

/** constructor of TreeItem - create child item from given object
 @param parent QListViewItem which is parent of this object
 @param pdfObj Object contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param after Item after which this one will be inserted
*/
TreeItem::TreeItem(const QString &nameId,TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(nameId,parent,after) {
 _parent=dynamic_cast<TreeItem*>(parent); //If not TreeItem, _parent will be NULL
 data=_data;
 init(pdfObj,name);
}

/**
 "Constructor" that will create object of proper class based on type of IProperty <br>
 @copydoc TreeItem(const QString&,TreeData *,QListView *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
*/
TreeItem* TreeItem::create(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=QString::null*/) {
 assert(_data);
 QString useName;
 if (nameId.isNull()) useName=name; else useName=nameId;
 PropertyType type=pdfObj->getType();
 guiPrintDbg(debug::DBG_DBG,"create root:" << getTypeName(type) << "  " << name);
 if (type==pRef) return new TreeItemRef(_data,parent,pdfObj,name,after,useName);
 if (type==pDict) return new TreeItemDict(_data,parent,pdfObj,name,after,useName);
 if (type==pArray) return new TreeItemArray(_data,parent,pdfObj,name,after,useName);
 if (type==pStream) return new TreeItemCStream(_data,parent,pdfObj,name,after,useName);
 return new TreeItemSimple(_data,parent,pdfObj,name,after,useName);
}

/**
 "Constructor" that will create object of proper class based on type of IProperty <br>
 @copydoc TreeItem(const QString&,TreeData *,QListViewItem *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
*/
TreeItem* TreeItem::create(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=QString::null*/) {
 assert(_data);
 QString useName;
 if (nameId.isNull()) useName=name; else useName=nameId;
 PropertyType type=pdfObj->getType();
 guiPrintDbg(debug::DBG_DBG,"create item:" << getTypeName(type) << "  " << name);
 if (type==pRef) return new TreeItemRef(_data,parent,pdfObj,name,after,useName);
 if (type==pDict) return new TreeItemDict(_data,parent,pdfObj,name,after,useName);
 if (type==pArray) return new TreeItemArray(_data,parent,pdfObj,name,after,useName);
 if (type==pStream) return new TreeItemCStream(_data,parent,pdfObj,name,after,useName);
 return new TreeItemSimple(_data,parent,pdfObj,name,after,useName);
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
void TreeItem::init(boost::shared_ptr<IProperty> pdfObj,const QString &name) {
 obj=pdfObj;
 typ=obj->getType();
 guiPrintDbg(debug::DBG_DBG,"init type -> " << getTypeName(typ));
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,getTypeName(obj));
 setText(2,"");
}

/** return CObject stored inside this item
 @return stored object (IProperty) */
boost::shared_ptr<IProperty> TreeItem::getObject() {
 return obj;
}


/** Insert existing item as child of this item
 @param newChild child QListViewItem to be inserted
 */
void TreeItem::insertItem(QListViewItem *newChild) {
 guiPrintDbg(debug::DBG_DBG,"Insert existing item");
 QListViewItem::insertItem(newChild);
 TreeItem *oChild=dynamic_cast<TreeItem*> (newChild);
 if (oChild) oChild->setParent(this);
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
 virtual void notify (__attribute__((unused)) boost::shared_ptr<IProperty> newValue, __attribute__((unused)) boost::shared_ptr<const IProperty::ObserverContext> context) const throw() {
  if (!parent) {
   //Should never happen
   guiPrintDbg(debug::DBG_ERR,"BUG: Kernel is holding observer for item already destroyed");
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
 /** Destructor */
 virtual ~TreeItemObserver() throw(){
 };
private:
 /** Parent object holding observer property*/
 TreeItem *parent;
};

/** Sets observer for this item */
void TreeItem::initObserver() {
 guiPrintDbg(debug::DBG_DBG,"Set Observer");
 observer=boost::shared_ptr<TreeItemObserver>(new TreeItemObserver(this));
 obj->registerObserver(observer);
}

/** Unsets observer for this item */
void TreeItem::uninitObserver() {
 observer->deactivate();
 obj->unregisterObserver(observer);
 guiPrintDbg(debug::DBG_DBG,"UnSet Observer");
}

/** default destructor */
TreeItem::~TreeItem() {
}

//See TreeItemAbstract for description of this virtual method
void TreeItem::remove() {
 guiPrintDbg(debug::DBG_DBG,"Removing property");
 if (!_parent) {
  //Someone is trying to do something stupid, like removing whole document catalog.
  //We can't do that anyway
  return;
 }
 //Parent should be Array or Dict;
 // Is parent a Dict?
 TreeItemDict* dict=dynamic_cast<TreeItemDict*>(_parent);
 if (dict) { //removing from dict
  dict->remove(name());
  return;
 }
 // Is parent an Array?
 TreeItemArray* array=dynamic_cast<TreeItemArray*>(_parent);
 if (array) { //removing from array
  array->remove(name().toUInt());
  return;
 }
 guiPrintDbg(debug::DBG_DBG,"Removing property - from what?");
}

/**
 Overloaded version of TreeItemAbstract::unSelect
 \see TreeItemAbstract::unSelect
 */
void TreeItem::unSelect() {
 TreeItemAbstract::unSelect(data->tree());
}


//See TreeItemAbstract for description of this virtual method
void TreeItem::reloadSelf() {
 assert(typ!=pRef);//Must not be called on CRefs
 guiPrintDbg(debug::DBG_DBG,"This item will now reload data " << getTypeName(obj));
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItem::getQSObject() {
 return new QSIProperty(obj);
}

} // namespace gui
