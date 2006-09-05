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

#include "qsobject.h"
#include "qsclass.h"
#include "qsenv.h"
#include "qsengine.h"
#include "qstypes.h"
#include "qsreference.h"
#include "qsinternal.h"
#include "qsoperations.h"
#include "qsobject_object.h"
#include "qsfuncref.h"
#include "qserror_object.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <math.h>

#ifdef WORDS_BIGENDIAN
unsigned char NaN_Bytes[] = { 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 };
unsigned char Inf_Bytes[] = { 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 };
#elif defined(arm)
unsigned char NaN_Bytes[] = { 0, 0, 0xf8, 0x7f, 0, 0, 0, 0 };
unsigned char Inf_Bytes[] = { 0, 0, 0xf0, 0x7f, 0, 0, 0, 0 };
#else
unsigned char NaN_Bytes[] = { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f };
unsigned char Inf_Bytes[] = { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f };
#endif

// see the ugly hack in qsobject.h where NaN and Inf is defined
double NaN { return *(const double*) NaN_Bytes; }
double Inf { return *(const double*) Inf_Bytes; }

const double D16 = 65536.0;
const double D32 = 4294967296.0;

// TODO: -0

using namespace QS;

QSObject::QSObject()
    : clss( 0 )
{
#ifdef QSDEBUG_MEM
    count++;
#endif
}

QSObject::QSObject( const QSClass *c )
    : clss( c )
{
    Q_ASSERT( clss );
    Q_ASSERT(clss->name() != QString::fromLatin1("Type"));
}

/*! Takes ownership of \a s */

QSObject::QSObject( const QSClass *c, QSShared *s )
    : clss( c )
{
    Q_ASSERT( clss && s );
    env()->registerShared( s );
    setVal( s );
}

QSObject::QSObject( const QSObject &o )
{
    clss = o.clss;
    val = o.val;

    if ( clss )
	clss->ref( (QSObject*) this );
}

QSObject& QSObject::operator=( const QSObject &o )
{
    if ( o.clss )
	o.clss->ref( (QSObject*)&o );
    if ( clss )
	clss->deref( (QSObject*)this );
    clss = o.clss;
    val = o.val;

    return *this;
}

bool QSObject::operator==( const QSObject &a ) const
{
    return equals( a );
}


QSObject::~QSObject()
{
#if QS_DEBUG_MEM == 1
    extern bool qs_finalDeletion;
    if ( qs_finalDeletion && clss )
	qFatal( "~QSObject: attempt to clean up non-invalidated object" );
#endif
    if ( clss )
	clss->deref( (QSObject*)this );
}

bool QSObject::isDefined() const
{
    return !isUndefined();
}

/*!
  Returns true if this object is valid i.e. has a type and data attached.
  Return false otherwise.
*/

bool QSObject::isValid() const
{
    return clss;
}

/*!
  \fn QSObject::invalidate()

  Invalidates this object.
  \sa isValid
*/

QString QSObject::typeName() const
{
    return objectType()->name();
}

ValueType QSObject::valueType() const
{
    return objectType()->valueType();
}

QSEnv *QSObject::env() const
{
    return objectType()->env();
}

void QSObject::setType( QSClass *c )
{
    ((QSObject*)this)->clss = c;
}

#if QS_DEBUG_MEM >= 1
void QSObject::checkShared() const
{
    if ( objectType()->valueType() != TypeShared )
	qFatal( "QSObject::setVal: class type mismatch for " + typeName() );
}
#endif

/*! Returns TRUE if this object is an instance of the type \c; FALSE otherwise.
*/

bool QSObject::isA( const QSClass *c ) const
{
    return objectType()->inherits( c );
}

bool QSObject::isA( const char *s ) const
{
    Q_ASSERT( isValid() );
    return typeName() == QString::fromUtf8( s );
}

/*! Returns true if the object is not a primitive */

bool QSObject::isObject() const
{
    return !isPrimitive();
}

/*! Returns true if the object is a null object */
bool QSObject::isNull() const
{
    return objectType() == env()->nullClass();
}

/*! Returns true if the object is a undefined */
bool QSObject::isUndefined() const
{
    return objectType() == env()->undefinedClass();
}

/*! Returns true if the object is a number */
bool QSObject::isNumber() const
{
    return objectType() == env()->numberClass();
}

/*! Returns true if the object is a string */
bool QSObject::isString() const
{
    return objectType() == env()->stringClass();
}

/*! Returns true if the object is a boolean */
bool QSObject::isBoolean() const
{
    return objectType() == env()->booleanClass();
}

/*! Returns true if this object is a function object */

bool QSObject::isFunction() const
{
    return objectType() == env()->funcRefClass();
}

/*! Returns true if the object is a primitive, either a bool, number
  or string */
bool QSObject::isPrimitive() const
{
    return isNull() || isUndefined() || isNumber() ||
	isBoolean() || isString();
}

/*! Returns true if this object equals \other. In the case that
  this object does not know how to compare itself to other, other
  will try to compare itself to this.
*/
bool QSObject::equals( const QSObject &other ) const
{
    Q_ASSERT( isValid() );
    Q_ASSERT( other.isValid() );
    QSEqualsResult eq = clss->isEqual( *this, other );
    if ( eq == EqualsUndefined )
	eq = other.clss->isEqual( other, *this );
    return eq == EqualsIsEqual;
}

bool QSObject::strictEquals( const QSObject &other ) const
{
    Q_ASSERT( isValid() );
    Q_ASSERT( other.isValid() );
    QSEqualsResult eq = clss->isStrictEqual( *this, other );
    if( eq==EqualsUndefined )
	eq = other.clss->isStrictEqual( other, *this );
    return eq == EqualsIsEqual;
}

QSCompareResult QSObject::compareTo( const QSObject &b ) const
{
    QSCompareResult cmp = objectType()->compare( *this, b );
//     if( cmp==CompareUndefined ) {
// 	cmp = b.clss->compare( b, *this );
//     }
    return cmp;
}

#if 0
bool QSObject::derivedFrom(const char *s) const
{
    if (!s)
	return FALSE;

    assert( typ );
    const TypeInfo *info = typ->typeInfo();
    while ( info ) {
	if ( info->name && strcmp(s, info->name) == 0 )
	    return TRUE;
	info = info->base;
    }

    return FALSE;
}
#endif

QSObject QSObject::toPrimitive( const QSClass *preferred ) const
{
    return objectType()->toPrimitive( this, preferred );
}

bool QSObject::toBoolean() const
{
    return objectType()->toBoolean( this );
}

double QSObject::toNumber() const
{
    return objectType()->toNumber( this );
}

/*! Return this object's value converted to a QVariant. If this is not
  possible, or the object is invalid, an invalid QVariant will be returned.
  In case several conversion are possible \a t can be used to indicate the
  preferred type. Specifying QVariant::Invalid is good enough for most cases.
*/

QVariant QSObject::toVariant( QVariant::Type t ) const
{
    return isValid() ? objectType()->toVariant( this, t ) : QVariant();
}

// helper function for toInteger, toInt32, toUInt32 and toUInt16
double QSObject::round() const
{
    if ( isUndefined() ) /* TODO: see below */
	return 0.0;
    double n = toNumber();
    if( isNaN( n ) )
	return NaN;
    if ( n == 0.0 )   /* TODO: -0, NaN, Inf */
	return 0.0;
    double d = floor( fabs( n ) );
    if ( n < 0.0 )
	d *= -1;
    return d;
}

// ECMA 9.4
int QSObject::toInteger() const
{
    return int(round());
}

// ECMA 9.5
int QSObject::toInt32() const
{
    double d = round();
    double d32 = fmod(d, D32);

    if ( d32 >= D32 / 2.0 )
	d32 -= D32;

    return int(d32);
}

// ECMA 9.6
unsigned int QSObject::toUInt32() const
{
    double d = round();
    double d32 = fmod( d, D32 );

    return uint(d32);
}

// ECMA 9.7
unsigned short QSObject::toUInt16() const
{
    double d = round();
    double d16 = fmod( d, D16 );

    return ushort(d16);
}

QString QSObject::toString() const
{
    return objectType()->toString( this );
}

bool QSObject::isExecutable() const
{
    return isA( env()->funcRefClass() ); // is a function reference ?
}

// ### still used by Date -> QDateTime conversion (quickobjects.cpp)
QSObject QSObject::execute( const QSList &args )
{
    Q_ASSERT( isExecutable() );
    //    qFatal( "QSObject::execute()" );
    QSMember dummy;
    return invoke( dummy, args );
}

// [[call]]
QSObject QSObject::executeCall( QSObject *objPtr, const QSList *args )
{
    return objectType()->execute( this, objPtr, *args );
}

/*!
  Invoke member function \a mem on this object passing \a args
  as arguments. The 'this' value will point to this object
  during function execution.
 */
QSObject QSObject::invoke( const QSMember &mem, const QSList &args )
{
    const QSList *oldArgs = env()->arguments();
    env()->setArguments( &args );
    QSObject oldThis = env()->thisValue();
    if ( isA( env()->globalClass() ) )
	env()->setThisValue( env()->createUndefined() );
    else
	env()->setThisValue( *this );

    QSObject ret = objectType()->invoke( this, mem );

    if (env()->executionMode() == QSEnv::ReturnValue)
        env()->setExecutionMode(QSEnv::Normal);

    env()->setArguments( oldArgs );
    env()->setThisValue( oldThis );
    return ret;
}

QSObject QSObject::getQualified( const QString &name ) const
{
    QSObject obj = *this;
    QString s = name;

    // break up dotted notation ns like P1.P2.P3
    while ( !s.isEmpty() ) {
	int pos = s.find( '.' );
	if ( pos < 0 )
	    pos = s.length();
	// don't overwrite any existing objects, reuse them
        QString cname = s.left( pos );
        QSObject tmpobj = obj.get( cname );

        // Search the class hieriarchy for member if undefined
        if ( !tmpobj.isDefined() ) {
            QSClass *base = obj.objectType()->base();
            while (base && !tmpobj.isDefined()) {
                tmpobj = base->get(&obj, cname);
                base = base->base();
            }
            if (!tmpobj.isDefined())
                return env()->createUndefined();
        }
        obj = tmpobj;
	s = s.mid( pos + 1 );
    }
    return obj;
}

// ECMA 8.6.2.1
QSObject QSObject::get( const QString &p ) const
{
    return objectType()->get( this, p );
}

// ECMA 8.6.2.2
void QSObject::put( const QString &p, const QSObject& v )
{
    objectType()->put( this, p, v );
}

// provided for convenience.
void QSObject::put( const QString &p, double d )
{
    put( p, env()->createNumber( d ) );
}

// provided for convenience.
void QSObject::put(const QString &p, int i )
{
    put( p, env()->createNumber( i ) );
}

// provided for convenience.
void QSObject::put( const QString &p, unsigned int u )
{
    put( p, env()->createNumber( u ) );
}

// ECMA 8.6.2.4
bool QSObject::hasProperty( const QString &p ) const
{
    return objectType()->hasProperty( this, p );
}

// ECMA 8.6.2.5
bool QSObject::deleteProperty( const QString &p )
{
    QSMember mem;
    if ( !objectType()->member( this, p, &mem ) ||
	 mem.type() == QSMember::Identifier )
	return TRUE;
    return objectType()->deleteProperty( this, mem );
}

QSObject QSObject::fetchValue( const QSMember &mem )
{
    return objectType()->fetchValue( this, mem );
}

void QSObject::write( const QSMember &mem, const QSObject &val )
{
    objectType()->write( this, mem, val );
}

/*!
  Tries to resolve a member named \a name. If it is found \a mem is
  initialized with the result and the class defining this member is
  returned. In case of a failed search, 0 is returned and the content
  of \a mem is undefined.
*/
const QSClass* QSObject::resolveMember( const QString &name,
					QSMember *mem,
					const QSClass *owner,
					int *offset ) const
{
    Q_ASSERT( offset );
    Q_ASSERT( mem );
    Q_ASSERT( !name.isEmpty() );
    if ( !owner )
	owner = objectType();
    QPtrList<QSClass> enclosing;
    QSMember stackedMember;
    while ( owner ) {
	if ( owner->member( *offset==0 ? this : 0, name, mem ) ) {
 	    return owner;
	} else if ( mem->type() == QSMember::Identifier && !stackedMember.isDefined() ) {
	    stackedMember = *mem;
 	}
	if( owner->enclosingClass() )
	    enclosing.append( owner->enclosingClass() );
 	owner = owner->base();
    }

    const QSClass *eClass = enclosing.first();
    while( eClass ) {
	const QSClass *enc =
	    resolveMember( name, mem, eClass, &(++(*offset)) );
	if ( enc ) {
	    if( mem->type() == QSMember::Identifier )
		if( !stackedMember.isDefined() )
		    stackedMember = *mem;
		else
		return enc;
	} else {
	    --(*offset);
	}
	eClass = enclosing.next();
    }

    if ( stackedMember.isDefined() ) {
	*mem = stackedMember;
	return stackedMember.owner();

    }

    return 0;
}



void QSObject::mark()
{
    objectType()->mark( this );
}

/*!
  \sa Imp::debugString
 */

QString QSObject::debugString() const
{
    return objectType()->debugString( this );
}

QSWritable::QSWritable()
    : props( 0 )
{
}

QSWritable::~QSWritable()
{
    delete props;
}

void QSWritable::setProperty( const QString &n, const QSProperty &p )
{
    if ( props ) {
	props->replace( n, p );
    } else {
	props = new QSPropertyMap();
	props->insert( n, p );
    }
}

QSProperty *QSWritable::reference( const QString &n ) const
{
    if ( props ) {
	QSPropertyMap::Iterator it = props->find( n );
	if ( it != props->end() )
	    return &(*it);
    }
    return 0; // error case
}

bool QSWritable::hasProperty( const QString &n ) const
{
    return props && ( props->find( n ) != props->end() );
}

void QSWritable::invalidate()
{
    if( !props )
	return;
    QMap<QString,QSProperty>::Iterator it = props->begin();
    QMap<QString,QSProperty>::Iterator end = props->end();
    while( it != end ) {
	(*it).invalidate();
	it++;
    }
}

#ifdef QSDEBUG_MEM
int QSObject::count = 0;
int List::count = 0;
#endif

#if 0
// ECMA 8.6.2.6 (new draft)
QSObject Imp::defaultValue( const QSObject *o, Type hint ) const
{
#if 0
    QSObject obj;

    /* TODO String on Date object */
    if ( hint != StringType && hint != NumberType )
	hint = NumberType;

    if ( hint == StringType )
	obj = get( o, "toString" );
    else
	obj = get( o, "valueOf" );

    Imp *that = (Imp*)this;
    if ( obj.isExecutable() ) { // spec says "not primitive type" but ...
	FunctionImp *f = (FunctionImp*)obj.imp();
	QSObject t( that );
	QSObject s = f->executeCall( &t, 0 );
	if ( !s.isObject() )
	    return s;
    }

    if ( hint == StringType )
	obj = get( o, "valueOf" );
    else
	obj = get( o, "toString" );

    if ( obj.isExecutable()) {
	FunctionImp *f = (FunctionImp*)obj.imp();
	QSObject t( that );
	QSObject s = f->executeCall( &t, 0 );
	if (!s.isObject())
	    return s;
    }
#endif
    return Error::create( TypeError, "No default value" );
}

void Imp::mark( QSObject * )
{
    ref();

    if ( props ) {
	QSPropertyMap::Iterator it = props->begin();
	QSPropertyMap::Iterator end = props->end();
	for( ; it != end; ++it )
	    (*it).object.mark();
    }
}

#endif

QSObject QSReference::dereference() const
{
    if ( mem.isDefined() )
	return cls->fetchValue( &bs, mem );
    else
	return cls->createUndefined();
}

void QSReference::assign( const QSObject &o )
{
    Q_ASSERT( isWritable() );
    if( isReference() )
      cls->write( &bs, mem, o );
}

/*!
  Deletes the property referenced by this object. Returns true
  if the property could be deleted or didn't exist in the first place.
  Return false otherwise.
*/

bool QSReference::deleteProperty()
{
    if ( !isDefined() || mem.type() == QSMember::Identifier )
	return TRUE;
    return bs.objectType()->deleteProperty( &bs, mem );
}
