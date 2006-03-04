/** @file
 PdfEditWindow - class representing main application window
*/
#include "pdfeditwindow.h"
#include "propertyeditor.h"
#include "settings.h"
#include <iostream>
#include <qfile.h>
#include "aboutwindow.h"
#include "version.h"

using namespace std;

extern Settings *global;//global settings

/** number of active editor windows -> when last is closed, application terminates */
int windowCount;

//TODO: system konfigurovatelnch menu (castecne hotov), toolbar (nehotov). a klvesovch zkratek (neco uz je v shystemu menu)

/** application exit handler invoked when "Quit" is selected in menu/toolbar/etc ... */
void PdfEditWindow::exitApp() {
 printf("exiting...\n");
 qApp->closeAllWindows();
 //Application will exit after last window is closed
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
 global->saveWindow(this,"main"); 
}

/** Restores window state from application settings */
void PdfEditWindow::restoreWindowState() {
 global->restoreWindow(this,"main"); 
}

/** Runs given script code
 @param script QT Script code to run
 */
void PdfEditWindow::runScript(QString script) {
 qs->evaluate(script,this,"<GUI>");
}


/** Signal handler invoked on menu activation
 @param id Menu ID of clicked item
 */
void PdfEditWindow::menuActivated(int id) {
 QString action=global->getAction(id);
 cout << "Performing action: " << action << endl;
 if (action=="quit") exitApp();
 else if (action=="closewindow") closeWindow();
 else if (action=="newwindow") createNewWindow();
 runScript(action);
}

/** Load content of file to string. Empty string is returned if file does not exist or is unreadable.
 @param name Filename of file to load
 @return file contents in string.
 */
QString loadFromFile(QString name) {
 QFile *f=new QFile(name);
 f->open(IO_ReadOnly);
 int size=f->size();
 char* buffer=(char *)malloc(size);
 size=f->readBlock(buffer,size);
 if (size==-1) return "";
 f->close();
 delete f;
 QByteArray qb;
 qb.assign(buffer,size);
 QString res=QString(qb);
 return res;
}

/** DEBUG: print stringlist to stdout
 @param l String list to print
 */
void printList(QStringList l) {
 QStringList::Iterator it=l.begin();
 for (;it!=l.end();++it) { //load all subitems
  QString x=*it;
  cout << x << endl;
  cout.flush();
 }

}

/** constructor of PdfEditWindow, creates window and fills it with elements, parameters are ignored */
PdfEditWindow::PdfEditWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QMainWindow(parent,name,WDestructiveClose || WType_TopLevel) {
 //TODO: tohle je pokusny kod, dodelat
 setCaption(APP_NAME);
 //Gets new interpreter
 qs=new QSInterpreter();
 
 //run initscript
 QString initScript="init.qs";
 QString code=loadFromFile(initScript);
 qs->evaluate(code,this,initScript);

 /*
 //DEBUG 
 printf("<Functions\n");
 printList(qs->functions());
 printf("<Classes\n");
 printList(qs->classes(QSInterpreter::AllClasses));
 printf("<Var\n");
 printList(qs->variables());
 printf("<OK\n");*/
 
 //SPLITTER
 QSplitter *spl=new QSplitter(this);

 //Button QUIT
 QPushButton *quit=new QPushButton(tr("Quit"), spl);
 quit->resize(75, 30);
 quit->setFont(QFont("Times", 18, QFont::Bold));
 QObject::connect(quit, SIGNAL(clicked()), this, SLOT(exitApp()));
 quit->show();

 //Button NEW WINDOW
 PropertyEditor *prop=new PropertyEditor(spl);
 prop->show();
 prop->setObject(0);//fill with demonstration properties

// QPushButton *neww=new QPushButton(tr("Nothing"), spl);
// QObject::connect(neww, SIGNAL(clicked()), this, SLOT(createNewWindow()));
 this->setCentralWidget(spl);
// neww->show();

 //Menu
 QMenuBar *qb=global->loadMenu(this);
 QObject::connect(qb, SIGNAL(activated(int)), this, SLOT(menuActivated(int))); 

 //ToolBars
 ToolBarList tblist=global->loadToolBars(this);
 for (ToolBarList::Iterator toolbar=tblist.begin();toolbar!=tblist.end();++toolbar) {
  QObject::connect(*toolbar, SIGNAL(itemClicked(int)), this, SLOT(menuActivated(int))); 
 }
   
 //show splitter
 spl->show();
}

/** default destructor */
PdfEditWindow::~PdfEditWindow() {
 delete qs;
}
