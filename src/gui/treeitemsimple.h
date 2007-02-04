/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __TREEITEMSIMPLE_H__
#define __TREEITEMSIMPLE_H__

#include "treeitem.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

/**
 Class holding one simple object in tree (not reference, dict or array)<br>
 Simple types are: int, real, string, name, bool or null
 \brief Tree item containing simple property
*/
class TreeItemSimple : public TreeItem {
public:
 TreeItemSimple(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemSimple(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemSimple();
 virtual bool setObject(boost::shared_ptr<IProperty> newItem);
 //From TreeItemAbstract interface
 virtual bool validChild(const QString &name,QListViewItem *oldChild);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
};

} // namespace gui

#endif
