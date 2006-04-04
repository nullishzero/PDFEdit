/** @file
 TreeWindow - class with treeview of PDF objects
*/
#include <utils/debug.h>
#include "treewindow.h"
#include "settings.h"
#include "util.h"
#include <iostream>
#include <qlayout.h>

using namespace std;

/** constructor of TreeWindow, creates window and fills it with elements, parameters are ignored
 @param parent Parent widget
 @param name Name of this widget (not used, just passed to QWidget)
 */
TreeWindow::TreeWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name) {
 QBoxLayout *l=new QVBoxLayout(this);
 tree=new QListView(this);
 tree->setSorting(-1);
 QObject::connect(tree, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(treeSelectionChanged(QListViewItem *)));
 l->addWidget(tree);
 tree->addColumn(tr("Object"));
 tree->addColumn(tr("Type"));
 tree->addColumn(tr("Ref"));
 tree->setSelectionMode(QListView::Single);
 tree->setColumnWidthMode(0,QListView::Maximum);
 tree->show();
}

/** Called upon changing selection in the tree window
 @param item The item that was selected
 */
void TreeWindow::treeSelectionChanged(QListViewItem *item) {
 printDbg(debug::DBG_DBG,"Selected an item: " << item->text(0));
 TreeItem* it=dynamic_cast<TreeItem*>(item);
 if (!it) { //Not holding IProperty
  printDbg(debug::DBG_WARN,"Not a TreeItem: " << item->text(0));
  //todo: handle this type properly
  return;
 }
 //holding IProperty
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
  setUpdatesEnabled( FALSE );
  TreeItem *root=new TreeItem(tree, doc); 
  root->setOpen(TRUE);
  addChilds(root);
  setUpdatesEnabled( TRUE );
 }
}

/** Recursively add all child of given object to treeview. Childs will be added under parent item.
 @param parent Parent TreeItem of which to add childs
*/
void TreeWindow::addChilds(TreeItem *parent) {
 IProperty *obj=parent->getObject();

 if (obj->getType()==pDict) {	//Object is CDict
  CDict *dict=(CDict*)obj;
  vector<string> list;
  dict->getAllPropertyNames(list);
  vector<string>::iterator it;
  TreeItem *last=NULL;
  for( it=list.begin();it!=list.end();++it) { // for each property
   printDbg(debug::DBG_DBG,"Subproperty: " << *it);
   boost::shared_ptr<IProperty> property=dict->getProperty(*it);
   TreeItem *child=new TreeItem(parent, property.get(),*it,last); 
   last=child;
   addChilds(child);
  }
 }

 if (obj->getType()==pArray) {	//Object is CArray
  CArray *ar=(CArray*)obj;
  size_t n=ar->getPropertyCount();
  printDbg(debug::DBG_DBG,"Subproperties: " << n);
  TreeItem *last=NULL;
  for(size_t i=0;i<n;i++) { //for each property
   boost::shared_ptr<IProperty> property=ar->getProperty(i);
   QString name;
   name.sprintf("[%d]",i);
   TreeItem *child=new TreeItem(parent, property.get(),name,last); 
   last=child;
   addChilds(child);
  }
 }

 //Null, Bool, Int, Real, String -> These are simple types without any children
 //TODO: Name, pRef, pStream -> have they children?

}

/** default destructor */
TreeWindow::~TreeWindow() {
 delete tree;
}
