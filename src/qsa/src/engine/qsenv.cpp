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

#include "qsenv.h"
#include "qsclass.h"
#include "qsinternal.h"
#include "qsobject_object.h"
#include "qsfuncref.h"
#include "qsdate_object.h"
#include "qsmath_object.h"
#include "qsregexp_object.h"
#include "qsarray_object.h"
#include "qserror_object.h"
#ifdef QSDEBUGGER
#include "qsdebugger.h"
#endif

QSEnv::QSEnv( QSEngine *e )
    : eng( e )
{
    init();
}

QSEnv::~QSEnv()
{
    clear();
}

void QSEnv::init()
{
    stackDep = 0;
    shutDown = FALSE;
#ifndef QS_LEAK
    classList.setAutoDelete( TRUE );
#endif
    scopeChain = new ScopeChain();
    args = 0;

    sharedList = new QSShared();

    objClass = new QSObjectClass( this );
    undefClass = new QSUndefinedClass( objClass );
    nilClass = new QSNullClass( objClass );
    boolClass = new QSBooleanClass( objClass );
    numClass = new QSNumberClass( objClass );
    strClass = new QSStringClass( objClass );
    charClass = new QSCharacterClass( strClass );
    typClass = new QSTypeClass( objClass );

    datClass = new QSDateClass( objClass );
    matClass = new QSMathClass( objClass );
    regClass = new QSRegExpClass( objClass );
    arrClass = new QSArrayClass( objClass );
    errClass = new QSErrorClass( objClass );
    refClass = new QSFuncRefClass( objClass );

    globClass = new QSGlobalClass( objClass );
    argsClass = new QSArgumentsClass( objClass );
    dynClass = new QSDynamicClass( objClass );
    dbgClass = new QSDebugClass( objClass );
    sysClass = new QSSystemClass( objClass );

    objClass->init();
    boolClass->init();
    numClass->init();
    strClass->init();

    thVal = createUndefined();

    execMode = Normal;
    setCurrentLabel( QString::null );
}

#if QS_DEBUG_MEM == 1
bool qs_finalDeletion = FALSE;
#endif

void QSEnv::clear()
{
    shutDown = TRUE;

    Q_ASSERT( scopeChain ); // protection against consecutive calls
    delete scopeChain;
    scopeChain = 0;

    // lose class references
    thVal.invalidate();
    exVal.invalidate();
    retVal.invalidate();

    QPtrListIterator<QSClass> it( classList );
    QSClass *cur;
    while( (cur=it())!=0 )
 	cur->finalize();

    // invalidate & delete QSShared that might still hang
    // around due to cyclic references
    QSShared *sh = sharedList;
    while( ( sh = sh->next ) )
	sh->invalidate();

#if QS_DEBUG_MEM == 1
    // flag check for in ~QSObject()
    qs_finalDeletion = TRUE;
#endif
    sh = sharedList->next;
    while( sh ) {
	QSShared *next = sh->next;
	delete sh;
	sh = next;
    }
    delete sharedList;
#if QS_DEBUG_MEM == 1
    qs_finalDeletion = FALSE;
#endif

    it = QPtrListIterator<QSClass>(classList);
    while( (cur=it())!=0 ) {
	cur->clear();
    }

    classList.clear();

    objClass = 0;
    undefClass = 0;
    nilClass = 0;
    boolClass = 0;
    numClass = 0;
    strClass = 0;
    charClass = 0;
    typClass = 0;
    datClass = 0;
    matClass = 0;
    regClass = 0;
    arrClass = 0;
    errClass = 0;
    globClass = 0;
    argsClass = 0;
    dynClass = 0;
    refClass = 0;
    dbgClass = 0;
    sysClass = 0;
    shutDown = FALSE;
}


void QSEnv::unregisterClass(QSClass *cl)
{
    int pos = classList.findRef(cl);
    Q_ASSERT(pos >= 0);
    classList.take(pos);
}

void QSEnv::registerClass( QSClass * cl )
{
    Q_ASSERT(classList.findRef(cl) < 0);
    classList.append( cl );
}

/*!
  Return a pointer to the QSClass object named \a n. Returns 0 if no
  such class could be found.
*/

QSClass* QSEnv::classByName( const QString &n )
{
    QSClass *t = 0;

    // ### use an expandable dict
    if ( n == QString::fromLatin1("Null") )
	t = nullClass();
    else if ( n == QString::fromLatin1("Number") )
	t = numberClass();
    else if ( n == QString::fromLatin1("Boolean") )
	t = booleanClass();
    else if ( n == QString::fromLatin1("String") )
	t = stringClass();
    else
	qDebug( "TypeNode::rhs: Ignoring unknown type %s.", n.ascii() );

    return t;
}

QSClass* QSEnv::classByIdentifier( const QString &n )
{
    QPtrListIterator<QSClass> it(classList);
    QSClass *cl;
    while ((cl = it()))
	if (cl->identifier() == n)
	    return cl;
    return 0;
}

void QSEnv::pushScopeBlock()
{
    QSObject obj;
    pushScope( obj );
    labels.push_front( QString::null );
}

void QSEnv::pushScope( const QSObject &s )
{
    scopeChain->push_front( s );
}

void QSEnv::popScope()
{
    //    QSInstanceData * sh = (QSInstanceData*) scopeChain->front().shVal();
    scopeChain->pop_front();
    //    delete sh;
}

QSObject QSEnv::currentScope() const
{
    Q_ASSERT( !scopeChain->isEmpty() );
    return scopeChain->first();
}

ScopeChain QSEnv::scope() const
{
    ScopeChain chain;
    ScopeChain::const_iterator it = scopeChain->begin();
    while( it!=scopeChain->end() && (*it).isValid() ) {
	chain.push_back( *it );
	it++;
    }
    return chain;
}

/*!
  Pops all the instances on the scope stack until it is empty or until it
  it reaches a block identifier
  \sa pushScopeBlock
*/
void QSEnv::popScopeBlock() {
    while( !scopeChain->isEmpty() && scopeChain->first().isValid() )
	popScope();
    popScope();
    Q_ASSERT( labels.front().isNull() );
    labels.pop_front();

}

/*! Removes all elements of the scope chain.
 */

void QSEnv::clearScopeChain()
{
    scopeChain->clear();
}

void QSEnv::printScopeChain() const {
    QStringList lst;
    ScopeChain::const_iterator it = scopeChain->begin();
    while( it!=scopeChain->end() ) {
	QSObject obj = *it;
	QString tmp = !obj.isValid() ? QString::fromLatin1( "#" )
		      : obj.objectType()->identifier();
	lst.append( tmp );
	it++;
    }
    qDebug( "Current scope is:: %s", lst.join( QString::fromLatin1(", ")).latin1() );

}

/*!
  Returns the current this value, or QSUndefined if outside of any class
*/
QSObject QSEnv::thisValue() const
{
    Q_ASSERT( thVal.isValid() );
    return thVal;
}

/*!
  Set the current 'this' value to \a t.
 */
void QSEnv::setThisValue( const QSObject &t )
{
    thVal = t;
}

/*!
  Returns the global object.
*/
QSObject QSEnv::globalObject() const
{
    Q_ASSERT( !scopeChain->isEmpty() );
    return scopeChain->last();
}


/*!  Returns the value of the property \a ident or an invalid QSObject
 if no such property could be found in the current scope chain. The
 return value should therefore always be checked for validity before
 it is used any further.
*/

QSObject QSEnv::resolveValue( const QString &ident ) const
{
    Q_ASSERT( !ident.isEmpty() );
    ScopeChain::const_iterator it = scopeChain->begin();
    QSMember mem;
    int offset;
    while ( it != scopeChain->end() && (*it).isValid() ) {
	offset = 0;
	const QSClass * cl = (*it).resolveMember( ident, &mem, (*it).objectType(), &offset );
	if( cl && mem.type()!=QSMember::Identifier ) {
	    while( offset-- )
		it++;
	    return cl->fetchValue( &(*it), mem );
	}
	it++;
    }

    return QSObject();
}

void QSEnv::pushLabel( const QString &l )
{
    labels.push_front( l );
}

void QSEnv::popLabel()
{
    Q_ASSERT( !labels.isEmpty() );
    Q_ASSERT( !labels.front().isNull() );
    labels.pop_front();
}

bool QSEnv::containsLabel( const QString &label ) const
{
    QStringList::const_iterator it = labels.begin();
    while( it!=labels.end() && !(*it).isNull() ) {
	if( *it==label )
	    return TRUE;
	it++;
    }
    return FALSE;
}

/*!
  Throw an error of type \a e described with \a m. \a l denotes the line
  number of the error unless you don't know. Use the default -1 then.
*/

QSObject QSEnv::throwError( ErrorType e, const QString &m, int l )
{
    QSObject err = errorClass()->construct( e, m, l );

    if ( !isExceptionMode() )
	setException( err );
    setExecutionMode( Throw );
    return err;
}

/*!
  \overload

  Throws a general error described by \a msg.
*/

QSObject QSEnv::throwError( const QString &msg )
{
    return throwError( GeneralError, msg, -1 );
}

void QSEnv::setException( const QSObject &e )
{
  exVal = e;
  exMsg = "Exception"; // not right but we use !isNull() to test
  setExecutionMode( Throw );
}

void QSEnv::setException( const QString &msg )
{
  exMsg = msg;
  setExecutionMode( Throw );
}

QSObject QSEnv::exception()
{
  if ( exMsg.isNull() )
      return createUndefined();
  if ( exVal.isValid() )
      return exVal;
  return throwError( GeneralError, exMsg );
}

void QSEnv::clearException()
{
    setExecutionMode( Normal );
}

QSObject QSEnv::getValueDirect( int index, int level )
{
    QSObject &sclvl = *(scopeChain->at( level ));
    return *( ( (QSInstanceData*) sclvl.shVal() )->value( index ) );
}

void QSEnv::setValueDirect( int index, int level, const QSObject &val )
{
    QSObject &sclvl = *(scopeChain->at( level ));
    ( (QSInstanceData*) sclvl.shVal() )->setValue( index, val );
}

QSObject QSEnv::scopeObjectAt( int level ) const
{
    return *(scopeChain->at( level ) );
}

void QSEnv::setExecutionMode( ExecutionMode mode )
{
    if( mode==Normal ) {
	switch( execMode ) {
	case Normal:
	case ReturnValue:
	    break;
	case Continue:
	case Break:
	    setCurrentLabel( QString::null );
	    break;
	case Throw:
	    exMsg = QString::null;
#ifdef QSDEBUGGER
	    if( eng->debugger() )
		eng->debugger()->storeExceptionStack();
#endif
	    break;
	}
    }
    execMode = mode;
}

bool QSEnv::isCurrentLabelValid() const
{
    if( currentLabel().isNull() )
	return true;
    return labels.contains( currentLabel() );
}

QSObject QSEnv::createShared( const QSClass *cl, QSShared *sh ) const
{
    return QSObject( cl, sh );
}

void QSEnv::registerShared( QSShared *sh ) const
{
    Q_ASSERT( sh );
    Q_ASSERT( sh != sharedList );
    Q_ASSERT( !sh->isConnected() );

#if QS_DEBUG_MEM >= 1
    QSShared *s = sharedList;
    while ( (s = s->next ) )
	if ( s == sh )
	    qFatal( "QSEnv::registerShared: duplicate %p", sh );
#endif

    if (sharedList->next)
	sharedList->next->prev = sh;
    sh->next = sharedList->next;
    sharedList->next = sh;
    sh->prev = sharedList;
#if QS_DEBUG_MEM >= 1
    if ( sh->next && sh->next->prev != sh )
	qFatal( "QSEnv::registerShared::invalid NEXT" );
    if ( sh->prev && sh->prev->next != sh )
	qFatal( "QSEnv::registerShared::invalid PREV" );
#endif
}

void QSEnv::removeShared( QSShared *sh ) const
{
    Q_ASSERT( sh && sh->count==0 );
    Q_ASSERT( sh!=sharedList );
    Q_ASSERT( sh->isConnected() );
#if QS_DEBUG_MEM >= 1
    if ( sh->next && sh->next->prev != sh )
	qFatal( "QSEnv::removeShared::invalid NEXT" );
    if ( sh->prev && sh->prev->next != sh )
	qFatal( "QSEnv::removeShared::invalid PREV" );
#endif

    if( sh->next )
	sh->next->prev = sh->prev;
    if( sh->prev )
	sh->prev->next = sh->next;
    sh->next = 0;
    sh->prev = 0;
}

