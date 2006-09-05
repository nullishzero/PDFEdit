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

#include "quickcolorobject.h"
#include "quickinterpreter.h"

#include <qcolor.h>
#include <qpalette.h>
#include <assert.h>

class ColorShared : public QSShared
{
public:
    ColorShared( const QColor &c ) : color( c ) { }
    QColor color;
};

class PaletteShared : public QSShared
{
public:
    PaletteShared(const QPalette &p) : palette(p) {}
    QPalette palette;
};

class ColorGroupShared : public QSShared
{
public:
    ColorGroupShared(const QColorGroup &cg) : colorGroup(cg) {}
    QColorGroup colorGroup;
};

enum { Red, Green, Blue, Name, Rgb, Light, Dark, Hue, Saturation, Value };

QSColorClass::QSColorClass( QSClass *b )
  : QSSharedClass( b )
{
    // custom properties
    addMember( QString::fromLatin1("red"), QSMember( QSMember::Custom, Red, AttributeNone ) );
    addMember( QString::fromLatin1("green"), QSMember( QSMember::Custom, Green, AttributeNone ) );
    addMember( QString::fromLatin1("blue"), QSMember( QSMember::Custom, Blue, AttributeNone ) );
    addMember( QString::fromLatin1("name"), QSMember( QSMember::Custom, Name, AttributeNone ) );
    addMember( QString::fromLatin1("rgb"), QSMember( QSMember::Custom, Rgb, AttributeNone ) );
    addMember( QString::fromLatin1("hue"), QSMember( QSMember::Custom, Hue, AttributeNone ) );
    addMember( QString::fromLatin1("saturation"), QSMember( QSMember::Custom, Saturation, AttributeNone ) );
    addMember( QString::fromLatin1("value"), QSMember( QSMember::Custom, Value, AttributeNone ) );

    // member functions
    addMember( QString::fromLatin1("light"), QSMember( &light ) );
    addMember( QString::fromLatin1("dark"), QSMember( &dark ) );
    addMember( QString::fromLatin1("setRgb"), QSMember( &setRgb ) );
}

QColor *QSColorClass::color( const QSObject *obj )
{
    Q_ASSERT( obj->objectType()->name() == QString::fromLatin1("Color") );
    return &((ColorShared*)obj->shVal())->color;
}

#if QT_VERSION >= 0x030200
#define QSA_HSV_FUNCTION(a, b, c) getHsv( (a), (b), (c) )
#else
#define QSA_HSV_FUNCTION(a, b, c) hsv( (a), (b), (c) )
#endif

static inline int getHSV(QColor *col, int pos)
{
    int hsv[3];
    col->QSA_HSV_FUNCTION(&hsv[0], &(hsv[1]), &(hsv[2]));
    return hsv[pos];
}

QSObject QSColorClass::fetchValue( const QSObject *objPtr,
				   const QSMember &mem ) const
{
    if ( mem.type() != QSMember::Custom )
	return QSClass::fetchValue( objPtr, mem );
    QColor *col = color( objPtr );
    switch ( mem.idx ) {
    case Red:
	return createNumber( col->red() );
    case Green:
	return createNumber( col->green() );
    case Blue:
	return createNumber( col->blue() );
    case Rgb:
	return createNumber( col->rgb() );
    case Name:
	return createString( col->name() );
    case Hue:
	return createNumber( getHSV(col, 0) );
    case Saturation:
	return createNumber( getHSV(col, 1) );
    case Value:
	return createNumber( getHSV(col, 2) );
    default:
	qFatal( "QSColorClass::fetchValue: unhandled case" );
	return createUndefined();
    }
}

void QSColorClass::write( QSObject *objPtr, const QSMember &mem,
			  const QSObject &val ) const
{
    if ( mem.type() == QSMember::Custom ) {
	QColor *col = color( objPtr );
	if ( mem.idx == Name ) {
	    col->setNamedColor( val.toString() );
	} else if ( mem.idx == Rgb ) {
	    col->setRgb( val.toUInt32() );
	} else if ( mem.idx == Hue ) {
	    int	h, s, v;
	    col->QSA_HSV_FUNCTION(&h, &s, &v);
	    col->setHsv( (int) val.toNumber(), s, v );
	} else if ( mem.idx == Saturation ) {
	    int	h, s, v;
	    col->QSA_HSV_FUNCTION(&h, &s, &v);
	    col->setHsv( h, (int) val.toNumber(), v );
	} else if ( mem.idx == Value ) {
	    int	h, s, v;
	    col->QSA_HSV_FUNCTION(&h, &s, &v);
	    col->setHsv( h, s, (int) val.toNumber() );
	} else {
	    int r = col->red();
	    int g = col->green();
	    int b = col->blue();
	    if ( mem.idx == Red )
		r = val.toInteger();
	    else if ( mem.idx == Green )
		g = val.toInteger();
	    else if ( mem.idx == Blue )
		b = val.toInteger();
	    col->setRgb( r, g, b );
	}
    } else {
	QSClass::fetchValue( objPtr, mem );
    }
}

QString QSColorClass::toString( const QSObject *obj ) const
{
    return color( obj )->name();
}

double QSColorClass::toNumber( const QSObject *obj ) const
{
    return (uint)color(obj)->rgb();
}

QVariant QSColorClass::toVariant( const QSObject *obj, QVariant::Type type ) const
{
    switch (type) {
    case QVariant::Int: return QVariant((int)color(obj)->rgb());
    default: break;
    }
    return *color( obj );
}

QString QSColorClass::debugString( const QSObject *obj ) const
{
    QColor *c = color( obj );
    return QString::fromLatin1( "{name=%1:String,red=%2:Number,green=%3:Number,"
		    "blue=%4:Number}" ).arg( c->name() ).arg( c->red() )
	.arg( c->green() ).arg( c->blue() );
}

QSObject QSColorClass::construct( const QSList &args ) const
{
    if ( args.size() > 0 ) {
	if ( args.size() == 1 ) {
	    QSObject v( args[ 0 ] );
	    if ( v.isA( "Color" ) ) {
		QSObject col = args.at(0);
		return construct(QColor(*color(&col)));
	    } else {
		return construct( QColor( v.toString() ) );
	    }
	} else if ( args.size() == 3 ) {
	    QColor c( args[ 0 ].toInteger(),
		      args[ 1 ].toInteger(),
		      args[ 2 ].toInteger() );
	    return construct( c );
	}
    }

    return construct( QColor() );
}

QSObject QSColorClass::construct( const QColor &c ) const
{
    return QSObject( this, new ColorShared( c ) );
}

void QSColorClass::setRgb( QSEnv *env )
{
    QSObject t = env->thisValue();
    QColor *col = ((QSColorClass*)t.objectType())->color( &t );

    if ( env->numArgs() == 1 ) {
	col->setRgb( env->arg( 0 ).toInteger() );
    } else if ( env->numArgs() == 3 ) {
	col->setRgb( env->arg( 0 ).toInteger(),
		     env->arg( 1 ).toInteger(),
		     env->arg( 2 ).toUInt32() );
    }
}

QSObject QSColorClass::light( QSEnv *env )
{
    QSColorClass *cl = ((QuickInterpreter *)env->engine())->colorClass();
    QSObject that = env->thisValue();
    return cl->construct(color(&that)->light());
}

QSObject QSColorClass::dark( QSEnv *env )
{
    QSColorClass *cl = ((QuickInterpreter *)env->engine())->colorClass();
    QSObject that = env->thisValue();
    return cl->construct(color(&that)->dark());
}

QSPaletteClass::QSPaletteClass(QSClass *b)
    : QSSharedClass(b)
{
    addMember(QString::fromLatin1("active"), QSMember(QSMember::Custom, Active, AttributeNone));
    addMember(QString::fromLatin1("inactive"), QSMember(QSMember::Custom, Inactive, AttributeNone));
    addMember(QString::fromLatin1("disabled"), QSMember(QSMember::Custom, Disabled, AttributeNone));
};

QSObject QSPaletteClass::fetchValue(const QSObject *objPtr, const QSMember &mem) const
{
    if (mem.type() != QSMember::Custom)
	return QSClass::fetchValue(objPtr, mem);
    QSColorGroupClass *cgClass = ((QuickInterpreter*) env()->engine())->colorGroupClass();
    switch (mem.idx) {
    case Active:
	return cgClass->construct(palette(objPtr)->active());
    case Inactive:
	return cgClass->construct(palette(objPtr)->inactive());
    case Disabled:
	return cgClass->construct(palette(objPtr)->disabled());
    default:
	qFatal("QSPaletteClass::fetchValue() :: unhandled case");
    }
    return QSObject();
}

void QSPaletteClass::write(QSObject *objPtr, const QSMember &mem, const QSObject &val) const
{
    if (mem.type() != QSMember::Custom) {
	QSClass::write(objPtr, mem, val);
	return;
    }

    QSColorGroupClass *cgClass = ((QuickInterpreter*) env()->engine())->colorGroupClass();
    if (val.objectType() != cgClass) {
	 env()->throwError(QString::fromLatin1("Palette.%1: Value must be of type ColorGroup")
				 .arg(mem.name()));
	 return;
    }
    QColorGroup *colorGroup = QSColorGroupClass::colorGroup(&val);

    switch (mem.idx) {
    case Active:
	palette(objPtr)->setActive(*colorGroup);
	break;
    case Inactive:
	palette(objPtr)->setInactive(*colorGroup);
	break;
    case Disabled:
	palette(objPtr)->setDisabled(*colorGroup);
	break;
    default:
        break;
    }
}


QSObject QSPaletteClass::construct(const QSList &args) const
{
    if (args.size()<3)
	return env()->throwError(QString::fromLatin1("Palette constructor requires 3 parameters "
                                                     "of type ColorGroup"));

    QSColorGroupClass *cgClass = ((QuickInterpreter*) env()->engine())->colorGroupClass();

    QSObject active = args[0];
    if (active.objectType() != cgClass)
	return env()->throwError(QString::fromLatin1("Palette constructor: Argument 1 is not "
                                                     "of type ColorGroup"));

    QSObject disabled = args[1];
    if (disabled.objectType() != cgClass)
	return env()->throwError(QString::fromLatin1("Palette constructor: Argument 2 is not "
                                                     "of type ColorGroup"));

    QSObject inactive = args[2];
    if (inactive.objectType() != cgClass)
	return env()->throwError(QString::fromLatin1("Palette constructor: Argument 3 is not "
                                                     "of type ColorGroup"));


    return construct(QPalette(*QSColorGroupClass::colorGroup(&active),
			      *QSColorGroupClass::colorGroup(&disabled),
			      *QSColorGroupClass::colorGroup(&inactive)));
};


QSObject QSPaletteClass::construct(const QPalette &p) const
{
    return QSObject(this, new PaletteShared(p));
}


QPalette *QSPaletteClass::palette(const QSObject *obj)
{
    Q_ASSERT(obj->objectType()->name() == QString::fromLatin1("Palette"));
    return &(((PaletteShared *) obj->shVal())->palette);
};


QVariant QSPaletteClass::toVariant( const QSObject *obj, QVariant::Type type ) const
{
    switch (type) {
    case QVariant::Invalid:
    case QVariant::Palette:
        return QVariant(*palette(obj));
    default:
        break;
    }
    return QVariant();
}

QSColorGroupClass::QSColorGroupClass(QSClass *b)
    : QSSharedClass(b)
{
    addMember(QString::fromLatin1("foreground"), QSMember(QSMember::Custom, QColorGroup::Foreground, AttributeNone));
    addMember(QString::fromLatin1("button"), QSMember(QSMember::Custom, QColorGroup::Button, AttributeNone));
    addMember(QString::fromLatin1("light"), QSMember(QSMember::Custom, QColorGroup::Light, AttributeNone));
    addMember(QString::fromLatin1("midlight"), QSMember(QSMember::Custom, QColorGroup::Midlight, AttributeNone));
    addMember(QString::fromLatin1("dark"), QSMember(QSMember::Custom, QColorGroup::Dark, AttributeNone));
    addMember(QString::fromLatin1("mid"), QSMember(QSMember::Custom, QColorGroup::Mid, AttributeNone));
    addMember(QString::fromLatin1("text"), QSMember(QSMember::Custom, QColorGroup::Text, AttributeNone));
    addMember(QString::fromLatin1("brightText"), QSMember(QSMember::Custom, QColorGroup::BrightText, AttributeNone));
    addMember(QString::fromLatin1("buttonText"), QSMember(QSMember::Custom, QColorGroup::ButtonText, AttributeNone));
    addMember(QString::fromLatin1("base"), QSMember(QSMember::Custom, QColorGroup::Base, AttributeNone));
    addMember(QString::fromLatin1("background"), QSMember(QSMember::Custom, QColorGroup::Background, AttributeNone));
    addMember(QString::fromLatin1("shadow"), QSMember(QSMember::Custom, QColorGroup::Shadow, AttributeNone));
    addMember(QString::fromLatin1("highlight"), QSMember(QSMember::Custom, QColorGroup::Highlight, AttributeNone));
    addMember(QString::fromLatin1("highlightedText"), QSMember(QSMember::Custom, QColorGroup::HighlightedText,
					  AttributeNone));
    addMember(QString::fromLatin1("link"), QSMember(QSMember::Custom, QColorGroup::Link, AttributeNone));
    addMember(QString::fromLatin1("linkVisited"), QSMember(QSMember::Custom, QColorGroup::LinkVisited, AttributeNone));
}


QSObject QSColorGroupClass::fetchValue(const QSObject *objPtr, const QSMember &mem) const
{
    if (mem.type() != QSMember::Custom)
	return QSClass::fetchValue(objPtr, mem);
    QSColorClass *cl = ((QuickInterpreter*) env()->engine())->colorClass();
    QColorGroup *cg = colorGroup(objPtr);
    Q_ASSERT(mem.idx >= 0);
    Q_ASSERT(mem.idx < QColorGroup::NColorRoles);
    return cl->construct(cg->color((QColorGroup::ColorRole)mem.idx));
}


void QSColorGroupClass::write(QSObject *objPtr, const QSMember &mem, const QSObject &val) const
{
    if (mem.type() != QSMember::Custom) {
	QSClass::write(objPtr, mem, val);
	return;
    }
    QSColorClass *cl = ((QuickInterpreter*) env()->engine())->colorClass();
    if (val.objectType() != cl) {
	env()->throwError(QString::fromLatin1("Property %1 must be of type Color").arg(mem.name()));
	return;
    }
    QColor color = *cl->color(&val);
    QColorGroup *cg = colorGroup(objPtr);
    Q_ASSERT(mem.idx >= 0);
    Q_ASSERT(mem.idx < QColorGroup::NColorRoles);
    cg->setColor((QColorGroup::ColorRole)mem.idx, color);
}


QSObject QSColorGroupClass::construct(const QSList &) const
{
    return construct(QColorGroup());
}


QSObject QSColorGroupClass::construct(const QColorGroup &cg) const
{
    return QSObject(this, new ColorGroupShared(cg));
}


QColorGroup *QSColorGroupClass::colorGroup(const QSObject *obj)
{
    Q_ASSERT(obj->objectType()->name() == QString::fromLatin1("ColorGroup"));
    return &(((ColorGroupShared *) obj->shVal())->colorGroup);
}

QVariant QSColorGroupClass::toVariant( const QSObject *obj, QVariant::Type type ) const
{
    switch (type) {
    case QVariant::Invalid:
    case QVariant::ColorGroup:
        return QVariant(*colorGroup(obj));
    default:
        break;
    }
    return QVariant();
}
