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

#ifndef QUICKBYTEARRAYOBJECT_H
#define QUICKBYTEARRAYOBJECT_H

#include <qsclass.h>

class QUICKCORE_EXPORT QSByteArrayClass : public QSSharedClass {
public:
    QSByteArrayClass( QSClass *b );
    QString name() const { return QString::fromLatin1("ByteArray"); }

    virtual QSObject fetchValue( const QSObject *obj,
				 const QSMember &mem ) const;

    QString toString( const QSObject * ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;
    QString debugString( const QSObject *obj ) const;

    QSObject construct( const QSList &args ) const;
    QSObject construct( const QByteArray &ba ) const;

    QByteArray *byteArray( const QSObject *obj ) const;

    static QSObject toStringScript( QSEnv *env );
    static QSObject charAt( QSEnv *env );
};

#endif
