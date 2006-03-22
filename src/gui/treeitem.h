#ifndef __TREEITEM_H__
#define __TREEITEM_H__

#include <iproperty.h>
#include <qlistview.h>

using namespace pdfobjects;

class TreeItem : public QListViewItem {
public:
 TreeItem(QListView *parent, IProperty *pdfObj);
 TreeItem(QListViewItem *parent, IProperty *pdfObj);
 ~TreeItem();
private:
 void init(IProperty *pdfObj);
};

#endif
