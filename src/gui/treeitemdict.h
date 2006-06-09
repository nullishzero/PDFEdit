#ifndef __TREEITEMDICT_H__
#define __TREEITEMDICT_H__

#include "treeitem.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

/**
 class holding one CDict object in tree (dictionary)
 \brief Tree item containing CDict
 */
class TreeItemDict : public TreeItem {
public:
 void remove(const QString &name);
 TreeItemDict(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemDict(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemDict();
 //From TreeItemAbstract interface
 virtual bool validChild(const QString &name,QListViewItem *oldChild);
 virtual bool deepReload(const QString &childName,QListViewItem *oldItem);
 virtual QSCObject* getQSObject(BaseCore *_base);
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual QSCObject* getQSObject();
};

} // namespace gui

#endif
