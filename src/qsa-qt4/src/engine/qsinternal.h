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
#include <QList>

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
class LabelStack 
{
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

class QSArgumentsClass : public QSWritableClass 
{
public:
    QSArgumentsClass(QSClass *b) : QSWritableClass(b) { }
    QString name() const { return QString::fromLatin1("Arguments"); }
    QSObject construct(FunctionImp *func, const QSList *args) const;
};

class QSEnv;

class QSEngineImp 
{
    friend class QSEngine;
    friend class Global;
public:
    QSEngineImp(QSEngine *s);
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
    bool evaluate(const QString &code, const QSObject *thisV = 0,
		   bool onlyCheckSyntax = false, int checkMode = 0,
		   int lineZero = 0);
    bool call(QSObject *scope, const QString &func, const QSList &args);
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
    QList<int> errLines;
    QStringList errMsgs;
#ifdef QSDEBUGGER
    Debugger *dbg;
    int sid;
#endif
    QSObject retVal;
    int recursion;
};

#endif
