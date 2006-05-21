#ifndef __TREEITEMREF_H__
#define __TREEITEMREF_H__

#include <iproperty.h>
#include <qlistview.h>
#include "treeitem.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;
class TreeWindow;

class TreeItemObserver;//internal class (observer)

class TreeItemRef : public TreeItem {
public:
 TreeItemRef(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemRef(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemRef();
 QString getRef();
 virtual void setOpen(bool open);
 void unOpen();
 bool isComplete();
 //From TreeItemAbstract interface
 virtual bool deepReload(const QString &name,QListViewItem *oldChild);
 virtual bool validChild(const QString &name,QListViewItem *oldChild);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual void reloadSelf();
protected:
 void addData();
private:
 TreeItem* parentCheck();
 /** False, if childs of this items are yet unknown and to be parsed/added, true if child are parsed */
 bool complete;
 /** If this item contains a CRef, this string contain reference target in text form */
 QString selfRef;
};

} // namespace gui

#endif

