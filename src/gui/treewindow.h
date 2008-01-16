/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __TREEWINDOW_H__
#define __TREEWINDOW_H__

#include "qtcompat.h"
#include QLISTVIEW
#include <qwidget.h>
#include <kernel/cobject.h>
#include <kernel/cpage.h>
#include "types.h"
class Q_ListView;
class QListViewItem;
class Q_ListViewItemIterator;
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
 \brief Object tree view widget
*/
class TreeWindow : public QWidget {
 Q_OBJECT
public:
 void uninit();
 void init(const AnnotationVector &vec,boost::shared_ptr<pdfobjects::CPage> page,const QString &pName=QString::null);
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
 QSCObject* nextSelected();
 TreeItemAbstract* getSelectedItem();
 TreeItemAbstract* nextSelectedItem();
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
 /**
  Signal emitted when mouse moves over items.
  @param info Info sent to statusbar
 */
 void itemInfo(const QString &info);
protected:
 virtual void paintEvent(QPaintEvent *e);
private slots:
 void moveOnItem(QListViewItem *item);
 void moveOffItem();
 void treeSelectionChanged(QListViewItem *item); 
 void treeSelectionChanged();
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
 Q_ListView *tree;
#endif
 /** Tree data shared with tree items */
 TreeData *data;
 /** Name of root object - if applicable */
 QString rootName;
 /** Iterator for selected item */
 Q_ListViewItemIterator selIter;
};

} // namespace gui

#endif
