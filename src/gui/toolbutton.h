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
#ifndef __TOOLBUTTON_H__
#define __TOOLBUTTON_H__

#include "qtcompat.h"
#include <qtoolbutton.h>
class QPixmap;
class QString;
class QIcon;

namespace gui {

/**
 Class wrapping QToolButton, adding button ID and providing also
 a different constructors (with QPixmap or QIcon)<br>
 Also, emits clicked(id) signal in addition to normal clicked() signal
 \brief Pushbutton in application toolbar
 */
class ToolButton : public QToolButton {
 Q_OBJECT
public:
 ToolButton(const QIcon *iconSet, const QString tooltip, int id, QWidget *parent=0, const char *name="");
 ToolButton(const QPixmap *icon, const QString tooltip, int id, QWidget *parent=0, const char *name="");
protected slots:
 void slotClicked();
protected:
 virtual void enterEvent(QEvent *e);
 virtual void leaveEvent(QEvent *e);
private:
 /** ID number of this button */
 int b_id;
signals:
 /**
  Signal emitted when clicked on this button.
  The button sends ID of itself.
  @param id ID of button
 */
 void clicked(int id);
 /**
  Send help message when mouse cursor enters/leaves the button.
  Help message is sent on enter and QString::null on leave.
  @param message Help message
 */
 void helpText(const QString &message);
};

} // namespace gui

#endif
