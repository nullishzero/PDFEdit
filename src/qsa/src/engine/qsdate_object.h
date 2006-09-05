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

#ifndef QSDATE_OBJECT_H
#define QSDATE_OBJECT_H

#include "qsclass.h"
#include "qsobject.h"
#include "qsfunction.h"

class QSDateShared;

class QSDateClass : public QSSharedClass {
public:
    QSDateClass( QSClass *b );
    QString name() const { return QString::fromLatin1("Date"); }

    bool toBoolean( const QSObject *obj ) const;
    double toNumber( const QSObject *obj ) const;
    QString toString( const QSObject * ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;

    QSObject construct( const QSList &args ) const;
    QSObject cast( const QSList &args ) const;
    QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;

    QSObject construct(const QDateTime &) const;

    // static Date functions
    static QSObject parse( QSEnv *env );
    static QSObject utc( QSEnv *env );
    static QSObject toStringStatic( QSEnv *env );
    static QSObject toDateString( QSEnv *env );
    static QSObject toTimeString( QSEnv *env );
    static QSObject toGMTString( QSEnv *env );
    static QSObject valueOf( QSEnv *env );
    static QSObject getTime( QSEnv *env );
    static QSObject getYear( QSEnv *env );
//     static QSObject getFullYear( QSEnv *env );
    static QSObject getMonth( QSEnv *env );
    static QSObject getDate( QSEnv *env );
    static QSObject getDay( QSEnv *env );
    static QSObject getHours( QSEnv *env );
    static QSObject getMinutes( QSEnv *env );
    static QSObject getSeconds( QSEnv *env );
    static QSObject getMilliSeconds( QSEnv *env );
//     static QSObject getTimezoneOffset( QSEnv *env );
    static QSObject setTime( QSEnv *env );
    static QSObject setMilliseconds( QSEnv *env );
    static QSObject setSeconds( QSEnv *env );
    static QSObject setMinutes( QSEnv *env );
    static QSObject setHours( QSEnv *env );
    static QSObject setDate( QSEnv *env );
    static QSObject setMonth( QSEnv *env );
//     static QSObject setFullYear( QSEnv *env );
    static QSObject setYear( QSEnv *env );

protected:
    static QSObject invokeS( QSEnv *env, int id );

    static QSDateShared *dateShared( const QSObject *date );
};

#endif
