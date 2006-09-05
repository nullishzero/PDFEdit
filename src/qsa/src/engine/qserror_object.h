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

#ifndef QSERROR_OBJECT_H
#define QSERROR_OBJECT_H

#include "qsclass.h"

class QSErrorClass : public QSWritableClass {
public:
    QSErrorClass( QSClass *b ) : QSWritableClass( b ) { }
    QString name() const { return QString::fromLatin1("Error"); }

    QString toString( const QSObject * ) const;

    QSObject construct( const QSList &args ) const;
    QSObject cast( const QSList &args ) const;

    QSObject construct( ErrorType e, const QString &m = QString::null,
			int ln = - 1 ) const;

    static ErrorType errorType(const QSObject *objPtr);
    static QString errorName(const QSObject *objPtr);
    static QString errorMessage(const QSObject *objPtr);
    static int errorLine(const QSObject *objPtr);
    static void setErrorLine(QSObject *objPtr, int line);
#ifdef QSDEBUGGER
    static int errorSourceId(const QSObject *objPtr);
#endif

};

#endif
