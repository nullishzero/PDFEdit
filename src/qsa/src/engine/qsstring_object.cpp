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

#include "qsregexp_object.h"
#include "qsoperations.h"
#include "qstypes.h"
#include "qsclass.h"
#include "qsenv.h"
#include <qregexp.h>

using namespace QS;

QSStringClass::QSStringClass( QSClass *b )
    : QSClass( b, AttributeFinal )
{
}

void QSStringClass::init()
{
    // custom property
    addMember( QString::fromLatin1("length"), QSMember( QSMember::Custom, 0,
				   AttributeNonWritable ) );

    // static function
    addMember( QString::fromLatin1("fromCharCode"), QSMember( &fromCharCode, AttributeStatic ) );

    // member functions
    addMember( QString::fromLatin1("toString"), QSMember( &QSStringClass::toStringScript ) );
    addMember( QString::fromLatin1("valueOf"), QSMember( &QSStringClass::toStringScript ) );
    addMember( QString::fromLatin1("charAt"), QSMember( &charAt ) );
    addMember( QString::fromLatin1("charCodeAt"), QSMember( &charCodeAt ) );
    addMember( QString::fromLatin1("indexOf"), QSMember( &indexOf ) );
    addMember( QString::fromLatin1("lastIndexOf"), QSMember( &lastIndexOf ) );
    addMember( QString::fromLatin1("match"), QSMember( &match ) );
    addMember( QString::fromLatin1("search"), QSMember( &find ) );
    addMember( QString::fromLatin1("searchRev"), QSMember( &findRev ) );
    addMember( QString::fromLatin1("replace"), QSMember( &replace ) );
    addMember( QString::fromLatin1("split"), QSMember( &split ) );
    addMember( QString::fromLatin1("substring"), QSMember( &substr ) );
    addMember( QString::fromLatin1("toLowerCase"), QSMember( &toLowerCase ) );
    addMember( QString::fromLatin1("lower"), QSMember( &toLowerCase ) );
    addMember( QString::fromLatin1("toUpperCase"), QSMember( &toUpperCase ) );
    addMember( QString::fromLatin1("upper"), QSMember( &toUpperCase ) );
    addMember( QString::fromLatin1("isEmpty"), QSMember( &isEmpty ) );
    addMember( QString::fromLatin1("left"), QSMember( &left ) );
    addMember( QString::fromLatin1("mid"), QSMember( &mid ) );
    addMember( QString::fromLatin1("right"), QSMember( &right ) );
    addMember( QString::fromLatin1("find"), QSMember( &find ) );
    addMember( QString::fromLatin1("findRev"), QSMember( &findRev ) );
    addMember( QString::fromLatin1("startsWith"), QSMember( &startsWith ) );
    addMember( QString::fromLatin1("endsWith"), QSMember( &endsWith ) );
    addMember( QString::fromLatin1("argInt"), QSMember( &argInt ) );
    addMember( QString::fromLatin1("argDec"), QSMember( &argDec ) );
    addMember( QString::fromLatin1("argStr"), QSMember( &argStr ) );
    addMember( QString::fromLatin1("arg"), QSMember( &arg ) );
}

void QSStringClass::ref( QSObject *o ) const
{
    o->val.str->ref();
}

void QSStringClass::deref( QSObject *o ) const
{
    o->val.str->deref();
    if ( o->val.str->count == 0 ) {
	delete o->val.str;
    }
}

QSObject QSStringClass::fetchValue( const QSObject *objPtr,
				    const QSMember &mem ) const
{
    if ( mem.type() == QSMember::Custom )
	if ( mem.idx == 0 ) {
	    return createNumber( objPtr->sVal().length() );
	} else {
	    qFatal( "QSStringClass::fetchValue: unhandled member" );
	    return createUndefined();
	}
    else
	return QSClass::fetchValue( objPtr, mem );
}

bool QSStringClass::toBoolean( const QSObject *obj ) const
{
    return !obj->sVal().isEmpty();
}

double QSStringClass::toNumber( const QSObject *obj ) const
{
    return QSString::toDouble( obj->sVal() );
}

QString QSStringClass::toString( const QSObject *obj ) const
{
    return obj->sVal();
}

QSObject QSStringClass::toPrimitive( const QSObject *obj,
				     const QSClass * ) const
{
    return *obj;
}

QVariant QSStringClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return obj->sVal();
}

QSObject QSStringClass::construct( const QSList &args ) const
{
    QString s = args.isEmpty() ? QString::fromLatin1( "" ) : args[0].toString();

    return createString( s );
}

/*!
  \reimp
*/
QSObject QSStringClass::cast( const QSList &args ) const
{
    return construct( args );
}

QSEqualsResult QSStringClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    Q_ASSERT( a.isA( this ) );
    if ( b.isString() )
	return (QSEqualsResult) (a.sVal() == b.sVal() || (a.sVal().isEmpty() && b.sVal().isEmpty()));
    else if ( b.isNumber() )
	return ( QSEqualsResult ) ( a.sVal() == b.toString() );
    else if ( !b.isPrimitive() )
	return isEqual( a, b.toPrimitive() );
    else
	return EqualsUndefined;

}

// String.fromCharCode()
QSObject QSStringClass::fromCharCode( QSEnv *env )
{
    const QSList *args = env->arguments();
    QString s;
    if ( args->size() ) {
	s.setLength( args->size() );
	QSListIterator it = args->begin();
	int i = 0;
	while ( it != args->end() ) {
	    ushort u = it->toUInt16();
	    s[i] = QChar( u );
	    it++;
	    i++;
	}
    } else {
	s = "";
    }

    return env->createString( s );
}

QSObject QSStringClass::toStringScript( QSEnv *env )
{
    return env->createString( env->thisValue().sVal() );
}

QSObject QSStringClass::charAt( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    int pos = env->arg( 0 ).toInteger();
    if ( pos < 0 || pos >= (int)s.length() )
	s = "";
    else
	s = s.mid( pos, 1 );
    return env->createString( s );
}

QSObject QSStringClass::charCodeAt( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    int pos = env->arg( 0 ).toInteger();
    double d;
    if ( pos < 0 || pos >= (int)s.length())
	d = NaN;
    else
	d = s[pos].unicode();
    return env->createNumber( d );
}

QSObject QSStringClass::indexOf( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    QSObject arg0 = env->arg(0);
    if ( arg0.objectType() == env->regexpClass() ) {
	QRegExp *reg = QSRegExpClass::regExp(&arg0);
	return env->createNumber( reg->search( s ) );
    } else if ( arg0.objectType() == env->stringClass() ) {
	QString s2 = arg0.toString();
	int pos = env->numArgs() >= 2 ? env->arg( 1 ).toInteger() : 0;
	if ( pos < 0 )
	    pos = 0;
	return env->createNumber( s.find( s2, pos ) );
    }
    return env->createUndefined();
}

QSObject QSStringClass::lastIndexOf( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    QString s2 = env->arg( 0 ).toString();
    int pos = env->numArgs() >= 2 ? env->arg( 1 ).toInteger() : s.length() - 1;
    return env->createNumber( s.findRev( s2, pos ) );
}

QSObject QSStringClass::match( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    QSObject arg0 = env->arg(0);
    if ( arg0.objectType() == env->regexpClass() ) {
	QRegExp *reg = QSRegExpClass::regExp(&arg0);

	int spos = reg->search(s);
	if (spos == -1) // No match
	    return env->createUndefined();

	if (QSRegExpClass::isGlobal(&arg0)) {
	    QSArray lst(env);
	    int index = 0;
	    while (spos>=0) {
		lst.put(QString::number(index++), env->createString(reg->cap()));
		spos = reg->search(s, spos+1);
	    }
	    if (index == 1)  // only one element, return it
		return lst.get(QString::number(0));
	    return lst;
	} else {
	    return env->createString(reg->cap());
	}
	env->regexpClass()->lastCaptures = reg->capturedTexts();
	QString mstr = reg->cap();
	if ( mstr.isNull() )
	// ### return an array, with the matches
	return env->createString( mstr );
    }
    return env->createUndefined();
}

QSObject QSStringClass::replace( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    int pos, len;
    QSObject arg0 = env->arg(0);
    if ( arg0.objectType() == env->regexpClass() ) {
	QRegExp *reg = QSRegExpClass::regExp(&arg0);
	bool global = QSRegExpClass::isGlobal(&arg0);
	if (global) {
	    QString result = s.replace(*reg, env->arg( 1 ).toString());
	    return env->createString( result );
	}
	pos = reg->search( s );
	len = reg->matchedLength();
    } else {
	QString s2 = arg0.toString();
	pos = s.find( s2 );
	len = s2.length();
    }
    if ( pos == -1 )
	return env->createString( s );
    QString r = s.mid( 0, pos) + env->arg( 1 ).toString() + s.mid( pos + len );
    return env->createString( r );
}

QSObject QSStringClass::split( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    QSArray result( env );
    int i = 0, p0 = 0;
    double d = env->numArgs() >= 2 ? env->arg( 1 ).toInteger() : -1; // optional max
    QSObject arg0 = env->arg(0);
    if ( arg0.objectType() == env->regexpClass()) {
	QRegExp *reg = QSRegExpClass::regExp(&arg0);
	if ( s.isEmpty() && reg->search( s, 0 ) >= 0) {
	    // empty string matched by regexp -> empty array
	    result.put( QString::fromLatin1("length"), env->createNumber( 0 ) );
	    return result;
	}
	int pos = 0;
	for ( ;; ) {
	    /* TODO: back references */
	    int mpos = reg->search( s, pos );
	    if ( mpos < 0 )
		break;
	    QString mstr = reg->cap( 0 );
	    pos = mpos + ( mstr.isEmpty() ? 1 : mstr.length() );
	    if ( mpos != p0 || !mstr.isEmpty() ) {
		result.put( QString::number( i ),
			    env->createString( s.mid( p0, mpos-p0 ) ));
		p0 = mpos + mstr.length();
		i++;
	    }
	}
    } else if ( env->numArgs() >= 1 ) {
	QString u2 = arg0.toString();
	if ( u2.isEmpty() ) {
	    if ( s.isEmpty() ) {
		// empty separator matches empty string -> empty array
		result.put( QString::fromLatin1("length"), env->createNumber( 0 ) );
		return result;
	    } else {
		while ( i != d && i < (int)s.length() )
		    result.put( QString::number( i++ ),
				env->createString( s.mid( p0++, 1 ) ) );
	    }
	} else {
	    int pos;
	    while (i != d && (pos = s.find(u2, p0)) >= 0) {
		result.put( QString::number( i ),
			    env->createString( s.mid( p0, pos-p0 ) ) );
		p0 = pos + u2.length();
		i++;
	    }
	}
    }
    // add remaining string, if any
    if ( i != d && (p0 <= (int)s.length() || i == 0) )
	result.put( QSString::from( i++ ), env->createString( s.mid( p0 ) ) );
    result.put( QString::fromLatin1("length"), i );
    return result;
}

QSObject QSStringClass::substr( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    uint len = s.length();
    int n = env->arg( 0 ).toInteger();
    int m = env->arg( 1 ).toInteger();
    double d = QMIN( QMAX( n, 0 ), (int)len );
    double d2;
    if ( env->numArgs() < 2 ) {
	d2 = len - d;
    } else {
	d2 = min( max( m, 0), len );
	d2 = max(d2-d, 0);
    }
    return env->createString( s.mid((int)d, (int)d2 ) );
}

QSObject QSStringClass::toLowerCase( QSEnv *env )
{
    return env->createString( env->thisValue().sVal().lower() );
}

QSObject QSStringClass::toUpperCase( QSEnv *env )
{
    return env->createString( env->thisValue().sVal().upper() );
}

QSObject QSStringClass::isEmpty( QSEnv *env )
{
    return env->createBoolean( env->thisValue().sVal().isEmpty() );
}

QSObject QSStringClass::left( QSEnv *env )
{
    return env->createString( env->thisValue().sVal().left( env->arg( 0 ).toInteger() ) );
}

QSObject QSStringClass::mid( QSEnv *env )
{
    int len = -1;
    if ( env->arg( 1 ).isNumber() )
	len = env->arg( 1 ).toInteger();
    return env->createString( env->thisValue().sVal().mid( env->arg( 0 ).toInteger(), len ) );
}

QSObject QSStringClass::right( QSEnv *env )
{
    return env->createString( env->thisValue().sVal().right( env->arg( 0 ).toInteger() ) );
}

QSObject QSStringClass::find( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    int idx = env->numArgs() >= 2 ? env->arg( 1 ).toInteger() : 0;
    QSObject arg0 = env->arg(0);
    if ( arg0.objectType() == env->regexpClass() ) {
	QRegExp *re = QSRegExpClass::regExp(&arg0);
	return env->createNumber( s.find( *re, idx ) );
    } else {
	bool cs = env->numArgs() >= 3 ? env->arg( 2 ).toBoolean() : TRUE;
	return env->createNumber( s.find(arg0.toString(), idx, cs) );
    }
}

QSObject QSStringClass::findRev( QSEnv *env )
{
    QString s = env->thisValue().sVal();
    int idx = env->numArgs() >= 2 ? env->arg( 1 ).toInteger() : -1;
    QSObject arg0 = env->arg(0);
    if ( arg0.objectType() == env->regexpClass() ) {
	QRegExp *re = QSRegExpClass::regExp(&arg0);
	return env->createNumber(s.findRev(*re, idx));
    } else {
	bool cs = env->numArgs() >= 3 ? env->arg( 2 ).toBoolean() : TRUE;
	return env->createNumber( s.findRev(arg0.toString(), idx, cs ));
    }
}

QSObject QSStringClass::startsWith( QSEnv *env )
{
    return env->createBoolean( env->thisValue().sVal().startsWith( env->arg( 0 ).toString() ) );
}

QSObject QSStringClass::endsWith( QSEnv *env )
{
    return env->createBoolean( env->thisValue().sVal().endsWith( env->arg( 0 ).toString() ) );
}

QSObject QSStringClass::argInt( QSEnv *env )
{
    int nargs = env->numArgs();
    if (nargs == 0)
	return env->throwError(QString::fromLatin1("Missing argument to function argInt"));

    double val = env->arg(0).toNumber();
    int width = 0;
    int radix = 10;

    if (nargs >= 2) {
	double tmpwidth = env->arg(1).toNumber();
	if (!isNaN(tmpwidth))
	    width = (int) tmpwidth;
    }

    if (isNaN(val))
	return env->createString(env->thisValue().sVal().arg(QString::fromLatin1("NaN"), width));

    if (nargs >= 3) {
	double tmpradix = env->arg(2).toNumber();
	if (!isNaN(tmpradix))
	    radix = (int) tmpradix;
    }

    return env->createString(env->thisValue().sVal().arg((int) val, width, radix));
}

QSObject QSStringClass::argDec( QSEnv *env )
{
    int nargs = env->numArgs();

    if (nargs == 0)
	return env->throwError(QString::fromLatin1("Missing argument to function argDec"));

    double val = env->arg(0).toNumber();
    int width = 0;
    char format = 'g';
    int prec = -1;

    if (nargs >= 2) {
	double tmpwidth = env->arg(1).toNumber();
	if (!isNaN(tmpwidth))
	    width = (int) tmpwidth;
    }

    if (isNaN(val))
	return env->createString(env->thisValue().sVal().arg(QString::fromLatin1("NaN"), width));

    if (nargs>=3) {
	QString s = env->arg(2).toString();
	char c = s.at(0).latin1();
	if (!s.isEmpty() && (c == 'e' || c == 'E' || c == 'f' || c == 'g' || c == 'G'))
	    format = c;
    }

    if (nargs >= 4) {
	double tmpprec = env->arg(3).toNumber();
	if (!isNaN(tmpprec))
	    prec = (int) tmpprec;
    }

    return env->createString( env->thisValue().sVal().arg((double)val, width, format, prec));
}


QSObject QSStringClass::argStr( QSEnv *env )
{
    int nargs = env->numArgs();

    if (nargs == 0)
	return env->throwError(QString::fromLatin1("Missing argument to function argStr"));

    int width = 0;

    if (nargs >= 2) {
	double tmpwidth = env->arg(1).toNumber();
	if (!isNaN(tmpwidth))
	    width = (int) tmpwidth;
    }

    return env->createString( env->thisValue().sVal().arg(env->arg(0).toString(), width));
}

QSObject QSStringClass::arg( QSEnv *env )
{
    int nargs = env->numArgs();

    if (nargs == 0)
	return env->throwError(QString::fromLatin1("Missing argument to function arg"));

    int width = 0;
    if (nargs >= 2) {
	double tmpwidth = env->arg(1).toNumber();
	if (!isNaN(tmpwidth))
	    width = (int) tmpwidth;
    }

    QSObject arg0 = env->arg(0);
    if (arg0.objectType() == env->numberClass())
	return env->createString( env->thisValue().sVal().arg(arg0.toNumber(), width) );
    return env->createString( env->thisValue().sVal().arg(arg0.toString(), width));
}
