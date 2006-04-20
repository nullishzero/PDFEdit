#ifndef __TREEITEMPAGE_H__
#define __TREEITEMPAGE_H__

#include <qlistview.h>
#include "treeitemabstract.h"
class QString;
namespace pdfobjects {
class CPage;
}

namespace gui {

class TreeData;

using namespace pdfobjects;

class TreeItemPage : public TreeItemAbstract {
public:
 TreeItemPage(TreeData *_data,CPage *_page,QListView *parent,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItemPage(TreeData *_data,CPage *_page,QListViewItem *parent,const QString name=QString::null,QListViewItem *after=NULL);
 ~TreeItemPage();
 CPage* getObject();
 //From TreeItemAbstract interface
 virtual TreeItemAbstract* createChild(const QString &name,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual void reloadSelf();
private:
 void init(CPage *page,const QString &name);
 /** CPage stored in this TreeItem */
 CPage *obj;
 /** Data from treewindow containing reference to it and other needed data */
 TreeData *data;
};

} // namespace gui

#endif
