/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech AS. All rights reserved.
**
** This file is part of the QSA of the Qt Toolkit.
**
** For QSA Commercial License Holders (non-open source):
** 
** Licensees holding a valid Qt Script for Applications (QSA) License Agreement
** may use this file in accordance with the rights, responsibilities and
** obligations contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of the Licensing Agreement are
** not clear to you.
** 
** Further information about QSA licensing is available at:
** http://www.trolltech.com/products/qsa/licensing.html or by contacting
** info@trolltech.com.
** 
** 
** For Open Source Edition:  
** 
** This file may be used under the terms of the GNU General Public License
** version 2 as published by the Free Software Foundation and appearing in the
** file LICENSE.GPL included in the packaging of this file.  Please review the
** following information to ensure GNU General Public Licensing requirements
** will be met:  http://www.trolltech.com/products/qt/opensource.html 
** 
** If you are unsure which license is appropriate for your use, please review
** the following information:
** http://www.trolltech.com/products/qsa/licensing.html or contact the 
** sales department at sales@trolltech.com.

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "quickdispatchobject.h"
#include "quickinterpreter.h"
#include "quickobjects.h"
#include "qsenv.h"
#include "../qsa/qsobjectfactory.h"
#include "../qsa/qswrapperfactory.h"

#include <QByteArray>
#include <QList>
#include <QMetaObject>
#include <QObject>

class QuickDispatchObjectFactoryPrivate
{
public:
    QList<QSObjectFactory *> objectFactories;
    QList<QSWrapperFactory *> wrapperFactories;
    QMap<QString,QSObjectFactory*> objectsCache;
    QMap<QString,QList<QSWrapperFactory *> > wrappersCache;
    QMap<QString,QString> instanceDescriptors;
    QMap<QString,QObject*> staticDescriptors;
    QStringList classes;
};

QuickPtrDispatchObject::QuickPtrDispatchObject(const char *n, void *p)
    : QuickUnnamedObject(0, n), ptr(p)
{
}

QuickPtrDispatchObject::~QuickPtrDispatchObject()
{
}

void QuickPtrDispatchObject::setPointer(void *p)
{
    ptr = p;
}

QuickDispatchObjectFactory::QuickDispatchObjectFactory(QuickInterpreter *i)
    : ip(i), recurseBlock(false)
{
    d = new QuickDispatchObjectFactoryPrivate;
}

QuickDispatchObjectFactory::~QuickDispatchObjectFactory()
{
    delete d;
}

bool QuickDispatchObjectFactory::constructInterface(const QByteArray &className, void *ptr,
                                                    QVector<QObject *> *result)
{
    Q_ASSERT(result);

    if (!ptr)
        return false;
    if (recurseBlock) {
        qWarning("recursive construction of interfaces detected");
        return false;
    }
    recurseBlock = true;
    bool ret = createInterface(className, ptr, result);
    recurseBlock = false;
    return ret;
}

bool QuickDispatchObjectFactory::constructInstance(const QString &className,
                                                    const QVariantList &args,
                                                    QVector<QObject *> *result)
{
    Q_ASSERT(result);

    if (recurseBlock) {
        qWarning("recursive construction of interfaces detected");
        return false;
    }
    recurseBlock = true;
    int oldIdx = result->size();
    bool ret = createInstance(className, args, result);
    if (!ret)
	result->resize(oldIdx);
    recurseBlock = false;
    return ret;
}

void QuickDispatchObjectFactory::throwError(const QString &message) const
{
    QString msg(message);
    ip->env()->throwError(message);
}

void QuickDispatchObjectFactory::addInterface(QObject *iface, QVector<QObject *> *result)
{
    Q_ASSERT(iface);
    Q_ASSERT(!iface->isWidgetType());
    Q_ASSERT(!iface->parent());
    iface->setParent(interpreter());
    result->append(iface);
}

void QuickDispatchObjectFactory::addObject(QObject *obj, QVector<QObject *> *result)
{
    Q_ASSERT(obj);
    Q_ASSERT(result->isEmpty());
    result->resize(1);
    result->replace(0, obj);
}


static void initObjectFactory(QSObjectFactory *factory,
                              QuickDispatchObjectFactoryPrivate *d)
{
    d->objectFactories.append(factory);
    QMap<QString,QString> descriptors = factory->instanceDescriptors();
    for(QMap<QString,QString>::ConstIterator it = descriptors.begin();
	 it != descriptors.end(); ++it) {
	if(!d->objectsCache.contains(it.key())) {
	    d->objectsCache[it.key()] = factory;
	    d->instanceDescriptors[it.key()] = *it;
	    d->classes << it.key();
	} else {
	    qWarning("QuickDispatchObjectFactory::addObjectFactory()\n"
		      "  class '%s' is already registerd", it.key().toLatin1().constData());
	}
    }

    QMap<QString,QObject*> sdesc = factory->staticDescriptors();
    for(QMap<QString,QObject*>::ConstIterator sit = sdesc.begin();
	 sit != sdesc.end(); ++sit) {
	if(!d->staticDescriptors.contains(sit.key())) {
	    d->staticDescriptors[sit.key()] = *sit;
	    d->classes << sit.key();
	} else {
	    qWarning("QuickDispatchObjectFactory::addObjectFactory()\n"
		      "  static class '%s'' is already registered", sit.key().toLatin1().constData());
	}
    }
}


static void initWrapperFactory(QSWrapperFactory *factory,
				 QuickDispatchObjectFactoryPrivate *d)
{
    d->wrapperFactories.append(factory);
    QList<QString> classes = factory->wrapperDescriptors().keys();
    for(int i=0; i<classes.size(); ++i) {
	d->wrappersCache[classes.at(i)].append(factory);
	d->classes << classes.at(i);
    }
}

static void initFactories(QuickDispatchObjectFactoryPrivate *d)
{
    d->objectsCache.clear();
    d->wrappersCache.clear();
    d->instanceDescriptors.clear();
    d->staticDescriptors.clear();
    d->classes.clear();

    int obj_size = d->objectFactories.size();
    for (int i=0; i<obj_size; ++i)
        initObjectFactory(d->objectFactories.at(i), d);

    int wrap_size = d->wrapperFactories.size();
    for (int i=0; i<wrap_size; ++i)
        initWrapperFactory(d->wrapperFactories.at(i), d);
}


void QuickDispatchObjectFactory::addObjectFactory(QSObjectFactory *factory)
{
    if(d->objectFactories.contains(factory))
	return;
    initObjectFactory(factory, d);
}


void QuickDispatchObjectFactory::addWrapperFactory(QSWrapperFactory *factory)
{
    if(d->wrapperFactories.contains(factory))
	return;
    initWrapperFactory(factory, d);
}


void QuickDispatchObjectFactory::removeObjectFactory(QSObjectFactory *factory)
{
    d->objectFactories.removeAll(factory);
    initFactories(d);
}


void QuickDispatchObjectFactory::removeWrapperFactory(QSWrapperFactory *factory)
{
    d->wrapperFactories.removeAll(factory);
    initFactories(d);
}


bool QuickDispatchObjectFactory::createInterface(const QByteArray &className, void *ptr,
                                                 QVector<QObject *> *result)
{
    const QList<QSWrapperFactory *> factories = d->wrappersCache.value(className);
    bool added = false;
    for (int i=0; i<factories.size(); ++i) {
        QSWrapperFactory *factory = factories.at(i);
	QObject *wrapper = factory->create(className, ptr);
	if(!wrapper) {
	    qWarning("QuickDispatchObjectFactory::createInterface(), "
		      "create returned null");
	    continue;
	}
	addInterface(wrapper, result);
	added = true;
    }
    return added;
}


bool QuickDispatchObjectFactory::createInstance(const QString &className,
                                                const QVariantList &args,
                                                QVector<QObject *> *result)
{
    if(!d->objectsCache.contains(className))
	return false;

    QSObjectFactory *factory = d->objectsCache[ className ];
    QObject *ctx = 0;
    QSObject obj = interpreter()->env()->currentScope();
    if (obj.isA(interpreter()->wrapperClass()))
	ctx = interpreter()->wrapperClass()->shared(&obj)->objects[0];
    QObject *o = factory->create(className, args, ctx);
    if (o) {
	addObject(o, result);
	return true;
    }

    return false;
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
    Q_ASSERT(parent());
    QuickInterpreter *ip = qobject_cast<QuickInterpreter *>(parent());

    Q_ASSERT(ip);
    return ip;
}

void QuickUnnamedObject::throwError(const QString &message) const
{
    interpreter()->env()->throwError(message);
}
