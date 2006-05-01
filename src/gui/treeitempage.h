#ifndef __TREEITEMPAGE_H__
#define __TREEITEMPAGE_H__

#include <qlistview.h>
#include "treeitemabstract.h"
#include <boost/shared_ptr.hpp>
class QString;
namespace pdfobjects {
class CPage;
}

namespace gui {

class TreeData;
class QSCObject;

using namespace pdfobjects;

class TreeItemPage : public TreeItemAbstract {
public:
 TreeItemPage(TreeData *_data,boost::shared_ptr<CPage> _page,QListView *parent,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItemPage(TreeData *_data,boost::shared_ptr<CPage> _page,QListViewItem *parent,const QString name=QString::null,QListViewItem *after=NULL);
 virtual ~TreeItemPage();
 boost::shared_ptr<CPage> getObject();
 //From TreeItemAbstract interface
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual void reloadSelf();
 virtual QSCObject* getQSObject();
private:
 void init(boost::shared_ptr<CPage> page,const QString &name);
 /** CPage stored in this TreeItem */
 boost::shared_ptr<CPage> obj;
 /** Data from treewindow containing reference to it and other needed data */
 TreeData *data;
};

} // namespace gui

#endif
