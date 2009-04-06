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
 ToolBar class, ancestor of QToolBar.
 This toolbar can handle only buttons derived from ToolButton class
 (any generic QWidget can be inserted, but the widget must then handle clicks, etc ... by itself)
 @author Martin Petricek
*/

#include "toolbar.h"
#include "toolbutton.h"
#include <qstring.h>

namespace gui {

/**
 Same constructor as QToolBar
 @param label Caption of toolbar
 @param mainWindow Main window in which toolbar will be managed
 */
ToolBar::ToolBar(const QString &label,QMainWindow *mainWindow) : QToolBar(label,mainWindow) {
 //Empty constructor
}

/**
 Same constructor as QToolBar
 @param parent Main window in which toolbar will be managed
 @param name Name of toolbar (for debugging)
 */
ToolBar::ToolBar(QMainWindow *parent,const char *name/*=0*/) : QToolBar(parent,name) {
 //Empty constructor
}

/**
 Add button to toolbar and link slots, so clicks from buttons are passed along with button ID to the application
 @param qb Button to add to this toolbar
 */
void ToolBar::addButton(ToolButton *qb) {
 QObject::connect(qb,SIGNAL(clicked(int)),this,SLOT(slotClicked(int)));
 QObject::connect(qb,SIGNAL(helpText(const QString&)),this,SLOT(receiveHelpText(const QString&)));
}

/**
 slot that will emit clicked with ID of button
 @param id Id of button
*/
void ToolBar::slotClicked(int id) {
 emit itemClicked(id);
}

/**
 Signal called when receiving help message.
 Forward the message via helpText
 @param message Help message
*/
void ToolBar::receiveHelpText(const QString &message) {
 emit helpText(message);
}

} // namespace gui
