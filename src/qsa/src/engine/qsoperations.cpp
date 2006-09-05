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

#include "qsoperations.h"
#include "qsobject.h"
#include "qstypes.h"
#include "qsenv.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef HAVE_FLOAT_H   /* just for !Windows */
#define HAVE_FLOAT_H 0
#define HAVE_FUNC__FINITE 0
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#ifndef HAVE_FUNC_ISINF
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#endif /* HAVE_FUNC_ISINF */

#ifdef HAVE_FLOAT_H
#include <float.h>
#endif

#if defined (__APPLE__) || defined (__FREEBSD__)
extern "C" {
    int finite( double );
}
#endif

using namespace QS;

bool QS::isNaN( double d )
{
    uchar *ch = (uchar *)&d;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    return (ch[0] & 0x7f) == 0x7f && ch[1] > 0xf0;
#else
    return (ch[7] & 0x7f) == 0x7f && ch[6] > 0xf0;
#endif
}

bool QS::isInf( double d )
{
#if defined(HAVE_FUNC_ISINF)
    return isinf( d );
#elif defined(HAVE_FUNC_FINITE) || defined(__APPLE__)
    return finite( d ) == 0 && d == d;
#elif defined(HAVE_FUNC__FINITE)
    return
#  if defined( Q_CC_BOR ) // Crashes with out this test...
	!isNaN( d ) &&
#  endif
	_finite( d ) == 0 && d == d;
#else
    return FALSE;
#endif
}

double QS::max( double d1, double d2 )
{
    /* TODO: check for NaN */
    return ( d1 > d2 ) ? d1 : d2;
}

double QS::min( double d1, double d2 )
{
    /* TODO: check for NaN */
    return ( d1 < d2 ) ? d1 : d2;
}

// ECMA 11.6
QSObject QS::add( const QSEnv *env, const QSObject &v1, const QSObject &v2, char oper )
{
    QSObject p1 = v1.toPrimitive();
    QSObject p2 = v2.toPrimitive();

    if ( ( p1.isString() || p2.isString() ) && oper == '+' )
	return env->createString( p1.toString() + p2.toString() );

    double n1 = p1.toNumber();
    double n2 = p2.toNumber();

    return env->createNumber( oper == '+' ? n1 + n2 : n1 - n2 );
}

// ECMA 11.5
QSObject QS::mult( const QSEnv *env, const QSObject &v1, const QSObject &v2, char oper )
{
    double n1 = v1.toNumber();
    double n2 = v2.toNumber();

    double result;

    if ( oper == '*' )
	result = n1 * n2;
    else if ( oper == '/' ) {
#if defined( Q_CC_BOR )
	if( isInf( n1 ) && isInf( n2 ) ) {
	    return env->createNumber( NaN );
	} else if( isNaN( n1 ) || isNaN( n2 ) ) {
	    return env->createNumber( NaN );
	} else if( n2 == 0 ) {
	    if( n1 == 0 )
		return env->createNumber( NaN );
	    else if( n1 > 0 )
		return env->createNumber( +Inf );
	    else if( n1 < 0 )
		return env->createNumber( -Inf );
	}
#endif
	result = n1 / n2;
    } else
	result = fmod( n1, n2 );

    return env->createNumber( result );
}

QString QSString::from( double d )
{
    // ### -0
    if ( isNaN( d ) )
	return QString::fromLatin1("NaN");
    else if( isInf( d ) )
	return d > 0 ? QString::fromLatin1("+Infinity") : QString::fromLatin1("-Infinity");

    return QString::number( d, 'G', 16 );
}

double QSString::toDouble( const QString &s )
{
    // ### hex, Infinity
    bool ok;
    double d = s.toDouble( &ok );
    if( !ok ) {
	if( s.stripWhiteSpace().isEmpty() ) {
	    return 0;
	}
	return NaN;
    }
    return d;
}

ulong QSString::toULong( const QString &s, bool *ok )
{
    double d = QSString::toDouble( s );
    bool b = TRUE;

    if ( isNaN(d) || d != ulong(d) ) {
	b = FALSE;
	d = 0;
    }

    if ( ok )
	*ok = b;

    return ulong(d);
}
