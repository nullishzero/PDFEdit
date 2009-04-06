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
 QSTreeItem - QObject wrapper around TreeItemAbstract (single tree item in tree view)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qstreeitem.h"
#include "qsiproperty.h"
#include "treeitemabstract.h"
#include "qsimporter.h"
#include "basecore.h"

namespace gui {

/**
 Construct wrapper with given TreeItemAbstract<br>
 This constructor is available only to subclasses - they can set their own class name here
 @param className Item type to use.
 @param item Tree item
 @param _base scripting base
*/
QSTreeItem::QSTreeItem(const QString &className,TreeItemAbstract *item,BaseCore *_base) : QSCObject (className,_base) {
 obj=item;
 assert(obj);
 //gui object wrapper must have base
 assert(_base);
 _base->addTreeItemToList(this);
}

/**
 Construct wrapper with given TreeItemAbstract
 @param item Tree item
 @param _base scripting base
*/
QSTreeItem::QSTreeItem(TreeItemAbstract *item,BaseCore *_base) : QSCObject ("TreeItem",_base) {
 obj=item;
 assert(obj);
 //gui object wrapper must have base
 assert(_base);
 _base->addTreeItemToList(this);
}

/**
 Return item inside this tree item, wrapped by QObject wrapper.
 Caller is responsible for freeing the object
 @return QObject wrapper around data inside treeitem
*/
QSCObject* QSTreeItem::item() {
 if (nullPtr(obj,"item")) return NULL;
 return obj->getQSObject();
}

/**
 Return item inside this tree item, as item() does, but with one exception:
 If the item is reference, return reference target instead of the reference
 Caller is responsible for freeing the object
 \see item
 @return QObject wrapper around data inside treeitem
*/
QSCObject* QSTreeItem::itemref() {
 if (nullPtr(obj,"itemref")) return NULL;
 QSCObject* rItem=obj->getQSObject();
 QSIProperty* ip=dynamic_cast<QSIProperty*>(rItem);
 if (!ip) return rItem;//Not IProperty
 QSCObject* refItem=ip->ref();
 if (refItem==rItem) return rItem;//Wasn't a reference
 delete rItem;//Delete original
 return refItem;//Return reference
}

/**
 Check if the item is selected
 @return True if the item is selected, false if not
*/
bool QSTreeItem::selected() {
 if (nullPtr(obj,"selected")) return false;
 return obj->isSelected();
}

/**
 Select or unselect given item
 @param selected Specifies what to do: true to select, false to unselect
*/
void QSTreeItem::setSelected(bool selected/*=true*/) {
 if (nullPtr(obj,"setSelected")) return;
 obj->setSelect(selected);
}

/**
 Return type of item inside this tree item
 @return Type of item
*/
QString QSTreeItem::itemtype() {
 if (nullPtr(obj,"itemType")) return QString::null;
 QSCObject* it=obj->getQSObject();

 //Some tree item does not have actually any "item" in them
 if (!it) return QString::null;

 QString type=it->type();
 delete it;
 return type;
}

/**
 Check if the tree item wrapper is valid,
 i.e. if corresponding tree item still exist in the tree view
 @return True if valid, false if not
*/
bool QSTreeItem::valid() {
 return (obj!=NULL);
}

/** Explicitly reload contents of this item and its subtree from current state of PDF document */
void QSTreeItem::reload() {
 if (nullPtr(obj,"reload")) return;
 obj->reload();
}

/**
 Set the item to be opened or closed
 @param opened True if the item is to be opened, false if closed
*/
void QSTreeItem::setOpen(bool opened) {
 if (nullPtr(obj,"setOpen")) return;
 obj->setOpen(opened);
}

/**
 Explicitly force reload contents of this item and its subtree from current state of PDF document
 Add "force reload" flag, to test for possible tree item reloading-related bugs
 Should only be used for debugging, reload() should normally work well
 */
void QSTreeItem::reload_force() {
 if (nullPtr(obj,"reload_force")) return;
 obj->reload(true,true);
}

/** Remove itself from Dict/Array where this property is held (and from document) */
void QSTreeItem::remove() {
 if (nullPtr(obj,"remove")) return;
 try {
  obj->remove();
 } catch (CObjInvalidOperation &e) {
  base->errorException("TreeItem","remove",QObject::tr("Invalid operation"));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("TreeItem","remove",QObject::tr("Document is read-only"));
 }
}

/**
 Return list of all possibly visible childs names of this treeitem.
 Childs that are not shown in treeview (because of treeview settings, etc ...) are not returned.
 Childs that are "not yet known" (unopened references) are not returned too.
 @return list of childs
*/
QStringList QSTreeItem::getChildNames() {
 if (nullPtr(obj,"getChildNames")) return QStringList();
 return obj->getChildNames();
}

/**
 Return parent of this Tree Item, or NULL if this item have not parent,
 or parent is not subclass of TreeItemAbstract
 @return parent of this TreeItem
*/
QSTreeItem* QSTreeItem::parent() {
 if (nullPtr(obj,"parent")) return NULL;
 TreeItemAbstract* parent=dynamic_cast<TreeItemAbstract*>(obj->parent());
 if (!parent) return NULL;
 return dynamic_cast<QSTreeItem*>(QSImporter::createQSObject(parent,base));
}

/**
 Return child of this Tree Item with given name, or NULL if such child does not exist,
 or is not subclass of TreeItemAbstract
 @param name Name of child to get
 @return child of this TreeItem
*/
QSTreeItem* QSTreeItem::child(const QString &name) {
 if (nullPtr(obj,"child")) return NULL;
 TreeItemAbstract* child=dynamic_cast<TreeItemAbstract*>(obj->child(name));
 if (!child) return NULL;
 return dynamic_cast<QSTreeItem*>(QSImporter::createQSObject(child,base));
}

/**
 Return name of this item.
 @return name of this item
*/
QString QSTreeItem::id() {
 if (nullPtr(obj,"id")) return QString::null;
 return obj->name();
}

/**
 Return caption of this item in tree window. Most times it is identical to name, or at least similar.
 @return caption of this item
*/
QString QSTreeItem::text() {
 if (nullPtr(obj,"text")) return QString::null;
 return obj->text(0);
}

/**
 Disable this wrapper.
 Use when the tree item it contains is deleted
 Further usage of wrapper will result in a null pointer error
*/
void QSTreeItem::disable() {
 //Plain and simple
 guiPrintDbg(debug::DBG_DBG,"Disabling tree item " << (intptr_t)this);
 obj=NULL;
}

/**
 Return path of this item (sequence of names from root to this item, separated by slash "/").
 @return path of this item
*/
QString QSTreeItem::path() {
 if (nullPtr(obj,"path")) return QString::null;
 return obj->path();
}

/** get TreeItemAbstract held inside this class. Not exposed to scripting */
TreeItemAbstract* QSTreeItem::get() const {
 return obj;
}

/** destructor */
QSTreeItem::~QSTreeItem() {
 if (obj) {
  //If the wrapper is still valid ... remove it from list
  base->removeTreeItemFromList(this);
 }
}

} // namespace gui
