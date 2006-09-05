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

#ifndef QUICKDISPATCHOBJECT_H
#define QUICKDISPATCHOBJECT_H

#include <qobject.h>
#include <qptrvector.h>
#include <qvaluelist.h>

class QuickInterpreter;
class QuickDispatchObjectFactoryPrivate;

class QSObjectFactory;
class QSWrapperFactory;

class QuickDispatchObjectFactory
{
public:
    QuickDispatchObjectFactory( QuickInterpreter *i );
    ~QuickDispatchObjectFactory();

    bool constructInterface( const QCString &className,
			     void *ptr,
			     QPtrVector<QObject> &result );

    bool constructInstance( const QString &className,
			    const QValueList<QVariant> &args,
			    QPtrVector<QObject> &result );

    QMap<QString,QObject*> staticDescriptors() const;
    QMap<QString,QString> instanceDescriptors() const;

    QStringList classes() const;

    void throwError( const QString &message ) const;

    QuickInterpreter *interpreter() const { return ip; }

    void addObjectFactory( QSObjectFactory *factory );
    void addWrapperFactory( QSWrapperFactory *factory );

    void removeObjectFactory( QSObjectFactory *factory );
    void removeWrapperFactory( QSWrapperFactory *factory );

private:
    bool createInterface( const QCString &className, void *ptr, QPtrVector<QObject> *);
    bool createInstance( const QString &className,
			 const QValueList<QVariant> &args,
                         QPtrVector<QObject> *);

    void addInterface( QObject *iface, QPtrVector<QObject> *);
    void addObject( QObject *obj, QPtrVector<QObject> *);

private:
    QuickDispatchObjectFactoryPrivate *d;
    QuickInterpreter *ip;
    bool recurseBlock;
};


class QuickUnnamedObject : public QObject
{
    Q_OBJECT
public:
    QuickUnnamedObject( QObject *p = 0, const char *n = 0 ) : QObject( p, n ) {}
    QuickInterpreter* interpreter() const;
    void throwError( const QString &message ) const;
};


class QuickPtrDispatchObject : public QuickUnnamedObject
{
    Q_OBJECT
public:
    QuickPtrDispatchObject( const char *n, void *p );
    ~QuickPtrDispatchObject();

    void* pointer() const { return ptr; }
    void setPointer( void *p );

public slots:
    bool isValid() const { return !!ptr; }

private:
    void *ptr;
};

#endif
