/** @file
 MultiTreeWindow - class providing multiple treeviews, switchable by tabs
 It always have at least one (main) tree tabs, and 0 or more "secondary" tree tabs,
 that can be opened/closed as necessary. Main tree cannot be closed
 @author Martin Petricek
*/

#include "multitreewindow.h"
#include "util.h"
#include "treewindow.h"
#include "treeitemabstract.h"
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <utils/debug.h>
#include "iconcache.h"

namespace gui {

using namespace std;
using namespace util;

/**
 constructor of MultiTreeWindow, creates window and fills it with elements, parameters are ignored
 @param _base Scripting base
 @param parent Parent widget
 @param name Name of this widget (not used, just passed to QWidget)
*/
MultiTreeWindow::MultiTreeWindow(Base *_base,QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name) {
 //Store base
 base=_base;

 //Box layout for this (may be usable to add hintbar/statusbar later)
 QBoxLayout *l=new QVBoxLayout(this);
 //TODO: add hintbar/statusbar, maybe show "nice names" there

 //Tab widget
 tab=new QTabWidget(this,"treeview_tab");
 l->add(tab);
 
 //Main tree
 mainTree=new TreeWindow(this,base,tab,"main_tree_view");
 connectSig(mainTree);
 tree=mainTree;

 //Add main tree to Tab widget
 tab->addTab(mainTree,tr("Tree"));

 IconCache ic;

 QString closeIconName="close_widget.png";
 QPixmap *closeIcon=ic.getIcon(closeIconName);

 //Corner button to close current tree
 QToolButton *theCorner=new QToolButton(tab,"close_tree");
 if (closeIcon) {
  theCorner->setIconSet(*closeIcon);
 } else {
  guiPrintDbg(debug::DBG_WARN,"Icon not found: " << closeIconName);
 }
 theCorner->setTextLabel(tr("Close current tab"));
 corner=theCorner;

 tab->setCornerWidget(corner,Qt::TopRight);
 QObject::connect(corner,SIGNAL(pressed()),this,SLOT(deleteCurrent()));
 QObject::connect(tab,SIGNAL(currentChanged(QWidget*)),this,SLOT(pageChange(QWidget*)));
}

/**
 Called to delete currently shown treeview
*/
void MultiTreeWindow::deleteCurrent() {
 guiPrintDbg(debug::DBG_DBG,"Trying to delete tree");
 if (tree!=mainTree) {
  guiPrintDbg(debug::DBG_DBG,"deleting tree");
  deleteWindow(tree);
 }
}

/**
 Called when page is changed
 @param newPage New page. Should be TreeWindow
*/
void MultiTreeWindow::pageChange(QWidget *newPage) {
 guiPrintDbg(debug::DBG_DBG,"PageChange");
 tree=dynamic_cast<TreeWindow*>(newPage);
 assert(tree);
}

/**
 Connect signals from underlying tree window to this MultiTreeWindow
 @param tr TreeWindow to connect
*/
void MultiTreeWindow::connectSig(TreeWindow *tr) {
 QObject::connect(tr,SIGNAL(treeClicked(int,QListViewItem*)),this,SLOT(treeMouseClicked(int,QListViewItem*)));
 QObject::connect(tr,SIGNAL(itemSelected()),this,SLOT(treeItemSelected()));
}

/**
 Reload part of tree that have given item as root (including that item)
 Reloading will stop at unopened reference targets
 @param item root of subtree to reload
 */
void MultiTreeWindow::reloadFrom(TreeItemAbstract *item) {
 item->reload();
}

/**
 Slot called when someone click with mouse button anywhere in one of the trees
 @param button Which button(s) are clicked (1=left, 2=right, 4=middle)
 @param item Which item is clicked upon (NULL if clicked outside item)
*/
void MultiTreeWindow::treeMouseClicked(int button,QListViewItem* item) {
 emit treeClicked(button,item);
}

/** Called upon changing selection in the tree window */
void MultiTreeWindow::treeItemSelected() {
 emit itemSelected();
}

/** reinitialize/reload entire tree after some major change */
void MultiTreeWindow::reload() {
 mainTree->reload();
 //Pass to all trees
 TreeWindowList::iterator it=trees.begin();
 while (it!=trees.end()) {
  (*it)->reload();
  it++;
 }
}

/**
 Return root item of currently selected tree.
 @return root item
 */
TreeItemAbstract* MultiTreeWindow::root() {
 return tree->root();
}

/**
 Return root item of main tree.
 @return root item
 */
TreeItemAbstract* MultiTreeWindow::rootMain() {
 return mainTree->root();
}

/**
 Called when any settings are updated (in script, option editor, etc ...) 
 @param key Key of setting which have changed
*/
void MultiTreeWindow::settingUpdate(QString key) {
 mainTree->settingUpdate(key);
 //Pass to all trees
 TreeWindowList::iterator it=trees.begin();
 while (it!=trees.end()) {
  (*it)->settingUpdate(key);
  it++;
 }
}

/** 
 Return QSCObject from currently selected item (in current tree)
 Caller is responsible for freeing object
 @return QSCObject from current item
*/
QSCObject* MultiTreeWindow::getSelected() {
 return tree->getSelected();
}

/** 
 Return pointer to currently selected tree item (in current tree)
 @return current item
*/
TreeItemAbstract* MultiTreeWindow::getSelectedItem() {
 return tree->getSelectedItem();
}

/** Clears all items from MultiTreeWindow */
void MultiTreeWindow::clear() {
 mainTree->uninit();
 clearSecondary();
}

/** Close all secondary Trees. Only the main tree remain active */
void MultiTreeWindow::clearSecondary() {
 //Close all non-primary trees
 TreeWindowList::iterator it=trees.begin();
 while (it!=trees.end()) {
  delete (*it);
  it++;
 }
 //No trees exist, clear both lists
 trees.clear();
 treesReverse.clear();
}

/** Init contents of main tree from given PDF document
 @param pdfDoc Document used to initialize treeview
 @param fileName Name of PDF document (will be shown in treeview as name of root element)
 */
void MultiTreeWindow::init(CPdf *pdfDoc,const QString &fileName) {
 mainTree->init(pdfDoc,fileName);
}

/**
 Init first (main) tab of treeview from given IProperty (dictionary, etc ...)
 @param doc IProperty used to initialize treeview
 @param pName Optional name used for this property
 */
void MultiTreeWindow::init(boost::shared_ptr<IProperty> doc,const QString &pName/*=QString::null*/) {
 mainTree->init(doc,pName);
}

/**
 Create new tab and insert new empty TreeWindow in it
 @param caption Caption of tab containing the new tree
 @return Pointer to new treewindow
*/
TreeWindow* MultiTreeWindow::createPage(const QString &caption) {
 TreeWindow* t=new TreeWindow(this,base,tab);
 connectSig(t);
 tab->addTab(t,caption);
 return t;
}

/**
 Notification function, that will be called when tree item is deleting itself
 @param dest Treeitem about to be deleted
*/
void MultiTreeWindow::notifyDelete(TreeItemAbstract *dest) {
 emit itemDeleted(dest);
}

/**
 Try to activate secondary tree that is identified by it key
 @param ptr pointer used to identify tree to activate
 @return true if the tree exists and was activated, false if the tree does not exist.
 */
bool MultiTreeWindow::activate(TreeKey ptr) {
 if (ptr.first==Tree_Main) { //Activate main tree view
  //todo: remove, handle tree_main (almost) normally
  activateMain();
  return true;
 }
 if (trees.contains(ptr)) { //Try to activate secondary tree view
  tree=trees[ptr];
  int index=tab->indexOf(tree);
  tab->setCurrentPage(index);
  return true;
 }
 return false;
}

/**
 Create if not exist and then activate secondary tree that contains given CContentStream as root item
 @param cs CContentStream used to identify secondary treeview
 @param pName Optional name used for this content stream
 */
void MultiTreeWindow::activate(boost::shared_ptr<CContentStream> cs,QString pName/*=QString::null*/) {
 TreeKey tk(Tree_ContentStream,cs.get());
 //If the page already exist, just switch to it
 if (activate(tk)) return;

 //Invent some default name if none specified
 if (pName.isNull()) pName=tr("Stream");

 //Create the page
 TreeWindow* t=createPage(pName);
 t->init(cs,pName);
 trees.insert(tk,t);
 treesReverse.insert(t,tk);
 //Switch to new page
 activate(tk);
}

/**
 Activate the Main tree
*/
void MultiTreeWindow::activateMain() {
 tab->setCurrentPage(0);//Main tree is always the first
 tree=mainTree;
}

/**
 Create if not exist and then activate secondary tree that contains given IProperty as root item
 @param doc IProperty used to identify secondary treeview
 @param pName Optional name used for this property
 */
void MultiTreeWindow::activate(boost::shared_ptr<IProperty> doc,QString pName/*=QString::null*/) {
 TreeKey tk(Tree_IProperty,doc.get());
 //If the page already exist, just switch to it
 if (activate(tk)) return;

 //Invent some default name if none specified
 if (pName.isNull()) pName=tr("Property");	//Default name

 //Create the page
 TreeWindow* t=createPage(pName);
 t->init(doc,pName);
 trees.insert(tk,t);
 treesReverse.insert(t,tk);
 //Switch to new page
 activate(tk);
}

/**
 Create if not exist and then activate secondary tree that contains given vector of PDF operators as root item
 This type of item is special, as if the tree already exist, its contents is replaced
 @param vec Operator vector
 @param pName Optional name used for this property
 */
void MultiTreeWindow::activate(const OperatorVector &vec,QString pName/*=QString::null*/) {
 TreeKey tk(Tree_OperatorVector,NULL);
 if (!activate(tk)) {
  //Create the page if it does not exist
  TreeWindow* t=createPage(pName);
  t->init(vec,pName);
  trees.insert(tk,t);
  treesReverse.insert(t,tk);
 }
 trees[tk]->init(vec,pName);
 activate(tk);
}

/**
 Delete specified secondary tree that contains given IProperty as root item
 @param doc IProperty used to identify secondary treeview
*/
void MultiTreeWindow::deactivate(boost::shared_ptr<IProperty> doc) {
 TreeKey tk(Tree_IProperty,doc.get());
 deactivate(tk);
}

/**
 Delete specified secondary tree that contains Operator Vector
 @param vec  Operator vector, but in fact it is ignored
*/
void MultiTreeWindow::deactivate(__attribute__((unused)) const OperatorVector &vec) {
 TreeKey tk(Tree_OperatorVector,NULL);
 deactivate(tk);
}

/**
 Try to delete secondary tree that is identified by key
 @param ptr pointer used to identify tree to activate
 */
void MultiTreeWindow::deactivate(TreeKey ptr) {
 if (ptr.first==Tree_Main) return; //Deactivate main tree view? Not possible
 if (trees.contains(ptr)) { //Try to deactivate secondary tree view
  deleteWindow(trees[ptr]);
 }
}

/**
 Try to delete specified secondary tree
 @param tr Secondary tree to delete
*/
void MultiTreeWindow::deleteWindow(TreeWindow *tr) {
 guiPrintDbg(debug::DBG_DBG,"deleteWindow");
 assert(tr);
 TreeKey ptr=treesReverse[tr];
 assert(ptr.first!=Tree_Invalid);
 //Remove from both mappings
 trees.remove(ptr);
 treesReverse.remove(tr);
 //Delete the tree
 delete tr;

 //Current page is likely the one that was deleted, request the new page that is now active
 tree=dynamic_cast<TreeWindow*>(tab->currentPage());
 assert(tree);
 if (!tree) { // <- If this happen, it is probably a QT bug ...
  activateMain();
 }
}

/**
 Delete specified secondary tree that contains given CContentStream as root item
 @param cs CContentStream used to identify secondary treeview
 */
void MultiTreeWindow::deactivate(boost::shared_ptr<CContentStream> cs) {
 TreeKey tk(Tree_ContentStream,cs.get());
 deactivate(tk);
}

/** Resets the tree to be empty and show nothing */
void MultiTreeWindow::uninit() {
 clear();
}

/** default destructor */
MultiTreeWindow::~MultiTreeWindow() {
 //Nothing for now ... 
}

} // namespace gui
