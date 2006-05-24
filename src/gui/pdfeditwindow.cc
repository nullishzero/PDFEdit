/** @file
 PdfEditWindow - class representing main application window
*/
#include "pdfeditwindow.h"
#include <iostream>
#include <utils/debug.h>
#include <qfile.h>
#include <qregexp.h>
#include <qmenubar.h>
#include <qmessagebox.h> 
#include <qsplitter.h>
#include <qstring.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include "pagespace.h"
#include "toolbar.h"
#include "settings.h"
#include "util.h"
#include "pdfutil.h"
#include "version.h"
#include "dialog.h"
#include "propertyeditor.h"
#include "treewindow.h"
#include "menu.h"
#include "commandwindow.h"
#include "additemdialog.h"
#include "treeitemabstract.h"
#include "GlobalParams.h"
#include "base.h"

namespace gui {

using namespace std;
using namespace util;

/** number of active editor windows -> when last is closed, application terminates */
int windowCount;

/** application exit handler invoked when "Quit" is selected in menu/toolbar/etc ... */
void PdfEditWindow::exitApp() {
 guiPrintDbg(debug::DBG_INFO,"Exiting program");
 qApp->closeAllWindows();
 //Application will exit after last window is closed
 //todo: if invoked from qscript then handle specially
}

/** This is called on attempt to close window. If there is unsaved work,
 dialog asking to save it would appear, otherwise the windows is closed. */
void PdfEditWindow::closeEvent(QCloseEvent *e) {
 if (!closeFile(true)) { //user does not want to close and lose unsaved work
  e->ignore();
  return;
 }
 e->accept();
 saveWindowState();
 windowCount--;
 delete this;
 //The PdfEditWindow itself will be deleted on close();
}

/**
 Creates new windows and displays it.
 @param fName Name of file to open in new window. If no file specified, new window is initially empty
 */
void PdfEditWindow::create(const QString &fName/*=QString::null*/) {
 PdfEditWindow *main=new PdfEditWindow(fName);
 main->restoreWindowState();
 main->show();
 windowCount++;
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

/** Saves window state to application settings*/
void PdfEditWindow::saveWindowState() {
 globalSettings->saveWindow(this,"main"); 
 globalSettings->saveSplitter(spl,"spl_main"); 
 globalSettings->saveSplitter(splProp,"spl_right"); 
 globalSettings->saveSplitter(splCmd,"spl_left"); 
 menuSystem->saveToolbars();
}


/** Restores window state from application settings */
void PdfEditWindow::restoreWindowState() {
 globalSettings->restoreWindow(this,"main"); 
 globalSettings->restoreSplitter(spl,"spl_main"); 
 globalSettings->restoreSplitter(splProp,"spl_right"); 
 globalSettings->restoreSplitter(splCmd,"spl_left"); 
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
 Signal handler invoked on menu activation
 @param id Menu ID of clicked item
 */
void PdfEditWindow::menuActivated(int id) {
 QString action=menuSystem->getAction(id);
 guiPrintDbg(debug::DBG_INFO,"Performing menu action: " << action);
 /* quit and closewindow are special - these can't be easily called from script
    as regular function, because invoking them calls window destructor, removing
    script interpreter instance in the process -> after returning from script
    control is moved to already freed script interpreter
  */
 if (action=="quit") exitApp();
 else if (action=="closewindow") closeWindow();
 else base->runScript(action);
}

/**
 constructor of PdfEditWindow, creates window and fills it with elements
 @param parent parent widget containing this control
 @param name name of widget (currently unused)
 @param fName Name of file to open in this window. If empty or null, no file will be opened 
 */
PdfEditWindow::PdfEditWindow(const QString &fName/*=QString::null*/,QWidget *parent/*=0*/,const char *name/*=0*/):QMainWindow(parent,name,WDestructiveClose || WType_TopLevel) {
 setFileName(QString::null);
 document=NULL;
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
 base=new Base(this);

 //Init command window (interpreter, context)
 cmdLine->setInterpreter(base->interpreter(),base);

 //object treeview
 tree=new TreeWindow(base,splProp);

 //Property editor
 prop=new PropertyEditor(splProp);

 //Connections
 QObject::connect(cmdLine, SIGNAL(commandExecuted(QString)), this, SLOT(runScript(QString)));
 QObject::connect(tree, SIGNAL(itemSelected()), this, SLOT(setObject()));
 QObject::connect(tree, SIGNAL(objectSelected(const QString&,boost::shared_ptr<IProperty>)), prop, SLOT(setObject(const QString&,boost::shared_ptr<IProperty>)));
 QObject::connect(tree, SIGNAL(objectSelected(const QString&,boost::shared_ptr<IProperty>)), this, SLOT(setObject(const QString&,boost::shared_ptr<IProperty>)));
 QObject::connect(tree, SIGNAL(treeClicked(int,QListViewItem*)), this, SLOT(treeClicked(int,QListViewItem*)));
 QObject::connect(globalSettings, SIGNAL(settingChanged(QString)), tree, SLOT(settingUpdate(QString)));
 QObject::connect(globalSettings, SIGNAL(settingChanged(QString)), this, SLOT(settingUpdate(QString)));
 QObject::connect(pagespc,SIGNAL(changedPageTo(const QSPage&,int)),this,SLOT(pageChange(const QSPage&,int)));
 QObject::connect(pagespc,SIGNAL(popupMenu(const QPoint&)),this,SLOT(pagePopup(const QPoint&)));
 this->setCentralWidget(spl);

 //Menu
 try {
  QMenuBar *qb=menuSystem->loadMenu(this);
  QObject::connect(qb, SIGNAL(activated(int)), this, SLOT(menuActivated(int))); 

  //ToolBars
  ToolBarList tblist=menuSystem->loadToolBars();
  for (ToolBarList::Iterator toolbar=tblist.begin();toolbar!=tblist.end();++toolbar) {
   QObject::connect(*toolbar, SIGNAL(itemClicked(int)), this, SLOT(menuActivated(int))); 
  }
 } catch (InvalidMenuException &e) {
  guiPrintDbg(debug::DBG_WARN,"Exception in menu loading raised");
  fatalError(e.message());
 }
 //Script must be run AFTER creating all widgets
 //Script may need them, especially the command window
 //Run initscript
 //Any document-related classes are NOT available to the initscript, as no document is currently loaded
 //initscript may use onLoad() to perform some code after document is loaded
 base->runInitScript();

 if (fName.isNull()) { //start with empty editor
  emptyFile();
 //TODO: onEmptyFile() callbask
 } else { //open file
  openFile(fName);
 }
}

/** Called upon selecting object in treeview (if it is IProperty)
 @param name Name of the object that was selected
 @param obj Object that was selected
*/
void PdfEditWindow::setObject(__attribute__((unused)) const QString &name,boost::shared_ptr<IProperty> obj) {
 //Name is not important here, name is used only in property editor
 selectedProperty=obj;
}

/** Called upon selecting item in treeview */
void PdfEditWindow::setObject() {
 selectedTreeItem=tree->getSelectedItem();
}

/**
 Slot called on changing the page in preview window
 @param pg New page that was just changed to
 @param numberOfPage Number of page
*/
void PdfEditWindow::pageChange(const QSPage &pg, int numberOfPage) {
 selectedPage=pg.get();
 selectedPageNumber=numberOfPage;
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

/* \copydoc Base::runScript */
void PdfEditWindow::runScript(QString script) {
 base->runScript(script);
}

/**
 Called when any settings are updated (in script, option editor, etc ...)
 @param key Key of setting that was updated
 */
void PdfEditWindow::settingUpdate(QString key) {
 guiPrintDbg(debug::DBG_DBG,"Settings observer: " << key);
 if (key.startsWith("toolbar/")) { //Show/hide toolbar
  ToolBar *tb=menuSystem->getToolbar(key.mid(8));	// 8=strlen("toolbar/")
  if (!tb) return; //Someone put invalid toolbar in settings. Just ignore it
  bool vis=globalSettings->readBool(key,true);
  if (vis) tb->show();
   else    tb->hide();
  return;
 }
 if (key=="history/save_filePath") { //Do not remember path -> remove path
  if (!globalSettings->readBool("history/save_filePath")) globalSettings->remove("history/filePath");
  return;
 }
 if (key.startsWith("gui/CommandLine/")) {
  cmdLine->reloadSettings();
  return;
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
   if (!save()) return false; //Unable to save document -> not closing
  }
  if (answer==-1) return false; //Cancel: abort closing
  // No -> continue happily, no saving wanted
 }
 if (!onlyAsk) {
  destroyFile();
  emptyFile();
 }
 return true;
}

/**
 Save currently edited document to disk
 If the document have no name (newly opened/generated document), it is solicited from used via dialog.
 @return true if saved succesfully, false if failed to save because of any reason
*/
bool PdfEditWindow::save() {
 if (!document) {
  base->print(tr("No document to save"));
  return false;
 }
 if (fileName.isNull()) { //We need a name
  QString name=base->fileSaveDialog(filename());
  if (name.isNull()) return false; //Still no name? Not saving ...
  document->save(name);
 //TODO: if failure saving return false;
  setFileName(name);
  return true;
 }
 //TODO: handle failure and nummfileName
 document->save();
 //TODO: if failure saving return false;
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
 if (!document) {
  base->print(tr("No document to save"));
  return false;
 }
 return util::saveCopy(document,name);
}

/**
 Assign given filename for document in editor. Also updates window title, etc ...
 @param name New filename for document
*/
void PdfEditWindow::setFileName(const QString &name) {
 fileName=name;
 if (name.isNull()) { //No name
  setCaption(QString(APP_NAME));
  baseName=QString::null;
  return;
 }
 //Add name of file without path to window title
 baseName=name;
 baseName.replace(QRegExp("^.*/"),"");
 setTitle();
}

/** Set window title, according to stored baseName and active revision number
 @param revision Revision number
 */
void PdfEditWindow::setTitle(int revision/*=0*/) {
 QString revisionInfo="";
 if (revision) revisionInfo=QString(" - ")+tr("viewing revision")+" "+QString::number(revision);
 setCaption(QString(APP_NAME)+" - "+baseName+revisionInfo);
}

/** Closes file currently opened in editor, without opening new empty one */
void PdfEditWindow::destroyFile() {
 //Now it is good time to kill all those widgets
 emit selfDestruct();
 if (!document) return;
 tree->uninit();//clear treeview
 prop->clear();//clear property editor
 selectedProperty.reset();//no item selected
 selectedPage.reset();//no page selected
 selectedPageNumber=0;//no page selected
 document->close(false);
 base->destroyDocument();
 base->cleanup();//Garbage collection on scripting objects
 document=NULL;
 setFileName(QString::null);
}

/**
 Open file in editor.
 @param name Name of file to open
*/
void PdfEditWindow::openFile(const QString &name) {
 destroyFile();
 if (name.isNull()) return;
 CPdf::OpenMode mode=globalSettings->readBool("mode/advanced")?(CPdf::Advanced):(CPdf::ReadWrite);
 try {
  document=CPdf::getInstance(name,mode);
 } catch (PdfOpenException &ex) {
  string err;
  ex.getMessage(err);
  base->warn(tr("Error while loading document ")+name+"\n"+err);
  //File failed to open, keep window opened with empty file.
  emptyFile();
  return;
 } catch (...) {
  base->warn(tr("Unknown error while loading document ")+name);
  //File failed to open, keep window opened with empty file.
  emptyFile();
  return;
 }
 base->importDocument();
 setFileName(name);
 tree->init(document,baseName);
 emit documentChanged(document);
 base->print(tr("Loaded file")+" : "+name);
 base->call("onLoad");
 base->call("onLoadUser");
}

/** Opens new empty file in editor. */
void PdfEditWindow::emptyFile() {
 destroyFile();
 document=NULL;
 base->importDocument();
 tree->uninit();
 emit documentChanged(document);
 setFileName(QString::null);
 selectedPageNumber=0;
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
