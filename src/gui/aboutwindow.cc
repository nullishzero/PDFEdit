/** @file
 AboutWindow - class representing about window.
 Display authors, application name and version.
 @author Martin Petricek
*/
#include "aboutwindow.h"
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "version.h"
#include "util.h"
#include "iconcache.h"

namespace gui {

using namespace std;

/** Version of program */
QString app=APP_NAME " " VERSION;

/** About Dialog flags */
const Qt::WFlags aboutDialogFlags=Qt::WDestructiveClose | Qt::WType_Dialog;

/**
 constructor of AboutWindow, creates window and fills it with elements, parameters are ignored
 @param parent Parent window of this dialog
 @param name Name of this window (used only for debugging
 */
AboutWindow::AboutWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name,aboutDialogFlags) {
 QString info=QString("<big>")+tr("PDF editor for unix systems")+"</big><br><br>"+tr("Homepage")+" : http://pdfedit.petricek.net/";
 QString authors=QString("<b>")+tr("Project leader")+":</b><br>&nbsp; Martin Beran<br><b>"
                               +tr("Authors")+":</b><br>&nbsp; Michal Hocko<br>&nbsp; Miro Jahoda<br>&nbsp; Jozef Misutka<br>&nbsp; Martin Petricek<br>";
 setCaption(app+" - "+tr("About program"));
 QGridLayout *l=new QGridLayout(this,2,2);
 QLabel *lb=new QLabel(QString("<table><tr><td valign=\"top\"><h1>")+app+"</h1><br>"+tr("Compiled")+": "+COMPILE_TIME+"<br><br>"+
  info+"</td><td valign=\"bottom\">"+authors+"</td></tr><tr><td colspan=\"\2\">"+tr("This program is distributed under terms of GNU GPL")+"</td></tr></table>", this);
 lb->setTextFormat(Qt::RichText);
 QPushButton *ok=new QPushButton(QObject::tr("&Ok"), this);
 QObject::connect(ok, SIGNAL(clicked()), this, SLOT(close()));
 IconCache ic;
 QWidget* logo=new QWidget(this);
 QPixmap* logoImage=ic.getIcon("pdfedit_logo.png");
 logo->setErasePixmap(*logoImage);
 logo->setFixedSize(logoImage->size());
 l->addWidget(lb,0,0);
 l->addWidget(logo,0,1);
 l->addMultiCellWidget(ok,1,1,0,1);
 ok->show();
 lb->show();
}

/**
 This is called on attempt to close window.
 The window will delete itself
 @param e Event parameters (unused)
*/
void AboutWindow::closeEvent(__attribute__((unused)) QCloseEvent *e) {
 delete this;
}

/** default destructor */
AboutWindow::~AboutWindow() {
}

} // namespace gui
