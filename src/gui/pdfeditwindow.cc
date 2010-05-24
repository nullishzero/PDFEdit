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
 PdfEditWindow - class representing main application window
*/
#include "pdfeditwindow.h"
#include "qtcompat.h"
#include "xpdf/GlobalParams.h"
#include "additemdialog.h"
#include "basegui.h"
#include "commandwindow.h"
#include "dialog.h"
#include "menu.h"
#include "multitreewindow.h"
#include "pagespace.h"
#include "pdfutil.h"
#include "progressbar.h"
#include "propertyeditor.h"
#include "propertymodecontroller.h"
#include "settings.h"
#include "statusbar.h"
#include "toolbar.h"
#include "treeitem.h"
#include "treeitempdfoperator.h"
#include "util.h"
#include "main.h"
#include "version.h"
#include <iostream>
#include <qapplication.h>
#include <qfile.h>
#include <qfont.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qstring.h>
#include <utils/debug.h>
#include "optionwindow.h"

namespace gui {

using namespace std;
using namespace util;

/** number of active editor windows -> when last is closed, application terminates */
int windowCount;

/** application exit handler invoked when "Quit" is selected in menu/toolbar/etc ... */
void PdfEditWindow::exitApp() {
 guiPrintDbg(debug::DBG_INFO,"Exiting program");
 q_App->closeAllWindows();
 //Application will exit after last window is closed
 //todo: if invoked from qscript then handle specially
}

/**
 This is called on attempt to close window. If there is unsaved work,
 dialog asking to save it would appear, otherwise the windows is closed.
 @param e Close event
*/
void PdfEditWindow::closeEvent(QCloseEvent *e) {
 if (!closeFile(true)) { //user does not want to close and lose unsaved work
  e->ignore();
  return;
 }
 e->accept();
 saveWindowState();
 windowCount--;
 //The PdfEditWindow itself will be deleted on close();
}

/**
 Creates new windows and displays it.
 @param fName Name of file to open in new window. If no file specified, new window is initially empty
 @return Pointer to new window
 */
PdfEditWindow* PdfEditWindow::create(const QString &fName/*=QString::null*/) {
 PdfEditWindow *main=new PdfEditWindow(fName);
 main->restoreWindowState();
 main->show();
 windowCount++;
 return main;
}

/**
 Slot called when right-clicked in current page
 @param globalPos Position of mouse cursor
*/
void PdfEditWindow::pagePopup(__attribute__((unused)) const QPoint &globalPos) {
 base->call("onPageRightClick");
}

/** Close the window itself */
void PdfEditWindow::closeWindow() {
 base->stopScript();
 close();
}

/**
 Saves window visibility state to application settings
 @param w Widget to save
 @param name Name of key in settings
*/
void PdfEditWindow::saveVisibility(QWidget *w,const QString &name) {
 globalSettings->write(QString("windowstate/vis_")+name,w->isVisible());
}

/**
 Restore window visibility state from application settings
 @param w Widget to restore
 @param name Name of key in settings
*/
void PdfEditWindow::loadVisibility(QWidget *w,const QString &name) {
 bool vis=globalSettings->readBool(QString("windowstate/vis_")+name,true);
 if (vis) w->show(); else w->hide();
}

/** Saves window state to application settings*/
void PdfEditWindow::saveWindowState() {
 globalSettings->saveWindow(this,"main");
 globalSettings->saveSplitter(spl,"spl_main");
 globalSettings->saveSplitter(splProp,"spl_right");
 globalSettings->saveSplitter(splCmd,"spl_left");
 cmdLine->saveWindowState();
 saveVisibility(cmdLine,"cmd");
 saveVisibility(prop,"prop");
 saveVisibility(splProp,"right");
 saveVisibility(tree,"tre");
 menuSystem->saveToolbars();
}


/** Restores window state from application settings */
void PdfEditWindow::restoreWindowState() {
 globalSettings->restoreWindow(this,"main");
 globalSettings->restoreSplitter(spl,"spl_main");
 globalSettings->restoreSplitter(splProp,"spl_right");
 globalSettings->restoreSplitter(splCmd,"spl_left");
 cmdLine->restoreWindowState();
 loadVisibility(cmdLine,"cmd");
 loadVisibility(prop,"prop");
 loadVisibility(splProp,"right");
 loadVisibility(tree,"tre");
 menuSystem->restoreToolbars();
}

/**
 Show dialog for adding objects into given container.<br>
 Container must be Dict or Array, otherwise the dialog is not created.
 After creating, dialog is shown and usable and this function immediately returns.<br>
 @param ip Dict or Array into which objects will be added
*/
void PdfEditWindow::addObjectDialogI(boost::shared_ptr<IProperty> ip) {
 boost::shared_ptr<CDict> dict=boost::dynamic_pointer_cast<CDict>(ip);
 if (dict.get()) {
  AddItemDialog::create(this,dict);
  return;
 }
 boost::shared_ptr<CArray> array=boost::dynamic_pointer_cast<CArray>(ip);
 if (array.get()) {
  AddItemDialog::create(this,array);
  return;
 }
 guiPrintDbg(debug::DBG_WARN,"Trying to add objects into non-Dict/Array");
}

/**
 Add color selection tool to list of "known color selection tools"
 @param tool Tool to add
*/
void PdfEditWindow::addColorTool(ColorTool *tool) {
 base->addColorTool(tool);
}

/**
 Add line edit tool to list of "known line edit tools"
 @param tool Tool to add
*/
void PdfEditWindow::addEditTool(EditTool *tool) {
 base->addEditTool(tool);
}

/**
 Add number edit tool to list of "known number edit tools"
 @param tool Tool to add
*/
void PdfEditWindow::addNumberTool(NumberTool *tool) {
 base->addNumberTool(tool);
}

/**
 Add select text tool to list of "known select text tools"
 @param tool Tool to add
*/
void PdfEditWindow::addSelectTool(SelectTool *tool) {
 base->addSelectTool(tool);
}

/**
 Signal handler invoked on menu activation
 @param id Menu ID of clicked item
 */
void PdfEditWindow::menuActivated(int id) {
 QString action=menuSystem->getAction(id);
 guiPrintDbg(debug::DBG_INFO,"Performing menu action: " << Q_OUT(action));
 /* quit and closewindow are special - these can't be easily called from script
    as regular function, because invoking them calls window destructor, removing
    script interpreter instance in the process -> after returning from script
    control is moved to already freed script interpreter
  */
 if (action=="quit") exitApp();
 else if (action=="closewindow") closeWindow();
 else base->runScript(action);
}

/** Returns progress observer.
 * This progress observer holds qt progress bar (progressBar field) stored in
 * status bar.
 * It can be registered on ObserverHandler to display progress.
 */
boost::shared_ptr<pdfobjects::utils::ProgressObserver> PdfEditWindow::getProgressObserver() {
 return progressObserver;
}

/** Returns common progress bar.
 *
 * Note that this progress bar is shared and so user should keep in mind
 * that someone is using same progress bar while he starts to use it.
 *
 * @return Qt progress bar instance.
 */
QProgressBar * PdfEditWindow::getProgressBar() {
 return progressBar;
}

/**
 constructor of PdfEditWindow, creates window and fills it with elements
 @param parent parent widget containing this control
 @param name name of widget (currently unused)
 @param fName Name of file to open in this window. If empty or null, no file will be opened
 */
PdfEditWindow::PdfEditWindow(const QString &fName/*=QString::null*/,QWidget *parent/*=0*/,const char *name/*=0*/):QMainWindow(parent,name,Qt::WDestructiveClose | Qt::WType_TopLevel) {
 //Set the initial title
 setFileName(QString::null);
 document=boost::shared_ptr<CPdf>();
 selectedTreeItem=NULL;
 selectedPageNumber=0;
 //Horizontal splitter Preview + Commandline | Treeview + Property editor
 spl=new QSplitter(this,"horizontal_splitter");


 //Splitter between command line and preview window
 splCmd=new QSplitter(spl);
 splCmd->setOrientation(Vertical);

 //PageSpace
 pagespc = new PageSpace(splCmd,"PageSpace");

 //Some items from menu need to connect to PageSpace
 menuSystem=new Menu(this);

 //Commandline
 cmdLine=new CommandWindow(splCmd);

 //Splitter between treeview and property editor
 splProp=new QSplitter(spl);
 splProp->setOrientation(Vertical);

 //Base for scripting
 base=new BaseGUI(this);

 //Init command window (interpreter, context)
 cmdLine->setInterpreter(base->interpreter(),base);

 //object treeview
 tree=new MultiTreeWindow(base,splProp);

 //Property editor
 prop=new PropertyEditor(splProp);

 // creates qt progress bar and its adapter so it can be used in
 // ProgressObserver. progressObserver can be registered to observer handlers.
 progressBar=new QProgressBar();
 progressObserver=boost::shared_ptr<pdfobjects::utils::ProgressObserver>(
                 new pdfobjects::utils::ProgressObserver(
                         new ProgressBar(progressBar))
                 );

 // creates status bar and adds qt progress bar inside
 status=new StatusBar(this,"statusbar");
 status->addWidget(progressBar);

 // Read settings - which widget will initially have focus
 QString foc=globalSettings->read("gui/init_focus","pagespace");
 if (foc=="pagespace") {
  // Page preview window - scrolling, etc ...
  pagespc->setFocus();
 } else if (foc=="cmdline") {
  // Commandline - for more advanced users :)
  cmdLine->setFocus();
 }
 //Connections
 connect(cmdLine, SIGNAL(commandExecuted(QString)), this, SLOT(runScript(QString)));
 connect(tree, SIGNAL(itemSelected()), this, SLOT(setObject()));
 connect(tree, SIGNAL(itemDeleted(TreeItemAbstract*)), this, SLOT(unsetObjectIf(TreeItemAbstract*)));
 connect(tree, SIGNAL(itemDeleted(TreeItem*)), this, SLOT(unsetObjectIf(TreeItem*)));
 connect(tree, SIGNAL(treeClicked(int,QListViewItem*)), this, SLOT(treeClicked(int,QListViewItem*)));
 connect(globalSettings, SIGNAL(settingChanged(QString)), tree, SLOT(settingUpdate(QString)));
 connect(globalSettings, SIGNAL(settingChanged(QString)), this, SLOT(settingUpdate(QString)));
 connect(pagespc,SIGNAL(changedPageTo(const QSPage&,int)),this,SLOT(pageChange(const QSPage&,int)));
 connect(pagespc,SIGNAL(popupMenu(const QPoint&)),this,SLOT(pagePopup(const QPoint&)));
 connect(pagespc,SIGNAL(changeSelection(std::vector<boost::shared_ptr<PdfOperator> >)),this,SLOT(setSelection(std::vector<boost::shared_ptr<PdfOperator> >)));
 connect(pagespc,SIGNAL(changeSelection(std::vector<boost::shared_ptr<CAnnotation> >)),this,SLOT(setSelection(std::vector<boost::shared_ptr<CAnnotation> >)));
 connect(pagespc,SIGNAL(deleteSelection()),this,SLOT(pageDeleteSelection()));

 connect(pagespc, SIGNAL(executeCommand(QString)), this, SLOT(runScript(QString)));
 connect(prop,SIGNAL(infoText(const QString&)),status,SLOT(receiveInfoText(const QString&)));
 connect(prop,SIGNAL(warnText(const QString&)),status,SLOT(receiveWarnText(const QString&)));
 connect(tree,SIGNAL(itemInfo(const QString&)),status,SLOT(message(const QString&)));

 this->setCentralWidget(spl);

 //load and apply "uses big icon" settings
 bigPixmap();

 //Menu
 try {
  QMenuBar *qb=menuSystem->loadMenu(this);
  QObject::connect(qb, SIGNAL(activated(int)), this, SLOT(menuActivated(int)));

  //ToolBars
  ToolBarList tblist=menuSystem->loadToolBars();
  for (ToolBarList::Iterator toolbar=tblist.begin();toolbar!=tblist.end();++toolbar) {
   QObject::connect(*toolbar,SIGNAL(itemClicked(int)),this,SLOT(menuActivated(int)));
   QObject::connect(*toolbar,SIGNAL(helpText(const QString&)),this,SLOT(receiveHelpText(const QString&)));
  }
 } catch (InvalidMenuException &e) {
  guiPrintDbg(debug::DBG_WARN,"Exception in menu loading raised");
  fatalError(e.message());
 }

 //Icon
 const QPixmap *appIcon=menuSystem->getIcon(globalSettings->read("icon/app","pdfedit_icon_32.png"));
 if (appIcon) {
  setIcon(*appIcon);
 }

 //Script must be run AFTER creating all widgets
 //Script may need them, especially the command window
 //Run initscript
 //Any document-related classes are NOT available to the initscript, as no document is currently loaded
 //initscript may use onLoad() to perform some code after document is loaded

 // If the script will do something like loading a document, it will be unloaded after all scripts are done
 // (and replaced by document specified on commandline, if any)
 base->runInitScript();

 if (fName.isNull()) { //start with empty editor
  emptyFile();
  base->call("onEmptyFile");
 } else { //open file
  openFile(fName);
 }
}

/**
 Slot called when deleteSelection from pagespace is emitted.
 Script callback function is invoked.
*/
void PdfEditWindow::pageDeleteSelection() {
 base->call("onPageDelete");
}


/**
 Signal called when receiving help message.
 Show it in statusbar
 @param message Help message
*/
void PdfEditWindow::receiveHelpText(const QString &message) {
 status->message(message);
}

/** Load and apply current value of "Use big pixmap" setting */
void PdfEditWindow::bigPixmap() {
 setUsesBigPixmaps(globalSettings->readBool("icon/theme/big"));
}

/**
 Unset selected object if it is the one given in parameter
 (because it is about to be deleted)
 @param theItem object to check (and possibly unselect)
*/
void PdfEditWindow::unsetObjectIf(TreeItemAbstract *theItem) {
 if (theItem==selectedTreeItem) {
  guiPrintDbg(debug::DBG_DBG,"Removed reference to deleted tree item");
  selectedTreeItem=NULL;
  //Unset from property editor
  prop->setObject(NULL);
  //Selection have probably just changed
  //base->call("onTreeSelectionChange");
 }
 // look for wrappers using this tree item and somehow disable them
 base->treeItemDeleted(theItem);
}

/**
 Function called when deleting tree item of type TreeItem
 @param ipItem object to check (and possibly unselect)
*/
void PdfEditWindow::unsetObjectIf(TreeItem *ipItem) {
 guiPrintDbg(debug::DBG_DBG,"Iproperty was just deleted");
 //Iproperty is about to be deleted -> warn all "add object" dialogs
 boost::shared_ptr<IProperty> it=ipItem->getObject();
 emit itemDeleted(it);
}

/** Called upon selecting some item in treeview */
void PdfEditWindow::setObject() {
 selectedTreeItem=tree->getSelectedItem(QString::null);
 guiPrintDbg(debug::DBG_DBG,"selectedTreeItem " << (intptr_t)selectedTreeItem );
 selectedProperty.reset();
 selectedOperator.reset();
 TreeItemPdfOperator *pdfOp=dynamic_cast<TreeItemPdfOperator*>(selectedTreeItem);
 if (pdfOp) {
  // Give operator to property editor to edit its parameters
  selectedOperator=pdfOp->getObject();
  prop->setObject(selectedOperator);
  base->call("onTreeSelectionChange");
  return;
 }
 TreeItem *iProp=dynamic_cast<TreeItem*>(selectedTreeItem);
 if (iProp) {
  // Give iproperty to property editor to edit it
  selectedProperty=iProp->getObject();
  prop->setObject(selectedTreeItem->text(0),selectedProperty);
  base->call("onTreeSelectionChange");
  return;
 }
 //Some unknown type
 prop->unsetObject();
 base->call("onTreeSelectionChange");
}

/**
 Slot called on selecting something from page -> display it in the tree
 @param vec Vector with operators
*/
void PdfEditWindow::setSelection(std::vector<boost::shared_ptr<PdfOperator> > vec) {
 if (vec.size()) {
  //Selected objects
  tree->activate(vec,tr("Selection"),tr("Selected operators"));
 } else {
  //Selection is empty
  tree->deactivate(vec);
 }
}

/**
 Slot called on selecting something from page -> display it in the tree
 @param vec Vector with annotations
*/
void PdfEditWindow::setSelection(std::vector<boost::shared_ptr<CAnnotation> > vec) {
 if (vec.size()) {
  //Selected objects
  tree->activate(vec,selectedPage,tr("Selection"),tr("Selected annotations"));
 } else {
  //Selection is empty
  tree->deactivate(vec);
 }
}

/**
 Slot called on changing the page in preview window
 @param pg New page that was just changed to
 @param numberOfPage Number of page
*/
void PdfEditWindow::pageChange(const QSPage &pg, int numberOfPage) {
 selectedPage=pg.get();
 selectedPageNumber=numberOfPage;
 base->call("onPageChange");
}

/**
 Return currently selected page number
 @return current page number
 */
int PdfEditWindow::pageNumber() {
 return selectedPageNumber;
}

/**
 Change currently active revision in document
 @param revision New revision
*/
void PdfEditWindow::changeRevision(int revision) {
 if (revision<0) return;	//Revision is positive number
 if (!document) return;		//No document loaded
 document->changeRevision(revision);
 prop->clear();
 pagespc->refresh(selectedPageNumber,base->getQSPdf());//Try to keep on the same page in the new revision
 tree->clearSecondary();//TODO: secondary trees may survive, but we have no observers on stream contents. It is easier to close them for safety
 tree->reload();
 setTitle(revision);
 emit revisionChanged(revision);
 base->call("onChangeRevision");
}

/**
 Check whether currently opened document was modified
 since it was opened or last saved
 @return true if document was modified, false if not
*/
bool PdfEditWindow::modified() {
 if (!document) return false;//no document loaded at all
 return document->isChanged();
}

/** \copydoc Base::runScript */
void PdfEditWindow::runScript(QString script) {
 base->runScript(script);
}

/**
 Return list of available length units
 @return list of units
*/
QStringList PdfEditWindow::allUnits() {
 return pagespc->getAllUnits();
}

/**
 Return list of available length unit identifiers
 Identifiers are 1:1 with units returned by allUnits
 @return list of units
*/
QStringList PdfEditWindow::allUnitIds() {
 return pagespc->getAllUnitIds();
}

/**
 Called when any settings are updated (in script, option editor, etc ...)
 @param key Key of setting that was updated
 */
void PdfEditWindow::settingUpdate(QString key) {
 guiPrintDbg(debug::DBG_DBG,"Settings observer: " << Q_OUT(key));
 if (key.startsWith("toolbar/")) { //Show/hide toolbar
  ToolBar *tb=menuSystem->getToolbar(key.mid(8));	// 8=strlen("toolbar/")
  if (!tb) return; //Someone put invalid toolbar in settings. Just ignore it
  bool vis=globalSettings->readBool(key,true);
  if (vis) tb->show();
   else    tb->hide();
  return;
 }
 if (key=="gui/PageSpace/ViewedUnits") {
  //Changing units
  pagespc->setDefaultUnits(globalSettings->read(key));
  return;
 }
 if (key=="editor/charset") {
  //charset change
  setDefaultCharset(globalSettings->read("editor/charset"));
  prop->reloadItem();//since we changed the charset ...
  return;
 }
 if (key.startsWith("editor/")) { //Something from property editor
  prop->checkOverrides();
  return;
 }
 if (key=="tree/show_dict_sort") { //Sort dict keys
  prop->reloadItem();
  return;
 }
 if (key=="history/save_filePath") {
  //Do not remember path -> remove stored path(s)
  if (!globalSettings->readBool("history/save_filePath")) {
   globalSettings->removeAll("history/path");
  }
  return;
 }
 if (key.startsWith("gui/CommandLine/")) {
  cmdLine->reloadSettings();
  return;
 }
 if (key=="icon/theme/big") {
  //Size of icons changed
  bigPixmap();
  return;
 }
 if (key=="gui/style") {
  applyStyle();
 }
}

/**
 Closes file currently opened in editor.
 @param askSave Ask about saving changes?
 @param onlyAsk Only ask about closing/saving work, do not actually close file
 @return true if user accepted the close or saved the document before,
 false if user refuses the close
*/
bool PdfEditWindow::closeFile(bool askSave,bool onlyAsk/*=false*/) {
 if (modified() && askSave) {
  int answer=base->question_ync(fileName+"\n"+tr("Current file is not saved. Do you want to save it?"));
  if (answer==1) { //Yes: save it
   if (!save()) {
    base->warn(base->error());
    //Unable to save document -> not closing;
    return false;
   }
  }
  if (answer==-1) return false; //Cancel: abort closing
  // No -> continue happily, no saving wanted
 }
 if (!onlyAsk) {
  destroyFile();
  emptyFile();
  base->call("onEmptyFile");
 }
 return true;
}

/**
 Runs script from given file, looking in script path first
 @param scriptName name of file with QT Script to run
*/
void PdfEditWindow::run(const QString &scriptName) {
 base->run(scriptName);
}

/**
 Runs script from given file
 @param scriptName name of file with QT Script to run
*/
void PdfEditWindow::runFile(const QString &scriptName) {
 base->run(scriptName,true);
}

/**
 Run given script code
 @param code Code to evaluate
 */
void PdfEditWindow::eval(const QString &code) {
 base->runScript(code);
}

/**
 Save currently edited document to disk
 If the document have no name (newly opened/generated document), it is solicited from used via dialog.
 @param newRevision If true, create new revision while saving
 @return true if saved succesfully, false if failed to save because of any reason
*/
bool PdfEditWindow::save(bool newRevision/*=false*/) {
 base->setError("");//No error
 if (!document) {
  base->setError(tr("No document to save"));
  return false;
 }
 if (fileName.isNull()) { //We need a name
  //We don't know the filename and there is nothing like "save as",
  //so we are a bit out of luck here ...
  assert(0);//Should never get here anyway
  QString name=base->fileSaveDialog(filename());
  if (name.isNull()) {
   base->setError(tr("Name is empty"));
   return false; //Still no name? Not saving ...
  }
  try {
   //Exception can occur while saving, for example if document is read-only
   document->save(newRevision);
  } catch (ReadOnlyDocumentException &e) {
   base->setError(tr("Document is in read-only mode"));
   return false;
  } catch (NotImplementedException &e) {
   base->setError(tr("Saving not implemented:\n%1").arg(e.what()));
   return false;
  } catch (...) {
   base->setError(tr("Unknown error occured while saving document"));
   return false;
  }
  setFileName(name);
  return true;
 }
 try {
  //Exception can occur while saving, for example if document is read-only
  document->save(newRevision);
 } catch (ReadOnlyDocumentException &e) {
  base->setError(tr("Document is in read-only mode"));
  return false;
 } catch (...) {
  base->setError(tr("Unknown error occured while saving document"));
  return false;
 }
 guiPrintDbg(debug::DBG_DBG,"Saved document");
 if (newRevision) {
  guiPrintDbg(debug::DBG_DBG,"Emit new revision signal");
  //Reload the revision list
  emit documentChanged(document);
 }
 return true;
}

/**
 Return name of file loaded in editor. Return empty string if the document have no name
 (if it is not loaded from disk and have not yet been saved)
 @return Filename
*/
QString PdfEditWindow::filename() {
 if (fileName.isNull()) return "";
 return fileName;
}

/**
 Save currently edited document to disk, using provided filename
 @param name New filename to save document under
 @return true if saved succesfully, false if failed to save because of any reason
*/
bool PdfEditWindow::saveCopy(const QString &name) {
 base->setError("");//No error
 if (!document) {
  base->setError(tr("No document to save"));
  return false;
 }
 QString errorString;
 bool res=util::saveCopy(document,name,&errorString);
 if (!res) {
  base->setError(tr("Error occured while saving copy:\n%1").arg(errorString));
 }
 return res;
}

/**
 Assign given filename for document in editor. Also updates window title, etc ...
 @param name New filename for document
*/
void PdfEditWindow::setFileName(const QString &name) {
 fileName=name;
 if (name.isNull()) { //No name
  setCaption(QString(APP_NAME)+" "+QString(PDFEDIT_VERSION));
  baseName=QString::null;
  return;
 }
 //Add name of file without path to window title
 baseName=name;
 baseName.replace(QRegExp("^.*/"),"");
 setTitle();
}

/**
 Set window title, according to stored baseName and active revision number,
 and also some other facts (if document is linearized, etc ...)
 @param revision Revision number
 */
void PdfEditWindow::setTitle(int revision/*=0*/) {
 QString revisionInfo="";
 if (revision) revisionInfo=QString(" - ")+tr("viewing revision")+" "+QString::number(revision);
 QStringList docFlags;
 if (document->getMode()==CPdf::ReadOnly) docFlags+=tr("Read-only");
 if (document->isLinearized()) docFlags+=tr("Linearized PDF");
 if (pdfobjects::utils::isEncrypted(document)) docFlags+=tr("Encrypted");
 QString docInfo="";
 if (docFlags.count()) {
  docInfo=" ( "+docFlags.join(", ")+" )";
 }
 setCaption(QString(APP_NAME)+" - "+baseName+revisionInfo+docInfo);
}

/** Closes file currently opened in editor, without opening new empty one */
void PdfEditWindow::destroyFile() {
 //Now it is good time to kill all those widgets
 emit selfDestruct();
 if (!document) return;
 tree->uninit();//clear treeview
 prop->clear();//clear property editor
 selectedProperty.reset();//no item selected
 selectedOperator.reset();//no operator selected
 selectedPage.reset();//no page selected
 selectedPageNumber=0;//no page selected
 document.reset();
 base->destroyDocument();
// Doing GC on closing file may have side effects, so it's better not to do it
// base->cleanup();//Garbage collection on scripting objects
 document=boost::shared_ptr<CPdf>();
 setFileName(QString::null);
}

/**
 Open file in editor.
 @param name Name of file to open
 @param askPassword Ask user directly for password? If not, password have to be set other ways if necessary (via script for example)
 @return True if success, false if failure
*/
bool PdfEditWindow::openFile(const QString &name, bool askPassword/*=true*/) {
 destroyFile();
 if (name.isNull()) {
  base->setError(tr("Name is empty"));
  return false; //Still no name? Not saving ...
 }
 CPdf::OpenMode mode=globalSettings->readBool("mode/advanced")?(CPdf::Advanced):(CPdf::ReadWrite);
 try {
  guiPrintDbg(debug::DBG_DBG,"Opening document");
  document=getPdfInstance(this,name,mode,askPassword);
  if (askPassword && document->needsCredentials()) {
   //User failed to enter correct password when asked.
   //resets document instance to force closing
   document.reset();
   base->setError(tr("Password entered for document is not valid"));
   //File failed to open, keep window opened with empty file.
   emptyFile();
   base->call("onLoadError");
   return false;
  }
  // registers observer with progress bar on document writer
  // this will cause displaying of content saving progress
  pdfobjects::utils::IPdfWriter * writer=document->getPdfWriter();
  if (writer) {
   REGISTER_PTR_OBSERVER(writer, progressObserver);
  }
  assert(document);
  guiPrintDbg(debug::DBG_DBG,"Opened document");
  PropertyModeController *modeCtrl=PropertyModeController::getInstance();
  document->setModeController(modeCtrl->get());
 } catch (PdfOpenException &ex) {
  string err;
  ex.getMessage(err);
  base->setError(tr("Error while loading document ")+name+"\n"+util::convertToUnicode(err,UTF8));
  //File failed to open, keep window opened with empty file.
  emptyFile();
  base->call("onLoadError");
  return false;
 } catch (...) {
  base->setError(tr("Unknown error while loading document ")+name);
  //File failed to open, keep window opened with empty file.
  emptyFile();
  base->call("onLoadError");
  return false;
 }
 base->importDocument(document);
 setFileName(name);
 tree->init(document,baseName);
 emit documentChanged(document);
 base->print(tr("Loaded file")+" : "+name);
 base->call("onLoad");
 base->call("onLoadUser");
 return true;
}

/** Opens new empty file in editor. */
void PdfEditWindow::emptyFile() {
 destroyFile();
// document=boost::shared_ptr<CPdf>();
 document.reset();
 base->importDocument(document);
 tree->uninit();
 emit documentChanged(document);
 setFileName(QString::null);
 selectedPageNumber=0;
 pagespc->refresh((QSPage*)NULL,(QSPdf*)NULL);
}

/**
 Called when clicked anywhere in tree windows
 @param button Button used to click (1=left, 2=right, 4=middle, 8=doubleclick with left)
 @param item Item that was clicked on, if clicked outside item, NULL is sent
*/
void PdfEditWindow::treeClicked(int button,QListViewItem *item) {
 //clicks outside any items are irrelevant
 if (!item) return;
 TreeItemAbstract* it=dynamic_cast<TreeItemAbstract*>(item);
 //Some unknown tree item?
 if (!it) return;
 selectedTreeItem=it;
 if (button & 1) base->call("onTreeLeftClick");
 if (button & 2) base->call("onTreeRightClick");
 if (button & 4) base->call("onTreeMiddleClick");
 if (button & 8) base->call("onTreeDoubleClick");
}

/**
 Return reference to pagespace (needed for example by ZoomTool)
 @return PageSpace inside this window
 */
PageSpace* PdfEditWindow::getPageSpace() const {
 return pagespc;
}

/** default destructor */
PdfEditWindow::~PdfEditWindow() {
 destroyFile();
 base->stopScript();
 delete menuSystem;
 delete base;
}

} // namespace gui
