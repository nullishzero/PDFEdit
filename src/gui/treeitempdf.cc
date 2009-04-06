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
 TreeItemPdf - class holding CPDF in tree, descendant of TreeItemAbstract
 @author Martin Petricek
*/

#include "treeitempdf.h"
#include "qspdf.h"
#include "treedata.h"
#include "treeitem.h"
#include "treeitemdict.h"
#include "treeitemoutline.h"
#include "treeitemobserver.h"
#include "treeitempage.h"
#include "util.h"
#include "pdfutil.h"
#include <kernel/cobject.h>
#include <kernel/cpage.h>
#include <kernel/cpdf.h>
#include <qobject.h>

namespace gui {

//TODO: implement anotations

/** Child type specific for TreeItemPdf*/
enum childType {invalidItem=-1, pageList=1, outlineList, pageItem, outlineItem, dictItem};

using namespace std;
using namespace pdfobjects;

/** constructor of TreeItemPdf - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListView in which to put item
 @param _pdf CPdf Object contained in this item
 @param name Name of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPdf::TreeItemPdf(TreeData *_data,boost::shared_ptr<CPdf> _pdf,Q_ListView *parent,const QString &name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/):TreeItemAbstract("Document",_data,parent,after) {
 assert(_pdf);
 init(_pdf,name);
 reloadSelf();
 observePageDict();
}

/** constructor of TreeItemPdf - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListViewItem which is parent of this object
 @param _pdf CPdf Object contained in this item
 @param name Name of file - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPdf::TreeItemPdf(TreeData *_data,boost::shared_ptr<CPdf> _pdf,Q_ListViewItem *parent,const QString &name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/):TreeItemAbstract("Document",_data,parent,after) {
 init(_pdf,name);
 reloadSelf();
}

/**
 constructor of TreeItemPdf - create special child item of TreeItemPdf
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListViewItem which is parent of this object
 @param name Name (type) of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPdf::TreeItemPdf(TreeData *_data,TreeItemPdf *parent,const QString &name,Q_ListViewItem *after/*=NULL*/):TreeItemAbstract(name,_data,parent,after) {
 initSpec(parent->getObject(),name);
}

/** Initialize item from given CPdf object
 @param pdf CPdf used to initialize this item
 @param name Name of this item - will be shown in treeview (usually name of PDF file)
 */
void TreeItemPdf::init(boost::shared_ptr<CPdf> pdf,const QString &name) {
 obj=pdf;
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 reload(false); //Add all subchilds, etc ...
}

/** Try to install observer on page dictionary - so we will know about any change */
void TreeItemPdf::observePageDict() {
 //Add observer to pagecount
 try {
  assert(obj);
  pageDictionary=pdfobjects::utils::getPageTreeRoot(obj);
  assert(pageDictionary.get());
  if (pageDictionary.get()) {
   //Register observer on page dictionary (because of page count)
   observer=boost::shared_ptr<TreeItemObserver>(new TreeItemObserver(this));
   REGISTER_SHAREDPTR_OBSERVER(pageDictionary, observer);
  }
 } catch (...) {
  guiPrintDbg(debug::DBG_DBG,"Some exception");
  //Some problem, so we have to live without observer
 }
}

/** Uninstall obbserver from page dictionary - if it is installed */
void TreeItemPdf::removeObserver() {
 if (observer.get()) {
  assert(pageDictionary.get());
  //If we have installed observer, we need to deactivate it
  observer->deactivate();
  UNREGISTER_SHAREDPTR_OBSERVER(pageDictionary, observer);
  observer.reset();
 }
}


/**
 Initialize special PDF subitem from given CPdf object and its name (which defines also type of this item)
 @param pdf CPdf used to initialize this item
 @param name Name of this item - will be shown in treeview
 */
void TreeItemPdf::initSpec(boost::shared_ptr<CPdf> pdf,const QString &name) {
 obj=pdf;
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,QObject::tr(name));
 }
 // object type
 setText(1,QObject::tr("List"));
 nType=name;//Set node type
 if (nType=="Outlines") {
  obj->getOutlines(outlines);
 }
 reload(false);//Add all childs
}

/** return CPdf stored inside this item
 @return stored object (CPdf) */
boost::shared_ptr<CPdf> TreeItemPdf::getObject() {
 return obj;
}

/** default destructor */
TreeItemPdf::~TreeItemPdf() {
 removeObserver();
}

//See TreeItemAbstract for description of this virtual method
void TreeItemPdf::reloadSelf() {
 if (nType.isNull()) { ///Not special type
  // object type
  setText(1,QObject::tr("PDF"));
  // Page count
  int count=obj->getPageCount();
  QString pages=util::countString(count,"page","pages");
  setText(2,pages);
 }
 if (nType=="Outlines") {
  obj->getOutlines(outlines);
 }
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemPdf::createChild(const QString &name,ChildType typ,Q_ListViewItem *after/*=NULL*/) {
 if (typ==dictItem) return TreeItem::create(data,this,obj->getDictionary(),QObject::tr("Catalog"));
 if (typ==pageList) return new TreeItemPdf(data,this,QT_TRANSLATE_NOOP("gui::TreeItemPdf","Pages"),after);
 if (typ==outlineList) return new TreeItemPdf(data,this,QT_TRANSLATE_NOOP("gui::TreeItemPdf","Outlines"),after);
 if (typ==pageItem) { //Pages - get page given its number
  //name = Page number
  unsigned int i=name.toUInt();
  guiPrintDbg(debug::DBG_DBG,"Adding page by reload() - " << i);
  try {
   boost::shared_ptr<CPage> ourPage=obj->getPage(i);
   return new TreeItemPage(data,ourPage,this,name,after);
  } catch (...) {
   //Probably a PageNotFoundException
   //so ... someone deleted the page while we were not looking ...
   return NULL;
  }
 }
 if (typ==outlineItem) { //Outlines - get specific outline
  unsigned int i=name.toUInt();
  guiPrintDbg(debug::DBG_DBG,"Adding outline by reload() - " << i);
  return new TreeItemOutline(data,this,outlines[i],name,after);
 }
 assert(0);//Unknown
 return NULL;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemPdf::validChild(const QString &name,Q_ListViewItem *oldChild) {
 TreeItemDict *itc=dynamic_cast<TreeItemDict*>(oldChild);
 if (itc) { //Is a document catalog
  return obj->getDictionary().get()==itc->getObject().get();
 }
 TreeItemPage *itp=dynamic_cast<TreeItemPage*>(oldChild);
 if (itp) { //Is a page
  unsigned int i=name.toUInt();
  try {
   return obj->getPage(i).get()==itp->getObject().get();
  } catch (...) {
   //Probably PageNotFoundException
   //Someone deleted that page
   return false;
  }
 }
 TreeItemOutline *ito=dynamic_cast<TreeItemOutline*>(oldChild);
 if (ito) { //Is an outline
  unsigned int i=name.toUInt();
  return outlines[i].get()==itc->getObject().get();
 }
 //Something else
 return true;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemPdf::deepReload(const QString &childName,Q_ListViewItem *oldItem) {
 TreeItemOutline *ito=dynamic_cast<TreeItemOutline*>(oldItem);
 if (ito) { //Outline
  return false;
 }
 TreeItemDict *itc=dynamic_cast<TreeItemDict*>(oldItem);
 if (itc) { //Is a document catalog
  //If replaced, return success, otherwise failure
  guiPrintDbg(debug::DBG_DBG,"Replacing document dictionary: is_same = " << (obj->getDictionary().get()==itc->getObject().get()));
  return itc->setObject(obj->getDictionary());
 }
 TreeItemPage *itp=dynamic_cast<TreeItemPage*>(oldItem);
 if (itp) { //Is a page
  unsigned int i=childName.toUInt();
  try {
   boost::shared_ptr<CPage> ourPage=obj->getPage(i);
   return itp->setObject(ourPage);
  } catch(...) {
   //Page not found probably, so reloading failed
   return false;
  }
 }
 //Anything else=not supported
 return false;
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemPdf::getChildType(const QString &name) {
 if (nType.isNull()) {//PDF document
  if (name=="Dict") return dictItem;
  if (name=="Pages") return pageList;
  if (name=="Outlines") return outlineList;
  assert(0); //Error
  return invalidItem;
 }
 if (nType=="Pages") return pageItem;
 if (nType=="Outlines") return outlineItem;
 assert(0); //Error
 return invalidItem;
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemPdf::getChildNames() {
 if (nType.isNull()) {//PDF document
  QStringList items;
  if (data->showDict()) items += "Dict";
  if (data->showPage()) items += "Pages";
  if (data->showOutline()) items += "Outlines";
  return items;
 }
 if (nType=="Pages") {//Page list
  return util::countList(obj->getPageCount(),1);
 }
 if (nType=="Outlines") {//Outline list
  try {
   return util::countList(outlines.size());
  } catch(...) {
   guiPrintDbg(debug::DBG_WARN,"Exception in outlines");
   //TODO : some exception, inform
   return QStringList();
  }
 }
 assert(0); //Should not happen
 return QStringList();
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemPdf::haveChild() {
 if (nType.isNull()) {//PDF document
  if (data->showDict()) return true;
  if (data->showPage()) return true;
  if (data->showOutline()) return true;
  return false;
 }
 if (nType=="Pages") {
  unsigned int count=obj->getPageCount();
  return count>0;
 }
 if (nType=="Outlines") {
  return outlines.size()>0;
 }
 assert(0); //Should not happen
 return false;
}

//See TreeItemAbstract for description of this virtual method
void TreeItemPdf::remove() {
 //Cannot remove entire document, just return
 //Removing of special subelements (list of pages, list of outlines) does not make sense too
 return;
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemPdf::getQSObject() {
 return new QSPdf(obj,data->base());
}

} // namespace gui
