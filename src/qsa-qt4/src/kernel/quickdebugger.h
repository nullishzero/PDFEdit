/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech AS. All rights reserved.
**
** This file is part of the QSA of the Qt Toolkit.
**
** For QSA Commercial License Holders (non-open source):
** 
** Licensees holding a valid Qt Script for Applications (QSA) License Agreement
** may use this file in accordance with the rights, responsibilities and
** obligations contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of the Licensing Agreement are
** not clear to you.
** 
** Further information about QSA licensing is available at:
** http://www.trolltech.com/products/qsa/licensing.html or by contacting
** info@trolltech.com.
** 
** 
** For Open Source Edition:  
** 
** This file may be used under the terms of the GNU General Public License
** version 2 as published by the Free Software Foundation and appearing in the
** file LICENSE.GPL included in the packaging of this file.  Please review the
** following information to ensure GNU General Public Licensing requirements
** will be met:  http://www.trolltech.com/products/qt/opensource.html 
** 
** If you are unsure which license is appropriate for your use, please review
** the following information:
** http://www.trolltech.com/products/qsa/licensing.html or contact the 
** sales department at sales@trolltech.com.

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QUICKDEBUGGER_H
#define QUICKDEBUGGER_H

#ifndef QT_H
#include <QObject>
#include <QList>
#include <QStack>
#endif // QT_H

#include <qsdebugger.h>

struct QuickDebuggerStackFrame
{
    QuickDebuggerStackFrame(const QString &fn = QString::null,
        int ln = -1, int sid = -1) : function(fn), line(ln), sourceId(sid) {}
        QString function;
        int line;
        int sourceId;

        bool operator==(const QuickDebuggerStackFrame &other) const
        {
            return function == other.function &&
                   line == other.line &&
                   sourceId == other.sourceId;
        }
};

class QuickDebugger : public QObject, public Debugger
{
    Q_OBJECT
public:

    QuickDebugger(QSEngine *e);
    ~QuickDebugger();

    int freeSourceId();

    bool watch(const QString &var, QString &type, QString &value);
    bool setVariable(const QString &var, const QString &val);

    void clearAllBreakpoints(int i = 0);
    void clear();

    QList<QuickDebuggerStackFrame> backtrace();

    void storeExceptionStack();

signals:
    void stopped(bool &ret);
    void stackChanged(int level);

protected:
    bool stopEvent();
    void callEvent(const QString &fn, const QString &s);
    void returnEvent();
    QString varInfo(const QString &ident);

private:
    QList<QuickDebuggerStackFrame> cStack;
    QList<QuickDebuggerStackFrame> exceptionStack;
    QList<QSObject> *exceptionScope;
    QStack<bool> tmpCStack;

    uint hadError:1;
};

#endif
