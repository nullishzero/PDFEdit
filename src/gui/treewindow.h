#ifndef __TREEWINDOW_H__
#define __TREEWINDOW_H__

#include <qwidget.h>
#include <qlistview.h>
#include <cobject.h>
#include <cpdf.h>
#include "treeitem.h"

namespace gui {

using namespace pdfobjects;

//class private to TreeWindow
class ShowData;

class TreeWindow : public QWidget {
 Q_OBJECT
public:
 void clear();
 void init(CPdf *pdfDoc);
 void init(IProperty *doc);
 TreeWindow(QWidget *parent=0,const char *name=0);
 ~TreeWindow();
 void addChilds(TreeItem *parent,bool expandReferences=true);
public slots:
 void settingUpdate(QString key);
signals:
 /** Signal emitted when selecting one of the objects in the tree window.
     Pointer to CObject is sent in signal */
 void objectSelected(IProperty*);
private slots:
 void treeSelectionChanged(QListViewItem *item); 
private:
 bool isSimple(IProperty* prop);
 bool isSimple(boost::shared_ptr<IProperty> prop);
 void updateTreeSettings();
private:
 /** Treeview */
 QListView *tree;
 /** class with stored setting what to show */
 ShowData *sh;
 /** Root object of the tree*/
 IProperty *obj;
};

} // namespace gui

#endif
