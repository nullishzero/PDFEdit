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
#include "qsenv.h"
#include "qstypes.h"
#include "qsinternal.h"
#include "qsarray_object.h"
#include <qregexp.h>

class QSRegExpShared : public QSWritable
{
public:
    QSRegExpShared( const QString &pat, bool ic, bool glob )
	: reg( pat, !ic ),
	  source(pat),
	  global(glob),
	  ignoreCase(ic)
    {
    }

    QRegExp reg;
    QString source;
    bool global;
    bool ignoreCase;
};

enum { Valid, Empty, MLength, CTexts, Source, Global, IgnoreCase, LastIndex };

QSRegExpClass::QSRegExpClass( QSClass *b )
    : QSWritableClass( b )
{
    int a = AttributeNonWritable;
    addMember( QString::fromLatin1("valid"), QSMember( QSMember::Custom, Valid, a ) );
    addMember( QString::fromLatin1("empty"), QSMember( QSMember::Custom, Empty, a ) );
    addMember( QString::fromLatin1("matchedLength"), QSMember( QSMember::Custom, MLength, a ) );
    addMember( QString::fromLatin1("capturedTexts"), QSMember( QSMember::Custom, CTexts, a ) );
    addMember( QString::fromLatin1("source"), QSMember( QSMember::Custom, Source, a ) );
    addMember( QString::fromLatin1("global"), QSMember( QSMember::Custom, Global, AttributePublic ) );
    addMember( QString::fromLatin1("ignoreCase"), QSMember( QSMember::Custom, IgnoreCase, AttributePublic ) );

    addMember( QString::fromLatin1("toString"), QSMember( &QSRegExpClass::toStringScript ) );
    addMember( QString::fromLatin1("search"), QSMember( &search ) );
    addMember( QString::fromLatin1("searchRev"), QSMember( &searchRev ) );
    addMember( QString::fromLatin1("exactMatch"), QSMember( &exactMatch ) );
    addMember( QString::fromLatin1("pos"), QSMember( &pos ) );
    addMember( QString::fromLatin1("cap"), QSMember( &cap ) );
}

/*!
 * Convenience function that retrieves a pointer to the QRegExp
 * object contained in \a obj. Only to be used if \a obj is of
 * type RegExp.
 */

QRegExp *QSRegExpClass::regExp( const QSObject *obj )
{
    Q_ASSERT( obj->typeName() == QString::fromLatin1("RegExp") );
    return &((QSRegExpShared*)obj->shVal())->reg;
}

/*!
  \internal
  \overload
*/

QRegExp* QSRegExpClass::regExp( QSEnv *e )
{
    QSObject t = e->thisValue();
    Q_ASSERT( t.isA( e->regexpClass() ) );
    return &((QSRegExpShared*)t.shVal())->reg;
}

/*! \reimp */
QSObject QSRegExpClass::cast( const QSList &args ) const
{
    return construct( args );
}

QSObject QSRegExpClass::construct( const QSList &args ) const
{
    // ### regexp arguments
    QString p = args.isEmpty() ? QString::fromLatin1( "" ) : args[0].toString();

    if (args.size() < 2) {
	QSRegExpShared *sh = new QSRegExpShared(p, FALSE, FALSE);
	return env()->createShared(this, sh);
    }

    QString flags = args[1].toString();
    bool ignoreCase = (flags.find(QString::fromLatin1("i")) >= 0);
    bool global = (flags.find(QString::fromLatin1("g")) >= 0);
    QSRegExpShared *sh = new QSRegExpShared(p, ignoreCase, global); // ### apply flags
    return env()->createShared(this, sh);
}

QSObject QSRegExpClass::fetchValue( const QSObject *objPtr,
				    const QSMember &mem ) const
{
    if ( mem.type() != QSMember::Custom )
	return QSWritableClass::fetchValue( objPtr, mem );

    QRegExp *re = regExp( objPtr );
    switch ( mem.index() ) {
    case Valid:
	return createBoolean( re->isValid() );
    case Empty:
	return createBoolean( re->isEmpty() );
    case MLength:
	return createNumber( re->matchedLength() );
    case Source:
	return createString( source(objPtr) );
    case Global:
	return createBoolean( isGlobal(objPtr) );
    case IgnoreCase:
	return createBoolean( isIgnoreCase(objPtr) );
    case CTexts: {
 	QSArray array( env() );
 	QStringList ct = re->capturedTexts();
 	QStringList::ConstIterator it = ct.begin();
 	int i = 0;
 	for ( ; it != ct.end(); ++it, ++i )
 	    array.put( QString::number( i ), createString( *it ) );
	array.put( QString::fromLatin1("length"), createNumber( i ) );
 	return array;
    }
    default:
	return createUndefined();
    }
}

void QSRegExpClass::write(QSObject *objPtr, const QSMember &mem,
			  const QSObject &val ) const
{
    if (mem.type() != QSMember::Custom) {
	QSWritableClass::write(objPtr, mem, val);
	return;
    }

    Q_ASSERT(objPtr->objectType() == objPtr->objectType()->env()->regexpClass());

    switch (mem.index()) {
    case Source:
	((QSRegExpShared*)objPtr->shVal())->source = val.toString();
	break;
    case Global:
	((QSRegExpShared*)objPtr->shVal())->global = val.toBoolean();
	break;
    case IgnoreCase:
	{
	    bool ic = val.toBoolean();
	    ((QSRegExpShared*)objPtr->shVal())->ignoreCase = ic;
	    ((QSRegExpShared*)objPtr->shVal())->reg.setCaseSensitive(!ic);
	}
	break;
    default:
	QSWritableClass::write(objPtr, mem, val);
    }
}

#if 0
bool QSRegExpClass::hasStaticProperty( const QString &p ) const
{
    return ( p.length() == 2 && p[0] == '$' && p[1].isDigit() ) ||
	   ( p.length() == 3 && p[0] == '$' && p[1].isDigit() &&
	     p[2].isDigit() );
}

QSObject QSRegExpClass::getStatic( const QString &p ) const
{
    if ( ( p.length() == 2 && p[0] == '$' && p[1].isDigit() ) ||
	 ( p.length() == 3 && p[0] == '$' && p[1].isDigit() &&
	   p[2].isDigit() ) ) {
	int i = p.mid( 1 ).toInt();
	if ( i < (int)lastCaptures.count() )
	    return createString( lastCaptures[ i ] );
	else
	    return createString( "" );
    }

    return createUndefined();
}
#endif

QString QSRegExpClass::toString( const QSObject *obj ) const
{
    return QString::fromLatin1("/") + regExp( obj )->pattern() + QString::fromLatin1("/");
}

QSObject QSRegExpClass::exec( QSEnv *env )
{
    QSObject obj = env->thisValue();
    QRegExp *re = regExp( &obj );
    QString s = env->arg( 0 ).toString();
    uint length = s.length();
    int i = obj.get( QString::fromLatin1("lastIndex") ).toInt32();
    QSObject tmp = obj.get( QString::fromLatin1("global") );
    if ( !tmp.toBoolean() )
	i = 0;
    if ( i < 0 || i > (int)length ) {
	obj.put( QString::fromLatin1("lastIndex"), 0 );
	return env->createNull();
    }
    i = re->search( s, i );
    obj.env()->regexpClass()->lastCaptures = re->capturedTexts();
    // ### complete
    return env->createString( re->cap(0) );
}

QSObject QSRegExpClass::test( QSEnv *env )
{
    QSObject obj = env->thisValue();
    QRegExp *re = regExp( &obj );
    QString s = env->arg( 0 ).toString();
    uint length = s.length();
    int i = obj.get( QString::fromLatin1("lastIndex") ).toInt32();
    QSObject tmp = obj.get( QString::fromLatin1("global") );
    if ( !tmp.toBoolean() )
	i = 0;
    if ( i < 0 || i >(int) length ) {
	obj.put( QString::fromLatin1("lastIndex"), 0 );
	return env->createBoolean( FALSE );
    }
    i = re->search( s, i );
    obj.env()->regexpClass()->lastCaptures = re->capturedTexts();
    return env->createBoolean( i >= 0 );
}

QSObject QSRegExpClass::toStringScript( QSEnv *env )
{
    QSObject that = env->thisValue();
    Q_ASSERT(that.objectType() == env->regexpClass());
    QString pattern = QString::fromLatin1("/") + source(&that) + QString::fromLatin1("/");
    if (isIgnoreCase(&that))
	pattern += 'i';
    if (isGlobal(&that))
	pattern += 'g';
    return env->createString(pattern);
}

QSObject QSRegExpClass::search( QSEnv *env )
{
    int start = env->numArgs() >= 2 ? env->arg( 1 ).toInteger() : 0;
    return env->createNumber( regExp( env )->search( env->arg( 0 ).toString(), start ) );
}

QSObject QSRegExpClass::searchRev( QSEnv *env )
{
    int start = env->numArgs() >= 2 ? env->arg( 1 ).toInteger() : -1;
    return env->createNumber( regExp( env )->searchRev( env->arg( 0 ).toString(), start ) );
}

QSObject QSRegExpClass::exactMatch( QSEnv *env )
{
    return env->createBoolean( regExp( env )->exactMatch( env->arg( 0 ).toString() ) );
}

QSObject QSRegExpClass::pos( QSEnv *env )
{
    return env->createNumber( regExp( env )->pos( env->numArgs() >= 1 ?
					 env->arg( 0 ).toInteger() : 0 ) );
}

QSObject QSRegExpClass::cap( QSEnv *env )
{
    return env->createString( regExp( env )->cap( env->numArgs() >= 1 ?
					 env->arg( 0 ).toInteger() : 0 ) );
}

QString QSRegExpClass::source(const QSObject *re)
{
    Q_ASSERT(re->objectType() == re->objectType()->env()->regexpClass());
    return ( (QSRegExpShared*) re->shVal() )->source;
}

bool QSRegExpClass::isGlobal(const QSObject *re)
{
    Q_ASSERT(re->objectType() == re->objectType()->env()->regexpClass());
    return ( (QSRegExpShared*) re->shVal() )->global;
}

bool QSRegExpClass::isIgnoreCase(const QSObject *re)
{
    Q_ASSERT(re->objectType() == re->objectType()->env()->regexpClass());
    return ( (QSRegExpShared*) re->shVal() )->ignoreCase;
}
