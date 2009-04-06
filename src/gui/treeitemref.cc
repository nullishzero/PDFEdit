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
 TreeItemRef - class holding one CRef object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include "treeitemref.h"
#include "qtcompat.h"
#include "treedata.h"
#include "pdfutil.h"
#include "util.h"
#include <kernel/cobject.h>
#include <kernel/cpdf.h>

namespace gui {

using namespace std;
using namespace util;
using namespace pdfobjects;

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListView *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemRef::TreeItemRef(TreeData *_data,Q_ListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 complete=false;
 addData();
 reload(false);
 initObserver();
}

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListViewItem *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemRef::TreeItemRef(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 complete=false;
 addData();
 reload(false);
 initObserver();
}

/** Checks if any parent is not CRef referencing the same CObject as this item
 @return TreeItem with same reference as this TreeItem or NULL if no such item found */
TreeItem* TreeItemRef::parentCheck() {
 TreeItem* par=_parent;
 while (par) {
  TreeItemRef *parRef=dynamic_cast<TreeItemRef*>(par);
  if (parRef && parRef->getRef()==selfRef) return par;
  par=par->parent();
 }
 return NULL;
}

/** Add some info into third column of treeview (mostly value) */
void TreeItemRef::addData() {
 QString s;
 IndiRef iref=util::getRef(obj);
 selfRef=s.sprintf("%d,%d",iref.num,iref.gen);
 setText(2,QString("-> ")+selfRef);
 setExpandable(true);
}

/**
 Return reference target in text form.
 @return Refence target
*/
QString TreeItemRef::getRef() {
 return selfRef;
}

/** Remove itself from list of opened references and close itself */
void TreeItemRef::unOpen() {
 assert(typ==pRef); //paranoid check
 data->remove(this);
 complete=false;
 TreeItem::setOpen(false);
}

/**
 Slot that will be called when item is opened/closed
 @param open True if item is being opened, false if closed
*/
void TreeItemRef::setOpen(bool open) {
 assert(typ==pRef); //paranoid check
 if (!complete) { //not expanded
  if (open) {
   guiPrintDbg(debug::DBG_DBG," Opening referenced property pREF : " << Q_OUT(selfRef));
   TreeItem *up=parentCheck();
   if (up) { //some parent of this object is referencing the same object
    guiPrintDbg(debug::DBG_DBG,"Found ref in parent - not expanding item (-> infinite recursion)");
    data->tree()->setCurrentItem(up);
    data->tree()->ensureItemVisible(up);
    //TODO: nefunguje pri otevreni pomoci sipky.
    return;//Do not expand references and keep closed
   }
   TreeItemRef *other=data->find(selfRef);
   if (other && other!=this) { //subtree already found elsewhere -> reparent
    guiPrintDbg(debug::DBG_DBG,"Will relocate child, counts: "<< this->childCount() << " , " << other->childCount());
    moveAllChildsFrom(other);
    guiPrintDbg(debug::DBG_DBG,"Done relocate child, counts: "<< this->childCount() << " , " << other->childCount());
    other->unOpen();
    data->add(this);//re-add itself
    complete=true;
    TreeItem::setOpen(open);
    return;//Do not expand references
   } else { // subtree not found -> add to list
    guiPrintDbg(debug::DBG_DBG,"Subtree not found");
    data->add(this);
   }
   //Reload as "complete and opened item"
   complete=true;
   reload();
  }
 }
 //Call original method
 TreeItem::setOpen(open);
}

/** Check if childs of this items are yet unknown and to be parsed/added
 @return true if item is expanded, false if it is incomplete */
bool TreeItemRef::isComplete() {
 return complete;
}

/** default destructor */
TreeItemRef::~TreeItemRef() {
 uninitObserver();
 data->remove(this);
}

//See TreeItemAbstract for description of this virtual method
void TreeItemRef::reloadSelf() {
 guiPrintDbg(debug::DBG_DBG,"This item will now reload data " << Q_OUT(getTypeName(obj)));
 QString old=selfRef;
 //Update reference target
 addData();
 if (old!=selfRef) {
  //Remove old reference from list of opened and available items
  if (complete) data->remove(old);//Was complete -> remove data
  guiPrintDbg(debug::DBG_DBG,"Reference target changed: " << Q_OUT(old) << " -> " << Q_OUT(selfRef));
  //Close itself
  this->setOpen(false);
  //Set as incomplete
  complete=false;
  //delete all childs - just in case
  eraseItems();
 }
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemRef::createChild(const QString &name,__attribute__((unused)) ChildType typ,Q_ListViewItem *after/*=NULL*/) {
 assert(name=="Target");
 QString s;
 boost::shared_ptr<CPdf> pdf=obj->getPdf().lock();
 if (!pdf) return NULL; //No document opened -> cannot parse references
                   //Should happen only while testing
 CRef* cref=dynamic_cast<CRef*>(obj.get());
 IndiRef ref;
 cref->getValue(ref);
 guiPrintDbg(debug::DBG_DBG," LOADING referenced property: " << ref.num << "," << ref.gen);
 boost::shared_ptr<IProperty> rp=pdf->getIndirectProperty(ref);
 return TreeItem::create(data,this, rp,s.sprintf("<%d,%d>",ref.num,ref.gen),after,"Target");
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemRef::validChild(__attribute__((unused)) const QString &name,Q_ListViewItem *oldChild) {
 boost::shared_ptr<CPdf> pdf=obj->getPdf().lock();
 if (!pdf) return false; //No document opened -> cannot parse references
 IndiRef ref;
 IProperty::getSmartCObjectPtr<CRef>(obj)->getValue(ref);
 boost::shared_ptr<IProperty> rp=pdf->getIndirectProperty(ref);
 TreeItem* it=dynamic_cast<TreeItem*>(oldChild);
 if (!it) return false;//Should not happen
 return rp.get()==it->getObject().get();
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemRef::deepReload(__attribute__((unused)) const QString &childName,Q_ListViewItem *oldItem) {
 boost::shared_ptr<CPdf> pdf=obj->getPdf().lock();
 if (!pdf) return false; //No document opened -> cannot parse references
 CRef* cref=dynamic_cast<CRef*>(obj.get());
 IndiRef ref;
 cref->getValue(ref);
 boost::shared_ptr<IProperty> rp=pdf->getIndirectProperty(ref);
 TreeItem* it=dynamic_cast<TreeItem*>(oldItem);
 if (!it) return false;//Should not happen
 it->setObject(rp);
 return true;
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemRef::getChildType(const QString &name) {
 assert(name=="Target");
 boost::shared_ptr<IProperty> rp=dereference(obj);
 if (!rp.get()) return -1; //No document opened -> cannot parse references (Should happen only while testing)
 return rp->getType();
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemRef::getChildNames() {
 if (!complete) return QStringList(); //Childs not loaded yet
 return QStringList("Target");
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemRef::haveChild() {
 return true;
}

} // namespace gui
