#ifndef __TREEITEMPAGE_H__
#define __TREEITEMPAGE_H__

#include <qlistview.h>
class QString;
namespace pdfobjects {
class CPage;
}

namespace gui {

class TreeData;

using namespace pdfobjects;

class TreeItemPage : public QListViewItem {
public:
 TreeItemPage(TreeData *_data,CPage *_page,QListView *parent,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItemPage(TreeData *_data,CPage *_page,QListViewItem *parent,const QString name=QString::null,QListViewItem *after=NULL);
 ~TreeItemPage();
 CPage* getObject();
private:
 void init(CPage *page,const QString &name);
 /** CPage stored in this TreeItem */
 CPage *obj;
 /** Data from treewindow containing reference to it and other needed data */
 TreeData *data;
};

} // namespace gui

#endif
