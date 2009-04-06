/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 TreeItem - class holding one PDF object (IProperty) in tree, descendant of TreeItemAbstract
 @author Martin Petricek
*/

#include "treeitem.h"
#include "qtcompat.h"
#include "multitreewindow.h"
#include "settings.h"
#include "treedata.h"
#include "treeitemref.h"
#include "treeitemsimple.h"
#include "treeitemdict.h"
#include "treeitemarray.h"
#include "treeitemcstream.h"
#include "treeitemobserver.h"
#include "pdfutil.h"
#include "util.h"
#include "qsiproperty.h"
#include <kernel/cobject.h>

namespace gui {

using namespace std;
using namespace util;

/**
 constructor of TreeItem - create root item from given object
 @param parent Q_ListView in which to put item
 @param pdfObj Object contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param after Item after which this one will be inserted
 */
TreeItem::TreeItem(const QString &nameId,TreeData *_data,Q_ListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 _parent=NULL; //If not TreeItem, _parent will be NULL
 init(pdfObj,name);
}

/**
 constructor of TreeItem - create child item from given object
 @param parent Q_ListViewItem which is parent of this object
 @param pdfObj Object contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param after Item after which this one will be inserted
*/
TreeItem::TreeItem(const QString &nameId,TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 _parent=dynamic_cast<TreeItem*>(parent); //If not TreeItem, _parent will be NULL
 init(pdfObj,name);
}

/**
 "Constructor" that will create object of proper class based on type of IProperty <br>
 \copydoc TreeItem(const QString&,TreeData *,Q_ListView *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
*/
TreeItem* TreeItem::create(TreeData *_data,Q_ListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=QString::null*/) {
 assert(_data);
 QString useName;
 if (nameId.isNull()) useName=name; else useName=nameId;
 PropertyType type=pdfObj->getType();
 guiPrintDbg(debug::DBG_DBG,"create root:" << Q_OUT(getTypeName(type)) << "  " << Q_OUT(name));
 if (type==pRef) return new TreeItemRef(_data,parent,pdfObj,name,after,useName);
 if (type==pDict) return new TreeItemDict(_data,parent,pdfObj,name,after,useName);
 if (type==pArray) return new TreeItemArray(_data,parent,pdfObj,name,after,useName);
 if (type==pStream) return new TreeItemCStream(_data,parent,pdfObj,name,after,useName);
 return new TreeItemSimple(_data,parent,pdfObj,name,after,useName);
}

/**
 "Constructor" that will create object of proper class based on type of IProperty <br>
 \copydoc TreeItem(const QString&,TreeData *,Q_ListViewItem *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
*/
TreeItem* TreeItem::create(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=QString::null*/) {
 assert(_data);
 QString useName;
 if (nameId.isNull()) useName=name; else useName=nameId;
 PropertyType type=pdfObj->getType();
 guiPrintDbg(debug::DBG_DBG,"create item:" << Q_OUT(getTypeName(type)) << "  " << Q_OUT(name));
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
 guiPrintDbg(debug::DBG_DBG,"init type -> " << Q_OUT(getTypeName(typ)));
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,getTypeName(obj));
 setText(2,"");
 setDragEnabled(true);//Drag drop enabled for this item
}

/**
 return CObject stored inside this item
 @return stored object (IProperty)
*/
boost::shared_ptr<IProperty> TreeItem::getObject() {
 return obj;
}

/**
 This method is needed for "deep reload" to work
 Try to replace object inside this treeitem with a new one.
 @param newItem new object to replace the old one
 @return true if item replaced, false on error.
*/
bool TreeItem::setObject(boost::shared_ptr<IProperty> newItem) {
 if (obj->getType()==newItem->getType()) { //Check if same type
  //Same type. Replace and return success
  if (haveObserver()) {
   uninitObserver();
   obj=newItem;
   initObserver();
  } else {
   obj=newItem;
  }
  return true;
 }
 //forbid replacing with different type for safety reasons
 return false;
}


/** Insert existing item as child of this item
 @param newChild child Q_ListViewItem to be inserted
 */
void TreeItem::insertItem(Q_ListViewItem *newChild) {
 guiPrintDbg(debug::DBG_DBG,"Insert existing item");
 Q_ListViewItem::insertItem(newChild);
 TreeItem *oChild=dynamic_cast<TreeItem*> (newChild);
 if (oChild) oChild->setParent(this);
}

/**
 Return true, if observer is installed on this item
 @return Presence of observer
 */
bool TreeItem::haveObserver() {
 return (observer.get()!=0);
}


/** Sets observer for this item */
void TreeItem::initObserver() {
 guiPrintDbg(debug::DBG_DBG,"Set Observer");
 observer=boost::shared_ptr<TreeItemObserver>(new TreeItemObserver(this));
 REGISTER_SHAREDPTR_OBSERVER(obj, observer);
}

/** Unsets observer for this item */
void TreeItem::uninitObserver() {
 observer->deactivate();
 UNREGISTER_SHAREDPTR_OBSERVER(obj, observer);
 observer.reset();
 guiPrintDbg(debug::DBG_DBG,"UnSet Observer");
}

/** default destructor */
TreeItem::~TreeItem() {
 data->multi()->notifyDeleteItem(this);
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
/*
 // (commented out, as it can damage PDF beyond recovery - no functions to re-add the property)
 //This should be limited to advanced mode
 if (globalSettings->readBool("mode/advanced")) {
  // Is parent a Stream?
  TreeItemCStream* stream=dynamic_cast<TreeItemCStream*>(_parent);
  if (stream) { //removing from stream
   stream->remove(name());
   return;
  }
  //Maybe: remove from operator parameters (same issue as above)
 }
*/
 guiPrintDbg(debug::DBG_DBG,"Can remove property only from dict or array");
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
 guiPrintDbg(debug::DBG_DBG,"This item will now reload data " << Q_OUT(getTypeName(obj)));
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItem::getQSObject() {
 return new QSIProperty(obj,data->base());
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItem::getQSObject(BaseCore *_base) {
 //IProperty uses shared pointer, so it can be copied safely into another document
 return new QSIProperty(obj,_base);
}

} // namespace gui
