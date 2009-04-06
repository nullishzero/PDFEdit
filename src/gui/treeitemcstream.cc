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
 TreeItemCStream - class holding one CStream object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include "treeitemcstream.h"
#include "qtcompat.h"
#include "treedata.h"
#include "pdfutil.h"
#include "qsstream.h"
#include "util.h"
#include <kernel/cstream.h>
#include <kernel/iproperty.h>

namespace gui {

using namespace std;
using namespace util;

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListView *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemCStream::TreeItemCStream(TreeData *_data,Q_ListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListViewItem *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemCStream::TreeItemCStream(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemCStream::createChild(const QString &name,__attribute__((unused)) ChildType typ,Q_ListViewItem *after/*=NULL*/) {
 CStream *dict=dynamic_cast<CStream*>(obj.get());
 boost::shared_ptr<IProperty> property=dict->getProperty(convertFromUnicode(name,PDF));
 return TreeItem::create(data,this,property,name,after);
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemCStream::validChild(const QString &name,Q_ListViewItem *oldChild) {
 TreeItem* old=dynamic_cast<TreeItem*>(oldChild);
 if (!old) return false;
 CStream *dict=dynamic_cast<CStream*>(obj.get());
 boost::shared_ptr<IProperty> property=dict->getProperty(convertFromUnicode(name,PDF));
 return (old->getObject()==property);
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemCStream::getChildType(const QString &name) {
 CStream *dict=dynamic_cast<CStream*>(obj.get());
 boost::shared_ptr<IProperty> property=dict->getProperty(convertFromUnicode(name,PDF));
 return property->getType();
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemCStream::getChildNames() {
 QStringList itemList;
 CStream *dict=dynamic_cast<CStream*>(obj.get());
 vector<string> list;
 dict->getAllPropertyNames(list);
 vector<string>::iterator it;
 for( it=list.begin();it!=list.end();++it) { // for each property
  boost::shared_ptr<IProperty> property=dict->getProperty(*it);
  if (!data->showSimple() && isSimple(property)) continue; //simple item -> skip it
  itemList += convertToUnicode(*it,PDF);
 }
 return itemList;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemCStream::haveChild() {
 CStream *dict=dynamic_cast<CStream*>(obj.get());
 return dict->getPropertyCount()>0;
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemCStream::getQSObject() {
 boost::shared_ptr<CStream> stream=boost::dynamic_pointer_cast<CStream>(obj);
 assert(stream.get());
 return new QSStream(stream,data->base());
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemCStream::getQSObject(BaseCore *_base) {
 boost::shared_ptr<CStream> stream=boost::dynamic_pointer_cast<CStream>(obj);
 assert(stream.get());
 //Using shared pointer, so it can be copied safely into another document
 return new QSStream(stream,_base);
}

/**
 Remove property with given name from stream
 @param name Name of property to remove
*/
void TreeItemCStream::remove(const QString &name) {
 boost::shared_ptr<CStream> oDict=boost::dynamic_pointer_cast<CStream>(obj);
 assert(oDict.get());
 guiPrintDbg(debug::DBG_DBG,"Removing from CStream: " << Q_OUT(name));
 TreeItemAbstract* t=dynamic_cast<TreeItemAbstract*>(items[name]);
 if (t) t->unSelect(data->tree());
 oDict->delProperty(convertFromUnicode(name,PDF));
}

/** default destructor */
TreeItemCStream::~TreeItemCStream() {
 uninitObserver();
}


} // namespace gui
