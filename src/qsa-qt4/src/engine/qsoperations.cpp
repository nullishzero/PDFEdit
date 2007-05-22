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

#include "qsoperations.h"
#include "qsobject.h"
#include "qstypes.h"
#include "qsenv.h"
#include "qsnumeric.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

using namespace QS;


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
	if( qsaIsInf( n1 ) && qsaIsInf( n2 ) ) {
	    return env->createNumber( NaN );
	} else if( qsaIsNan( n1 ) || qsaIsNan( n2 ) ) {
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
    if ( qsaIsNan( d ) )
	return QString::fromLatin1("NaN");
    else if( qsaIsInf( d ) )
	return d > 0 ? QString::fromLatin1("+Infinity") : QString::fromLatin1("-Infinity");

    return QString::number( d, 'G', 16 );
}

double QSString::toDouble( const QString &s )
{
    // ### hex, Infinity
    bool ok;
    double d = s.toDouble( &ok );
    if( !ok ) {
	if( s.trimmed().isEmpty() ) {
	    return 0;
	}
	return NaN;
    }
    return d;
}

ulong QSString::toULong( const QString &s, bool *ok )
{
    double d = QSString::toDouble( s );
    bool b = true;

    if ( qsaIsNan(d) || d != ulong(d) ) {
	b = false;
	d = 0;
    }

    if ( ok )
	*ok = b;

    return ulong(d);
}
