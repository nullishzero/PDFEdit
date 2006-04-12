#ifndef __TREEITEM_H__
#define __TREEITEM_H__

#include <iproperty.h>
#include <qlistview.h>

namespace gui {

using namespace pdfobjects;

class TreeWindow;

class TreeItem : public QListViewItem {
public:
 TreeItem(TreeWindow *_tree,QListView *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItem(TreeWindow *_tree,QListViewItem *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 ~TreeItem();
 IProperty* getObject();
 virtual void setOpen(bool open);
private:
 void init(IProperty *pdfObj,const QString name);
 void addData();
 /** CObject stored in this TreeItem */
 IProperty *obj;
 /** Type of stored object */
 PropertyType typ;
 /** reference to tree window*/
 TreeWindow *tree;
 /** If childs of this items are yet unknown and to be parsed/added */
 bool complete;
};

} // namespace gui

#endif
