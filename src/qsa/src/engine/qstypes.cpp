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

#include "qstypes.h"
#include "qsobject.h"
#include "qsinternal.h"
#include "qsoperations.h"
#include "qsnodes.h"

QSUndefined::QSUndefined( const QSEnv *env ) : QSObject( env->undefinedClass() ) { }

QSNull::QSNull( const QSEnv *env ) :
    QSObject( env->nullClass() ) { }

QSBoolean::QSBoolean( const QSEnv *env, bool b )
    : QSObject( env->booleanClass() )
{
    setVal( b );
}

bool QSBoolean::value() const
{
    return bVal();
}

QSNumber::QSNumber( const QSEnv *env, double value )
    : QSObject( env->numberClass() )
{
    setVal( value );
}

double QSNumber::value() const
{
    return dVal();
}

int QSNumber::intValue() const
{
    return (int)dVal();
}

bool QSNumber::isNaN() const
{
    return QS::isNaN( dVal() );
}

bool QSNumber::isInf() const
{
    return QS::isInf( dVal() );
}

QSString::QSString( const QSEnv *env, const QString &s )
    : QSObject( env->stringClass() )
{
    setVal( s );
}

QString QSString::value() const
{
    return sVal();
}

QSListIterator::QSListIterator( const QSList &l )
    : node( l.hook->next )
{
}

QSList::QSList()
{
#ifdef QSDEBUG_MEM
    count++;
#endif
    init();
}

QSList::QSList( const QSObject &first )
{
#ifdef QSDEBUG_MEM
    count++;
#endif
    init();
    append( first );
}

void QSList::init()
{
    hook = new ListNode( QSObject(), 0L, 0L );
    hook->next = hook;
    hook->prev = hook;
}

QSList::~QSList()
{
#ifdef QSDEBUG_MEM
    count--;
#endif

    clear();
    delete hook;
}

void QSList::append( const QSObject& obj )
{
    ListNode *n = new ListNode( obj, hook->prev, hook );
    hook->prev->next = n;
    hook->prev = n;
}

void QSList::prepend( const QSObject& obj )
{
    ListNode *n = new ListNode( obj, hook, hook->next );
    hook->next->prev = n;
    hook->next = n;
}

void QSList::removeFirst()
{
    erase( hook->next );
}

void QSList::removeLast()
{
    erase( hook->prev );
}

void QSList::remove( const QSObject &obj )
{
    if ( !obj.isValid() )
	return;
#if 0 // ###
    ListNode *n = hook->next;
    while ( n != hook ) {
	if ( n->member.imp() == obj.imp() ) {
	    erase( n );
	    return;
	}
	n = n->next;
    }
#endif
}

void QSList::clear()
{
    ListNode *n = hook->next;
    while ( n != hook ) {
	n = n->next;
	delete n->prev;
    }

    hook->next = hook;
    hook->prev = hook;
}

QSList *QSList::copy() const
{
    QSList *newList = new QSList();
    QSListIterator e = end();
    QSListIterator it = begin();

    while( it != e ) {
	newList->append( *it );
	++it;
    }

    return newList;
}

void QSList::erase( ListNode *n )
{
    if ( n != hook ) {
	n->next->prev = n->prev;
	n->prev->next = n->next;
	delete n;
    }
}

int QSList::size() const
{
    int s = 0;
    ListNode *node = hook;
    while ( (node = node->next) != hook )
	s++;

    return s;
}

QSObject QSList::at( int i ) const
{
    if ( i < 0 || i >= size() ) {
	Q_ASSERT( 0 );
	return QSObject();
    }

    QSListIterator it = begin();
    int j = 0;
    while ( (j++ < i) )
	it++;

    return *it;
}
