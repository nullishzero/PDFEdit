#ifndef __TREEWINDOW_H__
#define __TREEWINDOW_H__

#include <qwidget.h>
#include <qlistview.h>
#include <cobject.h>
#include <cpdf.h>

using namespace pdfobjects;

class TreeWindow : public QWidget {
 Q_OBJECT
public:
 void clear();
 void init(CPdf *pdfDoc);
 void init(IProperty *doc);
 TreeWindow(QWidget *parent=0,const char *name=0);
 void addChilds(IProperty *obj);
 ~TreeWindow();
signals:
 /** Signal emitted when selecting one of the objects in the tree window.
     Pointer to CObject is sent in signal */
 void objectSelected(IProperty*);
private slots:
 void treeSelectionChanged(QListViewItem *item); 
private:
 /** Treeview */
 QListView *tree;
};

#endif
