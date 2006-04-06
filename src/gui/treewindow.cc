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

/** Subclass holding specific settings - what to show in treeview */
class ShowData {
private:
 /** Only TreeWindow can use this class */
 friend class TreeWindow;
 /** default constructor*/
 ShowData() {
  dirty=true;
 }
 /** destructor - empty */
 ~ShowData() {
 }

 /** Check if setting have changed, updating if necessary.
  If setting is changed, set dirty to true.
  @param key Setting to check
  @param target Pointer to setting to check */
 void check(bool &target,const QString &key) {
  bool tmp=globalSettings->readBool(key);
  if (target==tmp) return;
  target=tmp;
  dirty=true;
 }
 
 /** update internal data from settings */
 void update() {
  check(show_simple,"mode/show_simple");
 }
 /** Show simple objects (int,bool,string,name,real) in object tree? */
 bool show_simple;
 /** True, if any change since last time this was reset to false. Initial value is true */
 bool dirty;
};


/** constructor of TreeWindow, creates window and fills it with elements, parameters are ignored
 @param parent Parent widget
 @param name Name of this widget (not used, just passed to QWidget)
 */
TreeWindow::TreeWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name) {
 QBoxLayout *l=new QVBoxLayout(this);
 tree=new QListView(this);
 tree->setSorting(-1);
 obj=NULL;
 QObject::connect(tree, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(treeSelectionChanged(QListViewItem *)));
 l->addWidget(tree);
 tree->addColumn(tr("Object"));
 tree->addColumn(tr("Type"));
 tree->addColumn(tr("Ref"));
 tree->setSelectionMode(QListView::Single);
 tree->setColumnWidthMode(0,QListView::Maximum);
 tree->show();
 sh=new ShowData();
 sh->update();
}

/** Re-read tree settings from global settings */
void TreeWindow::updateTreeSettings() {
 sh->update();
 if (sh->dirty) {
  init(obj); //update object
  sh->dirty=false;
 }
}

/** Called when any settings are updated (in script, option editor, etc ...) */
void TreeWindow::settingUpdate(QString key) {
 //TODO: only once per buch of mode/show signals ... setting blocks
 printDbg(debug::DBG_DBG,"Settings observer: " << key);
 if (key.startsWith("mode/show")) { //Updated settings of what to show and what not
  updateTreeSettings();
 }
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
 obj=doc;
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

/** Return true, if this is simple property (editable as item in property editor and have no children), false otherwise
 @param prop IProperty to check
 @return true if simple property, false otherwise
 */
bool TreeWindow::isSimple(IProperty* prop) {
 PropertyType pt=prop->getType();
 switch(pt) {
  case pNull: 
  case pBool: 
  case pInt: 
  case pReal: 
  case pName: 
  case pString:
   return true;
  default:
   return false;
 } 
}

/** Return true, if this is simple property (editable as item in property editor and have no children), false otherwise
 @param prop IProperty to check
 @return true if simple property, false otherwise
 */
bool TreeWindow::isSimple(boost::shared_ptr<IProperty> prop) {
 return isSimple(prop.get());
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
   if (!sh->show_simple && isSimple(property)) continue; //simple item -> skip it
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
  QString name;
  for(size_t i=0;i<n;i++) { //for each property
   boost::shared_ptr<IProperty> property=ar->getProperty(i);
   name.sprintf("[%d]",i);
   if (!sh->show_simple && isSimple(property)) continue; //simple item -> skip it
   TreeItem *child=new TreeItem(parent, property.get(),name,last); 
   last=child;
   addChilds(child);
  }
 }

 //Null, Bool, Int, Real, Name, String -> These are simple types without any children
 //TODO: pRef, pStream -> have they children?

}

/** default destructor */
TreeWindow::~TreeWindow() {
 delete tree;
 delete sh;
}
