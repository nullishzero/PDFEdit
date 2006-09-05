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

#ifndef QUICKDEBUGGER_H
#define QUICKDEBUGGER_H

#ifndef QT_H
#include <qobject.h>
#include <qvaluelist.h>
#include <qvaluestack.h>
#include "dlldefs.h"
#endif // QT_H

#include <qsdebugger.h>

struct QUICKCORE_EXPORT QuickDebuggerStackFrame
{
    QuickDebuggerStackFrame( const QString &fn = QString::null,
			     int ln = -1, int sid = -1 ) : function( fn ), line( ln ), sourceId( sid ) {}
    QString function;
    int line;
    int sourceId;

    bool operator==( const QuickDebuggerStackFrame &other ) const
    {
	return function == other.function &&
	    line == other.line &&
	    sourceId == other.sourceId;
    }
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class QUICKCORE_EXPORT QValueList<QuickDebuggerStackFrame>;
template class QUICKCORE_EXPORT QValueStack<bool>;
// MOC_SKIP_END
#endif

class QUICKCORE_EXPORT QuickDebugger : public QObject, public Debugger
{
    Q_OBJECT
public:

    QuickDebugger( QSEngine *e );
    ~QuickDebugger();

    int freeSourceId();

    bool watch( const QString &var, QString &type, QString &value );
    bool setVariable( const QString &var, const QString &val );

    void clearAllBreakpoints( int i = 0 );
    void clear();

    QValueList<QuickDebuggerStackFrame> backtrace();

    void storeExceptionStack();

signals:
    void stopped( bool &ret );
    void stackChanged( int level );

protected:
    bool stopEvent();
    void callEvent( const QString &fn, const QString &s );
    void returnEvent();
    QString varInfo( const QString &ident );

private:
    QValueList<QuickDebuggerStackFrame> cStack;
    QValueList<QuickDebuggerStackFrame> exceptionStack;
    QValueList<QSObject> *exceptionScope;
    QValueStack<bool> tmpCStack;

    uint hadError:1;
};

#endif
