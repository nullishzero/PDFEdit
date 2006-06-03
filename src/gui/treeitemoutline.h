#ifndef __TREEITEMOUTLINE_H__
#define __TREEITEMOUTLINE_H__

#include "treeitemdict.h"

class QString;

namespace gui {

using namespace pdfobjects;

/** type representing one outline item */
typedef boost::shared_ptr<IProperty> OutlineItem;

class TreeData;

/**
 Class holding one outline in tree.<br>
 As outline is essentially a dictionary, this item behaves almost like a dictionary item<br>
 Most notable difference is showing the outline title in the tree.
*/
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
