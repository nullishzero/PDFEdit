/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __TREEITEMANNOTATIONCONTAINER_H__
#define __TREEITEMANNOTATIONCONTAINER_H__

#include "treeitemabstract.h"
#include "types.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

/**
 Tree item holding vector with Annotations
 The vector is specified in constructor and comes from arbitrary source
 (usually selected objects from page)
 \brief Tree item containing vector of Annotations
*/
class TreeItemAnnotationContainer : public TreeItemAbstract {
public:
 TreeItemAnnotationContainer(TreeData *_data,Q_ListView *parent,const AnnotationVector &_op,boost::shared_ptr<CPage> _page,const QString name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemAnnotationContainer(TreeData *_data,Q_ListViewItem *parent,const AnnotationVector &_op,boost::shared_ptr<CPage> _page,const QString name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemAnnotationContainer();
 //From TreeItemAbstract interface
 virtual bool validChild(const QString &name,Q_ListViewItem *oldChild);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,Q_ListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual QSCObject* getQSObject();
 virtual void remove();
 virtual void reloadSelf();
private:
 void init(const QString &name);
private:
 /** Vector with annotations */
 AnnotationVector op;
 /** CPage containing these annotation */
 boost::shared_ptr<CPage> page;
};

} // namespace gui

#endif
