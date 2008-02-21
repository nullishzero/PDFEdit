/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <qstring.h>
#include <qcolor.h>

class QWidget;

namespace gui {

QString openFileDialog(QWidget* parent,const QString &caption=QString::null,const QString &settingName=QString::null,const QString &filters=QString::null,const QString &savePath=QString::null);
QString openFileDialogPdf(QWidget* parent=NULL);
QString saveFileDialog(QWidget* parent,const QString &oldname,bool askOverwrite=true,const QString &caption=QString::null,const QString &settingName=QString::null,const QString &filters=QString::null,const QString &savePath=QString::null);
QString saveFileDialogPdf(QWidget* parent=NULL,const QString &oldname=QString::null,bool askOverwrite=true);
QString saveFileDialogXml(QWidget* parent=NULL,const QString &oldname=QString::null,bool askOverwrite=true);
QString readStringDialog(QWidget* parent,const QString &message, const QString &def="");
QColor colorDialog(QWidget* parent);
bool questionDialog(QWidget* parent,const QString &msg);

} // namespace gui

#endif
