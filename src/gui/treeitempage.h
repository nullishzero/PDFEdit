/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __TREEITEMPAGE_H__
#define __TREEITEMPAGE_H__

#include "treeitemabstract.h"
#include "treeitempageobserver.h"
#include <boost/shared_ptr.hpp>
#include <vector>
class QString;
namespace pdfobjects {
 class CPage;
 class CContentStream;
 class CAnnotation;
}

namespace gui {

class TreeData;
class QSCObject;
//class TreeItemPageObserver;

using namespace pdfobjects;

/**
 Class holding CPage (one page) in tree
 \brief Tree item containing CPage
*/
class TreeItemPage : public TreeItemAbstract {
public:
 TreeItemPage(TreeData *_data,boost::shared_ptr<CPage> _page,Q_ListView *parent,const QString name=QString::null,Q_ListViewItem *after=NULL);
 TreeItemPage(TreeData *_data,boost::shared_ptr<CPage> _page,Q_ListViewItem *parent,const QString name=QString::null,Q_ListViewItem *after=NULL);
 virtual ~TreeItemPage();
 boost::shared_ptr<CPage> getObject();
 //From TreeItemAbstract interface
 bool setObject(boost::shared_ptr<CPage> newPage);
 virtual bool deepReload(const QString &childName,Q_ListViewItem *oldItem);
 virtual bool validChild(const QString &name,Q_ListViewItem *oldChild);
 virtual QSCObject* getQSObject(BaseCore *_base);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,Q_ListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual void reloadSelf();
 virtual QSCObject* getQSObject();
 virtual void remove();
 void initObserver();
 void uninitObserver();
private:
 void init(boost::shared_ptr<CPage> page,const QString &name);
private:
 /** Observer registered for this item */
 boost::shared_ptr<TreeItemPageObserver> observer;
 /** CPage stored in this TreeItem */
 boost::shared_ptr<CPage> obj;
 /** Vector with content streams */
 std::vector<boost::shared_ptr<CContentStream> > streams;
 /** Vector with anotations */
 std::vector<boost::shared_ptr<CAnnotation> > anots;
};

} // namespace gui

#endif
