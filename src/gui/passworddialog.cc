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
 PasswordDialog - widget for inputing password
 @author Martin Petricek
*/

#include "passworddialog.h"
#include "version.h"
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <utils/debug.h>

namespace gui {

using namespace std;

/**
 Default constructor of password dialog
 @param title of dialog. If not specified, default will be used
 @param parent parent widget of this dialog
 */
PasswordDialog::PasswordDialog(QWidget *parent/*=0*/, QString title/*=QString::null*/) : QDialog(parent) {
 //Dialog caption
 if (title.isNull()) title=tr("Enter password");
 setCaption(QString(APP_NAME)+" - "+title);

 //Main layout
 QGridLayout* grl_up=new QGridLayout(this,4,1);
 grl_up->setMargin(8);
 grl_up->setSpacing(4);

 //Password entry box with caption
 grl_up->addWidget(new QLabel(tr("Password:"),this),0,0);
 password_query=new QLineEdit(this);
 password_query->setEchoMode(QLineEdit::Password);
 grl_up->addWidget(password_query,1,0);

 //Lower layout with buttons
 QFrame *low=new QFrame(this);
 QGridLayout* grl=new QGridLayout(low,1,3);
 grl->setColStretch(0,10);
 grl->setColStretch(1,1);
 grl->setColStretch(2,1);
 grl->setSpacing(16);
 grl->setMargin(2);
 QPushButton* btOk=    new QPushButton(QObject::tr("&Ok"),low,"opt_ok");
 QPushButton* btCancel=new QPushButton(QObject::tr("&Cancel"),low,"opt_cancel");
 grl->addWidget(btOk,0,1);
 grl->addWidget(btCancel,0,2);
 grl_up->addWidget(low,2,0);

 QObject::connect(btCancel, SIGNAL(clicked()), this, SLOT(cancel()));
 QObject::connect(btOk,	    SIGNAL(clicked()), this, SLOT(ok()));
}

/** Called on pushing 'OK' button */
void PasswordDialog::ok() {
 password=password_query->text();
 accept();
}

/** Called on pushing 'Cancel' button */
void PasswordDialog::cancel() {
 password=QString::null;
 reject();
}

/**
 Invoke password dialog, asking user for password
 @param parent Parent window of the dialog
 @param title Optional title, if not specified, some default title like "enter password" would be used
 @return entered password, or QString::null if dialog is cancelled
*/
QString PasswordDialog::ask(QWidget *parent/*=0*/,const QString &title/*=QString::null*/) {
 PasswordDialog pwd(parent,title);
 if (pwd.exec()==QDialog::Accepted) return pwd.password;
 return QString::null;
}

/** default destructor */
PasswordDialog::~PasswordDialog() {
 if (!password.isNull()) {
  //Wipe out password from memory for security
  for (unsigned int i=0;i<password.length();i++) password[i]=0;
 }
 guiPrintDbg(debug::DBG_DBG,"Options closing ...");
}

} // namespace gui


