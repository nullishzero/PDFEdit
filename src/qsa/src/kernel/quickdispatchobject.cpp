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

#include "quickdispatchobject.h"
#include "quickinterpreter.h"
#include "quickobjects.h"
#include "qsenv.h"
#include "../qsa/qsobjectfactory.h"
#include "../qsa/qswrapperfactory.h"

#include <qmetaobject.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qwidget.h>

class QuickDispatchObjectFactoryPrivate
{
public:
    QPtrList<QSObjectFactory> objectFactories;
    QPtrList<QSWrapperFactory> wrapperFactories;
    QMap<QString,QSObjectFactory*> objectsCache;
    QMap<QString,QPtrList<QSWrapperFactory> > wrappersCache;
    QMap<QString,QString> instanceDescriptors;
    QMap<QString,QObject*> staticDescriptors;
    QStringList classes;
};

QuickPtrDispatchObject::QuickPtrDispatchObject( const char *n, void *p )
    : QuickUnnamedObject( 0, n ), ptr( p )
{
}

QuickPtrDispatchObject::~QuickPtrDispatchObject()
{
}

void QuickPtrDispatchObject::setPointer( void *p )
{
    ptr = p;
}

QuickDispatchObjectFactory::QuickDispatchObjectFactory( QuickInterpreter *i )
    : ip(i), recurseBlock(false)
{
    d = new QuickDispatchObjectFactoryPrivate;
}

QuickDispatchObjectFactory::~QuickDispatchObjectFactory()
{
    delete d;
}

bool QuickDispatchObjectFactory::constructInterface( const QCString &className,
						     void *ptr,
						     QPtrVector<QObject> &result )
{
    if (!ptr)
        return false;
    if (recurseBlock) {
        qWarning("recursive construction of interfaces detected");
        return FALSE;
    }
    recurseBlock = TRUE;
    bool ret = createInterface( className, ptr, &result );
    recurseBlock = FALSE;
    return ret;
}

bool QuickDispatchObjectFactory::constructInstance( const QString &className,
                                                    const QValueList<QVariant> &args,
                                                    QPtrVector<QObject> &result )
{
    if (recurseBlock) {
        qWarning("recursive construction of interfaces detected");
        return FALSE;
    }
    recurseBlock = TRUE;
    int oldIdx = result.size();
    bool ret = createInstance( className, args, &result );
    if ( !ret ) {
	for ( uint i = oldIdx; i < result.size(); ++i )
	    result.insert( i, 0 );
    }
    recurseBlock = FALSE;
    return ret;
}

void QuickDispatchObjectFactory::throwError( const QString &message ) const
{
    QString msg( message );
    ip->env()->throwError( message );
}

void QuickDispatchObjectFactory::addInterface( QObject *iface, QPtrVector<QObject> *result )
{
    Q_ASSERT( iface );
    Q_ASSERT( !iface->isWidgetType() );
    Q_ASSERT( !iface->parent() );
    interpreter()->insertChild( iface );
    int idx = result->size();
    result->resize( idx + 1 );
    result->insert( idx, iface );
}

void QuickDispatchObjectFactory::addObject( QObject *obj, QPtrVector<QObject> *result )
{
    Q_ASSERT( obj );
    Q_ASSERT( result->isEmpty() );
    result->resize( 1 );
    result->insert( 0, obj );
}


static void initObjectFactory( QSObjectFactory *factory,
				QuickDispatchObjectFactoryPrivate *d )
{
    d->objectFactories.append( factory );
    QMap<QString,QString> descriptors = factory->instanceDescriptors();
    for( QMap<QString,QString>::ConstIterator it = descriptors.begin();
	 it != descriptors.end(); ++it ) {
	if( !d->objectsCache.contains( it.key() ) ) {
	    d->objectsCache[it.key()] = factory;
	    d->instanceDescriptors[it.key()] = *it;
	    d->classes << it.key();
	} else {
	    qWarning( "QuickDispatchObjectFactory::addObjectFactory()\n"
		      "  class '%s' is already registerd", it.key().latin1() );
	}
    }

    QMap<QString,QObject*> sdesc = factory->staticDescriptors();
    for( QMap<QString,QObject*>::ConstIterator sit = sdesc.begin();
	 sit != sdesc.end(); ++sit ) {
	if( !d->staticDescriptors.contains( sit.key() ) ) {
	    d->staticDescriptors[sit.key()] = *sit;
	    d->classes << sit.key();
	} else {
	    qWarning( "QuickDispatchObjectFactory::addObjectFactory()\n"
		      "  static class '%s'' is already registered", sit.key().latin1() );
	}
    }
}


static void initWrapperFactory( QSWrapperFactory *factory,
				 QuickDispatchObjectFactoryPrivate *d )
{
    d->wrapperFactories.append( factory );
    QValueList<QString> classes = factory->wrapperDescriptors().keys();
    for( QValueList<QString>::ConstIterator it = classes.begin();
	 it != classes.end(); ++it ) {
	d->wrappersCache[*it].append( factory );
	d->classes << *it;
    }
}

static void initFactories( QuickDispatchObjectFactoryPrivate *d )
{
    d->objectsCache.clear();
    d->wrappersCache.clear();
    d->instanceDescriptors.clear();
    d->staticDescriptors.clear();
    d->classes.clear();

    for( QSObjectFactory *ofac = d->objectFactories.first();
	 ofac; ofac = d->objectFactories.next() )
	initObjectFactory( ofac, d );

    for( QSWrapperFactory *wfac = d->wrapperFactories.first();
	 wfac; wfac = d->wrapperFactories.next() )
	initWrapperFactory( wfac, d );
}


void QuickDispatchObjectFactory::addObjectFactory( QSObjectFactory *factory )
{
    if( d->objectFactories.find( factory ) >= 0 )
	return;
    initObjectFactory( factory, d );
}


void QuickDispatchObjectFactory::addWrapperFactory( QSWrapperFactory *factory )
{
    if( d->wrapperFactories.find( factory ) >= 0 )
	return;
    initWrapperFactory( factory, d );
}


void QuickDispatchObjectFactory::removeObjectFactory( QSObjectFactory *factory )
{
    d->objectFactories.removeRef( factory );
    initFactories( d );
}


void QuickDispatchObjectFactory::removeWrapperFactory( QSWrapperFactory *factory )
{
    d->wrapperFactories.removeRef( factory );
    initFactories( d );
}


bool QuickDispatchObjectFactory::createInterface( const QCString &className, void *ptr,
                                                  QPtrVector<QObject> *result )
{
    QPtrList<QSWrapperFactory> factories = *(d->wrappersCache.find( className ));
    bool added = FALSE;
    for( QSWrapperFactory *factory = factories.first(); factory;
 	 factory = factories.next() ) {
 	QObject *wrapper = factory->create( className, ptr );
	if( !wrapper ) {
	    qWarning( "QuickDispatchObjectFactory::createInterface(), "
		      "create returned null" );
	    continue;
	}
	addInterface( wrapper, result );
	added = TRUE;
    }
    return added;
}


bool QuickDispatchObjectFactory::createInstance( const QString &className,
                                                 const QValueList<QVariant> &args,
                                                 QPtrVector<QObject> *result )
{
    if( !d->objectsCache.contains( className ) )
	return FALSE;

    QSArgumentList qsArgs;
    for ( QValueList<QVariant>::ConstIterator it = args.begin();
	  it != args.end(); ++it ) {
	if ( (*it).type() == QVariant::String ) {
		static const int length_of_Pointer = 7;
		static const QString pointer_header = QString::fromLatin1("Pointer");
		QString s = (*it).toString();
		if ( s.left( length_of_Pointer ) == pointer_header ) {
		    QStringList l = QStringList::split( ':', s );
		    if ( l.count() == 3 ) {
			if ( l[2] != QString::fromLatin1("QObject") ) {
			    ulong lng = l[1].toULong();
			    void *ptr = (void*)lng;
			    qsArgs.append( QSArgument( ptr ) );
			} else {
			    ulong lng = l[1].toULong();
			    QObject *o = (QObject*)lng;
			    qsArgs.append( QSArgument( o ) );
			}
			continue;
		    }
		}
	}
	qsArgs.append( QSArgument( *it ) );
    }

    QSObjectFactory *factory = d->objectsCache[ className ];
    QObject *ctx = 0;
    QSObject obj = interpreter()->env()->currentScope();
    if ( obj.isA( interpreter()->wrapperClass() ) )
	ctx = interpreter()->wrapperClass()->shared( &obj )->objects[0];
    QObject *o = factory->create( className, qsArgs, ctx );
    if ( o ) {
	addObject( o, result );
	return TRUE;
    }

    return FALSE;
}


QStringList QuickDispatchObjectFactory::classes() const
{
    return d->classes;
}


QMap<QString,QString> QuickDispatchObjectFactory::instanceDescriptors() const
{
    return d->instanceDescriptors;
}


QMap<QString,QObject*> QuickDispatchObjectFactory::staticDescriptors() const
{
    return d->staticDescriptors;
}


QuickInterpreter* QuickUnnamedObject::interpreter() const
{
    Q_ASSERT( parent() && parent()->inherits( "QuickInterpreter" ) );
    return (QuickInterpreter*)parent();
}

void QuickUnnamedObject::throwError( const QString &message ) const
{
    interpreter()->env()->throwError( message );
}
