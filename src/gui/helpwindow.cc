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
 HelpWindow - class representing help window.<br>
 Help window is basically just a limited HTML browser
 @author Martin Petricek
*/

#include "helpwindow.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qstring.h>
#include QTEXTBROWSER
#include "settings.h"
#include "version.h"
#include "util.h"

namespace gui {

/**
 constructor of HelpWindow, creates window and fills it with elements, parameters are ignored
 @param pageName Name of page to open in help window initially
 @param parent Parent of this window
 @param name Name of this window (for debugging)
*/
HelpWindow::HelpWindow(const QString &pageName/*=QString::null*/,QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name,Qt::WDestructiveClose | Qt::WType_TopLevel | Qt::WStyle_Minimize | Qt::WStyle_SysMenu | Qt::WStyle_Title | Qt::WStyle_Customize | Qt::WStyle_NormalBorder) {
 globalSettings->restoreWindow(this,"help");
 setCaption(QString(APP_NAME)+" - "+tr("help"));
// QBoxLayout *l=new QVBoxLayout(this);
 QGridLayout* grl=new QGridLayout(this,2,1);
 grl->setRowStretch(0,0);
 grl->setRowStretch(1,1);

 //Help bar;
 QFrame *bar=new QFrame(this,"help_bar");
 QGridLayout* grl_u=new QGridLayout(bar,1,3);
 grl_u->setColStretch(1,100);
 grl_u->setSpacing(4);
 grl_u->setMargin(2);
 QPushButton* btIndex=new QPushButton(QObject::tr("&Index"),bar,"help_index");
 QPushButton* btClose=new QPushButton(QObject::tr("&Close"),bar,"help_close");
 url=new QLineEdit(pageName,bar,"help_url");
 grl_u->addWidget(btIndex,0,0);
 grl_u->addWidget(url,0,1);
 grl_u->addWidget(btClose,0,2);
 grl->addWidget(bar,0,0);
 QObject::connect(btIndex,  SIGNAL(clicked()), this, SLOT(index()));
 QObject::connect(btClose,  SIGNAL(clicked()), this, SLOT(close()));

 //Help window
 help=new Q_TextBrowser(this,"help_browser");
 grl->addWidget(help,1,0);

 help->mimeSourceFactory()->setFilePath(globalSettings->readPath("help"));
 help->setFrameStyle(QFrame::Panel | QFrame::Sunken);
 connect(help,SIGNAL(sourceChanged(const QString&)),url,SLOT(setText(const QString&)));
 connect(url, SIGNAL(returnPressed()),this,SLOT(goUrl()));
 setPage(pageName);
}

/** Try to go to URL that is typed in the location box */
void HelpWindow::goUrl() {
 setFile(url->text());
}

/**
 Navigate to certain page of the help
 @param name Name of page to show in help
*/
void HelpWindow::setPage(QString name) {
 if (name.isNull()) name="index";
 setFile(name+".html");
}

/** Navigate to certain fileName
 @param fName Name of file to show in help */
void HelpWindow::setFile(const QString &fName) {
 fileName=fName;
 url->setText(fileName);
 help->setSource(fileName);
}


/** Navigate to index page of the help */
void HelpWindow::index() {
 setPage(QString::null);
}

/** default destructor */
HelpWindow::~HelpWindow() {
 globalSettings->saveWindow(this,"help");
 delete help;
}

}
