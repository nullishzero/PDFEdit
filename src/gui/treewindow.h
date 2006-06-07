#ifndef __TREEWINDOW_H__
#define __TREEWINDOW_H__

#include <qwidget.h>
#include <cobject.h>
#include "types.h"
class QListView;
class QListViewItem;
namespace pdfobjects {
 class CPdf;
 class CContentStream;
}

namespace gui {

class DragListView;
class TreeData;
class MultiTreeWindow;
class TreeItemAbstract;
class QSCObject;
class Base;

/**
 Class providing treeview of PDF objects<br>
 It can contain items of TreeItemAbstract class, which represent objects in PDF file
 (actually its subclasses, as that class is abstract)
*/
class TreeWindow : public QWidget {
 Q_OBJECT
public:
 void uninit();
 void init(pdfobjects::CPdf *pdfDoc,const QString &fileName);
 void init(const OperatorVector &vec,const QString &pName=QString::null);
 void init(boost::shared_ptr<pdfobjects::IProperty> doc,const QString &pName=QString::null);
 void init(boost::shared_ptr<pdfobjects::CContentStream> cs,const QString &pName=QString::null);
 TreeWindow(MultiTreeWindow *multi,Base *base,QWidget *parent=0,const char *name=0);
 ~TreeWindow();
 void reloadFrom(TreeItemAbstract *item);
 TreeItemAbstract* root();
 void reload();
 QSCObject* getSelected();
 TreeItemAbstract* getSelectedItem();
 void deleteNotify(TreeItemAbstract *notifyItem);
public slots:
 void settingUpdate(QString key);
signals:
 /**
  Signal emitted when selection in the tree window is changes
  (and another object(s) is now selected).
  You can call getSelected to obtain QSCObject from currently selected item
  \see getSelected
 */
 void itemSelected();
 /**
  Signal emitted when clicks anywhere in tree windows
  Button used to click is sent as first parameter (1=left, 2=right, 4=middle, 8=doubleclick with left)
  If clicked on tree item, it is sent in second parameter (otherwise NULL is sent)
 */
 void treeClicked(int,QListViewItem*);
protected:
 virtual void paintEvent(QPaintEvent *e);
private slots:
 void treeSelectionChanged(QListViewItem *item); 
 void mouseClicked(int button,QListViewItem* item,const QPoint &coord,int column);
 void mouseDoubleClicked(QListViewItem* item,const QPoint &coord,int column);
private:
 void clear();
 void updateTreeSettings();
private:
 /** Treeview */
#ifdef DRAGDROP
 DragListView *tree;
#else
 QListView *tree;
#endif
 /** Tree data shared with tree items */
 TreeData *data;
 /** Name of root object - if applicable */
 QString rootName;
 /** Root treeitem */
 //TreeItemAbstract *rootItem;
 /** Selected treeitem */
 //TreeItemAbstract *selected;
};

} // namespace gui

#endif
