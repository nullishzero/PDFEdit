#ifndef __TREEWINDOW_H__
#define __TREEWINDOW_H__

#include <qwidget.h>
#include <qlistview.h>
#include <iproperty.h>

using namespace pdfobjects;

class TreeWindow : public QWidget {
 Q_OBJECT
public:
 void clear();
 void init(IProperty *pdfDoc);
 TreeWindow(QWidget *parent=0,const char *name=0);
 void addChilds(IProperty *obj);
 ~TreeWindow();
private:
 /** Treeview */
 QListView *tree;
};

#endif
