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

#include "qsarray_object.h"
#include "qscheck.h"
#include "qsclass.h"
#include "qsdate_object.h"
#include "qsdebugger.h"
#include "qsengine.h"
#include "qsenv.h"
#include "qserror_object.h"
#include "qsfuncref.h"
#include "qsinternal.h"
#include "qslexer.h"
#include "qsmath_object.h"
#include "qsnodes.h"
#include "qsobject.h"
#include "qsobject_object.h"
#include "qsoperations.h"
#include "qsregexp_object.h"
#include "qstypes.h"

#include <qapplication.h>
#include <qregexp.h>
#include <private/qmutexpool_p.h>
#include <stdio.h>
#include <assert.h>

extern int qsyyparse();

QString qs_format_error( const QString &err )
{
    if( err.startsWith( QString::fromLatin1("parse error,") ) ) {
	QString copy = err.mid( 13 );
	copy.replace( QString::fromLatin1("ABSTRACT"), QString::fromLatin1("abstract") );
	copy.replace( QString::fromLatin1("BREAK"), QString::fromLatin1("break") );
	copy.replace( QString::fromLatin1("CASE"), QString::fromLatin1("case") );
	copy.replace( QString::fromLatin1("CATCH"), QString::fromLatin1("catch") );
	copy.replace( QString::fromLatin1("CLASS"), QString::fromLatin1("class") );
	copy.replace( QString::fromLatin1("CONSTRUCTOR"), QString::fromLatin1("constructor") );
	copy.replace( QString::fromLatin1("CONTINUE"), QString::fromLatin1("continue") );
	copy.replace( QString::fromLatin1("DEFAULT"), QString::fromLatin1("default") );
	copy.replace( QString::fromLatin1("DO"), QString::fromLatin1("do") );
	copy.replace( QString::fromLatin1("ELSE"), QString::fromLatin1("else") );
	copy.replace( QString::fromLatin1("EXTENDS"), QString::fromLatin1("extends") );
	copy.replace( QString::fromLatin1("FALSETOKEN"), QString::fromLatin1("false") );
	copy.replace( QString::fromLatin1("FINAL"), QString::fromLatin1("final") );
	copy.replace( QString::fromLatin1("FINALLY"), QString::fromLatin1("finally") );
	copy.replace( QString::fromLatin1("FOR"), QString::fromLatin1("'for") );
	copy.replace( QString::fromLatin1("FUNCTION"), QString::fromLatin1("function") );
	copy.replace( QString::fromLatin1("IDENT"), QString::fromLatin1("identifier") );
	copy.replace( QString::fromLatin1("IF"), QString::fromLatin1("if") );
	copy.replace( QString::fromLatin1("IMPORT"), QString::fromLatin1("import") );
	copy.replace( QString::fromLatin1("INSTANCEOF"), QString::fromLatin1("instanceof") );
	copy.replace( QString::fromLatin1("IS"), QString::fromLatin1("is") );
	copy.replace( QString::fromLatin1("NEW"), QString::fromLatin1("new") );
	copy.replace( QString::fromLatin1("NULLTOKEN"), QString::fromLatin1("null") );
	copy.replace( QString::fromLatin1("PACKAGE"), QString::fromLatin1("package") );
	copy.replace( QString::fromLatin1("PRIVATE"), QString::fromLatin1("private") );
	copy.replace( QString::fromLatin1("PUBLIC"), QString::fromLatin1("public") );
	copy.replace( QString::fromLatin1("QS_CONST"), QString::fromLatin1("const") );
	copy.replace( QString::fromLatin1("QS_DELETE"), QString::fromLatin1("delete") );
	copy.replace( QString::fromLatin1("QS_IN"), QString::fromLatin1("in") );
	copy.replace( QString::fromLatin1("QS_VOID"), QString::fromLatin1("void") );
	copy.replace( QString::fromLatin1("RETURN"), QString::fromLatin1("return") );
	copy.replace( QString::fromLatin1("STATIC"), QString::fromLatin1("static") );
	copy.replace( QString::fromLatin1("SWITCH"), QString::fromLatin1("switch") );
	copy.replace( QString::fromLatin1("THIS"), QString::fromLatin1("this") );
	copy.replace( QString::fromLatin1("THROW"), QString::fromLatin1("throw") );
	copy.replace( QString::fromLatin1("TRUETOKEN"), QString::fromLatin1("true") );
	copy.replace( QString::fromLatin1("TRY"), QString::fromLatin1("try") );
	copy.replace( QString::fromLatin1("TYPEOF"), QString::fromLatin1("typeof") );
	copy.replace( QString::fromLatin1("VAR"), QString::fromLatin1("var") );
	copy.replace( QString::fromLatin1("WITH"), QString::fromLatin1("with") );
	copy.replace( QString::fromLatin1("RESERVED"), QString::fromLatin1("reserved keyword") );
	return copy;
    }
    return err;
}

QSObject QSArgumentsClass::construct( FunctionImp * /*func*/,
		 		      const QSList * /*args*/ ) const {
    QSObject obj( createWritable() );
#if 0
    obj.put( "callee", Function( func ), DontEnum );
    if ( args ) {
	obj.put( "length", createNumber( args->size() ), DontEnum );
	QSListIterator arg = args->begin();
	for ( int i = 0; arg != args->end(); arg++, i++ )
	    obj.put( QSString::from(i), *arg, DontEnum );
    }
#endif
    return obj;
}

int          QSEngineImp::instances = 0;

QSEngineImp::QSEngineImp( QSEngine *s )
    : scr(s),
      initialized(false),
      en( 0 ),
      glob( 0 )
#ifdef QSDEBUGGER
    ,dbg(0L)
#endif
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    QMutexLocker locker(qt_global_mutexpool ? qt_global_mutexpool->get(qApp) : 0);
#endif

    instances++;
    // are we the first interpreter instance ? Initialize some stuff
    if (instances == 1)
	globalInit();
}

QSEngineImp::~QSEngineImp()
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    QMutexLocker locker(qt_global_mutexpool ? qt_global_mutexpool->get(qApp) : 0);
#endif

#ifdef QSDEBUGGER
    attachDebugger(0L);
#endif

    clear();

    // are we the last of our kind ? Free global stuff.
    if (instances == 1)
	globalClear();
    instances--;
}

void QSEngineImp::globalInit()
{
    (void)new QSLexer();
}

void QSEngineImp::globalClear()
{
    delete QSLexer::lexer();
}

void QSEngineImp::mark()
{
    //   assert(glob.imp());
    glob->mark();
    //   exVal.mark();
    retVal.mark();
    //   if (con)
    //     con->mark();
}

void QSEngineImp::init()
{
    errType = 0;
    errLines.clear();
    errMsgs.clear();
    retVal.invalidate();
#ifdef QSDEBUGGER
    if( dbg )
	dbg->clear();
#endif

    if( en )
	en->setExecutionMode( QSEnv::Normal );

    if (!initialized) {
	en = new QSEnv( scr );
	glob = new Global( scr );
	glob->init();
	//     con = new Context();
	firstN = 0L;
	recursion = 0;
	initialized = true;
#ifdef QSDEBUGGER
	sid = -1;
#endif
    }


    if (scr->timeoutTrigger()) {
	scr->timeoutTrigger()->start();
    }
}

void QSEngineImp::clear()
{
    if (initialized) {
	// To correctly free references.
	if( retVal.isValid() )
	    retVal.objectType()->deref( &retVal );
	retVal.invalidate();

	en->clearScopeChain();
	delete glob; glob = 0;
	delete en; en = 0;
	//     delete con; con = 0L;

#ifdef QSDEBUGGER
	sid = -1;
#endif

	initialized = false;
    }
}


#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
#define QS_MUTEX_LOCK if (mutex) mutex->lock()
#define QS_MUTEX_UNLOCK if (mutex) mutex->unlock()
#else
#define QS_MUTEX_LOCK
#define QS_MUTEX_UNLOCK
#endif

bool QSEngineImp::evaluate( const QString &code, const QSObject *thisV,
			    bool onlyCheckSyntax, int checkMode, int lineZero )
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    QMutex *mutex = qt_global_mutexpool ? qt_global_mutexpool->get(qApp) : 0;
#endif
    QS_MUTEX_LOCK;
    QSProgramNode *progNode = 0;

    init();

#ifdef QSDEBUGGER
    incrSourceId();
    if (debugger())
	debugger()->setSourceId(sid);
#endif
    if (recursion > 7) {
	qWarning( "QtScript: breaking out of recursion" );
        QS_MUTEX_UNLOCK;
	return true;
    }

    assert(QSLexer::lexer());
    {
    QSLexer::lexer()->setCode( code,
#ifdef QSDEBUGGER
			       sourceId(),
#else
			       0,
#endif
			       lineZero );
    int parseError = qsyyparse();
    progNode = QSProgramNode::last();

    if( parseError || QSLexer::lexer()->lexerState() == QSLexer::Bad ) {
	errType = QSErrParseError;
	int l = QSLexer::lexer()->lineNo();
	errLines.append( l );
	errMsgs.append( QString::fromLatin1("Parse Error: ")
			+ qs_format_error( QSLexer::lexer()->errorMessage() ) );
	/* TODO: either clear everything or keep previously
	   parsed function definitions */
	//    QSNode::deleteAllNodes();
        if (progNode && progNode->deref()) {
            delete progNode;

        }
        QS_MUTEX_UNLOCK;
	return false;
    }
    }

    QSCheckData sem( env(), env()->globalClass() );
    if ( thisV ) {
	Q_ASSERT( thisV->isValid() );
	//       qDebug( "QSEngineImp::evaluate: entering %s",
	// 	      thisV->typeName().latin1());
	sem.enterClass( (QSClass*)thisV->objectType() );
	env()->pushScope( *thisV );
    }
    sem.setGlobalStatementsForbidden( checkMode & QSEngine::DisallowGlobal );
    progNode->check( &sem );
    if ( sem.hasError() ) {
	errType = sem.errorCode();
	errLines = sem.errorLines();
	errMsgs = sem.errorMessages();
	if (progNode->deref())
	    delete progNode;
        QS_MUTEX_UNLOCK;
	return FALSE;
    }

    if (onlyCheckSyntax) {
	if (progNode->deref()) {
	    delete progNode;
	}
        QS_MUTEX_UNLOCK;
	return true;
    }

    env()->clearException();

    recursion++;
    assert(progNode);

    QS_MUTEX_UNLOCK;
    QSObject res = progNode->execute( env() );
    QS_MUTEX_LOCK;

    recursion--;

    if ( env()->isExceptionMode( )) {
	QSObject err = env()->exception();
	errType = 99; /* TODO */
	errLines.append(QSErrorClass::errorLine(&err));
	errMsgs.append(QSErrorClass::errorName(&err) + QString::fromLatin1(". ") + QSErrorClass::errorMessage(&err));
#ifdef QSDEBUGGER
	if (dbg)
	    dbg->setSourceId(QSErrorClass::errorSourceId(&err));
#endif
	env()->clearException();
    } else {
	errType = 0;
	errLines.clear();
	errMsgs.clear();

	// catch return value
	retVal = res;
    }

    if ( thisV ) {
	env()->popScope();
    }

    if (progNode->deref()) {
	delete progNode;
    }

    QS_MUTEX_UNLOCK;
    return !errType;
}

bool QSEngineImp::call( QSObject *scope, const QString &func,
			const QSList &args )
{
    init();
    QSObject t;
    if ( !scope || !scope->isValid() ) {
	t = env()->globalObject();
	scope = &t;
    }

    QSObject v = scope->getQualified(func);
    Q_ASSERT(v.isValid());

    if ( !v.isDefined() ) {
	if (func != QString::fromLatin1("main")) {
	    errType = ReferenceError;
	    errMsgs.append( QString::fromLatin1("Unknown function: ") + func );
	    errLines.append( 0 );
	}
	return false;
    }

    if ( !v.isFunction() ) {
	errType = TypeError;
	errMsgs.append( func + QString::fromLatin1(" is not a function. Call failed.") );
	errLines.append( 0 );
	return false;
    }
    QSObject res = v.invoke( QSMember(), args );

    if ( env()->isExceptionMode() ) {
	QSObject err = env()->exception();
	errType = 99; /* TODO */
	errLines.append(QSErrorClass::errorLine(&err));
	errMsgs.append(QSErrorClass::errorName(&err) + QString::fromLatin1(". ") + QSErrorClass::errorMessage(&err));
#ifdef QSDEBUGGER
	if (dbg)
	    dbg->setSourceId(QSErrorClass::errorSourceId(&err));
#endif
	env()->clearException();
	return false;
    } else {
	errType = 0;
	errLines.clear();
	errMsgs.clear();

	// return value
	retVal = res;
	return true;
    }
}

#ifdef QSDEBUGGER
void QSEngineImp::attachDebugger(Debugger *d)
{
    Debugger *debugger = dbg;
    dbg = 0;
    if (debugger) {
	debugger->detach();
    }
    dbg = d;
}

bool QSEngineImp::setBreakpoint(int /*id*/, int /*line*/, bool /*set*/)
{
    Q_ASSERT(!"Unsupported action...");
    init();
//     return QSNode::setBreakpoint( /* firstNode() */ 0, id, line, set );
    return false;
}

#endif

bool LabelStack::push(const QString &id)
{
    if (id.isEmpty() || contains(id))
	return false;

    StackElm *newtos = new StackElm;
    newtos->id = id;
    newtos->prev = tos;
    tos = newtos;
    return true;
}

bool LabelStack::contains(const QString &id) const
{
    if (id.isEmpty())
	return true;

    for (StackElm *curr = tos; curr; curr = curr->prev)
	if (curr->id == id)
	    return true;

    return false;
}

void LabelStack::pop()
{
    if (tos) {
	StackElm *prev = tos->prev;
	delete tos;
	tos = prev;
    }
}

LabelStack::~LabelStack()
{
    StackElm *prev;

    while (tos) {
	prev = tos->prev;
	delete tos;
	tos = prev;
    }
}
