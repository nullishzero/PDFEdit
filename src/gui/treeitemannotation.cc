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
 TreeItemAnnotation - class holding one annotation in tree
 @author Martin Petricek
*/

#include "treeitemannotation.h"
#include "treeitem.h"
#include "treeitemdict.h"
#include "treedata.h"
#include "util.h"
#include "pdfutil.h"
#include "qsannotation.h"
#include <kernel/cannotation.h>
#include <kernel/cpage.h>

namespace gui {

using namespace std;
using namespace util;

/**
 constructor of TreeItemAnnotation - root item
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListView which holds this object
 @param pdfObj Annotation contained in this item
 @param _page Page holding this annotation
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
*/
TreeItemAnnotation::TreeItemAnnotation(TreeData *_data,Q_ListView *parent,boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/)
 : TreeItemAbstract(nameId,_data,parent,after) {
 assert(data);
 title=name;
 obj=pdfObj;
 page=_page;
 init();
 reload(false);
}

/**
 constructor of TreeItemAnnotation - child item
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListViewItem which is parent of this object
 @param pdfObj Annotation contained in this item
 @param _page Page holding this annotation
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
*/
TreeItemAnnotation::TreeItemAnnotation(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/)
 : TreeItemAbstract(nameId,_data,parent,after) {
 assert(data);
 title=name;
 obj=pdfObj;
 page=_page;
 init();
 reload(false);
}

/** Initialize the item */
void TreeItemAnnotation::init() {
 QString type=annotTypeName(obj);
 setText(0,title);
 setText(2,type);
}

//See TreeItemAbstract for description of this virtual method
void TreeItemAnnotation::reloadSelf() {
 init();
}

/** default destructor */
TreeItemAnnotation::~TreeItemAnnotation() {
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemAnnotation::getChildType(__attribute__((unused)) const QString &name) {
 return 1;//Just one type : Annotation dictinary
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemAnnotation::createChild(__attribute__((unused)) const QString &name,ChildType typ,Q_ListViewItem *after/*=NULL*/) {
 if (typ==1) { //Return object dictionary
  return TreeItem::create(data,this,obj->getDictionary(),QObject::tr("Dictionary"),after);
 }
 assert(0);
 return NULL;
}

/**
 Return CAnnotation inside this tree items
 @return stored annotation
*/
boost::shared_ptr<CAnnotation> TreeItemAnnotation::getObject() {
 return obj;
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemAnnotation::getChildNames() {
 QStringList childs;
 if (data->showODict()) childs+="Dict";
 return childs;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemAnnotation::haveChild() {
 return data->showODict();
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemAnnotation::getQSObject() {
 return new QSAnnotation(obj,page,data->base());
}

//See TreeItemAbstract for description of this virtual method
void TreeItemAnnotation::remove() {
 //Remove from CPage
 page->delAnnotation(obj);
 return;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemAnnotation::validChild(__attribute__((unused)) const QString &name,Q_ListViewItem *oldChild) {
 TreeItemDict *itp=dynamic_cast<TreeItemDict*>(oldChild);
 if (itp) { //Is a dictionary
  return obj->getDictionary().get()==itp->getObject().get();
 }
 return false;
}

} // namespace gui
