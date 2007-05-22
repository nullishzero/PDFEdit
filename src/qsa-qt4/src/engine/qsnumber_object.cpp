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

#include "qsnumber_object.h"
#include "qsoperations.h"
#include "qsclass.h"
#include "qstypes.h"

using namespace QS;

QSNumberClass::QSNumberClass( QSClass *b )
    : QSClass( b, AttributeFinal )
{
}

void QSNumberClass::init()
{
    // static members
    int a = AttributePublic|AttributeStatic;
    addStaticVariableMember( QString::fromLatin1("NaN"), createNumber( NaN ), a );
    addStaticVariableMember( QString::fromLatin1("NEGATIVE_INFINITY"), createNumber( -Inf ), a );
    addStaticVariableMember( QString::fromLatin1("POSITIVE_INFINITY"), createNumber( +Inf ), a );
    addStaticVariableMember( QString::fromLatin1("MAX_VALUE"), createNumber(1.7976931348623158e+308), a);
    addStaticVariableMember( QString::fromLatin1("MIN_VALUE"), createNumber(2.2250738585072014e-308), a);

    // member functions
    addMember( QString::fromLatin1("toString"), QSMember( &QSNumberClass::toStringScript ) );
//     addMember( "toLocaleString", QSMember( &QSNumberClass::toStringScript ) ); // ###
    addMember( QString::fromLatin1("valueOf"), QSMember( &valueOf ) );
}

bool QSNumberClass::toBoolean( const QSObject *obj ) const
{
    return !(( obj->dVal() == 0) || qsaIsNan( obj->dVal() ));
}

double QSNumberClass::toNumber( const QSObject *obj ) const
{
    return obj->dVal();
}

QString QSNumberClass::toString( const QSObject *obj ) const
{
    return QSString::from( obj->dVal() );
}

QSObject QSNumberClass::toPrimitive( const QSObject *obj,
				     const QSClass * ) const
{
    return *obj;
}

QVariant QSNumberClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return obj->dVal();
}

QSObject QSNumberClass::construct( const QSList &args ) const
{
    double n = args.isEmpty() ? 0.0 : args[0].toNumber();

    return createNumber( n );
}

QSObject QSNumberClass::cast( const QSList &args ) const
{
    return construct( args );
}

QSObject QSNumberClass::toStringScript ( QSEnv *env )
{
    return env->createString( QSString::from( env->thisValue().dVal() ) );
}

QSObject QSNumberClass::valueOf( QSEnv *env )
{
    return env->createNumber( env->thisValue().dVal() );
}

QSEqualsResult QSNumberClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    Q_ASSERT( a.isA( this ) );
    if ( b.isNumber() ) {
	double n1 = a.dVal();
	double n2 = b.dVal();
	if ( qsaIsNan( n1 ) || qsaIsNan( n2 ) )
	    return EqualsNotEqual;
	else
	    return ( QSEqualsResult ) ( n1 == n2 );
	// ### compare -0 agains +0
    } else if ( b.isString() ) {
	return ( QSEqualsResult ) ( a.dVal() == b.toNumber() );
    } else if ( !b.isPrimitive() ) {
	return isEqual( a, b.toPrimitive() );
    } else {
	return EqualsUndefined;
    }
}

