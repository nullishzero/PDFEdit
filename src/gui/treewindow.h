#ifndef __TREEWINDOW_H__
#define __TREEWINDOW_H__

#include <qwidget.h>
#include <cobject.h>
#include <cpdf.h>
class QListView;
class QListViewItem;

namespace gui {

using namespace pdfobjects;

class TreeData;
class TreeItemAbstract;
class QSCObject;

class TreeWindow : public QWidget {
 Q_OBJECT
public:
 void uninit();
 void init(CPdf *pdfDoc,const QString &fileName);
 void init(IProperty *doc);
 TreeWindow(QWidget *parent=0,const char *name=0);
 ~TreeWindow();
 void reloadFrom(TreeItemAbstract *item);
 void reinit();
 QSCObject* getSelected();
public slots:
 void settingUpdate(QString key);
signals:
 /** Signal emitted when selecting one of the IProperty objects in the tree window.
  Not emitted on other object types
  Pointer to CObject is sent in signal
 */
 void objectSelected(IProperty*);
 /** Signal emitted when selecting any of objects in the tree window.
  You can call getSelected to obtain QSCObject from currentyl selected item
  \see getSelected
 */
 void itemSelected();
protected:
 virtual void paintEvent(QPaintEvent *e);
private slots:
 void treeSelectionChanged(QListViewItem *item); 
 void mouseClicked(int button,QListViewItem* item,const QPoint &coord,int column);
private:
 void clear();
 void updateTreeSettings();
private:
 /** Treeview */
 QListView *tree;
 /** Tree data shared with tree items */
 TreeData *data;
 /** Name of root object - if applicable */
 QString rootName;
 /** Root treeitem */
 TreeItemAbstract *root;
 /** Selected treeitem */
 TreeItemAbstract *selected;
};

} // namespace gui

#endif
