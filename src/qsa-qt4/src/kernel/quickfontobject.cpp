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

#include "quickfontobject.h"
#include "qstypes.h"
#include <qfont.h>
#include <assert.h>

class FontShared : public QSShared {
public:
    FontShared( const QFont &f ) : font( f ) { }
    QFont font;
};

enum FontMemberType {
    FMT_Family,
    FMT_PointSize,
    FMT_PixelSize,
    FMT_Weight,
    FMT_Bold,
    FMT_Italic,
    FMT_Underline,
    FMT_StrikeOut,
    FMT_FixedPitch
};

QSFontClass::QSFontClass( QSClass *b )
  : QSSharedClass( b )
{
    int a = AttributeNone;
    addMember( QString::fromLatin1("family"), QSMember( QSMember::Custom, FMT_Family, a ) );
    addMember( QString::fromLatin1("pointSize"), QSMember( QSMember::Custom, FMT_PointSize, a ) );
    addMember( QString::fromLatin1("pixelSize"), QSMember( QSMember::Custom, FMT_PixelSize, a ) );
    addMember( QString::fromLatin1("weight"), QSMember( QSMember::Custom, FMT_Weight, a ) );
    addMember( QString::fromLatin1("bold"), QSMember( QSMember::Custom, FMT_Bold, a ) );
    addMember( QString::fromLatin1("italic"), QSMember( QSMember::Custom, FMT_Italic, a ) );
    addMember( QString::fromLatin1("underline"), QSMember( QSMember::Custom, FMT_Underline, a ) );
    addMember( QString::fromLatin1("strikeout"), QSMember( QSMember::Custom, FMT_StrikeOut, a ) );
    addMember( QString::fromLatin1("fixedPitch"), QSMember( QSMember::Custom, FMT_FixedPitch,
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
	case FMT_Family:
	    return createString( fon->family() );
	case FMT_PointSize:
	    return createNumber( fon->pointSizeF() );
	case FMT_PixelSize:
	    return createNumber( fon->pixelSize() );
	case FMT_Weight:
	    return createNumber( fon->weight() );
	case FMT_Bold:
	    return createBoolean( fon->bold() );
	case FMT_Italic:
	    return createBoolean( fon->italic() );
	case FMT_Underline:
	    return createBoolean( fon->underline() );
	case FMT_StrikeOut:
	    return createBoolean( fon->strikeOut() );
	case FMT_FixedPitch:
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
	case FMT_Family:
	    fon->setFamily( v.toString() );
	    break;
	case FMT_PointSize:
	    fon->setPointSizeF( v.toInteger() );
	    break;
	case FMT_PixelSize:
	    fon->setPixelSize( v.toInteger() );
	    break;
	case FMT_Weight:
	    fon->setWeight( v.toInteger() );
	    break;
	case FMT_Bold:
	    fon->setBold( v.toBoolean() );
	    break;
	case FMT_Italic:
	    fon->setItalic( v.toBoolean() );
	    break;
	case FMT_Underline:
	    fon->setUnderline( v.toBoolean() );
	    break;
	case FMT_StrikeOut:
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
