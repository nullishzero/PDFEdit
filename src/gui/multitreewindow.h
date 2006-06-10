#ifndef __MULTITREEWINDOW_H__
#define __MULTITREEWINDOW_H__

#include <qwidget.h>
#include "types.h"
namespace pdfobjects {
 class IProperty;
 class CContentStream;
 class CPdf;
}

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

/** Possible types of tree - which item is in root */
typedef enum {
 /** Invalid type*/ Tree_Invalid=0,
 /** Main - the document */ Tree_Main,
 /** Vector with operators */ Tree_OperatorVector,
 /** IPproperty */ Tree_IProperty,
 /** Content stream */ Tree_ContentStream
} TreeType;

/** Key type used to identify the tree in map*/
typedef std::pair<TreeType,void*> TreeKey;

/** Mapping from TreeKey to associated Tree Window */
typedef QMap<TreeKey,TreeWindow*> TreeWindowList;

/** Mapping from Tree Window to TreeKey associated with them (reverse mapping) */
typedef QMap<TreeWindow*,TreeKey> TreeWindowListReverse;

/**
 Class providing multiple treeviews, switchable by tabs
 It always have at least one (main) tree tabs, and 0 or more "secondary" tree tabs,
 that can be opened/closed as necessary. Main tree cannot be closed<br>
 The multi tree view contain widgets of TreeWindow class inside the tabs
 \see TreeWindow
*/
class MultiTreeWindow : public QWidget {
 Q_OBJECT
public:
 void uninit();
 void init(CPdf *pdfDoc,const QString &fileName);
 void init(boost::shared_ptr<IProperty> doc,const QString &pName=QString::null);
 void activateMain();
 void activate(const OperatorVector &vec,QString pName=QString::null);
 void activate(boost::shared_ptr<CContentStream> cs,QString pName=QString::null);
 void activate(boost::shared_ptr<IProperty> doc,QString pName=QString::null);
 void deactivate(const OperatorVector &vec);
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
 bool activate(TreeKey ptr);
 void deactivate(TreeKey ptr);
 void deleteWindow(TreeWindow *tr);
 void updateCorner();
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
