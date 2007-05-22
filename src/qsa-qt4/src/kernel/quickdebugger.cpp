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

#include "quickdebugger.h"

#include <QRegExp>
#include <QList>

QuickDebugger::QuickDebugger( QSEngine *e )
    : Debugger( e )
{
    hadError = false;
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
 * Returns true if \a var could be found in the current context, false
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
	return false;
    }
    int eq = info.indexOf( '=' );
    int sep = info.lastIndexOf( ':' );
    value = info.mid( eq+1, sep-eq-1 );
    type = info.mid( sep+1 );
    return true;
}

/*!
 * Set \a var to \val. Returns true on success, false if \var couldn't
 * be found in the current context, isn't writable or couldn't be
 * parse as s simple data type. Those types are null, undefined, a boolean,
 * a string or number.
 */

bool QuickDebugger::setVariable( const QString &var, const QString &val )
{
    QString simple = var;
    simple.replace( QRegExp( QString::fromLatin1("\\[") ), QString::fromLatin1(".") );
    simple.replace( QRegExp( QString::fromLatin1("\\]") ), QString::fromLatin1("") );
    QString v = val.toLower();
    QSObject value;
    bool ok;
    double d;
    if ( v == QString::fromLatin1("null") )
	value = env()->createNull();
    else if ( v == QString::fromLatin1("undefined") )
	value = env()->createUndefined();
    else if ( v == QString::fromLatin1("true") )
	value = env()->createBoolean( true );
    else if ( v == QString::fromLatin1("false") )
	value = env()->createBoolean( false );
    else if ( d = val.toDouble( &ok ), ok )
	value = env()->createNumber( d );
    else {
	v = val.trimmed();
	QChar c0 = v[ 0 ];
	if ( ( c0 == '"' || c0 == '\'' ) && v[ (int)v.length()-1 ] == c0 )
	    value = env()->createString( v.mid( 1, v.length()-2 ) );
	else
	    return false;

    }

    return setVar( simple, value );
}

void QuickDebugger::clearAllBreakpoints( int i )
{
    Debugger::clearAllBreakpoints( i );
}

bool QuickDebugger::stopEvent()
{
    bool ret = true;
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
	tmpCStack.push( false );
	return;
    }
    tmpCStack.push( true );
    QuickDebuggerStackFrame sf(QString().sprintf("%s", fn.toLatin1().constData()));

    cStack.prepend( sf );
    emit stackChanged( cStack.count() );
}

void QuickDebugger::returnEvent()
{
    if ( tmpCStack.pop() ) {
	cStack.erase( cStack.begin() );
	emit stackChanged( cStack.count() );
    }
}

QList<QuickDebuggerStackFrame> QuickDebugger::backtrace()
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
	exceptionScope = new QList<QSObject>();
    *exceptionScope = env()->scope();
    hadError = true;
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
    hadError = false;
    delete exceptionScope;
    exceptionScope = 0;
    exceptionStack.clear();
}
