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

#ifndef QSINTERNAL_H
#define QSINTERNAL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qsengine.h"
#include "qsobject.h"
#include "qsfunction.h"
#include "qsclass.h"
#include <qstring.h>

class QSNode;
class QSFunctionBodyNode;
class QSProgramNode;
class QSClassClass;

class Object;
class RegExp;
#ifdef QSDEBUGGER
class Debugger;
#endif

/**
   * @short The "label set" in Ecma-262 spec
   */
class LabelStack {
public:
    LabelStack(): tos(0L) {}
    ~LabelStack();

    /**
     * If id is not empty and is not in the stack already, puts it on top of
     * the stack and returns true, otherwise returns false
     */
    bool push(const QString &id);
    /**
     * Is the id in the stack?
     */
    bool contains(const QString &id) const;
    /**
     * Removes from the stack the last pushed id (what else?)
     */
    void pop();
private:
    struct StackElm {
	QString id;
	StackElm *prev;
    };

    StackElm *tos;
};

class FunctionImp;

class QSArgumentsClass : public QSWritableClass {
public:
    QSArgumentsClass( QSClass *b ) : QSWritableClass( b ) { }
    QString name() const { return QString::fromLatin1("Arguments"); }
    QSObject construct( FunctionImp *func, const QSList *args ) const;
};

class QSEnv;

class QSEngineImp {
    friend class QSEngine;
    friend class Global;
public:
    QSEngineImp( QSEngine *s );
    ~QSEngineImp();
    QSEngine *interpreter() const { return scr; }
    QSEnv *env() const { return en; }
    void mark();

#ifdef QSDEBUGGER
    /**
     * Attach debugger d to this engine. If there already was another instance
     * attached it will be detached.
     */
    void attachDebugger(Debugger *d);
    Debugger *debugger() const { return dbg; }
    int sourceId() const { return sid; }
    void incrSourceId() { sid++; }
    void decrSourceId() { sid--; }
    bool setBreakpoint(int id, int line, bool set);
#endif

private:
    /**
     * Initialize global object and context. For internal use only.
     */
    void init();
    void clear();
    /**
     * Called when the first interpreter is instanciated. Initializes
     * global pointers.
     */
    void globalInit();
    /**
     * Called when the last interpreter instance is destroyed. Frees
     * globally allocated memory.
     */
    void globalClear();
    bool evaluate( const QString &code, const QSObject *thisV = 0,
		   bool onlyCheckSyntax = FALSE, int checkMode = 0,
		   int lineZero = 0 );
    bool call( QSObject *scope, const QString &func, const QSList &args );
public:
    QSEngineImp *next, *prev;
    QSEngine *scr;

private:
    QSNode *firstN;

    static int instances; // total number of instances
    bool initialized;
    QSEnv *en;
    Global *glob;
    int errType;
    QValueList<uint> errLines;
    QStringList errMsgs;
#ifdef QSDEBUGGER
    Debugger *dbg;
    int sid;
#endif
    QSObject retVal;
    int recursion;
};

#endif
