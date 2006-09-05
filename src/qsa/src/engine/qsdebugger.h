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

#ifndef QSDEBUGGER_H
#define QSDEBUGGER_H

#include "qsinternal.h"

class QSStatementNode;
class QSEnv;

#ifdef QSDEBUGGER
class QUICKCORE_EXPORT Debugger {
    friend class QSEngineImp;
    friend class QSStatementNode;
    friend class QSClass;
public:
    /**
     * Available modes of the debugger.
     */
    enum Mode { Disabled = 0, Next, Step, Continue, Stop };
    /**
     * Construct a debugger and attach it to the scripting engine s.
     */
    Debugger( QSEngine *s );
    /**
     * Destruct the debugger and detach from the scripting engine we
     * might have been attached to.
     */
    virtual ~Debugger();
    /**
     * Attaches the debugger to specified scripting engine.
     */
    void attach( QSEngine *e );
    /**
     * Returns the engine the interpreter is currently attached to. Null
     * if there isn't any.
     */
    QSEngine* engine() const;
    /**
     * Detach the debugger from any scripting engine.
     */
    void detach();
    /**
     * Set debugger into specified mode. This will influence further behaviour
     * if execution of the programm is started or continued.
     */
    virtual void setMode( Mode m );
    /**
     * Returns the current operation mode.
     */
    Mode mode() const;
    /**
     * Returns the line number the debugger currently has stopped at.
     * -1 if the debugger is not in a break status.
     */
    int lineNumber() const { return l; }
    /**
     * Returns the source id the debugger currently has stopped at.
     * -1 if the debugger is not in a break status.
     */
    int sourceId() const { return sid; }
    /**
     * Sets a breakpoint in the first statement where line lies in between
     * the statements range. Returns true if sucessfull, false if no
     * matching statement could be found.
     */
    bool setBreakpoint( int id, int line );
    bool deleteBreakpoint( int id, int line );
    void clearAllBreakpoints( int id=-1 );
    static bool validBreakpoint( const QString &code, int line );
    /**
     * Returns the value of ident out of the current context in string form
     */
    QString varInfo( const QString &ident );
    /**
     * Set variable ident to value. Returns true if successful, false if
     * the specified variable doesn't exist or isn't writable.
     */
    bool setVar( const QString &ident, const QSObject &value );

    QSEnv *env() const;

    void setSourceId( int i ) { sid = i; }

    virtual void storeExceptionStack() { }
    virtual void clear();

protected:
    /**
     * Invoked in case a breakpoint or the next statement is reached in step
     * mode. The return value decides whether execution will continue. True
     * denotes continuation, false an abortion, respectively.
     *
     * The default implementation does nothing. Overload this method if
     * you want to process this event.
     */
    virtual bool stopEvent();
    /**
     * Returns an integer that will be assigned to the code passed
     * next to one of the QSEngine::evaluate() methods. It's basically
     * a counter to will only be reset to 0 on QSEngine::clear().
     *
     * This information is useful in case you evaluate multiple blocks of
     * code containing some function declarations. Keep a map of source id/
     * code pairs, query sourceId() in case of a stopEvent() and update
     * your debugger window with the matching source code.
     */
    int freeSourceId() const;
    /**
     * Invoked on each function call. Use together with @ref returnEvent
     * if you want to keep track of the call stack.
     */
    virtual void callEvent( const QString &fn = QString::null,
			    const QString &s = QString::null );
    /**
     * Invoked on each function exit.
     */
    virtual void returnEvent();

private:
    void reset();
    bool hit( int line, bool breakPoint );

    QSEngine *eng;
    Mode dmode;
    int l;
    int sid;
};
#endif

#endif
