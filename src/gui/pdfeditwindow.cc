/** @file
 PdfEditWindow - class representing main application window
*/
#include "pdfeditwindow.h"
#include "pagespace.h"
#include <utils/debug.h>
#include "settings.h"
#include "util.h"
#include "test.h"
#include <iostream>
#include <qfile.h>
#include <qregexp.h>
#include "aboutwindow.h"
#include "version.h"
#include <qmessagebox.h> 
#include "optionwindow.h"
#include "helpwindow.h"
#include "dialog.h"
//#include <qtextstream.h> 

namespace gui {

using namespace std;

/** number of active editor windows -> when last is closed, application terminates */
int windowCount;

//TODO: system konfigurovatelnych toolbaru (ne zcela hotov). a klvesovych zkratek (neco uz je v systemu menu)

/** application exit handler invoked when "Quit" is selected in menu/toolbar/etc ... */
void PdfEditWindow::exitApp() {
 printDbg(debug::DBG_INFO,"Exiting program");
 qApp->closeAllWindows();
 //Application will exit after last window is closed
 //todo: if invoked from qscript then handle specially
}

/** Shows "About" window */
void PdfEditWindow::about() {
 AboutWindow *aboutWin= new AboutWindow(this);
 aboutWin->show();
}

/** create new empty editor window and display it */
void PdfEditWindow::createNewWindow() {
 createNewEditorWindow();
}

/** Open file in a new editor window.
 @param name Name of file to open
*/
void PdfEditWindow::openFileNew(const QString &name) {
 createNewEditorWindow(name);
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

/** Check whether given file exists
 @param chkfileName Name of file to check
 @return true if file exists, false otherwise
*/
bool PdfEditWindow::exists(const QString &chkFileName) {
  return QFile::exists(chkFileName);
}

/** Check whether curretntly opened document was modified
    since it was opened or last saved
 @return true if document was modified, false if not
*/
bool PdfEditWindow::modified() {
 if (!document) return false;//no document loaded at all

 //TODO: implement. Now returns true for safety

 return true;
}

/** Show options dialog. Does not wait for dialog to finish. */
void PdfEditWindow::options() {
 OptionWindow::optionsDialog(menuSystem);
}

/** Show "open file" dialog and return file selected, or NULL if dialog was cancelled
 @return name of selected file.
 */
QString PdfEditWindow::fileOpenDialog() {
 printDbg(debug::DBG_DBG,"fileOpenDialog");
 return openFileDialog(this);
}

/** Show "save file" dialog and return file selected, or NULL if dialog was cancelled
 @param oldName Old name of the file (if known) - will be preselected
 @return name of selected file.
 */
QString PdfEditWindow::fileSaveDialog(const QString &oldName/*=QString::null*/) {
 printDbg(debug::DBG_DBG,"fileSaveDialog");
 QString ret=saveFileDialog(this,oldName);
 return ret;
}

/** Creates new windows and displays it.
 @param fName Name of file to open in new window. If no file specified, new window is initially empty
 */
void createNewEditorWindow(const QString &fName) {
 PdfEditWindow *main=new PdfEditWindow(fName);
 main->restoreWindowState();
 main->show();
 windowCount++;
}

/** Close the window itself */
void PdfEditWindow::closeWindow() {
/* TODO: Need QSA 1.1.4 for this
 if (qs->isRunning()) {
  qs->stopExecution();
  deleteLater();  //Delete window when returning back to main loop
  return;
 }*/
 close();
}


/** Saves window state to application settings*/
void PdfEditWindow::saveWindowState() {
 globalSettings->saveWindow(this,"main"); 
 globalSettings->saveSplitter(spl,"spl_main"); 
 globalSettings->saveSplitter(splProp,"spl_right"); 
 globalSettings->saveSplitter(splCmd,"spl_left"); 
 menuSystem->saveToolbars(this);
 //TODO: bug, toolbars change order
/* QString out;
 QTextStream qs(out,IO_WriteOnly);
 qs << this;
 printDbg(debug::DBG_DBG,"SAVETOOL: " << out);
 //Save toolbar states
 QStringList tbs=menuSystem->getToolbarList();
 for (unsigned int i=0;i<tbs.count();i++) {
  ToolBar* tb=menuSystem->getToolbar(tbs[i]);
  if (!tb) continue; //Someone put invalid toolbar in settings. Just ignore it
  menuSystem->saveToolbar(tb,tbs[i],this);
 }*/
}

/** Restores window state from application settings */
void PdfEditWindow::restoreWindowState() {
 globalSettings->restoreWindow(this,"main"); 
 globalSettings->restoreSplitter(spl,"spl_main"); 
 globalSettings->restoreSplitter(splProp,"spl_right"); 
 globalSettings->restoreSplitter(splCmd,"spl_left"); 
 menuSystem->restoreToolbars(this);
 //Restore toolbar states
/* QStringList tbs=menuSystem->getToolbarList();
 for (unsigned int i=0;i<tbs.count();i++) {
  ToolBar* tb=menuSystem->getToolbar(tbs[i]);
  if (!tb) continue; //Someone put invalid toolbar in settings. Just ignore it
  menuSystem->restoreToolbar(tb,tbs[i],this);
 }*/
}

/** Create objects that should be available to scripting from current CPdf and related objects*/
void PdfEditWindow::addDocumentObjects() {
 //Import page and item (Currently selected page and currently selected object)
 QSCObject *pg=import->createQSObject(page);
 QSCObject *it=import->createQSObject(item);
 import->addQSObj(pg,"page");
 import->addQSObj(it,"item");
}

/** Removes objects added with addDocumentObject */
void PdfEditWindow::removeDocumentObjects() {
 //delete page and item variables from script -> they may change while script is not executing
 qs->evaluate("item.deleteSelf();",this,"<delete_item>");
 qs->evaluate("page.deleteSelf();",this,"<delete_page>");
 //todo: run garbage collector? Is it needed?
}

/** Runs given script code
 @param script QT Script code to run
 */
void PdfEditWindow::runScript(QString script) {
 qs->setErrorMode(QSInterpreter::Nothing);
 cmdLine->addCommand(script);

 //Before running script, add document-related objects to scripting engine and remove tham afterwards
 addDocumentObjects();
 QSArgument ret=qs->evaluate(script,this,"<GUI>");
 if (globalSettings->readBool("console/showretvalue")) {
  switch (ret.type()) {
   case QSArgument::QObjectPtr: {
    QObject *ob=ret.qobject();
    print(QString("(Object:")+ob->className()+")");
    break;
   }
   case QSArgument::VoidPointer: {
    print("(Pointer)");
    break;
   }
   case QSArgument::Variant: {
    QVariant v=ret.variant();
    print(v.toString());
    break;
   }
   default: { 
    //Invalid - print nothing
    //print("(?)");
   }
  }
 }
 QString error=qs->errorMessage();
 if (error!=QString::null) { /// some error occured
  cmdLine->addError(error);
 }

 removeDocumentObjects();
}


/** Print given string to console, followed by newline
 @param str String to add
 */
void PdfEditWindow::print(const QString &str) {
 cmdLine->addString(str);
}

/** Signal handler invoked on menu activation
 @param id Menu ID of clicked item
 */
void PdfEditWindow::menuActivated(int id) {
 QString action=menuSystem->getAction(id);
 printDbg(debug::DBG_INFO,"Performing menu action: " << action);
 /* quit and closewindow are special - these can't be easily called from script
    as regular function, because invoking them calls window destructor, removing
    script interpreter instance in the process -> after returning from script
    control is moved to already freed script interpreter
  */
 if (action=="quit") exitApp();
 else if (action=="closewindow") closeWindow();
 else runScript(action);
}

/** constructor of PdfEditWindow, creates window and fills it with elements
 @param parent parent widget containing this control
 @param name name of widget (currently unused)
 @param fName Name of file to open in this window. If empty or null, no file will be opened 
 */
PdfEditWindow::PdfEditWindow(const QString &fName/*=QString::null*/,QWidget *parent/*=0*/,const char *name/*=0*/):QMainWindow(parent,name,WDestructiveClose || WType_TopLevel) {
 setFileName(QString::null);
 document=NULL; 
 menuSystem=new Menu();
 //Horizontal splitter Preview + Commandline | Treeview + Property editor
 spl=new QSplitter(this,"horizontal_splitter");


 //Splitter between command line and preview window
 splCmd=new QSplitter(spl);
 splCmd->setOrientation(Vertical);

 //PageSpace
 PageSpace *pagespc = new PageSpace(splCmd,"PageSpace");

 //Commandline
 cmdLine=new CommandWindow(splCmd);

 //Splitter between treeview and property editor
 splProp=new QSplitter(spl);
 splProp->setOrientation(Vertical);

 //object treeview
 tree=new TreeWindow(splProp);

 //Property editor
 prop=new PropertyEditor(splProp);

 QObject::connect(cmdLine, SIGNAL(commandExecuted(QString)), this, SLOT(runScript(QString)));
 QObject::connect(tree, SIGNAL(objectSelected(IProperty*)), prop, SLOT(setObject(IProperty*)));
 QObject::connect(globalSettings, SIGNAL(settingChanged(QString)), tree, SLOT(settingUpdate(QString)));
 QObject::connect(globalSettings, SIGNAL(settingChanged(QString)), this, SLOT(settingUpdate(QString)));

 this->setCentralWidget(spl);

 //Menu
 QMenuBar *qb=menuSystem->loadMenu(this);
 QObject::connect(qb, SIGNAL(activated(int)), this, SLOT(menuActivated(int))); 

 //ToolBars
 ToolBarList tblist=menuSystem->loadToolBars(this);
 for (ToolBarList::Iterator toolbar=tblist.begin();toolbar!=tblist.end();++toolbar) {
  QObject::connect(*toolbar, SIGNAL(itemClicked(int)), this, SLOT(menuActivated(int))); 
 }

 //Need to name objects, so they will become invisible to scripting
/* QLayout *w_l=this->layout();
 if (w_l) {
  w_l->setName("pdfedit_layout");
 }
 QMenuBar *w_m=this->menuBar();
 if (w_m) {
  w_m->setName("pdfedit_menu");
 }
 //TODO: can't do that for all objects anyway
*/
 //Script must be run AFTER creating all widgets
 // -> script may need them, especially the command window

 //Create new interpreter and project
 qp=new QSProject();
 qs=qp->interpreter();
 assert(globalSettings);
 qp->addObject(globalSettings);
 //Create and add importer to QSProject and related QSInterpreter
 import=new QSImporter(qp,this);
 //Run initscript
 QString initScriptFilename="init.qs";
 QString code=loadFromFile(initScriptFilename);
 //Any document-related classes are NOT available to the initscript, as no document is currently loaded
 qs->evaluate(code,this,initScriptFilename);
 printDbg(debug::DBG_DBG,"after initscript");

 if (fName.isNull()) { //start with empty editor
  emptyFile();
  print(tr("New document created"));
 } else { //open file
  openFile(fName);
 }
 prop->setObject(0);//fill with demonstration properties
}

/** Called when any settings are updated (in script, option editor, etc ...) */
void PdfEditWindow::settingUpdate(QString key) {
 printDbg(debug::DBG_DBG,"Settings observer: " << key);
 if (key.startsWith("toolbar/")) { //Show/hide toolbar
  ToolBar *tb=menuSystem->getToolbar(key.mid(8));	// 8=strlen("toolbar/")
  if (!tb) return; //Someone put invalid toolbar in settings. Just ignore it
  bool vis=globalSettings->readBool(key,true);
  if (vis) tb->show();
   else    tb->hide();
 }
}

/** Closes file currently opened in editor.
@param askSave Ask about saving changes?
@return true if user accepted the close or saved the document before,
 false if user refuses the close */
bool PdfEditWindow::closeFile(bool askSave) {
 if (modified() && askSave) {
  int answer=question_ync(fileName+"\n"+tr("Current file is not saved. Do you want to save it?"));
  if (answer==QMessageBox::Yes) { //save it
   if (!save()) return false; //Unable to save document -> not closing
  }
  if (answer==QMessageBox::Cancel) return false; //abort closing
  // QMessageBox::No -> continue happily, no saving wanted
 }
 destroyFile();
 emptyFile();
 return true;
}

/**
Save currently edited document to disk
If the document have no name (newly opened/generated document), it is solicited from used via dialog.
@return true if saved succesfully, false if failed to save because of any reason
 */
bool PdfEditWindow::save() {
 assert(document);
 if (fileName.isNull()) { //We need a name
  QString name=fileSaveDialog(filename());
  if (name.isNull()) return false; //Still no name? Not saving ...
  document->save(name);
 //TODO: if failure saving return false;
  setFileName(name);
  return true;
 }
 //TODO: handle failure and nummfileName
 document->save(fileName);
 //TODO: if failure saving return false;
 return true;
}


/**
Save currently edited document to disk, using provided filename
@param name New filename to save document under
@return true if saved succesfully, false if failed to save because of any reason
 */
bool PdfEditWindow::saveAs(const QString &name) {
 assert(document);
 //TODO: what is returned from CPDF::save()?
 document->save(name);
 //TODO: if failure saving return false;
 setFileName(name);
 return true;
}

/**
Assign given filename for document in editor. Also updates window title, etc ...
@param name New filename for document
 */
void PdfEditWindow::setFileName(const QString &name) {
 fileName=name;
 if (name.isNull()) { //No name
  setCaption(QString(APP_NAME));
  return;
 }
 //Add name of file without path to window title
 QString baseName=name;
 baseName.replace(QRegExp("^.*/"),"");
 setCaption(QString(APP_NAME)+" - "+baseName);
}

/** Closes file currently opened in editor, without opening new empty one */
void PdfEditWindow::destroyFile() {
 if (!document) return;
 item=NULL;//no item selected
 tree->clear();//clear treeview
 prop->clear();//clear property editor
 page.reset();
 document->close(false);
 delete qpdf;
 document=0;
 setFileName(QString::null);
}

/** Open file in editor.
 @param name Name opf file to open
*/
void PdfEditWindow::openFile(const QString &name) {
 destroyFile();
 if (name.isNull()) return;
 CPdf::OpenMode mode=globalSettings->readBool("mode/advanced")?(CPdf::Advanced):(CPdf::ReadWrite);
 document=CPdf::getInstance(name,mode);
 qpdf=import->createQSObject(document);
 import->addQSObj(qpdf,"document");
 tree->init(document);
 print(tr("Loaded file")+" : "+name);
 setFileName(name);
}

/** Opens new empty file in editor. */
void PdfEditWindow::emptyFile() {
 destroyFile();
 document=NULL;
 qpdf=import->createQSObject(document);
 import->addQSObj(qpdf,"document");
 tree->init(test::testDict()); //todo: testing, should be NULL
 setFileName(QString::null);
}

/** Print all objects that are in current script interpreter to console window*/
void PdfEditWindow::objects() {
 QObjectList objs=qs->presentObjects();
 QObjectListIterator it(objs);
 QObject *obj;
 while ((obj=it.current())!=0) {
  ++it;
  print(obj->name());
 }
}

/** Print all functions that are in current script interpreter to console window*/
void PdfEditWindow::functions() {
 QStringList objs=qs->functions(this);
 for (QStringList::Iterator it=objs.begin();it!=objs.end();++it) {
  print(*it);
 }
}

/** Return version of editor
 @return Version of editor (major.minor.release) */
QString PdfEditWindow::version() {
 return VERSION;
}

/** Brings up informational messagebox with given message
 @param msg Message to display
 */
void PdfEditWindow::message(const QString &msg) {
 QMessageBox::information(this,APP_NAME,msg,QObject::tr("&Ok"),QString::null,QString::null,QMessageBox::Ok | QMessageBox::Default);
}

/** Asks question with Yes/No answer. "Yes" is default. Return true if user selected "yes", false if user selected "no"
 @param msg Question to display
 @return True if yes, false if no
 */
bool PdfEditWindow::question(const QString &msg) {
 int answer=QMessageBox::question(this,APP_NAME,msg,QObject::tr("&Yes"),QObject::tr("&No"),QString::null,0,1);
//                                  QMessageBox::Yes | QMessageBox::Default,QMessageBox::No | QMessageBox::Escape);
 return (answer==0);//QMessageBox::Yes);
}

/** Asks question with Yes/No/Cancel answer. "Yes" is default. <br />
 Return on of these three according to user selection: <br />
  QMessageBox::Yes     <br />
  QMessageBox::No      <br />
  QMessageBox::Cancel
 @param msg Question to display
 @return Selection of user.
 */
int PdfEditWindow::question_ync(const QString &msg) {
 int answer=QMessageBox::question(this,APP_NAME,msg,QObject::tr("&Yes"),QObject::tr("&No"),QObject::tr("&Cancel"),0,2);
 // QMessageBox::Yes | QMessageBox::Default,QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape
 switch (answer) {
  case 0: return QMessageBox::Yes;
  case 1: return QMessageBox::No;
  case 2: return QMessageBox::Cancel;
  default: assert(0);
 }
}

/** Return name of file loaded in editor. Return empty string if the document have no name
 (if it is not loaded from disk and have not yet been saved)
 @return Filename */
QString PdfEditWindow::filename() {
 if (fileName.isNull()) return "";
 return fileName;
}

/** Print all variables that are in current script interpreter to console window*/
void PdfEditWindow::variables() {
 QStringList objs=qs->variables(this);
 for (QStringList::Iterator it=objs.begin();it!=objs.end();++it) {
  print(*it);
 }
}

/** Invokes program help. Optional parameter is topic - if invalid or not defined, help title page will be invoked
@param topic Starting help topic
 */
void PdfEditWindow::help(const QString &topic/*=QString::null*/) {
 HelpWindow *hb=new HelpWindow("TODO: topic");
 hb->show();
}

/** default destructor */
PdfEditWindow::~PdfEditWindow() {
 destroyFile();
 /* stopExecution is not in QSA 1.0.1, need 1.1.4
 if (qs->isRunning()) {
  qs->stopExecution();
  deleteLater();  //Delete object when returning back to main loop
 }*/
 delete menuSystem;
 delete qp;
}

} // namespace gui
