#include "pdfedit.h"
#include "settings.h"
#include <iostream>

using namespace std;

extern Settings *global;//global settings

/** number of active editor windows -> when last is closed, application terminates */
int windowCount;

//TODO: system konfigurovatelnch menu (castecne hotov), toolbar (nehotov). a klvesovch zkratek (neco uz je v shystemu menu)

/** application exit handler invoked when "Quit" is selected in menu/toolbar/etc ... */
void pdfEditWidget::exitApp() {
 printf("exiting...\n");
 qApp->closeAllWindows();
 //Application will exit after last window is closed
}

/** creates new windows and displays it */
void pdfEditWidget::createNewWindow() {
 createNewEditorWindow();
}

/** This is called on attempt to close window. If there is unsaved work,
 dialog asking to save it would appear (TODO), otherwise the windows is closed. */
void pdfEditWidget::closeEvent(QCloseEvent *e) {
 //TODO: ask for save/discard/cancel if unsaved work and refuse close if necessary
 e->accept();
 saveWindowState();
 windowCount--;
 delete this;
 //The pdfEditWidget itself will be deleted on close();
}

/** Creates new windows and displays it */
void createNewEditorWindow() {
 pdfEditWidget *main=new pdfEditWidget();
 main->restoreWindowState();
 main->show();
 windowCount++;
}

/** Close the window itself */
void pdfEditWidget::closeWindow() {
 close();
}

/** Saves window state to application settings*/
void pdfEditWidget::saveWindowState() {
 global->saveWindow(this,"main"); 
}

/** Restores window state from application settings */
void pdfEditWidget::restoreWindowState() {
 global->restoreWindow(this,"main"); 
}



/** Signal handler invoked on menu activation
 @param id Menu ID of clicked item
 */
void pdfEditWidget::menuActivated(int id) {
 QString action=global->getAction(id);
 cout << "Performing action: " << action << endl;
 //TODO: call scripting here instead... 
 if (action=="quit") exitApp();
 else if (action=="closewindow") closeWindow();
 else if (action=="newwindow") createNewWindow();
}

/** constructor of pdfEditWidget, creates window and fills it with elements, parameters are ignored */
pdfEditWidget::pdfEditWidget(QWidget *parent,const char *name):QMainWindow(parent,name,WDestructiveClose || WType_TopLevel) {
 //TODO: tohle je pokusny kod, dodelat

 //SPLITTER
 QSplitter *spl=new QSplitter(this);

 //Button QUIT
 QPushButton *quit=new QPushButton(tr("Quit"), spl);
 quit->resize(75, 30);
 quit->setFont(QFont("Times", 18, QFont::Bold));
 QObject::connect(quit, SIGNAL(clicked()), this, SLOT(exitApp()));
 quit->show();

 //Button NEW WINDOW
 QPushButton *neww=new QPushButton(tr("Nothing"), spl);
// QObject::connect(neww, SIGNAL(clicked()), this, SLOT(createNewWindow()));
 this->setCentralWidget(spl);
 neww->show();

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
