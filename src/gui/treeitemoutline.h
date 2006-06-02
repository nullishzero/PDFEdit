#ifndef __TREEITEMOUTLINE_H__
#define __TREEITEMOUTLINE_H__

#include <iproperty.h>
#include <qlistview.h>
#include "treeitemdict.h"

class QString;

namespace gui {

using namespace pdfobjects;

typedef boost::shared_ptr<IProperty> OutlineItem;

class TreeData;

class TreeItemOutline : public TreeItemDict {
public:
 TreeItemOutline(TreeData *_data,QListView *parent,OutlineItem pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemOutline(TreeData *_data,QListViewItem *parent,OutlineItem pdfObj,const QString name=QString::null,QListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemOutline();
 //From TreeItemAbstract interface
 virtual void reloadSelf();
private:
 void showTitle();
private:
 /** outline title */
 QString title;
};

} // namespace gui

#endif
