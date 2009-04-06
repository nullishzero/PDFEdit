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
#ifndef __TREEITEM_H__
#define __TREEITEM_H__

#include <kernel/iproperty.h>
#include "treeitemabstract.h"

class QString;

namespace gui {

using namespace pdfobjects;

class TreeWindow;
class TreeItemObserver;

/**
 Class holding one PDF object (IProperty) in tree
 \brief Tree item containing IProperty
 */
class TreeItem : public TreeItemAbstract {
public:
 static TreeItem* create(TreeData *_data,Q_ListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=QString::null);
 static TreeItem* create(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=QString::null);
 TreeItem* parent();
 virtual ~TreeItem();
 boost::shared_ptr<IProperty> getObject();
 void setParent(TreeItem *parent);
 virtual bool setObject(boost::shared_ptr<IProperty> newItem);
 virtual void insertItem(Q_ListViewItem *newChild);
 void unSelect();
 //From TreeItemAbstract interface
 virtual void reloadSelf();
 virtual QSCObject* getQSObject();
 virtual QSCObject* getQSObject(BaseCore *_base);
 virtual void remove();
protected:
 TreeItem(const QString &nameId,TreeData *_data,Q_ListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name=QString::null,Q_ListViewItem *after=NULL);
 TreeItem(const QString &nameId,TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString &name=QString::null,Q_ListViewItem *after=NULL);
 void initObserver();
 void uninitObserver();
 bool haveObserver();
protected:
 void init(boost::shared_ptr<IProperty> pdfObj,const QString &name);
 /** CObject stored in this TreeItem */
 boost::shared_ptr<IProperty> obj;
 /** Type of stored object */
 PropertyType typ;
 /** Parent of this window if it is TreeItem. NULL if no parent or parent is not a TreeItem */
 TreeItem *_parent;
 /** Observer registered for this item */
 boost::shared_ptr<TreeItemObserver> observer;
};

} // namespace gui

#endif
