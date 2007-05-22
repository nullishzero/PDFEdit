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

#include "quickbytearrayobject.h"
#include <qsfunction.h>
#include <QByteArray>
#include <assert.h>

class ByteArrayShared : public QSShared {
public:
    ByteArrayShared( const QByteArray &ba ) : byteArray( ba ) { }
    QByteArray byteArray;
};

enum { Length, Size };

QSByteArrayClass::QSByteArrayClass( QSClass *b )
    : QSSharedClass( b )
{
    // custom members
    addMember( QString::fromLatin1("length"), QSMember( QSMember::Custom, Length,
				   AttributeNonWritable ) );
    addMember( QString::fromLatin1("size"), QSMember( QSMember::Custom, Size,
				 AttributeNonWritable ) );

    // member functions
    addMember( QString::fromLatin1("toString"), QSMember( &QSByteArrayClass::toStringScript ) );
    addMember( QString::fromLatin1("charAt"), QSMember( &QSByteArrayClass::charAt ) );
}

QByteArray *QSByteArrayClass::byteArray( const QSObject *obj ) const
{
    assert( obj->objectType() == this );
    return &((ByteArrayShared*)obj->shVal())->byteArray;
}

QSObject QSByteArrayClass::fetchValue( const QSObject *obj,
				       const QSMember &mem ) const
{
    if ( mem.type() == QSMember::Custom ) {
	switch( mem.index() ) {
	case Length:
	case Size:
	    return createNumber( byteArray( obj )->size() );
	default:
	    qFatal( "QSByteArrayClass::fetchValue: unhandled case" );
	    return createUndefined();
	}
    } else {
	return QSClass::fetchValue( obj, mem );
    }
}

QString QSByteArrayClass::toString( const QSObject *obj ) const
{
    QByteArray *ba = byteArray( obj );
    QString s;
    for ( int i = 0; i < (int)ba->size() ; ++i )
	s += ba->data()[i];
    return s;
}

QVariant QSByteArrayClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return *byteArray( obj );
}

QSObject QSByteArrayClass::construct( const QSList &args ) const
{
    if ( args.size() > 0 ) {
	QSObject v( args[ 0 ] );
	if ( v.isA( this ) ) {
	    QSObject ba = args.at(0);
	    return construct(QByteArray(*byteArray(&ba)));
	} else {
	    QString s = v.toString();
	    return construct( QByteArray(s.toAscii()) );
	}
    }
    return construct( QByteArray() );
}

QSObject QSByteArrayClass::construct( const QByteArray &ba ) const
{
    return QSObject( this, new ByteArrayShared( ba ) );
}

QSObject QSByteArrayClass::toStringScript( QSEnv *env )
{
    return env->createString( env->thisValue().toString() );
}

QSObject QSByteArrayClass::charAt( QSEnv *env )
{
    QSByteArrayClass *bac = (QSByteArrayClass*)env->thisClass();
    QSObject obj = env->thisValue();
    QByteArray *ba = bac->byteArray( &obj );
    if ( env->numArgs() > 0 ) {
	int pos = env->arg( 0 ).toInteger();
	if ( pos >= 0 && pos < (int)ba->size() )
	    return env->createString( QString( (QChar)ba->data()[ pos ] ) );
    }
    return env->createUndefined();
}
