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
#ifndef __PASSWORDDIALOG_H__
#define __PASSWORDDIALOG_H__

#include <qdialog.h>
#include <qstring.h>
class QLineEdit;
class QCheckBox;

namespace gui {

/**
 Simple widget for asking password
 \brief Window for asking password
*/
class PasswordDialog : public QDialog {
 Q_OBJECT
public slots:
 void cancel();
 void ok();
public:
 static QString ask(QWidget *parent=0,const QString &title=QString::null);
 PasswordDialog(QWidget *parent=0, QString title=QString::null);
 ~PasswordDialog();
private:
 /** editbox with password  */
 QLineEdit* password_query;
 /** Current password */
 QString password;
};

} // namespace gui

#endif

