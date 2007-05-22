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
    return QVariant( obj->bVal() );
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
    bool b = args.isEmpty() ? false : args.begin()->toBoolean();

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
