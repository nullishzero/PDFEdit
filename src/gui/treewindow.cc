/** @file
 TreeWindow - class with treeview of PDF objects
*/
#include <utils/debug.h>
#include "treewindow.h"
#include "treeitem.h"
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
 QObject::connect(tree, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(treeSelectionChanged(QListViewItem *)));
 l->addWidget(tree);
 tree->addColumn("Object");
 tree->addColumn("Type");
 tree->setSelectionMode(QListView::Single);
 tree->setColumnWidthMode(0,QListView::Maximum);
 tree->show();
}

/** Called upon changing selection in the tree window */
void TreeWindow::treeSelectionChanged(QListViewItem *item) {
 printDbg(debug::DBG_DBG,"Selected an item: " << item->text(0));
 TreeItem* it=dynamic_cast<TreeItem*>(item);
 if (!it) {
  printDbg(debug::DBG_WARN,"Not a TreeItem: " << item->text(0));
  //todo: handle this type properly
  return;
 }
 printDbg(debug::DBG_DBG,"Is a TreeItem: " << item->text(0));
 //We have a TreeItem -> emit signal with selected object
 emit objectSelected(it->getObject());
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
void TreeWindow::init(CPdf *pdfDoc) {
 assert(pdfDoc);
//boost::shared_ptr<CDict> pd=pdfDoc->getDictionary();
 init(pdfDoc->getDictionary().get());
}

/** Init contents of treeview from given IProperty (dictionary, etc ...)
 @param prop IProperty used to initialize treeview
 */
void TreeWindow::init(IProperty *doc) {
 clear();
 if (!doc) {//nothing specified, fill with testing data
  QListViewItem *li=new QListViewItem(tree, "file.pdf","Document");
  li->setOpen(TRUE);
  (void) new QListViewItem(li,"Page 1","Page");
  li=new QListViewItem(li,"Page 2","Page");
  (void) new QListViewItem(li,"pic000.jpg","Picture");
 } else {
  TreeItem *root=new TreeItem(tree, doc); 
  root->setOpen(TRUE);
  addChilds(doc);
 }
}

/** Recursively add all child of given object to treeview. Childs will be added with obj as parent.
 @param obj Object to add*/
void TreeWindow::addChilds(IProperty *obj) {
 if (obj->getType()==pDict) {
  printDbg(debug::DBG_DBG,"is-a-dict");
  CDict *dict=(CDict*)obj;
  vector<string> list;
  printDbg(debug::DBG_DBG,"get-names");
  dict->getAllPropertyNames(list);
  printDbg(debug::DBG_DBG,"end..");
 }

//todo
// TreeItem *child=new TreeItem(tree, obj); 
}

/** default destructor */
TreeWindow::~TreeWindow() {
 delete tree;
}
