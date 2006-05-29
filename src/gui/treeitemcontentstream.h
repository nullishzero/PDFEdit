#ifndef __TREEITEMCONTENTSTREAM_H__
#define __TREEITEMCONTENTSTREAM_H__

#include <iproperty.h>
#include <qlistview.h>
#include <ccontentstream.h>
#include "treeitem.h"
class QString;

namespace gui {

using namespace pdfobjects;

class TreeData;

class TreeItemContentStream : public TreeItemAbstract {
public:
 boost::shared_ptr<CContentStream> getObject();
 TreeItemContentStream(TreeData *_data,QListView *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemContentStream(TreeData *_data,QListViewItem *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemContentStream();
 virtual void setOpen(bool open);
 //From TreeItemAbstract interface
 virtual ChildType getChildType(const QString &name);
 virtual TreeItemAbstract* createChild(const QString &name,ChildType typ,QListViewItem *after=NULL);
 virtual QStringList getChildNames();
 virtual bool haveChild();
 virtual QSCObject* getQSObject();
 virtual void remove();
 virtual void reloadSelf();
private:
 void init(const QString &name);
 /**  ContentStream object held in this item */
 boost::shared_ptr<CContentStream> obj;
 /** Vector with pdf operators */
 std::vector<boost::shared_ptr<PdfOperator> > op;
};

} // namespace gui

#endif
