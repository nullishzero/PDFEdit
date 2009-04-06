/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __MULTITREEWINDOW_H__
#define __MULTITREEWINDOW_H__

#include "qtcompat.h"
#include <qwidget.h>
#include <qmap.h>
#include "types.h"
namespace pdfobjects {
 class IProperty;
 class CContentStream;
 class CPdf;
}

class QListViewItem;
class QTabWidget;
class QAbstractButton;

namespace gui {

using namespace pdfobjects;

class TreeData;
class TreeWindow;
class TreeItemAbstract;
class TreeItem;
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
 \brief Treeview with tab support
*/
class MultiTreeWindow : public QWidget {
 Q_OBJECT
public:
 void uninit();
 void init(boost::shared_ptr<CPdf> pdfDoc,const QString &fileName);
 void init(boost::shared_ptr<IProperty> doc,const QString &pName,const QString &pToolTip);
 void activateMain();
 void activate(const OperatorVector &vec,const QString &pName,const QString &pToolTip);
 void activate(const AnnotationVector &vec,boost::shared_ptr<CPage> page,const QString &pName,const QString &pToolTip);
 void activate(boost::shared_ptr<CContentStream> cs,const QString &pName,const QString &pToolTip);
 void activate(boost::shared_ptr<IProperty> doc,const QString &pName,const QString &pToolTip);
 void deactivate(const OperatorVector &vec);
 void deactivate(boost::shared_ptr<IProperty> doc);
 void deactivate(boost::shared_ptr<CContentStream> cs);
 void deactivate(const AnnotationVector &vec);
 MultiTreeWindow(Base *_base,QWidget *parent=0,const char *name=0);
 ~MultiTreeWindow();
 TreeItemAbstract* root();
 TreeItemAbstract* rootMain();
 void reload();
 void reloadFrom(TreeItemAbstract *item);
 TreeWindow* getTree(const QString &name);
 QSCObject* getSelected(const QString &name);
 QSCObject* nextSelected();
 TreeItemAbstract* getSelectedItem(const QString &name);
 TreeItemAbstract* nextSelectedItem();
 void clear();
 void clearSecondary();
 void notifyDelete(TreeItemAbstract *dest);
 void notifyDeleteItem(TreeItem *dest);
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
 /**
  Signal emitted when tree item of type TreeItem is about to be deleted
  @param dest item to be deleted
 */
 void itemDeleted(TreeItem *dest);
 /**
  Signal emitted when mouse moves over items.
  @param info Info sent to statusbar
 */
 void itemInfo(const QString &info);
protected slots:
 void treeItemInfo(const QString &info);
 void tabSwitchPopup();
 void treeItemSelected();
 void treeMouseClicked(int button,QListViewItem* item);
 void pageChange(QWidget *newPage);
private:
 void connectSig(TreeWindow *tr);
 TreeWindow* createPage(const QString &caption,const QString &toolTip);
 bool activate(TreeKey ptr);
 void deactivate(TreeKey ptr);
 void deleteWindow(TreeWindow *tr);
 void updateCorner();
private:
 /** Current TreeWindow */
 TreeWindow *tree;
 /** TreeWindow to get selection from */
 TreeWindow *treeSelection;
 /** Main TreeWindow */
 TreeWindow *mainTree;
 /** TreeWindow map*/
 TreeWindowList trees;
 /** TreeWindow map - reverse */
 TreeWindowListReverse treesReverse;
 /** Tab Widget */
 QTabWidget *tab;
 /** Right Corner widget */
 QAbstractButton *cornerRight;
 /** Left Corner widget */
 QAbstractButton *cornerLeft;
 /** Scripting base */
 Base *base;
};

} // namespace gui

#endif
