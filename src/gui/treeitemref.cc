/** @file
 TreeItemRef - class holding one CRef object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitemref.h"
#include "treedata.h"
#include "pdfutil.h"
#include "util.h"

namespace gui {

using namespace std;
using namespace util;

/**
 @copydoc TreeItem(const QString&,TreeData *,QListView *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemRef::TreeItemRef(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 complete=false;
 addData();
 reload(false);
 initObserver();
}

/**
@copydoc TreeItem(const QString&,TreeData *,QListViewItem *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemRef::TreeItemRef(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
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
 //TODO: better dynamic generation, using reload() and TreeItemAbstract
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
 QListViewItem::setOpen(false);
}

/** Slot that will be called when item si opened/closed
 @param open True if item is being opened, false if closed
*/
void TreeItemRef::setOpen(bool open) {
 assert(typ==pRef); //paranoid check
 if (!complete) { //not expanded
  if (open) {
   guiPrintDbg(debug::DBG_DBG," Opening referenced property pREF : " << selfRef);
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
    QListViewItem::setOpen(open);
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
 //TODO: if closing, delete childs?

 //Call original method
 QListViewItem::setOpen(open);
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
 guiPrintDbg(debug::DBG_DBG,"This item will now reload data " << getTypeName(obj));
 QString old=selfRef;
 //Update reference target
 addData();
 if (old!=selfRef) {
  //Remove old reference from list of opened and available items
  if (complete) data->remove(old);//Was complete -> remove data
  guiPrintDbg(debug::DBG_DBG,"Reference target changed: " << old << " -> " << selfRef);
  //Close itself
  this->setOpen(false);
  //Set as incomplete
  complete=false;
  //delete all childs - just in case
  eraseItems();
 }
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemRef::createChild(const QString &name,__attribute__((unused)) ChildType typ,QListViewItem *after/*=NULL*/) {
 assert(name=="Target");
 QString s;
 CPdf* pdf=obj->getPdf();
 if (!pdf) return NULL; //No document opened -> cannot parse references
                   //Should happen only while testing
 CRef* cref=dynamic_cast<CRef*>(obj.get());
 IndiRef ref;
 cref->getPropertyValue(ref);
 guiPrintDbg(debug::DBG_DBG," LOADING referenced property: " << ref.num << "," << ref.gen);
 boost::shared_ptr<IProperty> rp=pdf->getIndirectProperty(ref);
 return TreeItem::create(data,this, rp,s.sprintf("<%d,%d>",ref.num,ref.gen),after,"Target");
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemRef::getChildType(const QString &name) {
 assert(name=="Target");
 CPdf* pdf=obj->getPdf();
 if (!pdf) return -1; //No document opened -> cannot parse references
                      //Should happen only while testing
 CRef* cref=dynamic_cast<CRef*>(obj.get());
 IndiRef ref;
 cref->getPropertyValue(ref);
 guiPrintDbg(debug::DBG_DBG," LOADING referenced property: " << ref.num << "," << ref.gen);
 boost::shared_ptr<IProperty> rp=pdf->getIndirectProperty(ref);
 return rp->getType();
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemRef::getChildNames() {
 if (!complete) return QStringList(); //Childs not loaded yet
 return QStringList("Target");
}

} // namespace gui
