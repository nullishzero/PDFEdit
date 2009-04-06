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
#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include <qtoolbar.h>

class QMainWindow;

namespace gui {

class ToolButton;

/**
 Class wrapping QToolBar, providing "click forwarding" from TButtons.
 Buttons must be added with addButton() for this to work properly
 \brief Application toolbar
 */
class ToolBar : public QToolBar {
 Q_OBJECT
public:
 ToolBar(const QString &label,QMainWindow *mainWindow);
 ToolBar(QMainWindow *parent,const char *name=0);
 void addButton(ToolButton *qb);
signals:
 /**
  Signal emitted when clicked on one of toolbar buttons.
  Send ID  number associated with it.
  @param id ID of button
 */
 void itemClicked(int id);
 /**
  Send help message when mouse cursor enters/leaves on of toolbar buttons.
  Help message is sent on entering and QString::null on leaving.
  @param message Help message
 */
 void helpText(const QString &message);
public slots:
 void slotClicked(int id);
 void receiveHelpText(const QString &message);
};

} // namespace gui

#endif
