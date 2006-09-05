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

#ifndef QSARRAY_OBJECT_H
#define QSARRAY_OBJECT_H

#include "qsclass.h"

class QSArrayClass : public QSWritableClass {
public:
    QSArrayClass( QSClass *b );
    QString name() const { return QString::fromLatin1("Array"); }

    QString toString( const QSObject * ) const;
    QSObject toPrimitive( const QSObject *obj, const QSClass *cl ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;

    QSObject construct( const QSList &args ) const;
    QSObject cast( const QSList &args ) const;

    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;

    static uint length( const QSObject * );
    static void setLength( QSObject *, uint l );

    static QString joinInternal( const QSObject &obj, const QString &sep );

    static QSObject concat( QSEnv * );
    static QSObject join( QSEnv * );
    static QSObject pop( QSEnv * );
    static QSObject push( QSEnv * );
    static QSObject reverse( QSEnv * );
    static QSObject shift( QSEnv * );
    static QSObject slice( QSEnv * );
    static QSObject sort( QSEnv * );
    static QSObject splice( QSEnv * );
    static QSObject unshift( QSEnv * );
};

#endif
