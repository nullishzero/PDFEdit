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

#include "qsarray_object.h"
#include "qsoperations.h"
#include "qstypes.h"
#include "qsfunction.h"
#include "qsenv.h"

QSArrayClass::QSArrayClass( QSClass *b )
    : QSWritableClass( b )
{
    addMember( QString::fromLatin1("toString"), QSMember( &QSArrayClass::join ) );
    addMember( QString::fromLatin1("toLocaleString"), QSMember( &QSArrayClass::join ) ); // ###
    addMember( QString::fromLatin1("concat"), QSMember( &concat ) );
    addMember( QString::fromLatin1("join"), QSMember( &QSArrayClass::join ) );
    addMember( QString::fromLatin1("pop"), QSMember( &pop ) );
    addMember( QString::fromLatin1("push"), QSMember( &push ) );
    addMember( QString::fromLatin1("reverse"), QSMember( &reverse ) );
    addMember( QString::fromLatin1("shift"), QSMember( &shift ) );
    addMember( QString::fromLatin1("slice"), QSMember( &slice ) );
    addMember( QString::fromLatin1("sort"), QSMember( &sort ) );
    addMember( QString::fromLatin1("splice"), QSMember( &splice ) );
    addMember( QString::fromLatin1("unshift"), QSMember( &unshift ) );
    int lengthIndex = addVariableMember( QString::fromLatin1("length"), AttributeNone );
    // setLength and length functions assume that lengthIndex is null
    Q_ASSERT( !lengthIndex );
}

/*!
  \reimp
*/
QSObject QSArrayClass::cast( const QSList &args ) const
{
    return construct( args );
}


QSObject QSArrayClass::construct( const QSList &args ) const
{
    QSArray result( env() );
    unsigned int len = 0;
    QSListIterator it = args.begin();
    // a single argument might denote the array size
    if ( args.size() == 1 && it->isNumber() ) {
	len = it->toUInt32();
	if ( it->toNumber() != double( len ) ) {
	    throwError( RangeError, QString::fromLatin1("Invalid array length.") );
	    return createUndefined();
	}
	setLength( &result, len );
    } else {
	// initialize array
	len = args.size();
	for (unsigned int u = 0; it != args.end(); it++, u++)
	    result.put( QSString::from( u ), *it );
    }

    // array size
    return result;
}

QSArray::QSArray( const QSEnv *env )
    : QSObject( env->arrayClass() )
{
    setVal( new QSInstanceData( 1, env->createUndefined() ) );
    env->registerShared( shVal() );
    QSArrayClass::setLength( this, 0 );
}

void QSArrayClass::write( QSObject *objPtr, const QSMember &mem,
			  const QSObject &val ) const
{
    if ( mem.name() == QString::fromLatin1("length") ) {
	uint oldLen = length( objPtr );
	uint newLen = val.toUInt32();
	if ( val.toNumber() != double( newLen ) ) {
	    throwError( RangeError, QString::fromLatin1("Invalid array length.") );
	    return;
	}
	// shrink array if necessary
	for ( uint u = newLen; u < oldLen; u++ ) {
	    QString pr = QString::number( u );
	    if ( objPtr->hasProperty( pr ) )
		//		deleteProperty( pr ); // ####
		objPtr->put( pr, createUndefined() );
	}
	setLength( objPtr, newLen );
    } else {
	QSWritableClass::write( objPtr, mem, val );
	bool ok;
	uint idx = mem.name().toUInt( &ok );
	if ( ok ) {
	    uint len = length( objPtr );
	    if ( idx >= len )
		setLength( objPtr, idx + 1 );
	}
    }
}

/*!
  \reimp

  Equivalent to join() with a "," seperator.
*/

QString QSArrayClass::toString( const QSObject *obj ) const
{
    return joinInternal( *obj, QString::fromLatin1(",") );
}

QVariant QSArrayClass::toVariant( const QSObject *obj, QVariant::Type t ) const
{
    QSObject v = *obj;
    int len = length( obj );
    if ( t == QVariant::StringList ) {
	QStringList l;
	for ( int i = 0; i < len; ++i )
	    l << v.get( QString::number( i ) ).toString();
	QVariant var = l;
	return var;
    } else if ( t == QVariant::Map ) {
	QMap<QString, QVariant> m;
	QSPropertyMap *propMap = properties( obj );
        if (!propMap)
            return m;
	for ( QSPropertyMap::ConstIterator it = propMap->begin();
	      it != propMap->end(); ++it )
	    m.insert( it.key(), (*it).object.toVariant( QVariant::Invalid ) );
	QVariant var = m;
	return var;
    } else if ( t == QVariant::String ) {
	QStringList l;
	for ( int i = 0; i < len; ++i )
	    l << v.get( QString::number( i ) ).toString();
	return QVariant( l.join( QString::fromLatin1(",") ) );
    } else {
	QValueList<QVariant> l;
	for ( int i = 0; i < len; ++i )
	    l << v.get( QString::number( i ) ).toVariant( QVariant::Invalid );
	QVariant var = l;
	return var;
    }
}

/*!
  Returns the length of array object \a a.
 */

uint QSArrayClass::length( const QSObject *a )
{
    Q_ASSERT( a && a->isA( a->env()->arrayClass() ) );
    return (uint)( (QSInstanceData*) a->shVal() )->value(0)->toNumber();
}

/*!
  Sets the length property of array object \a a to \a l.
 */

void QSArrayClass::setLength( QSObject *a, uint l )
{
    ( (QSInstanceData*) a->shVal() )->setValue( 0, a->env()->createNumber(l) );
}


#if QS_MAX_STACK>0
static int joinInternalRecursionDepth = 0;
#endif
/*!
  Returns a string concatenating all members of this array seperated
  by \a sep.
*/

QString QSArrayClass::joinInternal( const QSObject &obj, const QString &sep )
{
#if QS_MAX_STACK>0
    if( ++joinInternalRecursionDepth==QS_MAX_STACK ) {
	Q_ASSERT( obj.isValid() );
	obj.env()->throwError( RangeError,
                               QString::fromLatin1("Internal recursion level maxed out in: "
                                                   "QSArrayClass::joinInternal"), -1 );
	--joinInternalRecursionDepth;
	return QString::null;
    }
#endif
    uint len = length( &obj );
    QString str = QString::fromLatin1("");
    for ( uint i = 0; i < len; i++ ) {
	if ( i >= 1 )
	    str += sep;
	QSObject o = obj.get( QSString::from( i ) );
	if ( !o.isUndefined() && !o.isNull() )
	    str += o.toString();
    }
#if QS_MAX_STACK>0
    joinInternalRecursionDepth--;
#endif
    return str;
}

QSObject QSArrayClass::toPrimitive( const QSObject *obj,
				    const QSClass * ) const
{
    return createString( toString( obj ) );
}

QSObject QSArrayClass::concat( QSEnv *env )
{
    const QSList *args = env->arguments();
    QSArray result( env );
    int n = 0, len = 0;
    QSObject obj = env->thisValue();
    QSListIterator it = args->begin();
    for ( ;; ) {
	if ( obj.isA( "Array" ) ) {
	    len = length( &obj );
	    for( int k=0; k<len; ++k ) {
		QString p = QSString::from( k );
		if ( obj.hasProperty( p ) ) {
		    result.put( QSString::from( n ), obj.get( p ) );
		}
		++n;
	    }
	} else {
	    result.put( QSString::from( n ), obj );
	    n++;
	}
	if ( it == args->end() )
	    break;
	obj = it++;
    }
    result.put( QString::fromLatin1("length"), env->createNumber( n ) );
    return result;
}

/*!
  Returns a string contain all elements of array \a o joined by
  the first element in \a args or ',' if unspecified.
 */

QSObject QSArrayClass::join( QSEnv *env )
{
    QString separator = env->arg( 0 ).isDefined() ?
			env->arg( 0 ).toString() : QString::fromLatin1( "," );
    return env->createString( joinInternal( env->thisValue(), separator ) );
}

/*!
  Pop the last element of array \a o and return it. Returns undefined
  in case the array is empty.
*/

QSObject QSArrayClass::pop( QSEnv *env )
{
    QSObject t = env->thisValue();
    uint len = length( &t );
    if ( len == 0 ) {
	return env->createUndefined();
    } else {
	QSObject obj = env->thisValue();
	QString str = QSString::from( len - 1 );
	QSObject result = obj.get( str );
	obj.deleteProperty( str );
        setLength( &obj, len - 1 );
	return result;
    }
}

/*!
  Push all \a args to the end of array \a o.
 */

QSObject QSArrayClass::push( QSEnv *env )
{
    QSObject obj = env->thisValue();
    uint len = length( &obj );
    for ( int n = 0; n < env->numArgs(); n++ )
	obj.put( QSString::from( len + n ), env->arg( n ) );
    len += env->numArgs();
    setLength( &obj, len );
    return env->createNumber( len );
}

/*!
  Reverse the order of elements in array \a o and return the array.
*/

QSObject QSArrayClass::reverse( QSEnv *env )
{
    QSObject o = env->thisValue();
    uint len = length( &o );
    uint middle = len / 2;
    QSObject thisObj = env->thisValue();
    for ( uint k = 0; k < middle; k++ ) {
	QString str = QString::number( k );
	QString str2 = QString::number( len - k - 1 );
	QSObject obj = thisObj.get( str );
	QSObject obj2 = thisObj.get( str2 );
	if ( thisObj.hasProperty( str2 ) ) {
	    if ( thisObj.hasProperty( str ) ) {
		thisObj.put( str, obj2 );
		thisObj.put( str2, obj );
	    } else {
		thisObj.put( str, obj2 );
		thisObj.deleteProperty( str2 );
	    }
	} else {
	    if ( thisObj.hasProperty( str ) ) {
		thisObj.deleteProperty( str );
		thisObj.put( str2, obj );
	    } else {
		// why delete something that's not there ? Strange.
		thisObj.deleteProperty( str );
		thisObj.deleteProperty( str2 );
	    }
	}
    }
    return thisObj;
}

/*!
  Removes the first element from array \a o and returns it.
*/

QSObject QSArrayClass::shift( QSEnv *env )
{
    QSObject obj = env->thisValue();
    uint len = length( &obj );
    if ( len == 0 )
        return env->createUndefined();

    QSObject result = obj.get( QString::fromLatin1("0") );
    for( uint k = 1; k < len; k++ ) {
	QString str = QString::number( k );
	QString str2 = QString::number( k-1 );
	if ( obj.hasProperty( str ) )
	    obj.put( str2, obj.get( str ) );
	else
	    obj.deleteProperty( str2 );
    }
    obj.deleteProperty( QString::number( len - 1 ) );
    setLength( &obj, len - 1 );
    return result;
}

/*!
  Returns the elements \a args[0] up to (but excluding) \a args[1]
  from array \a o.
  ### meaning of negative indices
 */

QSObject QSArrayClass::slice( QSEnv *env )
{
    QSObject obj = env->thisValue();
    QSArray result( env );
    uint len = length( &obj );
    int begin = env->arg( 0 ).toUInt32();
    int end = len;
    if ( !env->arg( 1 ).isUndefined() )
	end = env->arg( 1 ).toUInt32();
    if ( end < 0 )
	end += len;
    // safety tests
    if ( begin < 0 || end < 0 || begin >= end ) {
	result.put( QString::fromLatin1("length"), env->createNumber( 0 ) );
	return result;
    }
    for( uint k = 0; k < (uint) end-begin; k++ ) {
	QString str = QString::number( k+begin );
	QString str2 = QString::number( k );
	if ( obj.hasProperty( str ) )
	    result.put( str2, obj.get( str ) );
    }
    result.put( QString::fromLatin1("length"), end - begin );
    return result;
}

static void quickSort( QSObject *obj, int start, int end )
{
    if( end - start < 1 )
	return;

    int pIndex = start + ( end - start ) / 2;

    QSObject pivot = obj->get( QString::number( pIndex ) );
    obj->put( QString::number( pIndex ), obj->get( QString::number( end ) ) );
    QString pivotString = pivot.toString();

    int low = start, high = end-1;
    QSObject lObj, hObj;

    while( low < high  ) {
	while( ( lObj = obj->get( QString::number( low ) ) ).toString() < pivotString
	       && low < high )
	    ++low;

	while( ( hObj = obj->get( QString::number( high ) ) ).toString() > pivotString
	       &&  high > low )
	    --high;

	if( low < high ) {
	    obj->put( QString::number( low ), hObj );
	    obj->put( QString::number( high ), lObj );
	    ++low;
	    --high;
	}
    }

    if( obj->get( QString::number( low ) ).toString() < pivotString )
	++low;

    obj->put( QString::number( end ), obj->get( QString::number( low ) ) );
    obj->put( QString::number( low ), pivot );

    quickSort( obj, start, low );
    quickSort( obj, low+1, end );

}


static inline int compare( const QSObject &a, const QSObject &b, QSObject *sort )
{
    QSList l;
    l.append( a );
    l.append( b );
    return int( sort->execute( l ).toNumber() );
}


static void quickSort( QSObject *obj, int start, int end, QSObject *sort )
{
    if( end - start < 1 )
	return;

    int pIndex = start + ( end - start ) / 2;
    QSObject pivot = obj->get( QString::number( pIndex ) );
    obj->put( QString::number( pIndex ), obj->get( QString::number( end ) ) );

    int low = start, high = end-1;

    QSObject lObj, hObj;

    while( low < high  ) {

	while( compare( lObj = obj->get( QString::number( low ) ), pivot, sort ) < 0
	       && low < high )
	    ++low;

	while( compare( hObj = obj->get( QString::number( high ) ), pivot, sort ) > 0
	       &&  high > low )
	    --high;

	if( low < high ) {
	    obj->put( QString::number( low ), hObj );
	    obj->put( QString::number( high ), lObj );
	    ++low;
	    --high;
	}
    }

    if( compare( obj->get( QString::number( low ) ), pivot, sort ) < 0 )
	++low;

    obj->put( QString::number( end ), obj->get( QString::number( low ) ) );
    obj->put( QString::number( low ), pivot );

    quickSort( obj, start, low, sort );
    quickSort( obj, low+1, end, sort );
}


/*!
  Sort the array \a o. If an argument is given it is used as a
  compare function.
*/
QSObject QSArrayClass::sort( QSEnv *env )
{
    QSObject sortFunction;
    bool useSortFunction = env->arg( 0 ).isDefined();
    if (useSortFunction) {
	sortFunction = env->arg( 0 );
	if ( !sortFunction.isExecutable() )
	    useSortFunction = false;
    }

    QSObject obj = env->thisValue();

    uint len = length( &obj );
    if ( len == 0 )
	return env->createUndefined();

    if( useSortFunction ) {
	quickSort( &obj, 0, len-1, &sortFunction );
    } else {
	quickSort( &obj, 0, len-1 );
    }

    return obj;
}

/*! Array.splice(start, deleteCount [,item1 [,item2 [, ...]]])
  \a deleteCount elements of array \a o at index \a start are
  replaced by the following arguments.
 */

QSObject QSArrayClass::splice( QSEnv *env )
{
    QSArray result( env );
    QSObject obj = env->thisValue();

    int k;
    int len = length( &obj );
    int start = env->arg( 0 ).toInteger();
    int del   = env->arg( 1 ).toInteger();
    start = ( start < 0 ) ? QMAX( len+start, 0 ) : QMIN( len, start );
    del = QMIN( QMAX( del, 0 ), len-start );
    for ( k=0; k<del; k++ ) {
	QString index = QString::number( k+start );
	if ( obj.hasProperty( index ) )
	    result.put( QString::number(k), obj.get( index ) );
    }
    int add = env->numArgs() - 2;
    if ( add>del ) {
	for ( k=len-del; k>start; k-- ) {
	    QString s39 = QString::number( k+del-1 );
	    QString s40 = QString::number( k+add-1 );
	    if ( obj.hasProperty( s39 ) )  // 42
		obj.put( s40, obj.get( s39 ) );
	    else  // 45
		obj.deleteProperty( s40 );
	}
    } else {
	for ( k=start; k<len-del; k++ ) {
	    QString s22 = QString::number( k+del );
	    QString s23 = QString::number( k+add );
	    if ( obj.hasProperty( s22 ) )  // 25
		obj.put( s23, obj.get( s22 ) );
	    else // 28
		obj.deleteProperty( s23 );
	} // 31
	for ( k=len; k>len-del+add; k-- )
	    obj.deleteProperty( QString::number( k-1 ) );
    }
    for( k=start; k<start+add; k++ )
	obj.put( QString::number(k), env->arg( k-start+2 ) );

    // Update length
    len = len - del + add;
    setLength(&obj, len < 0 ? 0 : len);

    return result;
}

/*!
  Prepend each element to the array \a o preserving order.
 */

QSObject QSArrayClass::unshift( QSEnv *env )
{
    QSObject obj = env->thisValue();
    uint len = length( &obj );
    int offset = env->numArgs();
    for ( uint k = len+offset-1; (int)k >= offset; k-- ) {
	QString str = QString::number( k );
	QString str2 = QString::number( k-offset );
	if ( obj.hasProperty( str2 ) )
	    obj.put( str, obj.get( str2 ) );
 	else
	    obj.deleteProperty( str );
    }
    for ( int n = 0; n < offset; n++ )
	obj.put( QSString::from( n ), env->arg( n ) );
    len += offset;
    setLength( &obj, len );
    return env->createNumber( len );
}
