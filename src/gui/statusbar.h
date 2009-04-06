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
#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#include <qstatusbar.h>
#include <qstring.h>

class QLabel;
class QTimer;
class QColor;

namespace gui {

/**
 Class representing statusbar.<br>
 The statusbar contain tho areas, large message area on left
 and smaller informative area on right.
 \brief Editor statusbar
 */
class StatusBar : public QStatusBar {
 Q_OBJECT
public:
 StatusBar(QWidget *parent=0,const char *name=0);
 ~StatusBar();
public slots:
 void message(const QString &theMessage);
 void receiveInfoText(const QString &theMessage);
 void receiveWarnText(const QString &theMessage);
private slots:
 void timeOut();
private:
 void normCol();
 void warnCol();
private:
 /** Informational label on right */
 QLabel *info;
 /** Message label on left */
 QLabel *msgLabel;
 /** Store old message in case of replacement by timed warning */
 QString storedMsg;
 /** Timer to watch for message timeouts*/
 QTimer *tm;
};

} // namespace gui

#endif
