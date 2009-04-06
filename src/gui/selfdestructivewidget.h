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
#ifndef __SELFDESTRUCTIVEWIDGET_H__
#define __SELFDESTRUCTIVEWIDGET_H__

#include <qwidget.h>
#include "qtcompat.h"

namespace gui {

/**
 Widget class that will destruct
 when appropriate signal from killer window is sent<br>
 Killer window can be any window, but must emit signal
 "selfDestruct()" for this widget to work<br>
 Used for helper dialogs that should close themselves
 when file/item they are editing is also closed<br>
 \brief Widget destroying itself on certain signal
*/
class SelfDestructiveWidget : public QWidget {
Q_OBJECT
public:
#ifdef QT4
 SelfDestructiveWidget(QWidget *killer,QWidget *parent=0, Qt::WindowFlags f=0);
 SelfDestructiveWidget(QWidget *killer,QWidget *parent=0, const char *name=0, Qt::WindowFlags f=0);
#else
 SelfDestructiveWidget(QWidget *killer,QWidget *parent=0, const char *name=0, WFlags f=0);
#endif
 ~SelfDestructiveWidget();
};

} // namespace gui

#endif
