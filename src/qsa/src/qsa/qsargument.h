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

#ifndef QSARGUMENT_H
#define QSARGUMENT_H

#include <qsaglobal.h>
#include <qvariant.h>
#include <qvaluelist.h>

class QObject;

class QSA_EXPORT QSArgument
{
    friend bool operator==( const QSArgument &a, const QSArgument &b );
public:
    enum Type { Invalid, Variant, QObjectPtr, VoidPointer };

    QSArgument();
    QSArgument( const QVariant &v );
    QSArgument( QObject *o );
    QSArgument( void *p );

    QVariant variant() const;
    QObject *qobject() const;
    void *ptr() const;
    Type type() const;
    QString typeName() const;


private:
    QVariant var;
    QObject *qobj;
    void *pt;
    Type typ;
};

bool operator==( const QSArgument &a, const QSArgument &b );

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
QSA_TEMPLATE_EXTERN template class QSA_EXPORT QValueList<QSArgument>;
// MOC_SKIP_END
#endif

class QSA_EXPORT QSArgumentList : public QValueList<QSArgument>
{
public:
    QSArgumentList() {}
    QSArgumentList( const QValueList<QVariant> &l ) {
	for ( QValueList<QVariant>::ConstIterator it = l.begin(); it != l.end(); ++it )
	    append( *it );
    }
    QSArgumentList( const QVariant &v ) { append( v ); }
    QSArgumentList( QObject *o ) { append( o ); }
    QSArgumentList( void *p ) { append( p ); }

};

#endif
