/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
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
#include <qsaglobal.h>
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
 IconCache ic;
 //Window title
 setCaption(app+" - "+tr("About program"));

 QGridLayout *l=new QGridLayout(this,2,2);

 //Text in about window
 QString info=QString("<big><b>")+tr("Free program for PDF document manipulation")+"</b></big><br><br>"
  +tr("Homepage")+" : http://pdfedit.petricek.net/<br>"
  +tr("Project page")+" : http://sourceforge.net/projects/pdfedit";

 QString authors=QString("<b>Copyright (C) 2006, 2007 PDFedit team:</b><br>")
  +QString::fromUtf8("&nbsp; Michal Hocko<br>&nbsp; Miro Jahoda<br>&nbsp; Jozef Mišutka<br>&nbsp; Martin Petříček<br>");

 QLabel *lb=new QLabel(QString("<table><tr><td valign=\"top\"><h1>")+app+"</h1>"+tr("Compiled")+": "+COMPILE_TIME+
  +"<br>"+tr("Using Qt %1").arg(QT_VERSION_STR)+
  +tr(", QSA %1").arg(QSA_VERSION_STRING)+
  "<br><br>"+info+"<br><br>"+authors+"</td></tr><tr><td colspan=\"\2\">"+tr("This program is distributed under terms of GNU GPL")+"</td></tr></table>", this);
 lb->setTextFormat(Qt::RichText);

 //Lower frame with Ok button
 QFrame *okFrame=new QFrame(this);
 QGridLayout *lFrame=new QGridLayout(okFrame,1,2,5);
// okFrame->setPaletteBackgroundColor(white);
// okFrame->setBackgroundMode(FixedColor);
 QPushButton *ok=new QPushButton(QObject::tr("&Ok"), okFrame);
 lFrame->addWidget(ok,0,1);
 QObject::connect(ok, SIGNAL(clicked()), this, SLOT(close()));

 //Logo on right
 QWidget* logo=new QWidget(this);
 QPixmap* logoImage=ic.getIcon("pdfedit_logo.png");
 QSize imageSize;
 QSize bgSize;
 if (logoImage) {
  logo->setErasePixmap(*logoImage);
  imageSize=logoImage->size();
  logo->setFixedSize(imageSize);
 }

 //Background of text
 QPixmap* bgImage=ic.getIcon("pdfedit_bg.png");
 if (bgImage) {
  lb->setErasePixmap(*bgImage);
  bgSize=bgImage->size();
  lb->setMaximumSize(bgSize);
  lb->setMaximumHeight(imageSize.height());
  okFrame->setFixedHeight(10+ok->sizeHint().height());
 }
 if (bgImage!=NULL & logoImage!=NULL) {
  setMinimumSize(imageSize);
  setMaximumSize(bgSize);
  setFixedHeight(10+ok->sizeHint().height()+imageSize.height());
 }
 l->addWidget(lb,0,0);
 l->addWidget(logo,0,1);
 l->addMultiCellWidget(okFrame,1,1,0,1);
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
