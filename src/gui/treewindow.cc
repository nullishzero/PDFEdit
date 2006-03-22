/** @file
 TreeWindow - class with treeview of PDF objects
*/
#include "treewindow.h"
#include "settings.h"
#include "util.h"
#include <iostream>
#include <qlayout.h>

using namespace std;

/** constructor of TreeWindow, creates window and fills it with elements, parameters are ignored */
TreeWindow::TreeWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name) {
 QBoxLayout *l=new QVBoxLayout(this);
 tree=new QListView(this);
 tree->setSorting(-1);
// QObject::connect(cmd, SIGNAL(returnPressed()), this, SLOT(execute()));
 l->addWidget(tree);
 tree->addColumn("Object");
 tree->addColumn("Type");
 tree->setSelectionMode(QListView::Single);
 tree->setColumnWidthMode(0,QListView::Maximum);
 tree->show();
 init(0);
 clear();
 init(0);
}

/** Clears all items from TreeWindow */
void TreeWindow::clear() {
 QListViewItem *li;
 while ((li=tree->firstChild())) {
  delete li;
 }
}

/** Init contents of treeview from given PDF document
 @param pdfDoc Document used to initialize treeview
 */
void TreeWindow::init(void *pdfDoc) {
//demo
 QListViewItem *li=new QListViewItem( tree, "file.pdf","Document");
 li->setOpen(TRUE);
// li->setRenameEnabled(0,FALSE);
// li->setRenameEnabled(1,FALSE);
 (void) new QListViewItem(li,"Page 1","Page");
 li=new QListViewItem(li,"Page 2","Page");
 (void) new QListViewItem(li,"pic000.jpg","Picture");
}

/** default destructor */
TreeWindow::~TreeWindow() {
 delete tree;
}
