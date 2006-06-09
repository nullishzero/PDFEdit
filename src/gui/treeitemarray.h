#ifndef __TREEITEMARRAY_H__
#define __TREEITEMARRAY_H__

#include "treeitem.h"
class QString;
class QListView;

namespace gui {

using namespace pdfobjects;

class TreeData;

/**
 class holding one CArray object in tree
 \brief Tree item containing CArray
 */
class TreeItemArray : public TreeItem {
public:
 TreeItemArray(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemArray(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 void remove(unsigned int idx);
 virtual ~TreeItemArray();
 //From TreeItemAbstract interface
 virtual bool validChild(const QString &name,QListViewItem *oldChild);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual QSCObject* getQSObject(BaseCore *_base);
 virtual QSCObject* getQSObject();
};

} // namespace gui

#endif
