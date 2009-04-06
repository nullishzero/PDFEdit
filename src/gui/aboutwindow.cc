/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 AboutWindow - class representing about window.
 Display authors, application name and version.
 @author Martin Petricek
*/
#include "aboutwindow.h"
#include "version.h"
#include "util.h"
#include "iconcache.h"
#include "imagewidget.h"
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qsaglobal.h>
#include <qsizepolicy.h>

namespace gui {

using namespace std;

/** Version of program */
QString app=QString(APP_NAME) +" "+ QString(PDFEDIT_VERSION);

/** About Dialog flags */
const Qt::WFlags aboutDialogFlags=Qt::WDestructiveClose | Qt::WType_Dialog;

/**
 constructor of AboutWindow, creates window and fills it with elements, parameters are ignored
 @param parent Parent window of this dialog
 @param name Name of this window (used only for debugging
 */
AboutWindow::AboutWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name,aboutDialogFlags) {
 ic=new IconCache();
 //Window title
 setCaption(app+" - "+tr("About program"));

 QGridLayout *l=new QGridLayout(this,2,2);
 l->setRowStretch(0,1);

 //Text in about window
 QString info=QString("<big><b>")+tr("Free program for PDF document manipulation")+"</b></big><br><br>"
  +tr("Homepage")+" : http://pdfedit.petricek.net/<br>"
  +tr("Project page")+" : http://sourceforge.net/projects/pdfedit";

 QString authors=QString("<b>Copyright (C) 2006 - 2009 PDFedit team:</b><br>")
  +QString::fromUtf8("&nbsp; Michal Hocko<br>&nbsp; Miro Jahoda<br>&nbsp; Jozef Mišutka<br>&nbsp; Martin Petříček<br>");

 QLabel *lb=new QLabel(QString("<table><tr><td valign=\"top\"><h1>")+app+"</h1>"+tr("Compiled")+": "+COMPILE_TIME
  +"<br>"+tr("Using Qt %1").arg(QT_VERSION_STR)
  +tr(", QSA %1").arg(QSA_VERSION_STRING)
  +"<br><br>"+info+"<br><br>"+authors+"</td></tr><tr><td colspan=\"\2\">"+tr("This program is distributed under terms of GNU GPL")+"</td></tr></table>", this);
 lb->setTextFormat(Qt::RichText);

 //Lower frame with Ok button
 QFrame *okFrame=new QFrame(this);
 QGridLayout *lFrame=new QGridLayout(okFrame,1,2,5);

 //Ok button
 QPushButton *ok=new QPushButton(QObject::tr("&Ok"), okFrame);
 lFrame->addWidget(ok,0,1);
 QObject::connect(ok, SIGNAL(clicked()), this, SLOT(close()));
 okFrame->setFixedHeight(10+ok->sizeHint().height());

 //Image sizes
 QSize imageSize;
 QSize bgSize;

 //Logo on right
 QPixmap* logoImage=ic->getIcon("pdfedit_logo.png");
 QWidget *logo=new ImageWidget(logoImage,QColor(255,255,255),this);
 if (logoImage) {
  imageSize=logoImage->size();
  logo->setFixedWidth(imageSize.width());
 }

 //Background of text
 QPixmap* bgImage=ic->getIcon("pdfedit_bg.png");
 if (bgImage) {
  lb->setErasePixmap(*bgImage);
  bgSize=bgImage->size();
  lb->setMaximumSize(bgSize);
 }

 //Set minumum/maximum sizes
 if (bgImage!=NULL && logoImage!=NULL) {
  // Two conditions should be met:
  // Background must not repeat
  // Logo must be shown completely
  setMaximumSize(QSize(bgSize.width()+imageSize.width(),10+ok->sizeHint().height()+bgSize.height()));
  setMinimumSize(QSize(imageSize.width(),10+ok->sizeHint().height()+imageSize.height()));
 }

 l->setResizeMode(QLayout::Minimum);
 l->addWidget(lb,0,0);
 l->addWidget(logo,0,1);
 l->addMultiCellWidget(okFrame,1,1,0,1);
}

/** default destructor */
AboutWindow::~AboutWindow() {
 delete ic;
}

} // namespace gui
