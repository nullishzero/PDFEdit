#ifndef __TREEITEMPAGE_H__
#define __TREEITEMPAGE_H__

#include <qlistview.h>
#include "treeitemabstract.h"
#include <boost/shared_ptr.hpp>

class QString;
namespace pdfobjects {
 class CPage;
 class CContentStream;
}

namespace gui {

class TreeData;
class QSCObject;

using namespace pdfobjects;

/**
 Class holding CPage (one page) in tree
*/
class TreeItemPage : public TreeItemAbstract {
public:
 TreeItemPage(TreeData *_data,boost::shared_ptr<CPage> _page,QListView *parent,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItemPage(TreeData *_data,boost::shared_ptr<CPage> _page,QListViewItem *parent,const QString name=QString::null,QListViewItem *after=NULL);
 virtual ~TreeItemPage();
 boost::shared_ptr<CPage> getObject();
 //From TreeItemAbstract interface
 bool setObject(boost::shared_ptr<CPage> newPage);
 virtual bool deepReload(const QString &childName,QListViewItem *oldItem);
 virtual bool validChild(const QString &name,QListViewItem *oldChild);
 virtual QSCObject* getQSObject(Base *_base);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual void reloadSelf();
 virtual QSCObject* getQSObject();
 virtual void remove();
private:
 void init(boost::shared_ptr<CPage> page,const QString &name);
 /** CPage stored in this TreeItem */
 boost::shared_ptr<CPage> obj;
 /** Vector with content streams */
 std::vector<boost::shared_ptr<CContentStream> > streams;
};

} // namespace gui

#endif
