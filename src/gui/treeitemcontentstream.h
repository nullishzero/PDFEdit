#ifndef __TREEITEMCONTENTSTREAM_H__
#define __TREEITEMCONTENTSTREAM_H__

#include <iproperty.h>
#include <qlistview.h>
#include "treeitem.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

class TreeItemContentStream : public TreeItemAbstract {
public:
 TreeItemContentStream(QListView *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemContentStream(QListViewItem *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemContentStream();
 //From TreeItemAbstract interface
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual QSCObject* getQSObject();
 virtual void remove();
private:
 void init(const QString &name);
 /**  ContentStream object held in this item */
 boost::shared_ptr<CContentStream> obj;
};

} // namespace gui

#endif
