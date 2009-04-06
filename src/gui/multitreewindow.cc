/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 MultiTreeWindow - class providing multiple treeviews, switchable by tabs
 It always have at least one (main) tree tabs, and 0 or more "secondary" tree tabs,
 that can be opened/closed as necessary. Main tree cannot be closed
 @author Martin Petricek
*/

#include "multitreewindow.h"
#include "qtcompat.h"
#include "iconcache.h"
#include "treeitemabstract.h"
#include "treewindow.h"
#include "util.h"
#include <qcursor.h>
#include <qlayout.h>
#include QPOPUPMENU
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <utils/debug.h>

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
 treeSelection=NULL;
 tree=mainTree;

 //Add main tree to Tab widget
 tab->addTab(mainTree,tr("Tree"));
 tab->setTabToolTip(mainTree,tr("Document tree"));

 IconCache ic;

 QString closeIconName="close_widget.png";
 QPixmap *closeIcon=ic.getIcon(closeIconName);

 //Corner button to close current tree
 QToolButton *theCorner=new QToolButton(tab,"close_tree");
 if (closeIcon) {
  theCorner->setIconSet(*closeIcon);
 } else {
  guiPrintDbg(debug::DBG_WARN,"Icon not found: " << Q_OUT(closeIconName));
 }
 theCorner->setTextLabel(tr("Close current tab"));
 cornerRight=theCorner;

 //Tab switching corner widget
 QString expandIconName="expand.png";
 QPixmap *expandIcon=ic.getIcon(expandIconName);
 //Corner button to switch to tab using popup menu
 QToolButton *tabExpand=new QToolButton(tab,"switch_tab");
 if (expandIcon) {
  tabExpand->setIconSet(*expandIcon);
 } else {
  guiPrintDbg(debug::DBG_WARN,"Icon not found: " << Q_OUT(expandIconName));
 }
 tabExpand->setTextLabel(tr("Switch to ..."));
 cornerLeft=tabExpand;
 tab->setCornerWidget(cornerLeft,Qt::TopLeft);
 QObject::connect(cornerLeft,SIGNAL(pressed()),this,SLOT(tabSwitchPopup()));

 tab->setCornerWidget(cornerRight,Qt::TopRight);
 QObject::connect(cornerRight,SIGNAL(clicked()),this,SLOT(deleteCurrent()));
 QObject::connect(tab,SIGNAL(currentChanged(QWidget*)),this,SLOT(pageChange(QWidget*)));
 updateCorner();
}

/**
 Invoke popup menu to switch tabs.
 Switch to selected tab.
*/
void MultiTreeWindow::tabSwitchPopup() {
 Q_PopupMenu m(this,"tab_switch_popup");
 m.insertItem(tr("Switch to ..."),0x3fffffff);
 m.setItemEnabled(0x3fffffff,false);
 int tabCount=tab->count();
 m.insertSeparator();
 for (int i=0;i<tabCount;i++) {
  QWidget* oneTab=tab->page(i);
  QString tName=tab->tabToolTip(oneTab);
  m.insertItem(tName,i);
 }
 int id= m.exec(QCursor::pos());
 cornerLeft->setDown(false);
 if (id==-1) return;//Nothing selected
 QWidget* targetTab=tab->page(id);
 tab->showPage(targetTab);
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
 Check the corner widget and update it's state
*/
void MultiTreeWindow::updateCorner() {
 cornerRight->setEnabled(tree!=mainTree);
}

/**
 Called when page is changed
 @param newPage New page. Should be TreeWindow
*/
void MultiTreeWindow::pageChange(QWidget *newPage) {
 guiPrintDbg(debug::DBG_DBG,"PageChange");
 tree=dynamic_cast<TreeWindow*>(newPage);
 assert(tree);
 updateCorner();
 //The selected item changed on selecting an other page
 treeItemSelected();
}

/**
 Connect signals from underlying tree window to this MultiTreeWindow
 @param tr TreeWindow to connect
*/
void MultiTreeWindow::connectSig(TreeWindow *tr) {
 QObject::connect(tr,SIGNAL(treeClicked(int,QListViewItem*)),this,SLOT(treeMouseClicked(int,QListViewItem*)));
 QObject::connect(tr,SIGNAL(itemSelected()),this,SLOT(treeItemSelected()));
 QObject::connect(tr,SIGNAL(itemInfo(const QString&)),this,SLOT(treeItemInfo(const QString&)));
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
 Try to find tree window given its name<br>
 If the name is NULL, return current<br>
 If the name is invalid or not found, return NULL<br>
 Possible names:<br>
  "main" - main tree<br>
  "current" - current tree<br>
  "select" - tree with selected operators<br>
  Or use a number to select tree by its position<br>
 @param name Name of tree
 @return Given tree (or NULL)
*/
TreeWindow* MultiTreeWindow::getTree(const QString &name) {
 if (name.isNull()) return tree;
 QString aName=name.lower().simplifyWhiteSpace();
  guiPrintDbg(debug::DBG_DBG,"getTree : " << Q_OUT(aName));
 if (aName=="main") return mainTree;
 if (aName=="current") return tree;
 if (aName=="select") {
  TreeKey tk(Tree_OperatorVector,NULL);
  if (trees.contains(tk)) {
   //Selection tree
   return trees[tk];
  } else {
   guiPrintDbg(debug::DBG_DBG,"No selection tree");
   //No selection tree exist....
   return NULL;
  }
 }
 bool ok;
 unsigned int pageNum=aName.toUInt(&ok);
 if (ok) {
  //Request for tree with specified number
  return dynamic_cast<TreeWindow*>(tab->page(pageNum));  //may return NULL if number invalid ...
 }
 //Nothing found
 return NULL;
}

/**
 In current tree:
 \copydoc TreeWindow::getSelected()
 @param name Name of tree to use for getting selection
 \see getTree
*/
QSCObject* MultiTreeWindow::getSelected(const QString &name) {
 treeSelection=getTree(name);
 if (!treeSelection) return NULL;
 return treeSelection->getSelected();
}

/**
 In specified tree:
 \copydoc TreeWindow::getSelectedItem()
 @param name Name of tree to use for getting selection
 \see getTree
*/
TreeItemAbstract* MultiTreeWindow::getSelectedItem(const QString &name) {
 treeSelection=getTree(name);
 if (!treeSelection) return NULL;
 return treeSelection->getSelectedItem();
}

/**
 In specified tree:
 \copydoc TreeWindow::nextSelected()
*/
QSCObject* MultiTreeWindow::nextSelected() {
 if (!treeSelection) return NULL;
 return treeSelection->nextSelected();
}

/**
 In current tree:
 \copydoc TreeWindow::nextSelectedItem()
*/
TreeItemAbstract* MultiTreeWindow::nextSelectedItem() {
 if (!treeSelection) return NULL;
 return treeSelection->nextSelectedItem();
}

/** Clears all items from MultiTreeWindow */
void MultiTreeWindow::clear() {
 mainTree->uninit();
 clearSecondary();
 assert(tree==mainTree);
 assert(!getSelectedItem(QString::null)); //Paranoid assert - check if really nothing is in any tree
 //The "selection" have changed
 // - nothing is selected as the tree is empty
 //QT does not emit selectionChanged on clearing the tree
 emit itemSelected();
}

/** Close all secondary Trees. Only the main tree remain active */
void MultiTreeWindow::clearSecondary() {
 if (treeSelection!=mainTree) {
  //Selection is one of trees to be deleted
  treeSelection=NULL;
 }

 TreeWindowList treesCopy=trees;
 //Both trees must be cleared before proceeding, otherwise double free might occur
 //(when root item of the tree is destroyed, it tries to deaallocate its tree window,
 //if it is in the trees list)

 //Clear both lists, so no non-primary trees are listed
 trees.clear();
 treesReverse.clear();
 //Close all non-primary trees
 TreeWindowList::iterator it=treesCopy.begin();
 while (it!=treesCopy.end()) {
  delete (*it);
  ++it;
 }
 //No non-primary trees exist now
}

/** Init contents of main tree from given PDF document
 @param pdfDoc Document used to initialize treeview
 @param fileName Name of PDF document (will be shown in treeview as name of root element)
 */
void MultiTreeWindow::init(boost::shared_ptr<CPdf> pdfDoc,const QString &fileName) {
 mainTree->init(pdfDoc,fileName);
 tab->setTabToolTip(mainTree,tr("Document tree")+" - "+fileName);
 //Emit "selection have changed", as the old tree structure is gone
 treeItemSelected();
}

/**
 Init first (main) tab of treeview from given IProperty (dictionary, etc ...)
 @param doc IProperty used to initialize treeview
 @param pName Name used for this property
 @param pToolTip Tooltip used for this property
 */
void MultiTreeWindow::init(boost::shared_ptr<IProperty> doc,const QString &pName,const QString &pToolTip) {
 mainTree->init(doc,pName);
 tab->setTabToolTip(mainTree,pToolTip);
 //Emit "selection have changed", as the old tree structure is gone
 treeItemSelected();
}

/**
 Create new tab and insert new empty TreeWindow in it
 @param caption Caption of tab containing the new tree
 @param toolTip Tooltip of tab containing the new tree
 @return Pointer to new treewindow
*/
TreeWindow* MultiTreeWindow::createPage(const QString &caption,const QString &toolTip) {
 TreeWindow* t=new TreeWindow(this,base,tab);
 connectSig(t);
 tab->addTab(t,caption);
 tab->setTabToolTip(t,toolTip);
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
 Notification function, that will be called when tree item
 of type "TreeItem" is deleting itself, in addition of notifyDelete
 @param dest Treeitem about to be deleted
*/
void MultiTreeWindow::notifyDeleteItem(TreeItem *dest) {
 emit itemDeleted(dest);
}

/**
 Signal called when received statusbar information from one of the trees
 (tree items).<br>
 Re-emit the signal
 @param info Information from tree item
*/
void MultiTreeWindow::treeItemInfo(const QString &info) {
 emit itemInfo(info);
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
 @param pName Name used for this content stream
 @param pToolTip Tooltip used for this content stream
 */
void MultiTreeWindow::activate(boost::shared_ptr<CContentStream> cs,const QString &pName,const QString &pToolTip) {
 TreeKey tk(Tree_ContentStream,cs.get());
 //If the page already exist, just switch to it
 if (activate(tk)) return;

 //Create the page
 TreeWindow* t=createPage(pName,pToolTip);
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
 @param pName Name used for this property
 @param pToolTip Tooltip used for this property
 */
void MultiTreeWindow::activate(boost::shared_ptr<IProperty> doc,const QString &pName,const QString &pToolTip) {
 TreeKey tk(Tree_IProperty,doc.get());
 //If the page already exist, just switch to it
 if (activate(tk)) return;

 //Create the page
 TreeWindow* t=createPage(pName,pToolTip);
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
 @param pName Name used for this property
 @param pToolTip Tooltip used for this property
 */
void MultiTreeWindow::activate(const OperatorVector &vec,const QString &pName,const QString &pToolTip) {
 TreeKey tk(Tree_OperatorVector,NULL);
 if (!activate(tk)) {
  //Create the page if it does not exist
  TreeWindow* t=createPage(pName,pToolTip);
  t->init(vec,pName);
  trees.insert(tk,t);
  treesReverse.insert(t,tk);
 }
 trees[tk]->init(vec,pName);
 activate(tk);
}

/**
 Create if not exist and then activate secondary tree that contains given vector of annotations as root item
 This type of item is special, as if the tree already exist, its contents is replaced
 @param vec Annotation vector
 @param page Page with annotations
 @param pName Name used for this property
 @param pToolTip Tooltip used for this property
 */
void MultiTreeWindow::activate(const AnnotationVector &vec,boost::shared_ptr<CPage> page,const QString &pName,const QString &pToolTip) {
 TreeKey tk(Tree_OperatorVector,NULL);
 if (!activate(tk)) {
  //Create the page if it does not exist
  TreeWindow* t=createPage(pName,pToolTip);
  t->init(vec,page,pName);
  trees.insert(tk,t);
  treesReverse.insert(t,tk);
 }
 trees[tk]->init(vec,page,pName);
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
 Delete specified secondary tree that contains Annotation Vector
 @param vec Annotation vector, but in fact it is ignored
*/
void MultiTreeWindow::deactivate(__attribute__((unused)) const AnnotationVector &vec) {
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
 } else {
  guiPrintDbg(debug::DBG_DBG,"Trying to deactivate nonexistent tree");
 }
}

/**
 Try to delete specified secondary tree
 @param tr Secondary tree to delete
*/
void MultiTreeWindow::deleteWindow(TreeWindow *tr) {
 if (treeSelection==tr) {
  //If we are deleting window that is source for selection ...
  treeSelection=NULL;
 }
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

 assert(tree!=tr);
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
