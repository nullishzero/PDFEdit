#ifndef __TREEWINDOW_H__
#define __TREEWINDOW_H__

#include <qwidget.h>
#include <qlistview.h>

class TreeWindow : public QWidget {
 Q_OBJECT
public:
 void clear();
 void init(void *pdfDoc);
 TreeWindow(QWidget *parent=0,const char *name=0);
 ~TreeWindow();
private:
 /** Treeview */
 QListView *tree;
};

#endif
