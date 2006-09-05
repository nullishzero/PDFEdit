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

#include "qsobject_object.h"
#include "qsclass.h"
#include "qsenv.h"
#include "qsoperations.h"
#include "qstypes.h"
#include "qsfunction.h"

QSObjectClass::QSObjectClass( QSEnv *e )
    : QSClass( e )
{
}

void QSObjectClass::init()
{
    addMember( QString::fromLatin1("toString"), QSMember( &QSObjectClass::toStringScript ) );
    addMember( QString::fromLatin1("valueOf"), QSMember( &QSObjectClass::valueOf ) );
}

QString QSObjectClass::toString( const QSObject *obj ) const
{
    return QString::fromLatin1("[object ") + obj->typeName() + QString::fromLatin1("]");
}

QSObject QSObjectClass::construct() const
{
    QSList dummy;
    return construct( dummy );
}

QSObject QSObjectClass::construct( const QSList &args ) const
{
    if ( args.isEmpty() ) {
	return env()->dynamicClass()->createWritable();
    } else {
	// ### warn about superfluous arguments ?
	return args[0];
    }
}

/*! \reimp */
QSObject QSObjectClass::cast( const QSList &args ) const
{
    // same as 'new'
    return construct( args );
}

// Object.toString()
QSObject QSObjectClass::toStringScript( QSEnv *env )
{
    QString str = QString::fromLatin1("[object ") + env->thisValue().typeName() + QString::fromLatin1("]");
    return env->createString( str );
}

// Object.valueOf()
QSObject QSObjectClass::valueOf( QSEnv *env )
{
    return env->thisValue();
}
