#include "pdfedit.h"
#include "settings.h"
#include <iostream>

using namespace std;

extern Settings *global;//global settings

//TODO: system konfigurovatelnch menu (castecne hotov), toolbar (nehotov). a klvesovch zkratek (neco uz je v shystemu menu)

/** application exit handler invoked when "Quit" is selected in menu/toolbar/etc ... */
void pdfEditWidget::exiting() {
 printf("exiting...\n");
 qApp->quit();
}

/** creates new windows and displays it */
void pdfEditWidget::createNewWindow() {
 pdfEditWidget *main=new pdfEditWidget();
 main->show();
}

/** Signal handler invoked on menu activation
 @param id Menu ID of clicked item
 */
void pdfEditWidget::menuActivated(int id) {
 QString action=global->getAction(id);
 cout << "Performing action: " << action << endl;
 //TODO: call scripting here instead... 
 if (action=="quit") exit(0);
 else if (action=="closewindow") delete this;
 else if (action=="newwindow") createNewWindow();
}

/** constructor of pdfEditWidget, creates window and fills it with elements, parameters are ignored */
pdfEditWidget::pdfEditWidget(QWidget *parent, const char *name) : QMainWindow(parent, name) {
 //TODO: tohle je pokusny kod, dodelat

 //SPLITTER
 QSplitter *spl=new QSplitter(this);

 //Button QUIT
 QPushButton *quit=new QPushButton(tr("Quit"), spl);
 quit->resize(75, 30);
 quit->setFont(QFont("Times", 18, QFont::Bold));
 QObject::connect(quit, SIGNAL(clicked()), this, SLOT(exiting()));
 quit->show();

 //Button NEW WINDOW
 QPushButton *neww=new QPushButton(tr("New Window"), spl);
 QObject::connect(neww, SIGNAL(clicked()), this, SLOT(createNewWindow()));
 this->setCentralWidget(spl);
 neww->show();

 //Menu
 QMenuBar *qb=global->loadMenu(this);
 QObject::connect(qb, SIGNAL(activated(int)), this, SLOT(menuActivated(int))); 
 
 //show splitter
 spl->show();
}
