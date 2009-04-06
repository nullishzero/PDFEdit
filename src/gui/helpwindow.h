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
#ifndef __HELPWINDOW_H__
#define __HELPWINDOW_H__

#include "qtcompat.h"
#include <qwidget.h>
class Q_TextBrowser;
class QString;
class QLineEdit;

namespace gui {

/**
 class representing help window.<br>
 Help window is basically just a limited HTML browser
 \brief Help window
*/
class HelpWindow : public QWidget {
Q_OBJECT
public:
 HelpWindow(const QString &pageName=QString::null,QWidget *parent=0,const char *name=0);
 ~HelpWindow();
public slots:
 void goUrl();
 void setFile(const QString &fName);
 void index();
 void setPage(QString name);
private:
 /** Rich text browser with hypertext navigation. Core of this help window */
 Q_TextBrowser *help;
 /** Name of file loaded into help */
 QString fileName;
 /** Line edit control used to show/edit name of current file */
 QLineEdit *url;
};

}

#endif
