#ifndef __TREEWINDOW_H__
#define __TREEWINDOW_H__

#include <qwidget.h>
#include <cobject.h>
#include <cpdf.h>
#include "treeitem.h"
class QListView;
class QListViewItem;

namespace gui {

using namespace pdfobjects;

class TreeData;

//class private to TreeWindow
class ShowData;

class TreeWindow : public QWidget {
 Q_OBJECT
public:
 void uninit();
 void init(CPdf *pdfDoc,const QString &fileName);
 void init(IProperty *doc);
 TreeWindow(QWidget *parent=0,const char *name=0);
 ~TreeWindow();
 void addChilds(TreeItem *parent,bool expandReferences=true);
 void reloadFrom(TreeItem *item);
 void reinit();
public slots:
 void settingUpdate(QString key);
signals:
 /** Signal emitted when selecting one of the objects in the tree window.
     Pointer to CObject is sent in signal */
 void objectSelected(IProperty*);
protected:
 virtual void paintEvent(QPaintEvent *e);
private slots:
 void treeSelectionChanged(QListViewItem *item); 
 void mouseClicked(int button,QListViewItem* item,const QPoint &coord,int column);
private:
 void clear();
 bool isSimple(IProperty* prop);
 bool isSimple(boost::shared_ptr<IProperty> prop);
 void updateTreeSettings();
private:
 /** Treeview */
 QListView *tree;
 /** class with stored setting what to show */
 ShowData *sh;
 /** Tree data shared with tree items */
 TreeData *data;
 /** Root object of the tree (if IProperty)*/
 IProperty *rootObj;
 /** Root object of the tree (if CPdf)*/
 CPdf *rootObjPdf;
 /** Name of root object - if applicable */
 QString rootName;

};

} // namespace gui

#endif
