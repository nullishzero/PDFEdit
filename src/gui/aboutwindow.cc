/** @file
 AboutWindow - class representing about window. Display authors, application name and version
*/
#include "aboutwindow.h"
#include "settings.h"
#include <iostream>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "version.h"

using namespace std;

/** Version of program*/
QString app=APP_NAME " " VERSION;

/** constructor of AboutWindow, creates window and fills it with elements, parameters are ignored */
AboutWindow::AboutWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name,WDestructiveClose || WType_TopLevel) {
 QString info=tr("PDF editor for unix systems");
 QString authors=tr("Project leader")+":\n Martin Beran\n"+tr("Authors")+":\n Michal Hocko\n Miro Jahoda\n Josef Misutka\n Martin Petricek\n";
 setCaption(tr("About program"));
 QBoxLayout *l=new QVBoxLayout(this);
// QVBox *qb=new QVBox(this,"about_vbox");
 QLabel *lb=new QLabel(app+"\n"+tr("Compiled")+": "+COMPILE_TIME+"\n\n"+info+"\n\n"+authors+"\n", this);
 QPushButton *ok=new QPushButton(tr("Ok"), this);
 QObject::connect(ok, SIGNAL(clicked()), this, SLOT(close()));
 l->addWidget(lb);
 l->addWidget(ok);
 ok->show();
 lb->show();
}

/** default destructor */
AboutWindow::~AboutWindow() {
}
