/** @file
 HelpWindow - class representing help window.
*/

#include "helpwindow.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "settings.h"
#include "version.h"

namespace gui {

/** constructor of HelpWindow, creates window and fills it with elements, parameters are ignored
@param fileName File to open in help window
@param parent Parent of this window
@param name Name of this window (for debugging)
 */
HelpWindow::HelpWindow(const QString &fileName,QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name,WDestructiveClose || WType_TopLevel || WStyle_Minimize || WStyle_SysMenu || WStyle_Title || WStyle_Customize) {
 globalSettings->restoreWindow(this,"help"); 
 setCaption(QString(APP_NAME)+" - "+tr("help"));
 QBoxLayout *l=new QVBoxLayout(this);
 help=new QTextBrowser(this,"help_browser");
 l->addWidget(help);

 help->mimeSourceFactory()->setFilePath(".");
 help->setFrameStyle(QFrame::Panel | QFrame::Sunken);
// connect( browser, SIGNAL( sourceChanged(const QString& ) ),
//             this, SLOT( sourceChanged( const QString&) ) );

 help->setSource("../../doc/user/scripting.html");//TODO: point to real help
}

/** This is called on attempt to close window. */
void HelpWindow::closeEvent(QCloseEvent *e) {
 delete this;
}

/** default destructor */
HelpWindow::~HelpWindow() {
 globalSettings->saveWindow(this,"help"); 
 delete help;
}

}
