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
 TreeItemOperatorContainer - class holding vector with pdf operators
 @author Martin Petricek
*/

#include <kernel/cobject.h>
#include "treeitemoperatorcontainer.h"
#include "treeitempdfoperator.h"
#include "multitreewindow.h"
#include "treedata.h"
#include "util.h"

namespace gui {

class TreeData;

using namespace std;

/**
 constructor of TreeItemOperatorContainer - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListView in which to put this item
 @param _op Vector with operators
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemOperatorContainer::TreeItemOperatorContainer(TreeData *_data,Q_ListView *parent,const OperatorVector &_op,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 op=_op;
 init(name);
 reload();
}

/**
 constructor of TreeItemOperatorContainer - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListViewItem under which to put this item
 @param _op Vector with operators
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemOperatorContainer::TreeItemOperatorContainer(TreeData *_data,Q_ListViewItem *parent,const OperatorVector &_op,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 op=_op;
 init(name);
 reload();
}

/**
 initialize captions of this item from name
 @param name Caption of item
 */
void TreeItemOperatorContainer::init(const QString &name) {
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,QObject::tr("Operators"));
 setText(2,"");
}

/** default destructor */
TreeItemOperatorContainer::~TreeItemOperatorContainer() {
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemOperatorContainer::createChild(const QString &name,__attribute__((unused)) ChildType typ,Q_ListViewItem *after/*=NULL*/) {
 size_t position=name.toUInt();
 return new TreeItemPdfOperator(data,this,op[position],name,after);
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemOperatorContainer::validChild(const QString &name,Q_ListViewItem *oldChild) {
 TreeItemPdfOperator* oper=dynamic_cast<TreeItemPdfOperator*>(oldChild);
 if (!oper) return false;
 size_t position=name.toUInt();
 return (oper->getObject()==op[position]);
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemOperatorContainer::getChildType(__attribute__((unused)) const QString &name) {
 return 1;//Just one type : PDF Operator
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemOperatorContainer::getChildNames() {
 return util::countList(op.size());
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemOperatorContainer::getQSObject() {
 return NULL;//TODO: return something reasonable
}

//See TreeItemAbstract for description of this virtual method
void TreeItemOperatorContainer::remove() {
 // Do nothing, it doesn't make sense to delete this container
 return;
}

//See TreeItemAbstract for description of this virtual method
void TreeItemOperatorContainer::reloadSelf() {
 //We have operators already inside, so do nothing
 return;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemOperatorContainer::haveChild() {
 return op.size()>0;
}


} // namespace gui
