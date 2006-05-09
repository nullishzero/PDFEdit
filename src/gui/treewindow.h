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
class Base;

class TreeWindow : public QWidget {
 Q_OBJECT
public:
 void uninit();
 void init(CPdf *pdfDoc,const QString &fileName);
 void init(boost::shared_ptr<IProperty> doc);
 TreeWindow(Base *base,QWidget *parent=0,const char *name=0);
 ~TreeWindow();
 void reloadFrom(TreeItemAbstract *item);
 void reinit();
 QSCObject* getSelected();
 TreeItemAbstract* getSelectedItem();
public slots:
 void settingUpdate(QString key);
signals:
 /** Signal emitted when selecting one of the IProperty objects in the tree window.
  Not emitted on other object types
  Name of object and a pointer to CObject is sent in signal
 */
 void objectSelected(const QString&,boost::shared_ptr<IProperty>);
 /** Signal emitted when selecting any of objects in the tree window.
  You can call getSelected to obtain QSCObject from currentyl selected item
  \see getSelected
 */
 void itemSelected();
 /** Signal emitted when clicked anywhere in tree windows
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
