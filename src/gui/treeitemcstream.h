#ifndef __TREEITEMCSTREAM_H__
#define __TREEITEMCSTREAM_H__

#include <iproperty.h>
#include <qlistview.h>
#include "treeitem.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

class TreeItemCStream : public TreeItemAbstract {
public:
 TreeItemCStream(QListView *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemCStream(QListViewItem *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemCStream();
 //From TreeItemAbstract interface
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual QSCObject* getQSObject();
private:
 void init(const QString &name);
 /**  ContentStream object held in this item */
 boost::shared_ptr<CContentStream> obj;
};

} // namespace gui

#endif
