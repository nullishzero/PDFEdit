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
 TreeItemDict - class holding one CDict object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include "treeitemdict.h"
#include "qtcompat.h"
#include <kernel/cobject.h>
#include "treedata.h"
#include "pdfutil.h"
#include "qsdict.h"
#include "util.h"

namespace gui {

using namespace std;
using namespace util;

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListView *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemDict::TreeItemDict(TreeData *_data,Q_ListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListViewItem *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemDict::TreeItemDict(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemDict::createChild(const QString &name,__attribute__((unused)) ChildType typ,Q_ListViewItem *after/*=NULL*/) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 try {
  boost::shared_ptr<IProperty> property=dict->getProperty(convertFromUnicode(name,util::PDF));
  return TreeItem::create(data,this,property,name,after);
 } catch (...) {
  //Should never happen, unless something else is seriously broken
  guiPrintDbg(debug::DBG_ERR,"Broken code: failure to get property");
  return NULL;
 }
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemDict::getChildType(const QString &name) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 try {
  boost::shared_ptr<IProperty> property=dict->getProperty(convertFromUnicode(name,util::PDF));
  return property->getType();
 } catch (...) {
  //Should never happen, unless something else is seriously broken
  guiPrintDbg(debug::DBG_ERR,"Broken code: failure to get property");
  return 0;//whatever ... it will fail again later when creating the child
 }
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemDict::getChildNames() {
 QStringList itemList;
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 vector<string> list;
 dict->getAllPropertyNames(list);
 vector<string>::iterator it;
 for( it=list.begin();it!=list.end();++it) { // for each property
  boost::shared_ptr<IProperty> property=dict->getProperty(*it);
  if (!data->showSimple() && isSimple(property)) continue; //simple item -> skip it
  itemList += convertToUnicode(*it,util::PDF);
 }
 if (data->sortDict()) {
  //Sort the keys
  itemList.sort();
 }
 return itemList;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemDict::haveChild() {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 return dict->getPropertyCount()>0;
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemDict::getQSObject() {
 boost::shared_ptr<CDict> dict=boost::dynamic_pointer_cast<CDict>(obj);
 assert(dict.get());
 return new QSDict(dict,data->base());
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemDict::getQSObject(BaseCore *_base) {
 boost::shared_ptr<CDict> dict=boost::dynamic_pointer_cast<CDict>(obj);
 assert(dict.get());
 //Using shared pointer, so it can be copied safely into another document
 return new QSDict(dict,_base);
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemDict::validChild(const QString &name,Q_ListViewItem *oldChild) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 try {
  boost::shared_ptr<IProperty> property=dict->getProperty(convertFromUnicode(name,util::PDF));
  TreeItem *it=dynamic_cast<TreeItem*>(oldChild);
  assert(it);
  if (!it) return false;//Probably error on unknown child
  //Same address = same item
  //Different address = probably different item
  return property.get()==it->getObject().get();
 } catch (...) {
  //Should never happen, unless something else is seriously broken
  guiPrintDbg(debug::DBG_ERR,"Broken code: failure to get property");
  return false;
 }
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemDict::deepReload(const QString &childName,Q_ListViewItem *oldItem) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 TreeItem *it=dynamic_cast<TreeItem*>(oldItem);
 if (it) { //Is an IProperty
  try {
   boost::shared_ptr<IProperty> property=dict->getProperty(convertFromUnicode(childName,util::PDF));
   //If replaced, return success, otherwise failure
   return it->setObject(property);
  } catch (...) {
   //Should never happen, unless something else is seriously broken
   guiPrintDbg(debug::DBG_ERR,"Broken code: failure to get property");
   return false;
  }
 }
 //Anything else=not supported
 return false;
}

/**
 Remove property with given name from dictionary
 @param name Name of property to remove
*/
void TreeItemDict::remove(const QString &name) {
 boost::shared_ptr<CDict> oDict=boost::dynamic_pointer_cast<CDict>(obj);
 assert(oDict.get());
 guiPrintDbg(debug::DBG_DBG,"Removing from dict: " << Q_OUT(name));
 TreeItemAbstract* t=dynamic_cast<TreeItemAbstract*>(items[name]);
 if (t) t->unSelect(data->tree());
 oDict->delProperty(convertFromUnicode(name,util::PDF));
}

/** default destructor */
TreeItemDict::~TreeItemDict() {
 uninitObserver();
}

} // namespace gui
