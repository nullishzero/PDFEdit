/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
/** @file
 TreeItemSimple - class holding one simple object in tree (not ref, dict or array), descendant of TreeItem
 @author Martin Petricek
*/

#include <kernel/cobject.h>
#include "treeitemsimple.h"
#include "util.h"
#include "pdfutil.h"

namespace gui {

class TreeData;

using namespace std;

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListView *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemSimple::TreeItemSimple(TreeData *_data,Q_ListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
}

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListViewItem *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemSimple::TreeItemSimple(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
}

/** default destructor */
TreeItemSimple::~TreeItemSimple() {
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemSimple::createChild(__attribute__((unused)) const QString &name,ChildType typ,__attribute__((unused)) Q_ListViewItem *after/*=NULL*/) {
 assert(typ!=pRef);//Must not be called on complex childs
 assert(typ!=pDict);
 assert(typ!=pArray);
 assert(0);//Simple tree items have no childs
 return NULL;
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemSimple::getChildType(__attribute__((unused)) const QString &name) {
 assert(typ!=pRef);//Must not be called on complex childs
 assert(typ!=pDict);
 assert(typ!=pArray);
 assert(0);//Simple tree items have no childs
 return 0;
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemSimple::getChildNames() {
 assert(typ!=pRef);//Must not be called on complex childs
 assert(typ!=pDict);
 assert(typ!=pArray);
 //Null, Bool, Int, Real, Name, String -> These are simple types without any children
 return QStringList(); 
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemSimple::haveChild() {
 return false;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemSimple::validChild(__attribute__((unused)) const QString &name,__attribute__((unused)) Q_ListViewItem *oldChild) {
 //no childs
 return false;
}

/** \copydoc TreeItem::setObject */
bool TreeItemSimple::setObject(boost::shared_ptr<IProperty> newItem) {
 //Do not check type. Simple type can be replaced safely with different simple type
 if (haveObserver()) {
  uninitObserver();
  obj=newItem;
  initObserver();
 } else {
  obj=newItem;
 }
 //Need to update the type, as it may have changed
 setText(1,util::getTypeName(obj));
 return true;
}

} // namespace gui
