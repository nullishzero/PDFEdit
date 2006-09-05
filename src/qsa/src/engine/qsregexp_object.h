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

#ifndef QSREGEXP_OBJECT_H
#define QSREGEXP_OBJECT_H

#include "qsclass.h"
#include <qstringlist.h>

class QSRegExpClass : public QSWritableClass {
public:
    QSRegExpClass( QSClass *b );
    QString name() const { return QString::fromLatin1("RegExp"); }

    virtual QSObject fetchValue( const QSObject *objPtr,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;


    QString toString( const QSObject *obj ) const;

    QSObject construct( const QSList &args ) const;
    QSObject cast( const QSList &args ) const;

    static QRegExp *regExp( const QSObject *obj );

    // ECMA API
    static QSObject exec( QSEnv *env );
    static QSObject test( QSEnv *env );
    static QSObject toStringScript( QSEnv *env );
    // QRegExp API
    static QSObject search( QSEnv *env );
    static QSObject searchRev( QSEnv *env );
    static QSObject exactMatch( QSEnv *env );
    static QSObject pos( QSEnv *env );
    static QSObject cap( QSEnv *env );

    QStringList lastCaptures;

    static QString source(const QSObject *re);
    static bool isGlobal(const QSObject *re);
    static bool isIgnoreCase(const QSObject *re);

private:
    static QRegExp* regExp( QSEnv *e );
};

#endif
