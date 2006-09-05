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

#include "quickpixmapobject.h"
#include "quickbytearrayobject.h"
#include "quickinterpreter.h"
#include <qsfunction.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qbuffer.h>
#include <assert.h>
#include <quickcoordobjects.h>
#include <quickcolorobject.h>
#include <qimage.h>
#include <qfileinfo.h>

class PixmapShared : public QSShared {
public:
    PixmapShared( const QPixmap &p ) : pixmap( p ) { }
    QPixmap pixmap;
};

enum { Width, Height, Rect, Size, Depth };

QSPixmapClass::QSPixmapClass( QSClass *b, QuickInterpreter *i )
    : QSSharedClass( b ), QuickEnvClass( i )
{
    addMember( QString::fromLatin1("width"), QSMember( QSMember::Custom, Width,
				  AttributeNonWritable ) );
    addMember( QString::fromLatin1("height"), QSMember( QSMember::Custom, Height,
				   AttributeNonWritable ) );
    addMember( QString::fromLatin1("rect"), QSMember( QSMember::Custom, Rect,
				 AttributeNonWritable ) );
    addMember( QString::fromLatin1("size"), QSMember( QSMember::Custom, Size,
				 AttributeNonWritable ) );
    addMember( QString::fromLatin1("depth"), QSMember( QSMember::Custom, Depth,
				 AttributeNonWritable ) );
    addMember( QString::fromLatin1("isNull"), QSMember( &isNull ) );
    addMember( QString::fromLatin1("fill"), QSMember( &fill ) );
    addMember( QString::fromLatin1("resize"), QSMember( &resize ) );
    addMember( QString::fromLatin1("load"), QSMember( &load ) );
    addMember( QString::fromLatin1("save"), QSMember( &save ) );
}

QPixmap *QSPixmapClass::pixmap( const QSObject *obj ) const
{
    assert( obj->objectType() == this );
    return &((PixmapShared*)obj->shVal())->pixmap;
}

QSObject QSPixmapClass::fetchValue( const QSObject *obj,
				    const QSMember &mem ) const
{
    if ( mem.type() == QSMember::Custom ) {
	switch ( mem.index() ) {
	case Width:
	    return createNumber( pixmap( obj )->width() );
	case Height:
	    return createNumber( pixmap( obj )->height() );
	case Rect:
	    return rectClass()->construct( pixmap( obj )->rect() );
	case Size:
	    return sizeClass()->construct( pixmap( obj )->size() );
	case Depth:
	    return createNumber( pixmap( obj )->depth() );
	default:
	    qWarning( "QSPixmapClass::fetchValue: unhandled case" );
	    return createUndefined();
	}
    } else {
	return QSClass::fetchValue( obj, mem );
    }
}

QVariant QSPixmapClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return *pixmap( obj );
}

QString QSPixmapClass::debugString( const QSObject * ) const
{
    return QString::fromLatin1("\"...\":Pixmap"); // ###
}

QSObject QSPixmapClass::construct( const QSList &args ) const
{
    if ( args.size() > 0 ) {
	QSObject v( args[ 0 ] );
	if ( v.isA( "Pixmap" ) ) {
	    return v;
	} else if ( v.isString() ) {
	    QPixmap pm( v.toString() );
	    return construct( pm );
	}
    }
    return construct( QPixmap() );
}

QSObject QSPixmapClass::construct( const QPixmap &p ) const
{
    return QSObject( this, new PixmapShared( p ) );
}

QSObject QSPixmapClass::isNull( QSEnv *env )
{
    QSObject t = env->thisValue();
    QSPixmapClass *pac = (QSPixmapClass*)t.objectType();

    QSObject result;
    QPixmap *pix = pac->pixmap( &t );
    return env->createBoolean( pix->isNull() );
}

void QSPixmapClass::fill( QSEnv *env )
{
    if ( env->numArgs() != 1 ) {
	env->throwError( QString::fromLatin1( "Pixmap.fill() called with %1 arguments. 1 arguments expected." ).
			 arg( env->numArgs() ) );
	return;
    }

    QSObject a0 = env->arg( 0 );
    if ( !a0.isA( "Color" ) ) {
	env->throwError( QString::fromLatin1( "Pixmap.fill() called with an argument of type %1. "
				  "Type Color is expeced" ).
			 arg( a0.typeName() ) );
	return;
    }

    QSObject t = env->thisValue();
    QSPixmapClass *pac = (QSPixmapClass*)t.objectType();
    QPixmap *pix = pac->pixmap( &t );
    pix->fill( *( (QSColorClass*)a0.objectType() )->color( &a0 ) );
    return;
}

void QSPixmapClass::resize( QSEnv *env )
{
    if ( env->numArgs() < 1 || env->numArgs() > 2 ) {
	env->throwError( QString::fromLatin1( "Pixmap.resize() called with %1 arguments. 1 or 2 arguments expected." ).
			 arg( env->numArgs() ) );
	return;
    }

    QSObject t = env->thisValue();
    QSPixmapClass *pac = (QSPixmapClass*)t.objectType();
    QPixmap *pix = pac->pixmap( &t );
    if ( env->numArgs() == 1 ) {
	QSObject a0 = env->arg( 0 );
	if ( !a0.isA( "Size" ) ) {
	    env->throwError( QString::fromLatin1( "Pixmap.resize() called with an argument of type %1. "
				  "Type Size is expeced" ).
			     arg( a0.typeName() ) );
	    return;
	}
	pix->resize( *( (QSSizeClass*)a0.objectType() )->size( &a0 ) );
    } else {
	if ( !env->arg( 0 ).isA( "Number" ) || !env->arg( 1 ).isA( "Number" ) ) {
	    env->throwError( QString::fromLatin1( "Pixmap.resize() called with arguments of type %1 and %2. "
				  "Type Number and Number is expeced" ).
			     arg( env->arg( 0 ).typeName() ).arg( env->arg( 1 ).typeName() ) );
	    return;
	}
	pix->resize( env->arg( 0 ).toInteger(), env->arg( 1 ).toInteger() );
    }

    return;
}

void QSPixmapClass::load( QSEnv *env )
{
    if ( env->numArgs() != 1 ) {
	env->throwError( QString::fromLatin1( "Pixmap.load() called with %1 arguments. 1 argument expected." ).
			 arg( env->numArgs() ) );
	return;
    }

    if ( !env->arg( 0 ).isString() ) {
	env->throwError( QString::fromLatin1( "Pixmap.load() called with an argument of type %1. "
				  "Type String is expeced" ).
			 arg( env->arg( 0 ).typeName() ) );
	return;
    }

    QSObject t = env->thisValue();
    QSPixmapClass *pac = (QSPixmapClass*)t.objectType();
    QPixmap *pix = pac->pixmap( &t );
    pix->load( env->arg( 0 ).toString() );

    return;
}

void QSPixmapClass::save( QSEnv *env )
{
    if ( env->numArgs() < 1 || env->numArgs() > 2 ) {
	env->throwError( QString::fromLatin1( "Pixmap.save() called with %1 arguments. 1 or 2 argument expected." ).
			 arg( env->numArgs() ) );
	return;
    }

    QSObject t = env->thisValue();
    QSPixmapClass *pac = (QSPixmapClass*)t.objectType();
    QPixmap *pix = pac->pixmap( &t );
    if ( !env->arg( 0 ).isString() ) {
	env->throwError( QString::fromLatin1( "Pixmap.save() called with an argument of type %1. "
				  "Type String is expeced" ).
			 arg( env->arg( 0 ).typeName() ) );
	return;
    }

    QString format = QFileInfo( env->arg( 0 ).toString() ).extension().upper();
    QStrList l = QImageIO::outputFormats();
    if ( l.find( format.latin1() ) == -1 )
	format = QString::fromLatin1("PNG");

    if ( env->numArgs() == 2 ) {
	if ( !env->arg( 1 ).isString() ) {
	    env->throwError( QString::fromLatin1( "Pixmap.save() called with an argument of type %1. "
				      "as second argument. Type String is expeced" ).
			 arg( env->arg( 1 ).typeName() ) );
	    return;
	}
	format = env->arg( 1 ).toString();
    }

    pix->save( env->arg( 0 ).toString(), format.latin1() );
}
