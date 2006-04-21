/** @file
 TreeItem - class holding one PDF object in tree, descendant of TreeItemAbstract
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitem.h"
#include "treeitemref.h"
#include "treeitemsimple.h"
#include "treeitemdict.h"
#include "treeitemarray.h"
#include "pdfutil.h"

namespace gui {

class TreeData;

using namespace std;
using namespace util;

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

/**
"Constructor" that will create object of proper class based on type of IProperty <br>
@copydoc TreeItem(TreeData *,QListView *,IProperty *,const QString,QListViewItem *)
*/
TreeItem* TreeItem::create(TreeData *_data,QListView *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/) {
 assert(_data);
 PropertyType type=pdfObj->getType();
 printDbg(debug::DBG_DBG,"create root:" << getTypeName(type) << "  " << name);
 if (type==pRef) return new TreeItemRef(_data,parent,pdfObj,name,after);
 if (type==pDict) return new TreeItemDict(_data,parent,pdfObj,name,after);
 if (type==pArray) return new TreeItemArray(_data,parent,pdfObj,name,after);
 return new TreeItemSimple(_data,parent,pdfObj,name,after);
}

/**
"Constructor" that will create object of proper class based on type of IProperty <br>
@copydoc TreeItem(TreeData *,QListViewItem *,IProperty *,const QString,QListViewItem *)
*/
TreeItem* TreeItem::create(TreeData *_data,QListViewItem *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/) {
 assert(_data);
 PropertyType type=pdfObj->getType();
 printDbg(debug::DBG_DBG,"create item:" << getTypeName(type) << "  " << name);
 if (type==pRef) return new TreeItemRef(_data,parent,pdfObj,name,after);
 if (type==pDict) return new TreeItemDict(_data,parent,pdfObj,name,after);
 if (type==pArray) return new TreeItemArray(_data,parent,pdfObj,name,after);
 return new TreeItemSimple(_data,parent,pdfObj,name,after);
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
void TreeItem::init(IProperty *pdfObj,const QString &name) {
 obj=pdfObj;
 typ=obj->getType();
 printDbg(debug::DBG_DBG,"init type -> " << getTypeName(typ));
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
IProperty* TreeItem::getObject() {
 return obj;
}

/** Insert existing item as child of this item
 @param newChild child QListViewItem to be inserted
 */
void TreeItem::insertItem(QListViewItem *newChild) {
 printDbg(debug::DBG_DBG,"Insert existing item");
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
 /** Destructor */
 virtual ~TreeItemObserver() throw(){
 };
private:
 /** Parent object holding observer property*/
 TreeItem *parent;
};

/** Sets observer for this item */
void TreeItem::initObserver() {
 printDbg(debug::DBG_DBG,"Set Observer");
 observer=boost::shared_ptr<TreeItemObserver>(new TreeItemObserver(this));
 obj->registerObserver(observer);
}

/** Unsets observer for this item */
void TreeItem::uninitObserver() {
 observer->deactivate();
 obj->unregisterObserver(observer);
 printDbg(debug::DBG_DBG,"UnSet Observer");
}

/** default destructor */
TreeItem::~TreeItem() {
}

//See TreeItemAbstract for description of this virtual method
void TreeItem::reloadSelf() {
 assert(typ!=pRef);//Must not be called on CRefs
 printDbg(debug::DBG_DBG,"This item will now reload data " << getTypeName(obj));
}

} // namespace gui
