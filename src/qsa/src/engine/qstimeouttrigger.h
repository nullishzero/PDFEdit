/****************************************************************************
** $Id$
**
** Copyright (C) 2001-2006 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Script for Applications framework (QSA).
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding a valid Qt Script for Applications license may use
** this file in accordance with the Qt Script for Applications License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about QSA Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
*****************************************************************************/

#ifndef QSTIMEOUTTRIGGER_H
#define QSTIMEOUTTRIGGER_H

#include <qobject.h>
#include <qdatetime.h>

class QSTimeoutTrigger : public QObject
{
    Q_OBJECT
public:
    QSTimeoutTrigger() : QObject() { }
    inline void start() { time.start(); lastTimeout = 0; }
    inline void fireTimeout()
    {
	if (time.elapsed()-lastTimeout >= ival) {
	    lastTimeout = time.elapsed();
	    emit timeout(lastTimeout);
	}
    }

    void setInterval(int i) { ival = i; }
    int interval() const { return ival; }

signals:
    void timeout(int runningTime);

private:
    QTime time;
    int lastTimeout;
    int ival;
};

#endif // QSTIMEOUTTRIGGER_H
