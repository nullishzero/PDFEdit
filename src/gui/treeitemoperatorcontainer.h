/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __TREEITEMOPERATORCONTAINER_H__
#define __TREEITEMOPERATORCONTAINER_H__

#include <qlistview.h>
#include "treeitemabstract.h"
#include "types.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

/**
 Tree item holding vector with pdf operators
 The vector is specified in constructor and comes from arbitrary source
 (usually selected objects from page)
 \brief Tree item containing vector of PDF operators
*/
class TreeItemOperatorContainer : public TreeItemAbstract {
public:
 TreeItemOperatorContainer(TreeData *_data,QListView *parent,const OperatorVector &_op,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemOperatorContainer(TreeData *_data,QListViewItem *parent,const OperatorVector &_op,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemOperatorContainer();
 //From TreeItemAbstract interface
 virtual bool validChild(const QString &name,QListViewItem *oldChild);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual QSCObject* getQSObject();
 virtual void remove();
 virtual void reloadSelf();
private:
 void init(const QString &name);
private:
 /** Vector with pdf operators */
 OperatorVector op;

};

} // namespace gui

#endif
