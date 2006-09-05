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

#ifndef QSOBJECTFACTORY_H
#define QSOBJECTFACTORY_H

#include <qsaglobal.h>
#include <qsargument.h>

class QObject;
class QSInterpreter;
class QSObjectFactoryPrivate;



class QSA_EXPORT QSObjectFactory
{
    friend class QSInterpreter;
public:
    QSObjectFactory();
    virtual ~QSObjectFactory();

    virtual QObject *create( const QString &className,
			     const QSArgumentList &arguments,
			     QObject *context );

    void registerClass( const QString &className,
			QObject *staticInstance = 0 );

    void registerClass( const QString &className,
			const QString &cppClassName,
			QObject *staticInstance = 0 );

    QMap<QString,QObject*> staticDescriptors() const;
    QMap<QString,QString> instanceDescriptors() const;

    void throwError( const QString &message );

protected:
    QSInterpreter *interpreter() const;

private:
    void setInterpreter( QSInterpreter *ip );

    QSObjectFactoryPrivate *d;
};

#endif
