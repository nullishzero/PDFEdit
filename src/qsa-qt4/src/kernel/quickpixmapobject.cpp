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

#include "quickpixmapobject.h"
#include "quickbytearrayobject.h"
#include "quickinterpreter.h"
#include <qsfunction.h>
#include <quickcoordobjects.h>
#include <quickcolorobject.h>

#include <assert.h>
#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <QImageWriter>
#include <QStringList>
#include <QImage>
#include <QFileInfo>
#include <qpainter.h>

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

QSObject QSPixmapClass::construct( const QSList &args ) const
{
    if ( args.size() > 0 ) {
	QSObject v( args[ 0 ] );
	if ( v.isA( "Pixmap" ) ) {
	    return v;
	} else if ( v.isString() ) {
	    QPixmap pm( v.toString() );
	    return construct( pm );
        } else {
            QVariant var = v.toVariant(QVariant::ByteArray);
            if (var.type() == QVariant::ByteArray) {
                QPixmap pm;
                pm.loadFromData( v.toVariant(QVariant::ByteArray).toByteArray() );
                return construct( pm );
            }
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

    int w=-1;
    int h=-1;
    if ( env->numArgs() == 1 ) {
	QSObject a0 = env->arg( 0 );
	if ( !a0.isA( "Size" ) ) {
	    env->throwError( QString::fromLatin1( "Pixmap.resize() called with an argument of type %1. "
				  "Type Size is expected" ).
			     arg( a0.typeName() ) );
	    return;
	}
    QSize size = *((QSSizeClass*)a0.objectType())->size(&a0);
    w = size.width();
    h = size.height();
    } else {
	if ( !env->arg( 0 ).isA( "Number" ) || !env->arg( 1 ).isA( "Number" ) ) {
	    env->throwError( QString::fromLatin1( "Pixmap.resize() called with arguments of type %1 and %2. "
				  "Type Number and Number is expected" ).
			     arg( env->arg( 0 ).typeName() ).arg( env->arg( 1 ).typeName() ) );
	    return;
	}
    w = env->arg( 0 ).toInteger();
    h = env->arg( 1 ).toInteger();
    }

    if (w >= 0 && h >= 0) {
        QPixmap tmppm(w, h);
        tmppm.fill(Qt::black);
        QPainter p(&tmppm);

        p.drawPixmap(0, 0, *pix);
        *pix = tmppm;
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

    QString format = QFileInfo( env->arg( 0 ).toString() ).completeSuffix().toUpper();
    QList<QByteArray> l = QImageWriter::supportedImageFormats();
    if ( l.indexOf( format.toLatin1() ) == -1 )
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

    pix->save( env->arg( 0 ).toString(), format.toLatin1().constData() );
}
