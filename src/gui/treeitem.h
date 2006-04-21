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
 static TreeItem* create(TreeData *_data,QListView *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 static TreeItem* create(TreeData *_data,QListViewItem *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItem* parent();
 virtual ~TreeItem();
 IProperty* getObject();
 void setParent(TreeItem *parent);
 virtual void insertItem(QListViewItem *newChild);
 //From TreeItemAbstract interface
// virtual ChildType getChildType(const QString &name);
// virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
// virtual QStringList getChildNames();
 virtual void reloadSelf();
protected:
 TreeItem(TreeData *_data,QListView *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItem(TreeData *_data,QListViewItem *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 void initObserver();
 void uninitObserver();
protected:
 void init(IProperty *pdfObj,const QString &name);
 /** CObject stored in this TreeItem */
 IProperty *obj;
 /** Type of stored object */
 PropertyType typ;
 /** Parent of this window if it is TreeItem. NULL if no parent or parent is not a TreeItem */
 TreeItem *_parent;
 /** Data from treewindow containing reference to it and other needed data */
 TreeData *data;
 /** Observer registered for this item */
 boost::shared_ptr<TreeItemObserver> observer;
};

} // namespace gui

#endif
