#ifndef __TREEITEM_H__
#define __TREEITEM_H__

#include <iproperty.h>
#include <qlistview.h>
#include "treeitemabstract.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;
class TreeWindow;

class TreeItemObserver;//internal class (observer)

class TreeItem : public TreeItemAbstract {
public:
 TreeItem* parent();
 TreeItem(TreeData *_data,QListView *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItem(TreeData *_data,QListViewItem *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 ~TreeItem();
 IProperty* getObject();
 QString getRef();
 virtual void setOpen(bool open);
 void unOpen();
 void setParent(TreeItem *parent);
 bool isComplete();
 virtual void insertItem( QListViewItem * newChild);
 //From TreeItemAbstract interface
 virtual TreeItemAbstract* createChild(const QString &name,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual void reloadSelf();
private:
 void initObserver();
 void uninitObserver();
private:
 TreeItem* parentCheck();
 void init(IProperty *pdfObj,const QString name);
 void addData();
 /** CObject stored in this TreeItem */
 IProperty *obj;
 /** Type of stored object */
 PropertyType typ;
 /** If childs of this items are yet unknown and to be parsed/added */
 bool complete;
 /** If this item contains a CRef, this string contain reference target in text form */
 QString selfRef;
 /** Parent of this window if it is TreeItem. NULL if no parent or parent is not a TreeItem */
 TreeItem *_parent;
 /** Data from treewindow containing reference to it and other needed data */
 TreeData *data;
 /** Observer registered for this item */
 boost::shared_ptr<TreeItemObserver> observer;
};

} // namespace gui

#endif
