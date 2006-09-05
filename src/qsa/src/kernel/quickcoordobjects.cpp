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

#include "quickcoordobjects.h"
#include "qstypes.h"
#include "qsfunction.h"
#include "qsenv.h"
#include <qpoint.h>
#include <qsize.h>
#include <qrect.h>
#include <assert.h>
#include <quickinterpreter.h>

class PointShared : public QSShared {
public:
    PointShared( const QPoint &p ) : point( p ) { }
    QPoint point;
};

QSPointClass::QSPointClass( QSClass *b, QuickInterpreter *i )
    : QSSharedClass( b ), QuickEnvClass( i )
{
    addMember( QString::fromLatin1("x"), QSMember( QSMember::Custom, 0, AttributePublic ) );
    addMember( QString::fromLatin1("y"), QSMember( QSMember::Custom, 1, AttributePublic ) );
}

QPoint *QSPointClass::point( const QSObject *obj ) const
{
    assert( obj->objectType() == this );
    return &((PointShared*)obj->shVal())->point;
}

QSObject QSPointClass::fetchValue( const QSObject *obj,
				   const QSMember &mem ) const
{
    if ( mem.type() == QSMember::Custom ) {
	switch ( mem.idx ) {
	case 0:
	    return createNumber( point( obj )->x() );
	case 1:
	    return createNumber( point( obj )->y() );
	default:
	    qFatal( "QSPointClass::fetchValue: unhandled case" );
	    return createUndefined();
	}
    }
    return QSClass::fetchValue( obj, mem );
}

void QSPointClass::write( QSObject *objPtr, const QSMember &mem,
			  const QSObject &val ) const
{
    if ( mem.type() == QSMember::Custom ) {
	switch ( mem.idx ) {
	case 0:
	    point( objPtr )->setX( val.toInteger() );
	    break;
	case 1:
	    point( objPtr )->setY( val.toInteger() );
	    break;
	default:
	    qDebug( "QSPointClass::write() Unhandled case" );
	}
    } else {
	QSClass::write( objPtr, mem, val );
    }
}

QString QSPointClass::toString( const QSObject *obj ) const
{
    QPoint *p = point( obj );
    return QString::fromLatin1( "(%1, %2)" ).arg( p->x() ).arg( p->y() );
}

QVariant QSPointClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return *point( obj );
}

QString QSPointClass::debugString( const QSObject *obj ) const
{
    QPoint *p = point( obj );
    return QString::fromLatin1( "{x=%1:Number,y=%2:Number}" ).arg( p->x() ).arg( p->y() );
}

QSObject QSPointClass::construct( const QSList &args ) const
{
    if ( args.size() == 1 ) {
	if ( args[ 0 ].objectType() == this ) {
	    QSObject pt = args.at(0);
            return construct(QPoint(*point(&pt)));
	}
    } else if ( args.size() == 2 ) {
	return construct( QPoint( args[ 0 ].toInteger(), args[ 1 ].toInteger() ) );
    }

    return construct( QPoint() );
}

QSObject QSPointClass::construct( const QPoint &p ) const
{
    return QSObject( this, new PointShared( p ) );
}

QSEqualsResult QSPointClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    if ( !b.isA( this ) )
	return EqualsNotEqual;
    return ( QSEqualsResult ) ( *point( &a ) == *point( &b ) );
}

//////////////////////////////////////////////////////////////////////////

class SizeShared : public QSShared {
public:
    SizeShared( const QSize &s ) : size( s ) { }
    QSize size;
};

static QSObject transpose( QSEnv *env )
{
    QSObject t = env->thisValue();
    ( (QSSizeClass*)t.objectType() )->size( &t )->transpose();
    return env->createUndefined();
}

QSSizeClass::QSSizeClass( QSClass *b, QuickInterpreter *i )
    : QSSharedClass( b ), QuickEnvClass( i )
{
    addMember( QString::fromLatin1("width"),  QSMember( QSMember::Custom, 0, AttributePublic ) );
    addMember( QString::fromLatin1("height"), QSMember( QSMember::Custom, 1, AttributePublic ) );
    addMember( QString::fromLatin1("transpose"), QSMember( &transpose ) );
}

QSize *QSSizeClass::size( const QSObject *obj ) const
{
    assert( obj->objectType() == this );
    return &((SizeShared*)obj->shVal())->size;
}

QSObject QSSizeClass::fetchValue( const QSObject *objPtr,
				  const QSMember &mem ) const
{
    if( mem.type()==QSMember::Custom ) {
	switch( mem.idx ) {
	case 0:
	    return createNumber( size( objPtr )->width() );
	case 1:
	    return createNumber( size( objPtr )->height() );
	default:
	    qFatal( "QSSizeClass::fetchValue, unhandled case" );
	    return createUndefined();
	}
    }
    return QSClass::fetchValue( objPtr, mem );
}

void QSSizeClass::write( QSObject *objPtr, const QSMember &mem,
			 const QSObject &val ) const
{
    if( mem.type()==QSMember::Custom ) {
	switch( mem.idx ) {
	case 0:
	    size( objPtr )->setWidth( val.toInteger() );
	    break;
	case 1:
	    size( objPtr )->setHeight( val.toInteger() );
	    break;
	default:
	    qFatal( "QSSizeClass::write, unhandled case" );
	    break;
	}
    } else {
	QSClass::write( objPtr, mem, val );
    }
}

// QSObject QSSizeClass::invoke( QSObject * objPtr, const QSMember &mem,
// 			 const QSList &args ) const
// {
//     if( mem.type()==QSMember::Custom ) {
// 	if( mem.idx==2 ) {
// 	    size( objPtr )->transpose();
// 	}
// 	return QSUndefined();
//     } else {
// 	return QSClass::invoke( objPtr, mem, args );
//     }
// }

QString QSSizeClass::toString( const QSObject *obj ) const
{
    QSize *s = size( obj );
    return QString::fromLatin1( "(%1, %2)" ).arg( s->width() ).arg( s->height() );
}

QVariant QSSizeClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return *size( obj );
}

QString QSSizeClass::debugString( const QSObject *obj ) const
{
    QSize *s = size( obj );
    return QString::fromLatin1( "{width=%1:Number,height=%2:Number}" )
	.arg( s->width() ).arg( s->height() );
}

QSObject QSSizeClass::construct( const QSList &args ) const
{
    if ( args.size() == 1 ) {
	if ( args[ 0 ].objectType() == this ) {
	    QSObject sz = args.at(0);
	    return construct(QSize(*size(&sz)));
	}
    } else if ( args.size() == 2 ) {
	return construct( QSize( args[ 0 ].toInteger(),
				 args[ 1 ].toInteger() ) );
    }

    return construct( QSize() );
}

QSObject QSSizeClass::construct( const QSize &s ) const
{
    return QSObject( this, new SizeShared( s ) );
}

//////////////////////////////////////////////////////////////////////////

class RectShared : public QSShared {
public:
    RectShared( const QRect &r ) : rect( r ) { }
    QRect rect;
};

QSRectClass::QSRectClass( QSClass * b, QuickInterpreter *i )
    : QSSharedClass( b ), QuickEnvClass( i )
{
    addMember( QString::fromLatin1("x"), QSMember( QSMember::Custom, 0, AttributePublic ) );
    addMember( QString::fromLatin1("left"), QSMember( QSMember::Custom, 1, AttributePublic ) );
    addMember( QString::fromLatin1("y"), QSMember( QSMember::Custom, 2, AttributePublic ) );
    addMember( QString::fromLatin1("top"), QSMember( QSMember::Custom, 3, AttributePublic ) );
    addMember( QString::fromLatin1("width"), QSMember( QSMember::Custom, 4, AttributePublic ) );
    addMember( QString::fromLatin1("height"), QSMember( QSMember::Custom, 5, AttributePublic ) );
    addMember( QString::fromLatin1("right"), QSMember( QSMember::Custom, 6, AttributePublic ) );
    addMember( QString::fromLatin1("bottom"), QSMember( QSMember::Custom, 7, AttributePublic ) );
    addMember( QString::fromLatin1("center"), QSMember( QSMember::Custom, 8, AttributePublic ) );
    addMember( QString::fromLatin1("isNull"), QSMember( &isNull ) );
    addMember( QString::fromLatin1("isEmpty"), QSMember( &isEmpty ) );
    addMember( QString::fromLatin1("contains"), QSMember( &contains ) );
    addMember( QString::fromLatin1("intersection"), QSMember( &intersection ) );
    addMember( QString::fromLatin1("union"), QSMember( &union_ ) );
    addMember( QString::fromLatin1("intersects"), QSMember( &intersects ) );
    addMember( QString::fromLatin1("normalize"), QSMember( &normalize ) );
    addMember( QString::fromLatin1("moveLeft"), QSMember( &moveLeft ) );
    addMember( QString::fromLatin1("moveRight"), QSMember( &moveRight ) );
    addMember( QString::fromLatin1("moveTop"), QSMember( &moveTop ) );
    addMember( QString::fromLatin1("moveBottom"), QSMember( &moveBottom ) );
    addMember( QString::fromLatin1("moveBy"), QSMember( &moveBy ) );
}


QSObject QSRectClass::fetchValue( const QSObject *o,
				  const QSMember &mem ) const
{
    if( mem.type() == QSMember::Custom ) {
	switch( mem.idx ) {
	case 0:
	case 1:
	    return createNumber( rect( o )->x() );
	case 2:
	case 3:
	    return createNumber( rect( o )->y() );
	case 4:
	    return createNumber( rect( o )->width() );
	case 5:
	    return createNumber( rect( o )->height() );
	case 6:
	    return createNumber( rect( o )->right() );
	case 7:
	    return createNumber( rect( o )->bottom() );
	case 8:
	    return pointClass()->construct( rect( o )->center() );
	default:
	    qFatal( "QSRectClass::fetchValue: unhandled case" );
	    return createUndefined();
	}
    }
    return QSClass::fetchValue( o, mem );
}


void QSRectClass::write( QSObject *o, const QSMember &mem,
			 const QSObject &val ) const
{
    if( mem.type() == QSMember::Custom ) {
	int i = val.toInteger();
	switch( mem.idx ) {
	case 0:
	case 1:
	    rect( o )->setX( i );
	    break;
	case 2:
	case 3:
	    rect( o )->setY( i );
	    break;
	case 4:
	    rect( o )->setWidth( i );
	    break;
	case 5:
	    rect( o )->setHeight( i );
	    break;
	case 6:
	    rect( o )->setRight( i );
	    break;
	case 7:
	    rect( o )->setBottom( i );
	    break;
	case 8:
	    break;
	default:
	    qFatal( "QSRectClass::write: unhandled case" );
	}
    } else {
	QSClass::write( o, mem, val );
    }
}


QRect *QSRectClass::rect( const QSObject *obj ) const
{
    assert( obj->objectType() == this );
    return &((RectShared*)obj->shVal())->rect;
}

QRect *QSRectClass::rect( QSEnv *e )
{
    return &((RectShared*)e->thisValue().shVal())->rect;
}

QString QSRectClass::toString( const QSObject *obj ) const
{
    QRect *r = rect( obj );
    return QString::fromLatin1( "(%1, %2, %3, %4)" ).arg( r->x() ).arg( r->y() )
	.arg( r->width() ).arg( r->height() );
}

QVariant QSRectClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return *rect( obj );
}

QString QSRectClass::debugString( const QSObject *obj ) const
{
    QRect *r = rect( obj );
    return QString::fromLatin1( "{x=%1:Number,y=%2:Number,width=%3,height=%3}" )
	.arg( r->x() ).arg( r->y() ).arg( r->width() ).arg( r->height() );
}

QSObject QSRectClass::construct( const QSList &args ) const
{
    if ( args.size() == 1 ) {
	if ( args[ 0 ].objectType() == this ) {
	    QSObject rt = args.at(0);
	    return construct(QRect(*rect(&rt)));
	}
    } else if ( args.size() == 4 ) {
	return construct( QRect( args[ 0 ].toInteger(),
				 args[ 1 ].toInteger(),
				 args[ 2 ].toInteger(),
				 args[ 3 ].toInteger() ) );
    }

    return construct( QRect() );
}

QSObject QSRectClass::construct( const QRect &r ) const
{
    return QSObject( this, new RectShared( r ) );
}

QSEqualsResult QSRectClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    if ( !b.isA( this ) )
	return EqualsNotEqual;
    return ( QSEqualsResult ) ( *rect( &a ) == *rect( &b ) );
}

QSObject QSRectClass::isNull( QSEnv *env )
{
    QRect *r = rect( env );
    return env->createBoolean( r->isNull() );
}

QSObject QSRectClass::isEmpty( QSEnv *env )
{
    QRect *r = rect( env );
    return env->createBoolean( r->isEmpty() );
}

QSObject QSRectClass::contains( QSEnv *env )
{
    QRect *r = rect( env );

    if ( env->numArgs() < 1 || env->numArgs() > 2 ) {
	env->throwError( QString::fromLatin1( "Rect.contains() called with %1 arguments. 1 or 2 arguments expected." ).
			 arg( env->numArgs() ) );
	return env->createUndefined();
    }

    if ( env->numArgs() == 1 ) {
	QSObject a0 = env->arg( 0 );
	if ( !a0.isA( "Point" ) ) {
	    env->throwError( QString::fromLatin1( "Rect.contains() called with an argument of type %1. "
				  "Type Point is expeced" ).
			     arg( a0.typeName() ) );
	    return env->createUndefined();
	}

	return env->createBoolean( r->contains( *( (QSPointClass*)a0.objectType() )->point( &a0 ) ) );
    } else {
	if ( !env->arg( 0 ).isNumber() || !env->arg( 1 ).isNumber() ) {
	    env->throwError( QString::fromLatin1( "Rect.contains() called with arguments of type %1 and %2. "
				  "Type Number and Number is expeced" ).
			     arg( env->arg( 0 ).typeName() ).arg( env->arg( 1 ).typeName() ) );
	    return env->createUndefined();
	}
	return env->createBoolean( r->contains( env->arg( 0 ).toInteger(),
						env->arg( 1 ).toInteger() ) );
    }

    return env->createUndefined();
}

QSObject QSRectClass::intersection( QSEnv *env )
{
    QSObject t = env->thisValue();
    QSRectClass *cl = (QSRectClass*)t.objectType();
    QRect *r = cl->rect( &t );

    if ( env->numArgs() != 1 ) {
	env->throwError( QString::fromLatin1( "Rect.intersection() called with %1 arguments. 1 argument expected." ).
			 arg( env->numArgs() ) );
	return env->createUndefined();
    }

    QSObject a0 = env->arg( 0 );
    if ( !a0.isA( cl ) ) {
	env->throwError( QString::fromLatin1( "Rect.intersection() called with an argument of type %1. "
				  "Type Rect is expeced" ).
			 arg( a0.typeName() ) );
	return env->createUndefined();
    }

    return cl->construct( r->intersect( *cl->rect( &a0 ) ) );
}

QSObject QSRectClass::union_( QSEnv *env )
{
    QSObject t = env->thisValue();
    QSRectClass *cl = (QSRectClass*)t.objectType();
    QRect *r = cl->rect( &t );

    if ( env->numArgs() != 1 ) {
	env->throwError( QString::fromLatin1( "Rect.union() called with %1 arguments. 1 argument expected." ).
			 arg( env->numArgs() ) );
	return env->createUndefined();
    }

    QSObject a0 = env->arg( 0 );
    if ( !a0.isA( cl ) ) {
	env->throwError( QString::fromLatin1( "Rect.union() called with an argument of type %1. "
				  "Type Rect is expeced" ).
			 arg( a0.typeName() ) );
	return env->createUndefined();
    }

    return cl->construct( r->unite( *cl->rect( &a0 ) ) );
}

QSObject QSRectClass::intersects( QSEnv *env )
{
    QSObject t = env->thisValue();
    QSRectClass *cl = (QSRectClass*)t.objectType();
    QRect *r = cl->rect( &t );

    if ( env->numArgs() != 1 ) {
	env->throwError( QString::fromLatin1( "Rect.intersects() called with %1 arguments. 1 argument expected." ).
			 arg( env->numArgs() ) );
	return env->createUndefined();
    }

    QSObject a0 = env->arg( 0 );
    if ( !a0.isA( cl ) ) {
	env->throwError( QString::fromLatin1( "Rect.intersects() called with an argument of type %1. "
				  "Type Rect is expeced" ).
			 arg( a0.typeName() ) );
	return env->createUndefined();
    }

    return env->createBoolean( r->intersects( *cl->rect( &a0 ) ) );
}

QSObject QSRectClass::normalize( QSEnv *env )
{
    QSObject t = env->thisValue();
    QSRectClass *cl = (QSRectClass*)t.objectType();
    QRect *r = cl->rect( &t );
    return cl->construct( r->normalize() );
}

void QSRectClass::moveLeft( QSEnv *env )
{
    QRect *r = rect( env );

    if ( env->numArgs() != 1 ) {
	env->throwError( QString::fromLatin1( "Rect.moveLeft() called with %1 arguments. 1 argument expected." ).
			 arg( env->numArgs() ) );
	return;
    }

    if ( !env->arg( 0 ).isNumber() ) {
	env->throwError( QString::fromLatin1( "Rect.moveLeft() called with an argument of type %1. "
				  "Type Number is expeced" ).
			 arg( env->arg( 0 ).typeName() ) );
	return;
    }

    r->moveLeft( env->arg( 0 ).toInteger() );
}

void QSRectClass::moveRight( QSEnv *env )
{
    QRect *r = rect( env );

    if ( env->numArgs() != 1 ) {
	env->throwError( QString::fromLatin1( "Rect.moveRight() called with %1 arguments. 1 argument expected." ).
			 arg( env->numArgs() ) );
	return;
    }

    if ( !env->arg( 0 ).isNumber() ) {
	env->throwError( QString::fromLatin1( "Rect.moveRight() called with an argument of type %1. "
				  "Type Number is expeced" ).
			 arg( env->arg( 0 ).typeName() ) );
	return;
    }

    r->moveRight( env->arg( 0 ).toInteger() );
}

void QSRectClass::moveTop( QSEnv *env )
{
    QRect *r = rect( env );

    if ( env->numArgs() != 1 ) {
	env->throwError( QString::fromLatin1( "Rect.moveTop() called with %1 arguments. 1 argument expected." ).
			 arg( env->numArgs() ) );
	return;
    }

    if ( !env->arg( 0 ).isNumber() ) {
	env->throwError( QString::fromLatin1( "Rect.moveTop() called with an argument of type %1. "
				  "Type Number is expeced" ).
			 arg( env->arg( 0 ).typeName() ) );
	return;
    }

    r->moveTop( env->arg( 0 ).toInteger() );
}

void QSRectClass::moveBottom( QSEnv *env )
{
    QRect *r = rect( env );

    if ( env->numArgs() != 1 ) {
	env->throwError( QString::fromLatin1( "Rect.moveBottom() called with %1 arguments. 1 argument expected." ).
			 arg( env->numArgs() ) );
	return;
    }

    if ( !env->arg( 0 ).isNumber() ) {
	env->throwError( QString::fromLatin1( "Rect.moveBottom() called with an argument of type %1. "
				  "Type Number is expeced" ).
			 arg( env->arg( 0 ).typeName() ) );
	return;
    }

    r->moveBottom( env->arg( 0 ).toInteger() );
}

void QSRectClass::moveBy( QSEnv *env )
{
    QRect *r = rect( env );

    if ( env->numArgs() != 2 ) {
	env->throwError( QString::fromLatin1( "Rect.moveBy() called with %1 arguments. 2 arguments expected." ).
			 arg( env->numArgs() ) );
	return;
    }

    if ( !env->arg( 0 ).isNumber() || !env->arg( 1 ).isNumber() ) {
	env->throwError( QString::fromLatin1( "Rect.moveBy() called with arguments of type %1 and %2. "
				  "Type Number and Number are expeced" ).
			 arg( env->arg( 0 ).typeName() ).arg( env->arg( 1 ).typeName() ) );
	return;
    }

    r->moveBy( env->arg( 0 ).toInteger(), env->arg( 1 ).toInteger() );
}

