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

#ifndef QUICKOBJECTS_H
#define QUICKOBJECTS_H

#include "quickenv.h"

#include <QVariant>
#include <QObject>
#include <QVector>
#include <QMap>
#include <QMetaObject>
#include <QList>

#include <qsclass.h>
#include <qstypes.h>
#include <qsfunction.h>

class QuickInterpreter;
class QuickScriptVariant;
class QuickScriptObjectFunc;
class QuickPtrDispatchObject;
class QSFactoryObjectProxy;

struct QSATypeInfo;

class QSWrapperClass;
class QSUserData;

class QSWrapperSharedWatcher: public QObject
{
  friend class QSWrapperShared;
  Q_OBJECT
  class QSWrapperShared *parent;

public slots:
    void objectDestroyed(QObject *);
};

class QSWrapperShared : public QSInstanceData
{
    friend class QSWrapperClass;

public:
    enum ObjectType { FactoryObject, GlobalObject };
    QSWrapperShared(const QSWrapperClass *cl);
    ~QSWrapperShared();

    void setUserData(QSUserData *d) { udata = d; }

    const QVector<QObject *> *interfaceObjects() const { return &objects; }
    QVector<QObject *> objects; // interfaces

    const QSWrapperClass* wrapperClass() const { return cls; }
    virtual void invalidate(); // overriding QSShared::invalidate()
    void invalidateWrapper();

    ObjectType objectType() const { return objTyp; }
    void setObjectType(ObjectType type) { objTyp = type; }

    QMap<QString, QList<int> > cached_slots;
    QMap<QString, QSMember> cached_members;

    QSFactoryObjectProxy *creator;

private:
//     EventId findEventId(const QString &event);
    const QSWrapperClass *cls;
    QSUserData *udata;
    ObjectType objTyp;
    QSWrapperSharedWatcher watcher;
};

class QSUserData : public QObjectUserData
{
public:
    QSUserData(QSWrapperShared *d) : dat(d) { }
    ~QSUserData();
    QSWrapperShared* data() const { return dat; }
    void setData(QSWrapperShared *d) { dat = d; }
    void invalidate() { /* dat->deref(); */ dat = 0; }
private:
    QSWrapperShared *dat;
};


class QSWrapperClass : public QSWritableClass,
   					   public QuickEnvClass
{
    friend class QSFactoryObjectProxy;
public:
    enum MemberType { PropertyType, ObjectType, SlotType, EnumType, UnknownType };

    QSWrapperClass(QSClass *b);
    ~QSWrapperClass();
    QString name() const { return QString::fromLatin1("QObject"); }

    virtual bool member(const QSObject *o, const QString &n,
			 QSMember *m) const;
    virtual QSObject fetchValue(const QSObject *objPtr,
				 const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem,
			const QSObject &val) const;

    virtual void deref(QSObject *o) const;

    bool toBoolean(const QSObject *) const;
    QString toString(const QSObject *) const;
    QVariant toVariant(const QSObject *obj, QVariant::Type) const;
    QSEqualsResult isEqual(const QSObject &a, const QSObject &b) const;

    QSObject wrap(const QVector<QObject *> &objs);
    QSWrapperShared* createShared(QObject *o) const;

    QSWrapperShared* shared(const QSObject *obj) const;
    QVector<QObject *>* objectVector(const QSObject *obj) const;

    static QObject *object(const QSObject *obj);

    void invalidate();

protected:
    virtual QSObject invoke(QSObject *objPtr, const QSMember &mem) const;

private:
};

class QSPointerClass : public QSWrapperClass
{
public:
    QSPointerClass(QSClass *b) : QSWrapperClass(b) { }
    QString name() const { return QString::fromLatin1("Pointer"); }

    double toNumber(const QSObject *) const;
    bool toBoolean(const QSObject *) const;
    QString toString(const QSObject *) const;
    QVariant toVariant(const QSObject *obj, QVariant::Type) const;

    QSObject wrapPointer(const char *name, void *p);
    QSObject wrapPointer(QuickPtrDispatchObject *ptr);

    void *pointer(const QSObject *obj) const;
    QString pointerType(const QSObject *obj) const;
};

class QSObjectConstructor;

class QSFactoryObjectProxy : public QSTypeClass, public QuickEnvClass
{
public:
    QSFactoryObjectProxy(QSClass *base, const QSObject &inst,
			  QSObjectConstructor *ctor)
	: QSTypeClass(base),
	  QuickEnvClass(base->env()),
	  sinst(inst),
	  constr(ctor) { }
    QString name() const { return QString::fromLatin1("FactoryObject"); }
    QSObject staticInstance() const { return sinst; }

    virtual bool member(const QSObject *o, const QString &n,
			 QSMember *m) const;
    virtual QSMemberMap members(const QSObject *obj) const;
    virtual QSObject fetchValue(const QSObject *o,
				 const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem,
			const QSObject &val) const;

    virtual ValueType valueType() const { return TypeClass; }

protected:
    virtual QSObject invoke(QSObject * objPtr, const QSMember &mem) const;

    virtual void finalize();
private:
    QSObject sinst;
    QSObjectConstructor *constr;
};

class QSObjectConstructor : public QSClass, public QuickEnvClass
{
public:
    enum Type { Class, Form };
    QSObjectConstructor(QSClass *b, const QString &className, Type t=Class);
    QString name() const { return QString::fromLatin1("ObjectConstructor"); }
    QString identifier() const { return cname; }
    QSObject construct(const QSList &args) const;
    QSObject cast(const QSList &args) const;
    bool member(const QSObject *objPtr, const QString &name, QSMember *m) const;
    QSObject fetchValue(const QSObject *o, const QSMember &mem) const;

    QSFactoryObjectProxy *proxy() const { return m_proxy; }
    void setFactoryObjectProxy(QSFactoryObjectProxy *proxy) { m_proxy = proxy; }

private:
    QString cname;
    Type type;
    QSFactoryObjectProxy *m_proxy;
};

class QSVariantShared;

class QSVariantClass : public QSSharedClass,
					   public QuickEnvClass
{
public:
    QSVariantClass(QSClass *b) : QSSharedClass(b), QuickEnvClass(b->env()) { }
    QString name() const { return QString::fromLatin1("Variant"); }

    virtual bool member(const QSObject *objPtr, const QString &n,
			 QSMember *m) const;
    virtual QSObject fetchValue(const QSObject *objPtr,
				 const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem,
			const QSObject &val) const;

    bool toBoolean(const QSObject *) const;
    double toNumber(const QSObject *) const;

    QString toString(const QSObject *) const;
    QSObject toPrimitive(const QSObject *obj,
			  const QSClass *preferred) const;
    QVariant toVariant(const QSObject *obj, QVariant::Type) const;

    QSObject construct(const QVariant &v) const;
    QSObject construct(const QSList &args) const;

    QVariant *variant(const QSObject *) const;
    QSVariantShared *shared(const QSObject *obj) const;

protected:
    virtual QSObject invoke(QSObject * objPtr, const QSMember &mem) const;
};

class QuickScriptVariant : public QSObject
{
public:
    QuickScriptVariant(QuickInterpreter *ip, const QVariant &v);

    QVariant value() const;

    bool isNative() const;
    QSObject toNative() const;

private:
    QSVariantShared *shared() const;
};

class QSApplicationClass : public QSWritableClass, public QuickEnvClass
{
public:
    QSApplicationClass(QSClass *b);
    QString name() const { return QString::fromLatin1("Application"); }

    virtual QSMemberMap members(const QSObject *obj) const;
    virtual bool member(const QSObject *o, const QString &n, QSMember *m) const;
    virtual QSObject fetchValue(const QSObject *objPtr, const QSMember &mem) const;
};

QSObject convert_qt2qsa(QSEnv *env, void *value, const QSATypeInfo &ti, QObject *obj);
QByteArray qsa_strip_stars(const QByteArray &name, int star_count = 1);

#endif
