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

#include "quickfontobject.h"
#include "qstypes.h"
#include <qfont.h>
#include <assert.h>

class FontShared : public QSShared {
public:
    FontShared( const QFont &f ) : font( f ) { }
    QFont font;
};

enum { Family, PointSize, PixelSize, Weight, Bold, Italic, Underline,
       StrikeOut, FixedPitch };

QSFontClass::QSFontClass( QSClass *b )
  : QSSharedClass( b )
{
    int a = AttributeNone;
    addMember( QString::fromLatin1("family"), QSMember( QSMember::Custom, Family, a ) );
    addMember( QString::fromLatin1("pointSize"), QSMember( QSMember::Custom, PointSize, a ) );
    addMember( QString::fromLatin1("pixelSize"), QSMember( QSMember::Custom, PixelSize, a ) );
    addMember( QString::fromLatin1("weight"), QSMember( QSMember::Custom, Weight, a ) );
    addMember( QString::fromLatin1("bold"), QSMember( QSMember::Custom, Bold, a ) );
    addMember( QString::fromLatin1("italic"), QSMember( QSMember::Custom, Italic, a ) );
    addMember( QString::fromLatin1("underline"), QSMember( QSMember::Custom, Underline, a ) );
    addMember( QString::fromLatin1("strikeout"), QSMember( QSMember::Custom, StrikeOut, a ) );
    addMember( QString::fromLatin1("fixedPitch"), QSMember( QSMember::Custom, FixedPitch,
				       AttributeNonWritable ) );
}

QFont *QSFontClass::font( const QSObject *obj ) const
{
    assert( obj->objectType() == this );
    return &((FontShared*)obj->shVal())->font;
}

QSObject QSFontClass::fetchValue( const QSObject *obj,
				  const QSMember &mem ) const
{
    if ( mem.type() == QSMember::Custom ) {
	const QFont *fon = font( obj );
	switch ( mem.index() ) {
	case Family:
	    return createString( fon->family() );
	case PointSize:
	    return createNumber( fon->pointSizeFloat() );
	case PixelSize:
	    return createNumber( fon->pixelSize() );
	case Weight:
	    return createNumber( fon->weight() );
	case Bold:
	    return createBoolean( fon->bold() );
	case Italic:
	    return createBoolean( fon->italic() );
	case Underline:
	    return createBoolean( fon->underline() );
	case StrikeOut:
	    return createBoolean( fon->strikeOut() );
	case FixedPitch:
	    return createBoolean( fon->fixedPitch() );
	default:
	    qFatal( "QSFontClass::fetchValue: unhandled case" );
	    return createUndefined();
	}
    } else {
	return QSClass::fetchValue( obj, mem );
    }
}

void QSFontClass::write( QSObject *obj, const QSMember &mem,
			 const QSObject &v ) const
{
    if ( mem.type() == QSMember::Custom ) {
	QFont *fon = font( obj );
	switch ( mem.index() ) {
	case Family:
	    fon->setFamily( v.toString() );
	    break;
	case PointSize:
	    fon->setPointSizeFloat( v.toInteger() );
	    break;
	case PixelSize:
	    fon->setPixelSize( v.toInteger() );
	    break;
	case Weight:
	    fon->setWeight( v.toInteger() );
	    break;
	case Bold:
	    fon->setBold( v.toBoolean() );
	    break;
	case Italic:
	    fon->setItalic( v.toBoolean() );
	    break;
	case Underline:
	    fon->setUnderline( v.toBoolean() );
	    break;
	case StrikeOut:
	    fon->setStrikeOut( v.toBoolean() );
	    break;
	default:
	    qFatal( "QSFontClass::write: unhandled case" );
	}
    } else {
	QSClass::write( obj, mem, v );
    }
}

QString QSFontClass::toString( const QSObject *obj ) const
{
    return font( obj )->toString();
}

QVariant QSFontClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return *font( obj );
}

QString QSFontClass::debugString( const QSObject *obj ) const
{
    const QFont *fon = font( obj );
    return QString::fromLatin1( "{family=%1:String,pointSize=%2:Number,bold=%3:Boolean,"
		    "italic=%4:Boolean,underline=%5:Boolean}" ).
	arg( fon->family() ).arg( fon->pointSize() ).
	arg( QString::number( (uint)fon->bold() ) ).
	arg( QString::number( (uint)fon->italic() ) ).
	arg( QString::number( (uint)fon->underline() ) );
}

QSObject QSFontClass::construct( const QSList &args ) const
{
    if ( args.size() > 0 ) {
	if ( args.size() == 1 ) {
	    QSObject v( args[ 0 ] );
	    if ( v.isA( "Font" ) ) {
		QSObject ft = args.at(0);
		return construct(QFont(*font(&ft)));
	    } else {
		return construct( QFont( v.toString() ) );
	    }
	}
    }
    return construct( QFont() );
}

QSObject QSFontClass::construct( const QFont &f ) const
{
    return QSObject( this, new FontShared( f ) );
}
