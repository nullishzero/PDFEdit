#ifndef __TREEITEMARRAY_H__
#define __TREEITEMARRAY_H__

#include <iproperty.h>
#include <qlistview.h>
#include "treeitem.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

class TreeItemArray : public TreeItem {
public:
 TreeItemArray(TreeData *_data,QListView *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 TreeItemArray(TreeData *_data,QListViewItem *parent,IProperty *pdfObj,const QString name=QString::null,QListViewItem *after=NULL);
 virtual ~TreeItemArray();
 //From TreeItemAbstract interface
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
};

} // namespace gui

#endif
