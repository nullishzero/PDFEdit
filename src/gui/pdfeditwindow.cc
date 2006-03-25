/** @file
 PdfEditWindow - class representing main application window
*/
#include <utils/debug.h>
#include "pdfeditwindow.h"
#include "settings.h"
#include "util.h"
#include "test.h"
#include <iostream>
#include <qfile.h>
#include "aboutwindow.h"
#include "version.h"

using namespace std;

/** One object for application, holding all global settings. */
extern Settings *globalSettings;

/** number of active editor windows -> when last is closed, application terminates */
int windowCount;

//TODO: system konfigurovatelnch menu (castecne hotov), toolbar (nehotov). a klvesovch zkratek (neco uz je v shystemu menu)

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

/** creates new windows and displays it */
void PdfEditWindow::createNewWindow() {
 createNewEditorWindow();
}

/** This is called on attempt to close window. If there is unsaved work,
 dialog asking to save it would appear (TODO), otherwise the windows is closed. */
void PdfEditWindow::closeEvent(QCloseEvent *e) {
 //TODO: ask for save/discard/cancel if unsaved work and refuse close if necessary
 e->accept();
 saveWindowState();
 windowCount--;
 delete this;
 //The PdfEditWindow itself will be deleted on close();
}

/** Creates new windows and displays it */
void createNewEditorWindow() {
 PdfEditWindow *main=new PdfEditWindow();
 main->restoreWindowState();
 main->show();
 windowCount++;
}

/** Close the window itself */
void PdfEditWindow::closeWindow() {
 close();
}

/** Saves window state to application settings*/
void PdfEditWindow::saveWindowState() {
 globalSettings->saveWindow(this,"main"); 
}

/** Restores window state from application settings */
void PdfEditWindow::restoreWindowState() {
 globalSettings->restoreWindow(this,"main"); 
}

/** Runs given script code
 @param script QT Script code to run
 */
void PdfEditWindow::runScript(QString script) {
 qs->setErrorMode(QSInterpreter::Nothing);
 cmdLine->addCommand(script);
 qs->evaluate(script,this,"<GUI>");
 QString error=qs->errorMessage();
 if (error!=QString::null) { /// some error occured
  cmdLine->addError(error);
 }
}


/** Print given string to console, followed by newline
 @param str String to add
 */
void PdfEditWindow::print(QString str) {
 cmdLine->addString(str);
}

/** Signal handler invoked on menu activation
 @param id Menu ID of clicked item
 */
void PdfEditWindow::menuActivated(int id) {
 QString action=globalSettings->getAction(id);
 printDbg(debug::DBG_INFO,"Performing menu action: " << action);
 /* quit and closewindow are special - these can't be easily called from script
    as regular function, because invoking them calls window destructor, removing
    script interpreter instance in the process -> after returning from script
    control is moved to already freed script interpreter
  */
 if (action=="quit") exitApp();
 else if (action=="closewindow") closeWindow();
// else if (action=="newwindow") createNewWindow();
 else runScript(action);
}

/** DEBUG: print stringlist to stdout
 @param l String list to print
 */
void printList(QStringList l) {
 QStringList::Iterator it=l.begin();
 for (;it!=l.end();++it) { //load all subitems
  QString x=*it;
  printDbg(debug::DBG_DBG,x);
 }

}

/** constructor of PdfEditWindow, creates window and fills it with elements, parameters are ignored */
PdfEditWindow::PdfEditWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QMainWindow(parent,name,WDestructiveClose || WType_TopLevel) {
 setCaption(APP_NAME);
 
 //Horizontal splitter Preview + Commandline | Treeview + Property editor
 QSplitter *spl=new QSplitter(this);

 //Splitter between command line and preview window
 QSplitter *splCmd=new QSplitter(spl);
 splCmd->setOrientation(Vertical);


 //Button QUIT -> will be replaced with preview window once completed
 QPushButton *quit=new QPushButton(tr("Quit"), splCmd);
 quit->resize(75, 30);
 quit->setFont(QFont("Times", 18, QFont::Bold));
 QObject::connect(quit, SIGNAL(clicked()), this, SLOT(exitApp()));

 //Commandline
 cmdLine=new CommandWindow(splCmd);
 QObject::connect(cmdLine, SIGNAL(commandExecuted(QString)), this, SLOT(runScript(QString)));

// quit->show();
// cmdLine->show();

 //Splitter between treeview and property editor
 QSplitter *splProp=new QSplitter(spl);
 splProp->setOrientation(Vertical);

 //object treeview
 tree=new TreeWindow(splProp);

 //Property editor
 prop=new PropertyEditor(splProp);
// prop->show();


 this->setCentralWidget(spl);

 //Menu
 QMenuBar *qb=globalSettings->loadMenu(this);
 QObject::connect(qb, SIGNAL(activated(int)), this, SLOT(menuActivated(int))); 

 //ToolBars
 ToolBarList tblist=globalSettings->loadToolBars(this);
 for (ToolBarList::Iterator toolbar=tblist.begin();toolbar!=tblist.end();++toolbar) {
  QObject::connect(*toolbar, SIGNAL(itemClicked(int)), this, SLOT(menuActivated(int))); 
 }
   
 //show splitter
 spl->show();

 //Script must be run AFTER creating all widgets
 // -> script may need them, especially the command window

 //Gets new interpreter
 qp=new QSProject();
// qs=new QSInterpreter();
 qs=qp->interpreter();//new QSInterpreter();
 assert(globalSettings);
 qp->addObject(globalSettings);
 //run initscript
 QString initScript="init.qs";
 QString code=loadFromFile(initScript);
 qs->evaluate(code,this,initScript);

 printDbg(debug::DBG_DBG,"after initscript");

 /*
 //DEBUG 
 printDbg(debug::DBG_DBG,"<Functions");
 printList(qs->functions());
 printDbg(debug::DBG_DBG,"<Classes");
 printList(qs->classes(QSInterpreter::AllClasses));
 printDbg(debug::DBG_DBG,"<Var");
 printList(qs->variables());
 printDbg(debug::DBG_DBG,"<OK");*/

 //create testing document
// document=CPdf::getInstance("../../doc/zadani.pdf",CPdf::ReadWrite);
 //document=test::testPDF();
 tree->init((IProperty*)NULL);
// tree->init(document);//not yet implemented in kernel
 prop->setObject(0);//fill with demonstration properties
}

/** default destructor */
PdfEditWindow::~PdfEditWindow() {
 /* stopExecution is not in QSA 1.0.1, need 1.1.4
 if (qs->isRunning()) {
  qs->stopExecution()
 }*/
 delete qp;
}
