/** @file
 TreeWindow - class with treeview of PDF objects
*/
#include <utils/debug.h>
#include "treewindow.h"
#include "settings.h"
#include "util.h"
#include <iostream>
#include <qlayout.h>
#include <qlistview.h>
#include "treedata.h"

namespace gui {

using namespace std;

/** Subclass holding specific settings - what to show in treeview */
class ShowData {
private:
 /** Only TreeWindow can use this class */
 friend class TreeWindow;
 /** default constructor*/
 ShowData() {
  dirty=false;
  needreload=false;
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
  needreload=true;
 }
 
 /** update internal data from settings */
 void update() {
  check(show_simple,"tree/show_simple");
 }
 /** Show simple objects (int,bool,string,name,real) in object tree? */
 bool show_simple;
 /** True, if any change since last time this was reset to false. Initial value is true */
 bool dirty;
 /** True if the tree needs reloading */
 bool needreload;
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
 tree->addColumn(tr("Data"));
 tree->setSelectionMode(QListView::Single/*Extended*/);
 tree->setColumnWidthMode(0,QListView::Maximum);
 tree->show();
 sh=new ShowData();
 sh->update();
 data=new TreeData(this,tree);
}

/** Re-read tree settings from global settings */
void TreeWindow::updateTreeSettings() {
 sh->update();
 if (sh->dirty) {
  printDbg(debug::DBG_DBG,"update tree settings: is dirty");
  sh->dirty=false;
  update();//Update treeview itself
 }
}

/** Paint event handler -> if settings have been changed, reload tree */
void TreeWindow::paintEvent(QPaintEvent *e) {
 if (sh->needreload) {
  printDbg(debug::DBG_DBG,"update tree settings: need reload");
  init(obj); //update object if necessary
  sh->needreload=false;
 }
 //Pass along
 QWidget::paintEvent(e);
}

/** Called when any settings are updated (in script, option editor, etc ...) */
void TreeWindow::settingUpdate(QString key) {
 //TODO: only once per bunch of tree/show... signals ... setting blocks
 printDbg(debug::DBG_DBG,"Settings observer: " << key);
 if (key.startsWith("tree/show")) { //Updated settings of what to show and what not
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
 @param doc IProperty used to initialize treeview
 */
void TreeWindow::init(IProperty *doc) {
 obj=doc;
 clear();
 if (doc) {
  setUpdatesEnabled( FALSE );
  TreeItem *root=new TreeItem(data,tree, doc); 
  root->setOpen(TRUE);
  addChilds(root,false);
  setUpdatesEnabled( TRUE );
 }
}

/** Resets th tree to be empty and show nothing */
void TreeWindow::uninit() {
 clear();
 obj=NULL;
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
 @param expandReferences If true, references will be expanded to show their target as child of the reference
*/
void TreeWindow::addChilds(TreeItem *parent,bool expandReferences/*=true*/) {
 IProperty *obj=parent->getObject();

 if (obj->getType()==pDict) {	//Object is CDict
  CDict *dict=(CDict*)obj;
  vector<string> list;
  dict->getAllPropertyNames(list);
  vector<string>::iterator it;
  TreeItem *last=NULL;
  for( it=list.begin();it!=list.end();++it) { // for each property
//   printDbg(debug::DBG_DBG,"Subproperty: " << *it);
   boost::shared_ptr<IProperty> property=dict->getProperty(*it);
   if (!sh->show_simple && isSimple(property)) continue; //simple item -> skip it
   TreeItem *child=new TreeItem(data,parent, property.get(),*it,last); 
   last=child;
   addChilds(child,expandReferences);
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
   TreeItem *child=new TreeItem(data,parent, property.get(),name,last); 
   last=child;
   addChilds(child,expandReferences);
  }
 }

 if (obj->getType()==pRef) {	//Object is CRef
  if (expandReferences) {
   //Add referenced object
   QString s;
   CPdf* pdf=obj->getPdf();
   if (!pdf) return; //No document opened -> cannot parse references
                     //Should happen only while testing
   CRef* cref=(CRef*)obj;
   IndiRef ref;
   cref->getPropertyValue(ref);
   printDbg(debug::DBG_DBG," LOADING referenced property: " << ref.num << "," << ref.gen);
   boost::shared_ptr<IProperty> rp=pdf->getIndirectProperty(ref);
   TreeItem *child=new TreeItem(data,parent, rp.get(),s.sprintf("<%d,%d>",ref.num,ref.gen));
   //TODO: store in index...
   addChilds(child,false);
  } else {
   printDbg(debug::DBG_DBG," MARKING referenced property");
   parent->setExpandable(true);
  }
 }

 //Null, Bool, Int, Real, Name, String -> These are simple types without any children
 //TODO: pRef, pStream -> have they children?

}

/** default destructor */
TreeWindow::~TreeWindow() {
 delete data;
 delete tree;
 delete sh;
}

} // namespace gui
