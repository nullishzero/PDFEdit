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
 SelfDestructiveWidget - widget class that will destruct
 when appropriate signal from killer window is sent<br>
 Killer window can be any window, but must emit signal
 "selfDestruct()" for this widget to work<br>
 Used for helper dialogs that should close when file they are editing is also closed<br>
 @author Martin Petricek
*/

#include "selfdestructivewidget.h"

namespace gui {

#ifdef QT4

/**
 Default constructor of Self destructive widget
 @param killer Window that will destruct this widget
 @param parent parent widget (passed to QWidget)
 @param f Widget flags (passed to QWidget)
 */
SelfDestructiveWidget::SelfDestructiveWidget(QWidget *killer,QWidget *parent/*=0*/, Qt::WindowFlags f/*=0*/)
 : QWidget(parent,f) {
QObject::connect(killer,SIGNAL(selfDestruct()),this,SLOT(close()));
}

/**
 Default constructor of Self destructive widget
 @param killer Window that will destruct this widget
 @param parent parent widget (passed to QWidget)
 @param name name of this widget (passed to QWidget)
 @param f Widget flags (passed to QWidget)
 */
SelfDestructiveWidget::SelfDestructiveWidget(QWidget *killer,QWidget *parent/*=0*/, const char *name/*=0*/, Qt::WindowFlags f/*=0*/)
 : QWidget(parent,f) {
QObject::connect(killer,SIGNAL(selfDestruct()),this,SLOT(close()));
}

#else

/**
 Default constructor of Self destructive widget
 @param killer Window that will destruct this widget
 @param parent parent widget (passed to QWidget)
 @param name name of this widget (passed to QWidget)
 @param f Widget flags (passed to QWidget)
 */
SelfDestructiveWidget::SelfDestructiveWidget(QWidget *killer,QWidget *parent/*=0*/, const char *name/*=0*/, WFlags f/*=0*/)
 : QWidget(parent,name,f) {
QObject::connect(killer,SIGNAL(selfDestruct()),this,SLOT(close()));
}

#endif

/** default destructor */
SelfDestructiveWidget::~SelfDestructiveWidget() {
}

} // namespace gui
