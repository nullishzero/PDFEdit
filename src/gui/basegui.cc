/** @file
 GUI Base - class containing extra functionalyty presen only in GUI
 @author Martin Petricek
*/

#include "basegui.h"
#include "aboutwindow.h"
#include "consolewritergui.h"
#include "commandwindow.h"
#include "dialog.h"
#include "helpwindow.h"
#include "multitreewindow.h"
#include "menu.h"
#include "optionwindow.h"
#include "pagespace.h"
#include "pdfeditwindow.h"
#include "propertyeditor.h"
#include "qscobject.h"
#include "qsimporter.h"
#include "qsiproperty.h"
#include "qsmenu.h"
#include "qspage.h"
#include "qspdf.h"
#include "qstreeitem.h"
#include "settings.h"
#include "statusbar.h"
#include "treeitemabstract.h"
#include "util.h"
#include "version.h"
#include <string.h>
#include <qdir.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qsplitter.h>
#include <utils/debug.h>

namespace gui {

using namespace util;

/**
 Create new Base class 
 @param parent Parent editor window containing this class
*/
BaseGUI::BaseGUI(PdfEditWindow *parent) : Base() {
 w=parent;
 import->addQSObj(w->pagespc,"PageSpace");
 import->addQSObj(w->cmdLine,"CommandWindow");

 //Console writer;
 consoleWriter=new ConsoleWriterGui(parent->cmdLine);
 setConWriter(consoleWriter);
}

/** destructor */
BaseGUI::~BaseGUI() {
 delete consoleWriter;
}

/**
 Run all initscripts.
 Gets name of initscripts from settings
*/
void BaseGUI::runInitScript() {
 QStringList initScripts=globalSettings->readPath("init","script/");
 int scriptsRun=runScriptList(initScripts);
 //Run list of initscripts from settings
 if (!scriptsRun) {
  //No init scripts found - print a warning
  warn(tr("No init script found - check your configuration")+"!\n"+tr("Looked for","scripts")+":\n"+initScripts.join("\n"));
 }
 // Run initscripts from paths listed in settings,
 // initscript with same name is executed only once,
 // initscripts in later paths take priority
 QStringList initScriptPaths=globalSettings->readPath("init_path","script/");
 runScriptsFromPath(initScriptPaths);
}

/**
 For given name return widget represented by that name
 @param widgetName Widget name
 @return Pointer to widget, or NULL if no widget matches the name
*/
QWidget* BaseGUI::getWidgetByName(const QString &widgetName) {
 QString widget=widgetName.lower();
 if (widget=="commandline") return w->cmdLine;
 if (widget=="statusbar") return w->status;
 if (widget=="propertyeditor") return w->prop;
 if (widget=="rightside") return w->splProp;
 if (widget=="tree") return w->tree;
 //Widget not found ...
 return NULL;
}

/**
 Function to be run before the script is executed
 @param script Script code;
 @param callback is it callback from script?
*/
void BaseGUI::preRun(const QString &script,bool callback/*=false*/) {
 Base::preRun(script,callback);
 if (callback) return;
 /*
  Commit currently edited property in property editor
  This must be done this way, because by clicking on some button in toolbar to perform some action,
  for example reloading a page, does not cause the property to lose focus and update automatically
 */
 w->prop->commitProperty();
}

/**
 Function to be run after the script is executed
*/
void BaseGUI::postRun() {
 Base::postRun();
 if (treeReloadFlag) {
  //Reload the tree
  w->tree->reload();
  treeReloadFlag=false;
 }
}

/**
 Removes objects added with addScriptingObjects
 \see addScriptingObjects
 */
void BaseGUI::removeScriptingObjects() {
 //delete page and item variables from script -> they may change while script is not executing
 deleteVariable("item");
//TODO: treeitem as property of this QObject
 deleteVariable("treeitem");
 Base::removeScriptingObjects();
}

/**
 Create objects that should be available to scripting from current CPdf and related objects
 \see removeScriptingObjects
*/
void BaseGUI::addScriptingObjects() {
 //Import treeitem and item (Currently selected treeitem and currently selected object)
 import->addQSObj(treeitem(),"treeitem");
 import->addQSObj(item(),"item");
 Base::addScriptingObjects();
}

/** Return current tree item (treeitem under cursor) */
QSCObject* BaseGUI::treeitem() {
 return import->createQSObject(w->selectedTreeItem);
}

/** Return current object (currently selected item in property editor) */
QSCObject* BaseGUI::item() {
 if (w->selectedProperty.get()) {
  //IProperty is selected and will be imported
  return import->createQSObject(w->selectedProperty);
 }
 if (w->selectedOperator.get()) {
  //PdfOperator is selected and will be imported
  return import->createQSObject(w->selectedOperator);
 }
 return NULL;
}

/**
 Callback from main window when the treeitem got just deleted
 This will look for script wrappers containing the mentioned item and invalidate them, so they will not cause a crash
 @param theItem deleted tree item
*/
void BaseGUI::treeItemDeleted(TreeItemAbstract* theItem) {
 //Get dictionary with all wrappers for given treeitem
 QPtrDict<void>* pDict=treeWrap[theItem];
 if (!pDict) {
  //No wrapper exists. Done.
  //guiPrintDbg(debug::DBG_DBG,"Item deleted that is not in wrapper"); 
  return;
 }
 //Ok, now disable all wrappers pointing to this item

 // For each wrapper
 QPtrDictIterator<void> it(*pDict);
 for(;it.current();++it) {
  QSTreeItem* theWrap=reinterpret_cast<QSTreeItem*>(it.currentKey());
  guiPrintDbg(debug::DBG_DBG,"Disabling wrapper " << (intptr_t)theWrap << " w. item "<< (intptr_t)theItem); 
  assert(theWrap);
  guiPrintDbg(debug::DBG_DBG,"Check type: " << theWrap->type()); 
  //Disable the wrapper, so calling it will not result in crash, but some error/exception instead
  theWrap->disable();
  guiPrintDbg(debug::DBG_DBG,"Disabled wrapper"); 
 }
 //Remove reference to subdictionary from dictionary
 treeWrap.remove(theItem);
 //Autodelete is on, so the inner dictionary will be deleted ....
}

// === Scripting functions ===

/** Shows "About" window */
void BaseGUI::about() {
 AboutWindow *aboutWin= new AboutWindow(w);
 aboutWin->show();
}

/** 
 Show dialog for adding objects into given container.<br>
 Container must be Dict or Array, otherwise the dialog is not created.
 If Container is NULL, currently selected object ihn property editor will be attempted to use as container..<br>
 After creating, dialog is shown and usable and this function immediately returns.<br>
 @param container Dict or Array into which objects will be added
*/
void BaseGUI::addObjectDialog(QSIProperty *container/*=NULL*/) {
 if (container) {
  w->addObjectDialogI(container->get());
 } else {
  w->addObjectDialogI(w->selectedProperty);
 }
}

/** \copydoc addObjectDialog(QSIProperty *) */
void BaseGUI::addObjectDialog(QObject *container) {
 //This method is fix for QSA Bug: QSA sometimes degrade QObject descendants to bare QObjects
 QSIProperty *_container=dynamic_cast<QSIProperty*>(container);
 if (_container) {
  w->addObjectDialogI(_container->get());
 } else {
  guiPrintDbg(debug::DBG_ERR,"type Error: " << container->className());
  w->addObjectDialogI(w->selectedProperty);
 }
}

/** \copydoc Menu::checkByName */
void BaseGUI::checkItem(const QString &name,bool check) {
 w->menuSystem->checkByName(name,check);
}

/** \copydoc PdfEditWindow::exitApp */
void BaseGUI::closeAll() {
 w->exitApp();
}

/** \copydoc PdfEditWindow::closeFile */
bool BaseGUI::closeFile(bool askSave,bool onlyAsk/*=false*/) {
 return w->closeFile(askSave,onlyAsk);
}

/** \copydoc PdfEditWindow::closeWindow */
void BaseGUI::closeWindow() {
 w->closeWindow();
}

/** create new empty editor window and display it */
void BaseGUI::createNewWindow() {
 PdfEditWindow::create();
}

/** \copydoc Menu::enableByName */
void BaseGUI::enableItem(const QString &name,bool enable) {
 w->menuSystem->enableByName(name,enable);
}

/** \copydoc PdfEditWindow::filename() */
QString BaseGUI::filename() {
 return w->filename();
}

/**
 Show "open file" dialog and return file selected, or NULL if dialog was cancelled
 @return name of selected file.
 */
QString BaseGUI::fileOpenDialog() {
 guiPrintDbg(debug::DBG_DBG,"fileOpenDialog");
 return openFileDialogPdf(w);
}

/**
 Show "save file" dialog and return file selected, or NULL if dialog was cancelled
 @param oldName Old name of the file (if known) - will be preselected
 @return name of selected file.
 */
QString BaseGUI::fileSaveDialog(const QString &oldName/*=QString::null*/) {
 guiPrintDbg(debug::DBG_DBG,"fileSaveDialog");
 QString ret=saveFileDialogPdf(w,oldName);
 return ret;
}

/**
 Invokes program help. Optional parameter is topic - if invalid or not defined, help title page will be invoked
 @param topic Starting help topic
*/
void BaseGUI::help(const QString &topic/*=QString::null*/) {
 HelpWindow *hb=new HelpWindow(topic);
 hb->show();
}

/**
 Check if a widget is visible
 \see getWidgetByName
 @param widgetName name of widget
 @return True if widget is visible, false if not
*/
bool BaseGUI::isVisible(const QString &widgetName) {
 QWidget *w=getWidgetByName(widgetName);
 if (!w) return false;
 return w->isVisible();
}

/**
 Brings up informational messagebox with given message
 @param msg Message to display
 */
void BaseGUI::message(const QString &msg) {
 QMessageBox::information(w,APP_NAME,msg,QObject::tr("&Ok"),QString::null,QString::null,QMessageBox::Ok | QMessageBox::Default);
}

/** \copydoc PdfEditWindow::modified */
bool BaseGUI::modified() {
 return w->modified();
}

/** \copydoc PdfEditWindow::openFile */
bool BaseGUI::openFile(const QString &name) {
 return w->openFile(name);
}

/**
 Open file in a new editor window.
 @param name Name of file to open
*/
void BaseGUI::openFileNew(const QString &name) {
 PdfEditWindow::create(name);
}

/** Show options dialog. Does not wait for dialog to finish. */
void BaseGUI::options() {
 OptionWindow::optionsDialog(w->menuSystem);
}

/**
 Return number of currently shown page
 @return page number
*/
int BaseGUI::pageNumber() {
 return w->selectedPageNumber;
}

/**
 Return currently shown page
 @return page
*/
QSPage* BaseGUI::page() {
 if (w->selectedPage.get()) {
  //REturn selected page
  return new QSPage(w->selectedPage,this);
 } else {
  //No page selected currently
  return NULL;
 }
}

/**
 Invoke dialog to select color.
 Last selected color is remembered and offered as default next time.
 The 'initial default color' is red
 @return selected color, or invalid color if the dialog was cancelled
*/
QVariant BaseGUI::pickColor() {
 QColor ret=colorDialog(w);
 if (!ret.isValid()) {
  return QVariant();
  guiPrintDbg(debug::DBG_DBG,"Color is not valid");
 }
 return QVariant(ret);
}

/**
 Create and return new popup menu, build from menu list/item  identified by this name.
 If no item specified, menu is initially empty
 @param menuName Name of menu inconfiguration to use as template
 @return initialized QSMenu Object
*/
QSMenu* BaseGUI::popupMenu(const QString &menuName/*=QString::null*/) {
 return new QSMenu(w->menuSystem,this,menuName);
}

/**
 Asks question with Yes/No answer. "Yes" is default.
 Return true if user selected "yes", false if user selected "no"
 @param msg Question to display
 @return True if yes, false if no
*/
bool BaseGUI::question(const QString &msg) {
 return questionDialog(w,msg);
}

/**
 Asks question with Yes/No/Cancel answer. "Yes" is default. <br>
 Return one of these three according to user selection: <br>
  Yes : return 1     <br>
  No : return 0      <br>
  Cancel : return -1
 @param msg Question to display
 @return Selection of user.
 */
int BaseGUI::question_ync(const QString &msg) {
 int answer=QMessageBox::question(w,APP_NAME,msg,QObject::tr("&Yes"),QObject::tr("&No"),QObject::tr("&Cancel"),0,2);
 // QMessageBox::Yes | QMessageBox::Default,QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape
 switch (answer) {
  case 0: return 1;	//QMessageBox::Yes
  case 1: return 0;	//QMessageBox::No
  case 2: return -1;	//QMessageBox::Cancel
  default: assert(0);
 }
}

/** \copydoc PdfEditWindow::restoreWindowState */
void BaseGUI::restoreWindowState() {
 w->restoreWindowState();
}

/**
 Save currently edited document to disk
 @return true if saved succesfully, false if failed to save because of any reason
 */
bool BaseGUI::save() {
 return w->save();
}

/** \copydoc PdfEditWindow::saveCopy */
bool BaseGUI::saveCopy(const QString &name) {
 return w->saveCopy(name);
}

/**
 Save currently edited document to disk, while creating a new revisiion
 @return true if saved succesfully, false if failed to save because of any reason
 */
bool BaseGUI::saveRevision() {
 return w->save(true);
}

/** \copydoc PdfEditWindow::saveWindowState */
void BaseGUI::saveWindowState() {
 w->saveWindowState();
}

/**
 Change active revision in current PDF document
 @param revision number of revision to activate
 */
void BaseGUI::setRevision(int revision) {
 w->changeRevision(revision);
}

/**
 Set widget to be either visible or invisible
 \see getWidgetByName
 @param widgetName name of widget
 @param visible action to be performed (true = show, false = hide)
*/
void BaseGUI::setVisible(const QString &widgetName, bool visible) {
 QWidget *w=getWidgetByName(widgetName);
 if (!w) return;
 if (visible) w->show(); else w-> hide();
}

/**
 Return root item of currently selected tree
 @return Current tree root item
*/
QSTreeItem* BaseGUI::treeRoot() {
 return dynamic_cast<QSTreeItem*>(import->createQSObject(w->tree->root()));
}

/**
 Return root item of main tree
 @return Main tree root item
*/
QSTreeItem* BaseGUI::treeRootMain() {
 return dynamic_cast<QSTreeItem*>(import->createQSObject(w->tree->rootMain()));
}

/**
 Show this string as a warning in a messagebox and also print it to console, followed by newline
 @param str String to use as warning
 */
void BaseGUI::warn(const QString &str) {
 conPrintLine(str);
 QMessageBox::warning(w,tr("Warning"),str);
}

// Tree-selection related slots

/**
 Return first selected tree item from specified tree
 Return NULL, if no item is selected
 @param name Name of tree to get selection from
 @return Selected tree item
*/
QSTreeItem* BaseGUI::firstSelectedItem(const QString &name/*=QString::null*/) {
 return dynamic_cast<QSTreeItem*>(import->createQSObject(w->tree->getSelectedItem(name)));
}

/**
 Return next selected tree item from specified tree
 Return NULL, if no next item is selected
 @return Selected tree item
*/
QSTreeItem* BaseGUI::nextSelectedItem() {
 return dynamic_cast<QSTreeItem*>(import->createQSObject(w->tree->nextSelectedItem()));
}

/**
 Return object held in first selected tree item from specified tree
 Return NULL, if no item is selected
 @param name Name of tree to get selection from
 @return Selected tree item's object
*/
QSCObject* BaseGUI::firstSelected(const QString &name/*=QString::null*/) {
 return w->tree->getSelected(name);
}

/**
 Return object held in  next selected tree item from specified tree
 Return NULL, if no next item is selected
 @return Selected tree item's object
*/
QSCObject* BaseGUI::nextSelected() {
 return w->tree->nextSelected();
}


// === Non-scripting slots ===

#ifdef DRAGDROP

/**
 Invoked when dragging one item to another within the same tree window
 @param source Source (dragged) item
 @param target Target item
*/
void BaseGUI::_dragDrop(TreeItemAbstract *source,TreeItemAbstract *target) {
 //Import items
 QSCObject *src=import->createQSObject(source);
 QSCObject *tgt=import->createQSObject(target);
 import->addQSObj(src,"source");
 import->addQSObj(tgt,"target");
 call("onDragDrop");
 //delete page and item variables from script -> they may change while script is not executing
 deleteVariable("source");
 deleteVariable("target");
}

/**
 Invoked when dragging one item to another in different tree window (possibly in different document)
 @param source Source (dragged) item
 @param target Target item
*/
void BaseGUI::_dragDropOther(TreeItemAbstract *source,TreeItemAbstract *target) {
 /*
  More complicated, if the script keeps something from second tree
  and the second document is closed, the editor may crash.
  We need to make local copy of source's content
 */
 QSCObject *src=source->getQSObject(this);//Rebased object. May be NULL if rebase not possible
 QSCObject *tgt=import->createQSObject(target);
 import->addQSObj(src,"source");
 import->addQSObj(tgt,"target");
 call("onDragDropOther");
 deleteVariable("source");
 deleteVariable("target");
}

#endif

} // namespace gui
