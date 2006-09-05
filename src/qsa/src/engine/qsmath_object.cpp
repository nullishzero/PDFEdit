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

#include "qsmath_object.h"
#include "qstypes.h"
#include "qsoperations.h"
#include <math.h>
#include <stdlib.h>

using namespace QS;

QSMathClass::QSMathClass( QSClass *b )
    : QSClass( b, AttributeAbstract )
{
    int a = AttributePublic|AttributeStatic;
    addStaticVariableMember( QString::fromLatin1("E"), createNumber( ::exp( 1.0 ) ), a );
    addStaticVariableMember( QString::fromLatin1("LN2"), createNumber( ::log( 2.0 ) ), a );
    addStaticVariableMember( QString::fromLatin1("LN10"), createNumber( ::log( 10.0 ) ), a );
    addStaticVariableMember( QString::fromLatin1("LOG2E"), createNumber( 1.0/::log( 2.0 ) ), a );
    addStaticVariableMember( QString::fromLatin1("LOG10E"), createNumber( 1.0/::log( 10.0 ) ), a );
    addStaticVariableMember( QString::fromLatin1("PI"), createNumber( 2.0 * ::asin( 1.0 ) ), a );
    addStaticVariableMember( QString::fromLatin1("SQRT1_2"), createNumber( ::sqrt( 0.5 ) ), a );
    addStaticVariableMember( QString::fromLatin1("SQRT2"), createNumber( ::sqrt( 2.0 ) ), a );

    // static functions
    addMember( QString::fromLatin1("abs"), QSMember( &abs, AttributeStatic ) );
    addMember( QString::fromLatin1("acos"), QSMember( &acos, AttributeStatic ) );
    addMember( QString::fromLatin1("asin"), QSMember( &asin, AttributeStatic ) );
    addMember( QString::fromLatin1("atan"), QSMember( &atan, AttributeStatic ) );
    addMember( QString::fromLatin1("atan2"), QSMember( &atan2, AttributeStatic ) );
    addMember( QString::fromLatin1("ceil"), QSMember( &ceil, AttributeStatic ) );
    addMember( QString::fromLatin1("cos"), QSMember( &cos, AttributeStatic ) );
    addMember( QString::fromLatin1("exp"), QSMember( &exp, AttributeStatic ) );
    addMember( QString::fromLatin1("floor"), QSMember( &floor, AttributeStatic ) );
    addMember( QString::fromLatin1("log"), QSMember( &log, AttributeStatic ) );
    addMember( QString::fromLatin1("max"), QSMember( &max, AttributeStatic ) );
    addMember( QString::fromLatin1("min"), QSMember( &min, AttributeStatic ) );
    addMember( QString::fromLatin1("pow"), QSMember( &pow, AttributeStatic ) );
    addMember( QString::fromLatin1("random"), QSMember( &random, AttributeStatic ) );
    addMember( QString::fromLatin1("round"), QSMember( &round, AttributeStatic ) );
    addMember( QString::fromLatin1("sin"), QSMember( &sin, AttributeStatic ) );
    addMember( QString::fromLatin1("sqrt"), QSMember( &sqrt, AttributeStatic ) );
    addMember( QString::fromLatin1("tan"), QSMember( &tan, AttributeStatic ) );
}

QSMathClass::~QSMathClass() {

}

QSObject QSMathClass::abs( QSEnv *env )
{
    double arg = env->arg( 0 ).toNumber();
    return env->createNumber( arg < 0 ? (-arg) : arg );
}

QSObject QSMathClass::acos( QSEnv *env )
{
    return env->createNumber( ::acos( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::asin( QSEnv *env )
{
    return env->createNumber( ::asin( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::atan( QSEnv *env )
{
    return env->createNumber( ::atan( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::atan2( QSEnv *env )
{
    return env->createNumber( ::atan2( env->arg( 0 ).toNumber(), env->arg( 1 ).toNumber() ) );
}

QSObject QSMathClass::ceil( QSEnv *env )
{
    return env->createNumber( ::ceil( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::cos( QSEnv *env )
{
    return env->createNumber( ::cos( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::exp( QSEnv *env )
{
    return env->createNumber( ::exp( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::floor( QSEnv *env )
{
    return env->createNumber( ::floor( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::log( QSEnv *env )
{
    return env->createNumber( ::log( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::max( QSEnv *env )
{
    double arg0 = env->arg( 0 ).toNumber();
    double arg1 = env->arg( 1 ).toNumber();
    return env->createNumber(  QMAX( arg0, arg1 ) );
}

QSObject QSMathClass::min( QSEnv *env )
{
    double arg0 = env->arg( 0 ).toNumber();
    double arg1 = env->arg( 1 ).toNumber();
    return env->createNumber(  QMIN( arg0, arg1 ) );
}

QSObject QSMathClass::pow( QSEnv *env )
{
    double arg0 = env->arg( 0 ).toNumber();
    double arg1 = env->arg( 1 ).toNumber();
    return env->createNumber( ::pow( arg0, arg1 ) );
}

QSObject QSMathClass::random( QSEnv *env )
{
    return env->createNumber( ::rand() / (double) RAND_MAX );
}

QSObject QSMathClass::round( QSEnv *env )
{
    double arg = env->arg( 0 ).toNumber();
    double result;
    if ( isNaN( arg ) )
	result = arg;
    else if ( isInf( arg ) || isInf( -arg ) )
	result = arg;
    else if ( arg == -0.5 )
	result = 0;
    else
	result = qRound( arg );

    return env->createNumber( result );
}

QSObject QSMathClass::sin( QSEnv *env )
{
    return env->createNumber( ::sin( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::sqrt( QSEnv *env )
{
    return env->createNumber( ::sqrt( env->arg( 0 ).toNumber() ) );
}

QSObject QSMathClass::tan( QSEnv *env )
{
    return env->createNumber( ::tan( env->arg( 0 ).toNumber() ) );
}
