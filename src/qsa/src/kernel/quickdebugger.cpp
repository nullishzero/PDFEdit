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

#include "quickdebugger.h"

#include <qregexp.h>

QuickDebugger::QuickDebugger( QSEngine *e )
    : Debugger( e )
{
    hadError = FALSE;
    exceptionScope = 0;
}

QuickDebugger::~QuickDebugger()
{
    delete exceptionScope;
}

int QuickDebugger::freeSourceId()
{
    return Debugger::freeSourceId();
}

/*!
 * Returns TRUE if \a var could be found in the current context, FALSE
 * otherwise. On success \a value and \a type are set to the respective
 * values.
 */

bool QuickDebugger::watch( const QString &var,
				 QString &type, QString &value )
{
    QString simple = var;
    simple.replace( QRegExp( QString::fromLatin1("\\[") ), QString::fromLatin1(".") );
    simple.replace( QRegExp( QString::fromLatin1("\\]") ), QString::fromLatin1("") );
    QString info = varInfo(simple);
    if ( info.isNull() ) {
	value = type = QString::null;
	return FALSE;
    }
    int eq = info.find( '=' );
    int sep = info.findRev( ':' );
    value = info.mid( eq+1, sep-eq-1 );
    type = info.mid( sep+1 );
    return TRUE;
}

/*!
 * Set \a var to \val. Returns TRUE on success, FALSE if \var couldn't
 * be found in the current context, isn't writable or couldn't be
 * parse as s simple data type. Those types are null, undefined, a boolean,
 * a string or number.
 */

bool QuickDebugger::setVariable( const QString &var, const QString &val )
{
    QString simple = var;
    simple.replace( QRegExp( QString::fromLatin1("\\[") ), QString::fromLatin1(".") );
    simple.replace( QRegExp( QString::fromLatin1("\\]") ), QString::fromLatin1("") );
    QString v = val.lower();
    QSObject value;
    bool ok;
    double d;
    if ( v == QString::fromLatin1("null") )
	value = env()->createNull();
    else if ( v == QString::fromLatin1("undefined") )
	value = env()->createUndefined();
    else if ( v == QString::fromLatin1("true") )
	value = env()->createBoolean( TRUE );
    else if ( v == QString::fromLatin1("false") )
	value = env()->createBoolean( FALSE );
    else if ( d = val.toDouble( &ok ), ok )
	value = env()->createNumber( d );
    else {
	v = val.stripWhiteSpace();
	QChar c0 = v[ 0 ];
	if ( ( c0 == '"' || c0 == '\'' ) && v[ (int)v.length()-1 ] == c0 )
	    value = env()->createString( v.mid( 1, v.length()-2 ) );
	else
	    return FALSE;

    }

    return setVar( simple, value );
}

void QuickDebugger::clearAllBreakpoints( int i )
{
    Debugger::clearAllBreakpoints( i );
}

bool QuickDebugger::stopEvent()
{
    bool ret = TRUE;
    emit stopped( ret );
    return ret;
}

void QuickDebugger::callEvent( const QString &fn, const QString & )
{
    if ( cStack.count() > 0 ) {
	if ( cStack[ 0 ].line == -1 ) {
	    cStack[ 0 ].line = lineNumber();
	    cStack[ 0 ].sourceId = sourceId();
	}
    }
    if ( fn == QString::fromLatin1("(internal)") ) {
	tmpCStack.push( FALSE );
	return;
    }
    tmpCStack.push( TRUE );
    QuickDebuggerStackFrame sf( QString().sprintf( "%s", fn.latin1() ) );

    cStack.prepend( sf );
    emit stackChanged( cStack.count() );
}

void QuickDebugger::returnEvent()
{
    if ( tmpCStack.pop() ) {
	cStack.remove( cStack.begin() );
	emit stackChanged( cStack.count() );
    }
}

QValueList<QuickDebuggerStackFrame> QuickDebugger::backtrace()
{
    if( hadError )
	return exceptionStack;
    if ( cStack.count() > 1 ) {
	QuickDebuggerStackFrame &sf = cStack.first();
	sf.line = lineNumber();
	sf.sourceId = sourceId();
    }
    return cStack;
}

void QuickDebugger::storeExceptionStack()
{
    if( hadError )
	return;

    exceptionStack = cStack;
    if( exceptionScope == 0 )
	exceptionScope = new QValueList<QSObject>();
    *exceptionScope = env()->scope();
    hadError = TRUE;
    if( exceptionStack.count() == 0 ) {
	QuickDebuggerStackFrame sf( QString::fromLatin1("No function"), lineNumber(), sourceId() );
	exceptionStack.append( sf );
    } else {
	QuickDebuggerStackFrame &sf = exceptionStack.first();
	sf.line = lineNumber();
	sf.sourceId = sourceId();
    }
}

QString QuickDebugger::varInfo( const QString &ident )
{
    if( !hadError )
	return Debugger::varInfo( ident );
    QSEnv *e = env();
    e->pushScopeBlock();
    for( int i=exceptionScope->count()-1; i>=0; i-- ) {
	e->pushScope( (*exceptionScope)[i] );
    }
    QString ret = Debugger::varInfo( ident );
    e->popScopeBlock();
    return ret;
}

void QuickDebugger::clear()
{
    hadError = FALSE;
    delete exceptionScope;
    exceptionScope = 0;
    exceptionStack.clear();
}
