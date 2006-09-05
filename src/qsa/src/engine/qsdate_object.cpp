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

#include "qsdate_object.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "qsoperations.h"
#include <qdatetime.h>
#include <qregexp.h>
#include <time.h>

class QSDateShared : public QSShared
{
public:
    QDateTime dt;
};


QSDateShared *QSDateClass::dateShared( const QSObject *date )
{
    Q_ASSERT( date->objectType()->name() == QString::fromLatin1("Date") );
    return (QSDateShared*) date->shVal();
}

#define get_date( obj ) QSDateClass::dateShared( obj )->dt
#define get_date_env() ( (QSDateShared*) env->thisValue().shVal() )->dt

QSDateClass::QSDateClass( QSClass *b )
    : QSSharedClass( b, AttributeFinal )
{
    addMember( QString::fromLatin1("parse"), QSMember( &parse, AttributeStatic ) );

    addMember( QString::fromLatin1("toString"), QSMember( &toStringStatic ) );

    addMember( QString::fromLatin1("getTime"), QSMember( &getTime ) );
    addMember( QString::fromLatin1("getYear"), QSMember( &getYear ) );
    addMember( QString::fromLatin1("getMonth"), QSMember( &getMonth ) );
    addMember( QString::fromLatin1("getDate"), QSMember( &getDate ) );
    addMember( QString::fromLatin1("getDay"), QSMember( &getDay ) );
    addMember( QString::fromLatin1("getHours"), QSMember( &getHours ) );
    addMember( QString::fromLatin1("getMinutes"), QSMember( &getMinutes ) );
    addMember( QString::fromLatin1("getSeconds"), QSMember( &getSeconds ) );
    addMember( QString::fromLatin1("getMilliseconds"), QSMember( &getMilliSeconds ) );

    addMember( QString::fromLatin1("setTime"), QSMember( &setTime ) );
    addMember( QString::fromLatin1("setYear"), QSMember( &setYear ) );
    addMember( QString::fromLatin1("setMonth"), QSMember( &setMonth ) );
    addMember( QString::fromLatin1("setDate"), QSMember( &setDate ) );
    addMember( QString::fromLatin1("setHours"), QSMember( &setHours ) );
    addMember( QString::fromLatin1("setSeconds"), QSMember( &setSeconds ) );
    addMember( QString::fromLatin1("setMinutes"), QSMember( &setMinutes ) );
    addMember( QString::fromLatin1("setMilliseconds"), QSMember( &setMilliseconds ) );

#if 0
    addMember( QString::fromLatin1("utc"), QSMember( &utc, AttributeStatic ) );
    addMember( QString::fromLatin1("valueOf"), QSMember( &valueOf ) );
    addMember( QString::fromLatin1("toDateString"), QSMember( &toDateString ) );
    addMember( QString::fromLatin1("toTimeString"), QSMember( &toTimeString ) );
    addMember( QString::fromLatin1("toGMTString"), QSMember( &toGMTString ) );
    addMember( QString::fromLatin1("setUTCMonth"), QSMember( &setMonth ) );
    addMember( QString::fromLatin1("setUTCDate"), QSMember( &setDate ) );
    addMember( "setUTCMinutes", QSMember( &setMinutes ) );
    addMember( "setUTCSeconds", QSMember( &setSeconds ) );
    addMember( "setUTCMilliseconds", QSMember( &setMilliseconds ) );
    addMember( "getUTCMilliseconds", QSMember( &getMilliSeconds ) );
    addMember( "getUTCMonth", QSMember( &getMonth ) );
    addMember( "getUTCDate", QSMember( &getDate ) );
    addMember( "getUTCHours", QSMember( &getHours ) );
    addMember( "getUTCMinutes", QSMember( &getMinutes ) );
    addMember( "getUTCMinutes", QSMember( &getMinutes ) );
    addMember( "getUTCMilliseconds", QSMember( &getMilliSeconds ) );
#endif
}

QSObject QSDateClass::construct( const QSList &args ) const
{
    QSDateShared *shared = new QSDateShared;
    int numArgs = args.size();
    if( numArgs == 0 ) {
	shared->dt = QDateTime::currentDateTime();
    }
    if( numArgs == 1 ) {
	QSObject arg = args[0];
	if( arg.isString() ) {
	    shared->dt = QDateTime::fromString( arg.toString() );
	} else {
            Q_INT64 msecs = (Q_INT64) arg.toNumber();
	    shared->dt.setTime_t(time_t(msecs/1000));

            QTime time = shared->dt.time().addMSecs(int(msecs % 1000));
	    shared->dt.setTime(time);
	}
    } else if ( numArgs>1 ) {
	int year  = args[0].toInt32();
	int month = args[1].toInt32();
	int day   = numArgs >= 3 ? args[2].toInt32() : 1;
	int hour  = numArgs >= 4 ? args[3].toInt32() : 0;
	int min   = numArgs >= 5 ? args[4].toInt32() : 0;
	int sec   = numArgs >= 6 ? args[5].toInt32() : 0;
	int milli = numArgs >= 7 ? args[6].toInt32() : 0;
	shared->dt.setDate( QDate( year, month, day ) );
	shared->dt.setTime( QTime( hour, min, sec, milli ) );
    }

    return QSObject( this, shared );
}

QSObject QSDateClass::cast( const QSList &args ) const
{
    return construct( args );
}

QSEqualsResult QSDateClass::isEqual( const QSObject &a,
				     const QSObject &b ) const {
    return a.toNumber() == b.toNumber() ? EqualsIsEqual : EqualsNotEqual;
}

bool QSDateClass::toBoolean( const QSObject *obj ) const
{
    return get_date( obj ).isValid(); // ### Correct assumtion?
}

double QSDateClass::toNumber( const QSObject *obj ) const
{
    QDateTime dt = get_date( obj );
    return dt.toTime_t() * 1000.0 + dt.time().msec();
}

QString QSDateClass::toString( const QSObject *o ) const
{
    return get_date( o ).toString( Qt::ISODate );
}

QVariant QSDateClass::toVariant( const QSObject *obj, QVariant::Type t ) const
{
    switch( t ) {
    case QVariant::Time:
	return get_date( obj ).time();
    case QVariant::Date:
	return get_date( obj ).date();
    default:
	return get_date( obj );
    }
}


// Date.parse()
QSObject QSDateClass::parse( QSEnv *env )
{
    if ( env->arg( 0 ).isA( env->stringClass() ) ) {
        QRegExp re(QString::fromLatin1("(\\d\\d\\d\\d)-(\\d\\d)-(\\d\\d)(T(\\d\\d):(\\d\\d):(\\d\\d))?"));
        Q_ASSERT(re.isValid());
        QString str = env->arg(0).toString();
        if (re.search(str) < 0) {
            return env->throwError(QString::fromLatin1("Date.parse expect date on format:"
                                                       " YYYY-MM-DD or YYYY-MM-DDTHH:MM:SS"));
        }
        if (re.numCaptures() >= 5)
            str += QString::fromLatin1("T00:00:00");
	QDateTime dt = QDateTime::fromString(str, Qt::ISODate);
	return env->createNumber( dt.toTime_t() * 1000.0 );
    }
    return env->createUndefined();
}


// Date.UTC()
QSObject QSDateClass::utc( QSEnv *env )
{
    const QSList *args = env->arguments();
    int numArgs = args->size();
    if ( numArgs >= 2 ) {
	int year  = args->at( 0 ).toInt32();
	int month = args->at( 1 ).toInt32();
	int day   = numArgs >= 3 ? args->at( 2 ).toInt32() : 1;
	int hour  = numArgs >= 4 ? args->at( 3 ).toInt32() : 0;
	int min   = numArgs >= 5 ? args->at( 4 ).toInt32() : 0;
	int sec   = numArgs >= 6 ? args->at( 5 ).toInt32() : 0;
	int milli = numArgs >= 7 ? args->at( 6 ).toInt32() : 0;
	QDateTime dt;
	dt.setDate( QDate( year, month, day ) );
	dt.setTime( QTime( hour, min, sec, milli ) );
	return env->createNumber( dt.toTime_t() );
    }
    return env->createUndefined();
}

QSObject QSDateClass::toStringStatic( QSEnv *env )
{
    return env->createString( get_date_env().toString( Qt::ISODate ) );
}

QSObject QSDateClass::toDateString( QSEnv *env )
{
    return env->createString( get_date_env().toString() );
}

QSObject QSDateClass::toTimeString( QSEnv *env )
{
    return env->createString( get_date_env().toString() );
}

QSObject QSDateClass::toGMTString( QSEnv *env )
{
    return env->createString( get_date_env().toString() );
}

QSObject QSDateClass::valueOf( QSEnv *env )
{
    return env->createNumber( get_date_env().toTime_t() );
}

QSObject QSDateClass::getTime( QSEnv *env )
{
    QSObject obj = env->thisValue();
    QDateTime dt = get_date( &obj );
    return env->createNumber( dt.toTime_t() * 1000.0 + dt.time().msec() );
}

QSObject QSDateClass::getYear( QSEnv *env )
{
    return env->createNumber( get_date_env().date().year() );
}

QSObject QSDateClass::getMonth( QSEnv *env )
{
    return env->createNumber( get_date_env().date().month() );
}

QSObject QSDateClass::getDate( QSEnv *env )
{
    return env->createNumber( get_date_env().date().day() );
}

QSObject QSDateClass::getDay( QSEnv *env )
{
    return env->createNumber( get_date_env().date().dayOfWeek() );
}

QSObject QSDateClass::getHours( QSEnv *env )
{
    return env->createNumber( get_date_env().time().hour() );
}

QSObject QSDateClass::getMinutes( QSEnv *env )
{
    return env->createNumber( get_date_env().time().minute() );
}

QSObject QSDateClass::getSeconds( QSEnv *env )
{
    return env->createNumber( get_date_env().time().second() );
}

QSObject QSDateClass::getMilliSeconds( QSEnv *env )
{
    return env->createNumber( get_date_env().time().msec() );
}

QSObject QSDateClass::setTime( QSEnv *env )
{
    QDateTime &dt = get_date_env();

    Q_INT64 msecs = (Q_INT64) env->arg( 0 ).toNumber();
    dt.setTime_t( (time_t) (msecs / 1000) );

    QTime time = dt.time().addMSecs(int(msecs % 1000));
    dt.setTime(time);

    return env->thisValue();
}


QSObject QSDateClass::setMilliseconds( QSEnv *env )
{
    QDateTime &dt = get_date_env();
    QTime time = dt.time();
    dt.setTime( QTime( time.hour(),
		       time.minute(),
		       time.second(),
		       (int) env->arg(0).toNumber() ) );
    return env->thisValue();
}


QSObject QSDateClass::setSeconds( QSEnv *env )
{
    QDateTime &dt = get_date_env();
    QTime time = dt.time();
    dt.setTime( QTime( time.hour(),
		       time.minute(),
		       (int) env->arg(0).toNumber(),
		       time.msec() ) );
    return env->thisValue();
}


QSObject QSDateClass::setMinutes( QSEnv *env )
{
    QDateTime &dt = get_date_env();
    QTime time = dt.time();
    dt.setTime( QTime( time.hour(),
		       (int) env->arg( 0 ).toNumber(),
		       time.second(),
		       time.msec() ) );
    return env->thisValue();
}


QSObject QSDateClass::setHours( QSEnv *env )
{
    QDateTime &dt = get_date_env();
    QTime time = dt.time();
    dt.setTime( QTime( (int) env->arg( 0 ).toNumber(),
		       time.minute(),
		       time.second(),
		       time.msec() ) );
    return env->thisValue();
}


QSObject QSDateClass::setDate( QSEnv *env )
{
    QDateTime &dt = get_date_env();
    QDate date = dt.date();
    dt.setDate( QDate( date.year(),
		       date.month(),
		       (int) env->arg( 0 ).toNumber() ) );
    return env->thisValue();
}

QSObject QSDateClass::setMonth( QSEnv *env )
{
    QDateTime &dt = get_date_env();
    QDate date = dt.date();
    dt.setDate( QDate( date.year(),
		       (int) env->arg( 0 ).toNumber(),
		       date.day() ) );
    return env->thisValue();
}

QSObject QSDateClass::setYear( QSEnv *env )
{
    QDateTime &dt = get_date_env();
    QDate date = dt.date();
    dt.setDate( QDate( (int) env->arg( 0 ).toNumber(),
		       date.month(),
		       date.day() ) );
    return env->thisValue();
}

QSObject QSDateClass::construct(const QDateTime &dt) const
{
    QSDateShared *sh = new QSDateShared;
    sh->dt = dt;
    return QSObject(this, sh);
}
