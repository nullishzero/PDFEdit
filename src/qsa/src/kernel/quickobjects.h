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

#ifndef QUICKOBJECTS_H
#define QUICKOBJECTS_H

#include "quickenv.h"

#include <qvariant.h>
#include <qobject.h>
#include <qptrvector.h>
#include <qptrdict.h>
#include <qmetaobject.h>
#include "dlldefs.h"
#include <qintdict.h>
#include <qmap.h>

#include <qsclass.h>
#include <qstypes.h>
#include <qsfunction.h>

class QuickInterpreter;
class QuickScriptVariant;
class QuickScriptObjectFunc;
class QuickPtrDispatchObject;
class QSFactoryObjectProxy;


extern void stopAllTimers();

QSObject qsStartTimer( QSEnv * );
void qsKillTimer( QSEnv * );
void qsKillTimers( QSEnv * );

QUICKCORE_EXPORT QObjectList *extraChildren( QObject *o );
QUICKCORE_EXPORT QObject *objectChild( QObject *wid, const char *objName, const char *inheritsClass, bool recursiveSearch );
extern QUICKCORE_EXPORT QValueList<QString> *appArgs;

struct EventTarget {
public:
    QuickInterpreter *eng;
    struct Target {
	Target() : ctx( 0 ), qsctx( QSObject() ) {}
	Target( QObject *c, QSObject qs, const QString &f )
	    : ctx( c ), qsctx( qs ), func( f ) {}
	QObject *ctx;
	QSObject qsctx;
	QString func;
    };
    QValueList<Target> targets;

};

struct QuickMetaData : public QMetaData
{
    QuickMetaData():id(-1) {}
    QuickMetaData( const QMetaData* md, int funcid ):  QMetaData(*md), id( funcid ) {}
    int id;

    bool operator==( const QuickMetaData &md ) const {
	return ( md.id == id &&
		 md.name == name &&
		 md.method == method &&
		 md.access == access );
    }
};

typedef QMap<int, EventTarget> QuickScriptEventMap;

class QuickScriptReceiver : public QObject
{
public:
    QuickScriptReceiver( QObject *o );
    ~QuickScriptReceiver();

    void setEventHandler( QuickInterpreter *ip, int id, QObject *scope,
			  const QString &func, QSObject qsctx = QSObject() );
    void removeEventHandler( int id, QObject *scope, const QString &func, QSObject qsctx = QSObject() );
    bool qt_invoke( int id, QUObject *_o );
    void invalidate();

private:
    QObject *qobj;
    QuickScriptEventMap *handler;
};

class QUICKCORE_EXPORT QSOT
{
public:
    enum QuickScriptObjectType { Property, Object, Slot, Enum, Unknown };
};

class QUICKCORE_EXPORT QuickScriptProperty
{
public:
    QuickScriptProperty( QSOT::QuickScriptObjectType t, QuickScriptVariant *v, int on ) : type( t ), objNum( on ) { id.var = v; }
    QuickScriptProperty( QSOT::QuickScriptObjectType t, QuickScriptObjectFunc *f, int on ) : type( t ), objNum( on ) { id.func = f; }
    QuickScriptProperty( QSOT::QuickScriptObjectType t, int v, int on ) : type( t ), objNum( on ) { id.enumValue = v; }
    QuickScriptProperty() : type( QSOT::Unknown ) {}

    QSOT::QuickScriptObjectType type;
    union {
	QuickScriptVariant *var;
	QuickScriptObjectFunc *func;
	int enumValue;
    } id;
    int objNum;
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class QUICKCORE_EXPORT QPtrList<QuickScriptReceiver>;
template class QUICKCORE_EXPORT QMap<QObject*, QuickScriptReceiver*>;
template class QUICKCORE_EXPORT QMap<QString, QSOT::QuickScriptObjectType>;
template class QUICKCORE_EXPORT QPtrVector<QObject>;
template class QUICKCORE_EXPORT QMap<QString, QuickScriptProperty>;
template class QUICKCORE_EXPORT QValueList<QuickMetaData>;
template class QUICKCORE_EXPORT QIntDict<char>;
template class QUICKCORE_EXPORT QPtrList<QMetaProperty>;
//template class QUICKCORE_EXPORT QValueList<QVariant>;
// MOC_SKIP_END
#endif

struct QUICKCORE_EXPORT EventId
{
    EventId() : id( -1 ), obj( -1 ) {}
    EventId( int i, int o ) : id( i ), obj( o ) {}
    int id, obj;
};

class QSWrapperClass;
class QSUserData;

class QUICKCORE_EXPORT QSWrapperSharedWatcher: public QObject {
  friend class QSWrapperShared;
  Q_OBJECT
  class QSWrapperShared *parent;

public slots:
    void objectDestroyed(QObject *);
};

class QUICKCORE_EXPORT QSWrapperShared : public QSInstanceData
{
    friend class QSWrapperClass;

public:
    enum ObjectType { FactoryObject, GlobalObject };
    QSWrapperShared( const QSWrapperClass *cl );
    ~QSWrapperShared();

    void setUserData( QSUserData *d ) { udata = d; }
    bool setEventHandler( QuickInterpreter *ip, const QString &event, QObject *ctx,
			  const QString &func, QSObject qsctx = QSObject() );
    bool removeEventHandler( const QString &event, QObject *ctx,
			     const QString &func, QSObject qsctx = QSObject() );
    const QPtrVector<QObject> *interfaceObjects() const { return &objects; }

    QPtrVector<QObject> objects; // interfaces
    QMap<QObject*, QuickScriptReceiver*> receivers; // event receivers
    QMap<QString, QSOT::QuickScriptObjectType> hasPropCache; // caches
    QMap<QString, QuickScriptProperty> propertyCache;
    const QSWrapperClass* wrapperClass() const { return cls; }
    virtual void invalidate(); // overriding QSShared::invalidate()
    void invalidateWrapper();

    ObjectType objectType() const { return objTyp; }
    void setObjectType( ObjectType type ) { objTyp = type; }

    QSFactoryObjectProxy *creator;

private:
    EventId findEventId( const QString &event );
    const QSWrapperClass *cls;
    QSUserData *udata;
    ObjectType objTyp;
    QSWrapperSharedWatcher watcher;
};

class QUICKCORE_EXPORT QSUserData : public QObjectUserData {
public:
    QSUserData( QSWrapperShared *d ) : dat( d ) { }
    ~QSUserData();
    QSWrapperShared* data() const { return dat; }
    void setData( QSWrapperShared *d ) { dat = d; }
    void invalidate() { /* dat->deref(); */ dat = 0; }
private:
    QSWrapperShared *dat;
};


class QUICKCORE_EXPORT QSWrapperClass : public QSWritableClass,
					public QuickEnvClass {
    friend class QSFactoryObjectProxy;
public:
    QSWrapperClass( QSClass *b );
    ~QSWrapperClass();
    QString name() const { return QString::fromLatin1("QObject"); }

    virtual bool member( const QSObject *o, const QString &n,
			 QSMember *m ) const;
    virtual QSObject fetchValue( const QSObject *objPtr,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;

    virtual void deref( QSObject *o ) const;

    bool toBoolean( const QSObject * ) const;
    QString toString( const QSObject * ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;
    QString debugString( const QSObject *obj ) const;
    QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;

    QSObject wrap( const QPtrVector<QObject> &objs );
    QSWrapperShared* createShared( QObject *o ) const;

    QSWrapperShared* shared( const QSObject *obj ) const;
    QPtrVector<QObject>& objectVector( const QSObject *obj ) const;

    static QObject *object(const QSObject *obj);

    void invalidate();

protected:
    virtual QSObject invoke( QSObject *objPtr, const QSMember &mem ) const;

private:
};

class QUICKCORE_EXPORT QSPointerClass : public QSWrapperClass {
public:
    QSPointerClass( QSClass *b ) : QSWrapperClass( b ) { }
    QString name() const { return QString::fromLatin1("Pointer"); }

    double toNumber( const QSObject * ) const;
    bool toBoolean( const QSObject * ) const;
    QString debugString( const QSObject *obj ) const;
    QString toString( const QSObject * ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;

    QSObject wrapPointer( const char *name, void *p );
    QSObject wrapPointer( QuickPtrDispatchObject *ptr );

    void *pointer( const QSObject *obj ) const;
    const char *pointerType( const QSObject *obj ) const;
};

class QSObjectConstructor;

class QSFactoryObjectProxy : public QSTypeClass, public QuickEnvClass
{
public:
    QSFactoryObjectProxy( QSClass *base, const QSObject &inst,
			  QSObjectConstructor *ctor )
	: QSTypeClass( base ),
	  QuickEnvClass( base->env() ),
	  sinst( inst ),
	  constr( ctor ) { }
    QString name() const { return QString::fromLatin1("FactoryObject"); }
    QSObject staticInstance() const { return sinst; }

    virtual bool member( const QSObject *o, const QString &n,
			 QSMember *m ) const;
    virtual QSMemberMap members( const QSObject *obj ) const;
    virtual QSObject fetchValue( const QSObject *o,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;

    virtual ValueType valueType() const { return TypeClass; }

protected:
    virtual QSObject invoke( QSObject * objPtr, const QSMember &mem ) const;

    virtual void finalize();
private:
    QSObject sinst;
    QSObjectConstructor *constr;
};

class QSObjectConstructor : public QSClass, public QuickEnvClass
{
public:
    enum Type { Class, Form };
    QSObjectConstructor( QSClass *b, const QString &className, Type t=Class );
    QString name() const { return QString::fromLatin1("ObjectConstructor"); }
    QString identifier() const { return cname; }
    QSObject construct( const QSList &args ) const;
    QSObject cast( const QSList &args ) const;
    bool member( const QSObject *objPtr, const QString &name, QSMember *m ) const;
    QSObject fetchValue( const QSObject *o, const QSMember &mem ) const;

    QSFactoryObjectProxy *proxy() const { return m_proxy; }
    void setFactoryObjectProxy(QSFactoryObjectProxy *proxy) { m_proxy = proxy; }

private:
    QString cname;
    Type type;
    QSFactoryObjectProxy *m_proxy;
};

class QSVariantShared;

class QUICKCORE_EXPORT QSVariantClass : public QSSharedClass,
					public QuickEnvClass {
public:
    QSVariantClass( QSClass *b ) : QSSharedClass( b ), QuickEnvClass( b->env() ) { }
    QString name() const { return QString::fromLatin1("Variant"); }

    virtual bool member( const QSObject *objPtr, const QString &n,
			 QSMember *m ) const;
    virtual QSObject fetchValue( const QSObject *objPtr,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;

    bool toBoolean( const QSObject * ) const;
    double toNumber( const QSObject * ) const;

    QString toString( const QSObject * ) const;
    QSObject toPrimitive( const QSObject *obj,
			  const QSClass *preferred ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;
    QString debugString( const QSObject *obj ) const;

    QSObject construct( const QVariant &v, const QMetaProperty *m ) const;
    QSObject construct( const QSList &args ) const;

    QVariant *variant( const QSObject * ) const;
    QSVariantShared *shared( const QSObject *obj ) const;

protected:
    virtual QSObject invoke( QSObject * objPtr, const QSMember &mem ) const;
};

class QUICKCORE_EXPORT QuickScriptVariant : public QSObject {
public:
    QuickScriptVariant( QuickInterpreter *ip,
			const QVariant &v, const QMetaProperty *m = 0 );

    QVariant value() const;

    const QMetaProperty *metaProperty() const;

    bool isNative() const;
    QSObject toNative() const;

private:
    QSVariantShared *shared() const;
};

class QSApplicationClass : public QSWritableClass, public QuickEnvClass
{
public:
    QSApplicationClass( QSClass *b );
    QString name() const { return QString::fromLatin1("Application"); }

    virtual bool member( const QSObject *o, const QString &n,
			 QSMember *m ) const;
    virtual QSObject fetchValue( const QSObject *objPtr,
				 const QSMember &mem ) const;

    static void exit( QSEnv * );
    static QSObject startTimer( QSEnv * );
    static QSObject killTimer( QSEnv * );
    static QSObject killTimers( QSEnv * );
};

#endif
