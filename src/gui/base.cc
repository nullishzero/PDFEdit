/** @file
 Base - class that host scripts and contain static script functions
 This class is also responsible for garbage collection of scripting objects
 @author Martin Petricek
*/

#include "aboutwindow.h"
#include "base.h"
#include "commandwindow.h"
#include "dialog.h"
#include "helpwindow.h"
#include "optionwindow.h"
#include "pdfeditwindow.h"
#include "propertyeditor.h"
#include "qsarray.h"
#include "qsdict.h"
#include "qsimporter.h"
#include "qsiproperty.h"
#include "qsmenu.h"
#include "qspdf.h"
#include "settings.h"
#include "util.h"
#include "version.h"
#include <qfile.h>
#include <qmessagebox.h>
#include <qsinterpreter.h>
#include <qsutilfactory.h> 
#include <utils/debug.h>
#include "pagespace.h"

namespace gui {

using namespace util;

/**
 Create new Base class 
 @param parent Parwent editor window containing this class
*/
Base::Base(PdfEditWindow *parent) {
 w=parent;
 //Create new interpreter and project
 qp=new QSProject(this,"qs_project");
 qs=qp->interpreter();

 //Add ability to open files, directories and run processes
 qs->addObjectFactory(new QSUtilFactory());

 //Add settings object
 assert(globalSettings);
 qp->addObject(globalSettings);
 //Create and add importer to QSProject and related QSInterpreter
 import=new QSImporter(qp,this,this);
 import->addQSObj(w->pagespc,"PageSpace");
 import->addQSObj(w->cmdLine,"CommandWindow");
 qpdf=NULL;
}

/** Get interpreter */
QSInterpreter* Base::interpreter() {
//TODO: doc
 return qs;
}

/** Import currently edited document (QSPDF wrapper) into scripting */
void Base::importDocument() {
 qpdf=import->createQSObject(w->document);
 import->addQSObj(qpdf,"document");
}

/** destroy document - destrouy it also in scripting */
void Base::destroyDocument() {
 if (qpdf) delete qpdf;
 qpdf=NULL;
}

/** if any script is running, stop it */
void Base::stopScript() {
 if (qs->isRunning()) {
  qs->stopExecution();
 }
}

/** 
 Call a callback function (no arguments, no return value) in a script
 @param name Function name
*/
void Base::call(const QString &name) {
 guiPrintDbg(debug::DBG_INFO,"Performing callback: " << name);
 addDocumentObjects();
 try {
  //Call the function. Do not care about result
  qs->evaluate(name+"();",this,"<GUI>");
  if (globalSettings->readBool("console/show_handler_errors")) { //Show return value on console;
   QString error=qs->errorMessage();
   if (error!=QString::null) { /// some error occured
    w->cmdLine->addError(tr("Error in callback handler: ")+name);
    w->cmdLine->addError(error);
   }
  }
 } catch (...) {
  //Do not care about exception in callbacks either ... 
  if (globalSettings->readBool("console/show_handler_errors")) { //Show return value on console;
   w->cmdLine->addError(tr("Exception in callback handler: ")+name);
  }
 }
 removeDocumentObjects();
}

/** Run initscript. Gets name of initscript from settings */
void Base::runInitScript() {
 QString initScriptFilename=globalSettings->read("script/init");
 //Any document-related classes are NOT available to the initscript, as no document is currently loaded
 run(initScriptFilename);
 guiPrintDbg(debug::DBG_DBG,"Initscript executed");
}

/** Create objects that should be available to scripting from current CPdf and related objects*/
void Base::addDocumentObjects() {
 qs->setErrorMode(QSInterpreter::Nothing);
 //Import page and item (Currently selected page and currently selected object)
 QSCObject *page=import->createQSObject(w->selectedPage);
 QSCObject *trit=import->createQSObject(w->selectedTreeItem);
 QSCObject *item=import->createQSObject(w->selectedProperty);
 import->addQSObj(page,"page");
 import->addQSObj(trit,"treeitem");
 import->addQSObj(item,"item");
}

/**
 Add QSCObject to list of object to delete when file in editor window is closed
 @param o Object to add to cleanup-list
*/
void Base::addGC(QSCObject *o) {
 baseObjects.replace(o,0);
}

/**
 Remove QSCObject from list of object to delete when file in editor window is closed
 @param o Object to remove from cleanup-list
*/
void Base::removeGC(QSCObject *o) {
 baseObjects.remove(o);
}

/** Delete all objects in cleanup list */
void Base::cleanup() {
 guiPrintDbg(debug::DBG_INFO,"Garbage collection: " << baseObjects.count() << " objects");
 //Set autodelete and clear the list
 baseObjects.setAutoDelete(true);
 baseObjects.clear();
 baseObjects.setAutoDelete(false);
 guiPrintDbg(debug::DBG_DBG,"Garbage collection done");
}

/** Removes objects added with addDocumentObject */
void Base::removeDocumentObjects() {
 qs->setErrorMode(QSInterpreter::Nothing);
 //delete page and item variables from script -> they may change while script is not executing
 qs->evaluate("item.deleteSelf();",this,"<delete_item>");
 qs->evaluate("treeitem.deleteSelf();",this,"<delete_item>");
 qs->evaluate("page.deleteSelf();",this,"<delete_page>");
 //todo: run garbage collector? Is it needed?
}

/**
 Runs given script code
 @param script QT Script code to run
*/
void Base::runScript(QString script) {
 qs->setErrorMode(QSInterpreter::Nothing);
 w->cmdLine->addCommand(script);
 //Commit currently edited property in property editor
 w->prop->commitProperty();
 //Before running script, add document-related objects to scripting engine and remove tham afterwards
 addDocumentObjects();
 QSArgument ret;
 try {
  ret=qs->evaluate(script,this,"<GUI>");
 } catch (...) {
  print(tr("Unknown exception in script occured"));
 }

 if (globalSettings->readBool("console/showretvalue")) { //Show return value on console;
  switch (ret.type()) {
   case QSArgument::QObjectPtr: { //QObject -> print type
    QObject *ob=ret.qobject();
    print(QString("(Object:")+ob->className()+")");
    break;
   }
   case QSArgument::VoidPointer: { //void * 
    print("(Pointer)");
    break;
   }
   case QSArgument::Variant: { //Variant - simple type.
    QVariant v=ret.variant();
    QString retVar=v.toString();
    if (!retVar.isNull()) print(retVar);
    break;
   }
   default: { 
    //Invalid - print nothing (void, etc ...)
   }
  }
 }
 QString error=qs->errorMessage();
 if (error!=QString::null) { /// some error occured
  w->cmdLine->addError(error);
 }
 removeDocumentObjects();
}

// === Scripting functions ===

/** Shows "About" window */
void Base::about() {
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
void Base::addObjectDialog(QSIProperty *container/*=NULL*/) {
 if (container) {
  w->addObjectDialogI(container->get());
 } else {
  w->addObjectDialogI(w->selectedProperty);
 }
}

/** @copydoc addObjectDialog(QSIProperty *) */
void Base::addObjectDialog(QObject *container) {
 //This method is fix for QSA Bug: QSA sometimes degrade QObject descendants to bare QObjects
 QSIProperty *_container=dynamic_cast<QSIProperty*>(container);
 if (_container) {
  w->addObjectDialogI(_container->get());
 } else {
  guiPrintDbg(debug::DBG_ERR,"type Error: " << container->className());
  w->addObjectDialogI(w->selectedProperty);
 }
}

/** create new empty editor window and display it */
void Base::createNewWindow() {
 PdfEditWindow::create();
}

/** \copydoc PdfEditWindow::closeFile */
bool Base::closeFile(bool askSave,bool onlyAsk/*=false*/) {
 return w->closeFile(askSave,onlyAsk);
}

/** \copydoc PdfEditWindow::closeWindow */
void Base::closeWindow() {
 w->closeWindow();
}

/**
 Check whether given file exists
 @param chkFileName Name of file to check
 @return true if file exists, false otherwise
*/
bool Base::exists(const QString &chkFileName) {
  return QFile::exists(chkFileName);
}

/** \copydoc PdfEditWindow::exitApp */
void Base::exitApp() {
 w->exitApp();
}

/** \copydoc PdfEditWindow::filename() */
QString Base::filename() {
 return w->filename();
}

/**
 Show "open file" dialog and return file selected, or NULL if dialog was cancelled
 @return name of selected file.
 */
QString Base::fileOpenDialog() {
 guiPrintDbg(debug::DBG_DBG,"fileOpenDialog");
 return openFileDialog(w);
}

/**
 Show "save file" dialog and return file selected, or NULL if dialog was cancelled
 @param oldName Old name of the file (if known) - will be preselected
 @return name of selected file.
 */
QString Base::fileSaveDialog(const QString &oldName/*=QString::null*/) {
 guiPrintDbg(debug::DBG_DBG,"fileSaveDialog");
 QString ret=saveFileDialog(w,oldName);
 return ret;
}

/** Print all functions that are in current script interpreter to console window*/
void Base::functions() {
 QStringList objs=qs->functions(this);
 for (QStringList::Iterator it=objs.begin();it!=objs.end();++it) {
  print(*it);
 }
}

/**
 Invokes program help. Optional parameter is topic - if invalid or not defined, help title page will be invoked
 @param topic Starting help topic
*/
void Base::help(const QString &topic/*=QString::null*/) {
 HelpWindow *hb=new HelpWindow(topic);
 hb->show();
}

/**
 Brings up informational messagebox with given message
 @param msg Message to display
 */
void Base::message(const QString &msg) {
 QMessageBox::information(w,APP_NAME,msg,QObject::tr("&Ok"),QString::null,QString::null,QMessageBox::Ok | QMessageBox::Default);
}

/** \copydoc PdfEditWindow::modified */
bool Base::modified() {
 return w->modified();
}

/** Print all objects that are in current script interpreter to console window*/
void Base::objects() {
 QObjectList objs=qs->presentObjects();
 QObjectListIterator it(objs);
 QObject *obj;
 while ((obj=it.current())!=0) {
  ++it;
  print(obj->name());
 }
}

/** \copydoc PdfEditWindow::openFile */
void Base::openFile(const QString &name) {
 w->openFile(name);
}

/**
 Open file in a new editor window.
 @param name Name of file to open
*/
void Base::openFileNew(const QString &name) {
 PdfEditWindow::create(name);
}

/** Show options dialog. Does not wait for dialog to finish. */
void Base::options() {
 OptionWindow::optionsDialog(w->menuSystem);
}

/**
 Create and return new popup menu, build from menu list/item  identified by this name.
 If no item specified, menu is initially empty
 @param menuName Name of menu inconfiguration to use as template
 @return initialized QSMenu Object
*/
QSMenu* Base::popupMenu(const QString &menuName/*=QString::null*/) {
 return new QSMenu(w->menuSystem,this,menuName);
}

/**
 Print given string to console, followed by newline
 @param str String to add
 */
void Base::print(const QString &str) {
 consoleLog(str,globalSettings->readExpand("path/console_log"));
 w->cmdLine->addString(str);
}

/**
 Asks question with Yes/No answer. "Yes" is default.
 Return true if user selected "yes", false if user selected "no"
 @param msg Question to display
 @return True if yes, false if no
 */
bool Base::question(const QString &msg) {
 int answer=QMessageBox::question(w,APP_NAME,msg,QObject::tr("&Yes"),QObject::tr("&No"),QString::null,0,1);
//                                  QMessageBox::Yes | QMessageBox::Default,QMessageBox::No | QMessageBox::Escape);
 return (answer==0);//QMessageBox::Yes);
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
int Base::question_ync(const QString &msg) {
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
void Base::restoreWindowState() {
 w->restoreWindowState();
}

/**
 Runs script from given file
 @param scriptName name of file with QT Script to run
 */
void Base::run(QString scriptName) {
 QString code=loadFromFile(scriptName);
 qs->evaluate(code,this,scriptName);
}

/** \copydoc PdfEditWindow::save */
bool Base::save() {
 return w->save();
}

/** \copydoc PdfEditWindow::saveAs */
bool Base::saveAs(const QString &name) {
 return w->saveAs(name);
}

/** \copydoc PdfEditWindow::saveWindowState */
void Base::saveWindowState() {
 w->saveWindowState();
}

/**
 Set new debug verbosity level
 @param param New debug verbosity level
 \see util::setDebugLevel
*/
void Base::setDebugLevel(const QString &param) {
 util::setDebugLevel(param);
}

/** call QObject::tr to translate specific string
 @param text text to translate to current locale
 @param context Optional context identifier for localized text
 @return translated text
*/
QString Base::tr(const QString &text,const QString &context/*=QString::null*/) {
 if (context.isNull()) return QObject::tr(text);
 return QObject::tr(text,context);
}


/** Print all variables that are in current script interpreter to console window*/
void Base::variables() {
 QStringList objs=qs->variables(this);
 for (QStringList::Iterator it=objs.begin();it!=objs.end();++it) {
  print(*it);
 }
}

/** Return version of editor
 @return Version of editor (major.minor.release) */
QString Base::version() {
 return VERSION;
}

/**
 Show this string as a warning in a messagebox and also print it to console, followed by newline
 @param str String to use as warning
 */
void Base::warn(const QString &str) {
 print(str);
 QMessageBox::warning(w,tr("Warning"),str);
}

/** destructor */
Base::~Base() {
 delete import;
 delete qp;
}


} // namespace gui
