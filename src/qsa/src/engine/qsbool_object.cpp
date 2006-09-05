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

#include "qsbool_object.h"
#include "qsclass.h"
#include "qsoperations.h"
#include "qstypes.h"

// Boolean.toString()
static QSObject bToString( QSEnv *env  )
{
    return env->createString( env->thisValue().toString() );
}

// Boolean.valueOf()
static QSObject bValueOf( QSEnv *env )
{
    return env->thisValue();
}

QSBooleanClass::QSBooleanClass( QSClass *b )
    : QSClass( b )
{
}

void QSBooleanClass::init()
{
    addMember( QString::fromLatin1("toString"), QSMember( bToString ) );
    addMember( QString::fromLatin1("valueOf"), QSMember( bValueOf ) );
}

bool QSBooleanClass::toBoolean( const QSObject *obj ) const
{
    return obj->bVal();
}

double QSBooleanClass::toNumber( const QSObject *obj ) const
{
    return obj->bVal() ? 1.0 : 0.0;
}

QSObject QSBooleanClass::toPrimitive( const QSObject *obj,
				      const QSClass * ) const
{
    return *obj;
}

QVariant QSBooleanClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return QVariant( obj->bVal(), 0 );
}

/*!
  \reimp
*/
QSObject QSBooleanClass::cast( const QSList &args ) const
{
    return construct( args );
}

QString QSBooleanClass::toString( const QSObject *obj ) const
{
    return obj->bVal() ? QString::fromLatin1("true") : QString::fromLatin1("false");
}

QSObject QSBooleanClass::construct( bool b ) const
{
    return createBoolean( b );
}

QSObject QSBooleanClass::construct( const QSList &args ) const
{
    bool b = args.isEmpty() ? FALSE : args.begin()->toBoolean();

    return createBoolean( b );
}

QSEqualsResult QSBooleanClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    Q_ASSERT( a.isA( this ) );
    if ( b.isBoolean() )
	return ( QSEqualsResult ) ( a.bVal() == b.bVal() );
    else
	return  env()->numberClass()->isEqual( createNumber( int( a.bVal() ) ), b );
}
