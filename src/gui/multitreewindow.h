#ifndef __MULTITREEWINDOW_H__
#define __MULTITREEWINDOW_H__

#include <qwidget.h>
#include <cobject.h>
#include <ccontentstream.h>
#include <cpdf.h>

class QListViewItem;
class QTabWidget;
class QButton;

namespace gui {
 
using namespace pdfobjects;

class TreeData;
class TreeWindow;
class TreeItemAbstract;
class QSCObject;
class Base;

/** Mapping from void* to associated Tree Window */
typedef QMap<void*,TreeWindow*> TreeWindowList;

/** Mapping from Tree Window to void* associated with them (reverse mapping) */
typedef QMap<TreeWindow*,void*> TreeWindowListReverse;

class MultiTreeWindow : public QWidget {
 Q_OBJECT
public:
 void uninit();
 void init(CPdf *pdfDoc,const QString &fileName);
 void init(boost::shared_ptr<IProperty> doc,const QString &pName=QString::null);
 void activate(boost::shared_ptr<CContentStream> cs,QString pName=QString::null);
 void activate(boost::shared_ptr<IProperty> doc,QString pName=QString::null);
 void deactivate(boost::shared_ptr<IProperty> doc);
 void deactivate(boost::shared_ptr<CContentStream> cs);
 MultiTreeWindow(Base *_base,QWidget *parent=0,const char *name=0);
 ~MultiTreeWindow();
 TreeItemAbstract* root();
 TreeItemAbstract* rootMain();
 void reload();
 void reloadFrom(TreeItemAbstract *item);
 QSCObject* getSelected();
 TreeItemAbstract* getSelectedItem();
 void clear();
 void clearSecondary();
 void notifyDelete(TreeItemAbstract *dest);
public slots:
 void deleteCurrent();
 void settingUpdate(QString key);
signals:
 /** \copydoc TreeWindow::itemSelected */
 void itemSelected();
 /** \copydoc TreeWindow::treeClicked */
 void treeClicked(int,QListViewItem*);
 /**
  Signal emitted when tree item is about to be deleted
  @param dest item to be deleted
 */
 void itemDeleted(TreeItemAbstract *dest);
protected slots:
 void treeItemSelected();
 void treeMouseClicked(int button,QListViewItem* item);
 void pageChange(QWidget *newPage);
private:
 void connectSig(TreeWindow *tr);
 TreeWindow* createPage(const QString &caption);
 bool activate(void *ptr);
 void deactivate(void *ptr);
 void deleteWindow(TreeWindow *tr);
private:
 /** Current TreeWindow */
 TreeWindow *tree;
 /** Main TreeWindow */
 TreeWindow *mainTree;
 /** TreeWindow map*/
 TreeWindowList trees;
 /** TreeWindow map - reverse */
 TreeWindowListReverse treesReverse;
 /** Tab Widget */
 QTabWidget *tab;
 /** Corner widget */
 QButton *corner;
 /** Scripting base */
 Base *base;
};

} // namespace gui

#endif
