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

#include "quickobjects.h"

#include <qsaglobal.h>
#include <qsfunction.h>
#include <qsoperations.h>
#include <qsenv.h>
#include <qsinternal.h>
#include <qsdate_object.h>
#include "quickinterpreter.h"
#include "quickdispatchobject.h"

#ifndef QSA_NO_GUI
#include <QPixmap>
#include <QFont>
#include <QWidget>
#endif
#include <QStringList>
#include <QByteArray>
#include <QList>
#include <QTimerEvent>
#include <qmetaobject.h>
#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QPoint>
#include <QSize>
#include <QRect>
#include "quickbytearrayobject.h"
#ifndef QSA_NO_GUI
#include "quickpixmapobject.h"
#include "quickcolorobject.h"
#include "quickfontobject.h"
#endif
#include "quickcoordobjects.h"
#include <qsarray_object.h>
#include "../engine/qsfuncref.h"

#include <qdebug.h>

// A back door for those who abosolutly don't want to have
// QSInterpreter collect their object factory created objects.
static bool qsa_owns_factoryobjects = true;
static bool qsa_parent_owns_factoryobject = true;
static bool qsa_converts_map_to_object = false;
void QSA_EXPORT qsa_set_factoryobject_ownership(bool enable) {
    qsa_owns_factoryobjects = enable;
}
void QSA_EXPORT qsa_set_factoryobject_parent_ownership(bool enable) {
    qsa_parent_owns_factoryobject = enable;
}
void QSA_EXPORT qsa_set_map_to_object_conversion(bool enable) {
    qsa_converts_map_to_object = enable;
}

extern bool qsa_is_non_gui_thread();

using namespace QS;

// Returns a uint that has the \a x lowest bit set.
static inline uint low_bits(int x) {
    return 0xffffffff >> (32 - x);
}

const int OBJECT_INDEX_SIZE = 5;
const int MEMBER_TYPE_SIZE = 3;
const int MEMBER_INDEX_SIZE = 24;

static inline uint  qsa_encode_member_index(QSWrapperClass::MemberType member_type, uint object_index, uint member_index)
{
    // make sure we have enough space...
    Q_ASSERT(object_index < (1<<OBJECT_INDEX_SIZE));
    Q_ASSERT(member_type < (1<<MEMBER_TYPE_SIZE));
    Q_ASSERT(member_index < (1<<MEMBER_INDEX_SIZE));

    return (object_index << (32 - OBJECT_INDEX_SIZE))
        | (member_type << (32 - MEMBER_TYPE_SIZE - OBJECT_INDEX_SIZE))
        | member_index;
}

static inline uint qsa_decode_index_object(uint index) {
    return index >> (32 - OBJECT_INDEX_SIZE);
}

static inline uint qsa_decode_index_type(uint index) {
    return (index >> (32 - OBJECT_INDEX_SIZE - MEMBER_TYPE_SIZE)) & low_bits(MEMBER_TYPE_SIZE);
}

static inline uint qsa_decode_index_member(uint index) {
    return index & low_bits(MEMBER_INDEX_SIZE);
}


QSObject qsa_execute_slot(QSEnv *env, QObject *qobject, const QList<int> &slot_indexes);

/////////////////// internal functions /////////////////////////////

class QSVariantShared : public QSShared
{
public:
    QSVariantShared(const QVariant &v);
    virtual void invalidate();
    void createObject(QuickInterpreter *);

    QVariant variant;
    QSObject iobj;
    uint native : 1;
};

void QSWrapperSharedWatcher::objectDestroyed(QObject *obj) {
    if (!parent->objects.isEmpty() && parent->objects[0] == obj)
        parent->objects.data()[0] = 0;
}


QSWrapperShared::QSWrapperShared(const QSWrapperClass *cl)
    : QSInstanceData(0, cl->createUndefined()), cls(cl),
      udata(0), objTyp(FactoryObject)
{
    watcher.parent = this;
    creator = 0;
}

QSWrapperShared::~QSWrapperShared()
{
    if (!cls->env()->isShuttingDown() && isConnected())
        cls->env()->removeShared(this);

//     for(QMap<QString,QuickScriptProperty>::Iterator cached = propertyCache.begin();
//          cached != propertyCache.end(); ++cached)
//         if((*cached).type == QSOT::Property) {
//             QuickScriptVariant *variant = (*cached).id.var;
//             variant->setVal((QSShared*) 0);
//             variant->invalidate();
//             delete variant;
//         }

    invalidateWrapper();
//     Q_ASSERT(objects.isEmpty() && receivers.isEmpty());
}

void QSWrapperShared::invalidate()
{
    invalidateWrapper();
    if (cls->env()->isShuttingDown())
        QSInstanceData::invalidate();
}

void QSWrapperShared::invalidateWrapper()
{
    // let QObject lose the reference to us
    if (udata) {
        udata->invalidate();
        udata = 0;
    }
    for (int i = 1; i < objects.size(); ++i)
        delete objects[ i ];

    if(objects.count() > 0
       && objTyp == FactoryObject
       && objects.at(0)) {
        if (qsa_owns_factoryobjects
            && (!qsa_parent_owns_factoryobject || !objects.at(0)->parent())) {
                delete objects[0];
            }
    }

    objects.clear();
}

QSWrapperClass::QSWrapperClass(QSClass *b)
    : QSWritableClass(b), QuickEnvClass(b->env())
{
}

QSWrapperClass::~QSWrapperClass()
{
    invalidate();
}

QSWrapperShared *QSWrapperClass::shared(const QSObject *obj) const
{
    Q_ASSERT(obj->isA(this));
    return (QSWrapperShared*)obj->shVal();
}

QVector<QObject*> *QSWrapperClass::objectVector(const QSObject *obj) const
{
    return &shared(obj)->objects;
}

void QSWrapperClass::invalidate()
{
}

void QSWrapperClass::deref(QSObject *o) const
{
#ifndef QS_LEAK
    o->shVal()->deref();
    if (o->shVal()->count==0) {
        ((QSWrapperClass*)this)->invalidate();
        env()->removeShared(o->shVal());
        delete o->shVal();
        o->setVal((QSShared*)0);
    }
#endif
}

bool QSWrapperClass::member(const QSObject *objPtr, const QString &p,
                             QSMember *mem) const
{
//     printf("QSWrapperClas::member(), looking for: %s\n", qPrintable(p));

    if (!objPtr)
        return QSWritableClass::member(objPtr, p, mem);

    QSWrapperShared *sh = shared(objPtr);
    if(!sh)
        return false;

    // check in cache first...
    QSMember cached_member = sh->cached_members.value(p, QSMember());
    if (cached_member.isDefined()) {
//         printf(" - found match in cache...\n");
        *mem = cached_member;
        return true;
    }

    mem->setType(QSMember::Custom);
    mem->setOwner(this);
    mem->setName(p);

    const QVector<QObject *> &objects = sh->objects;

    if (objects.isEmpty())
        return QSWritableClass::member(objPtr, p, mem);

    QByteArray member_name_ba = p.toLatin1();
    const char *member_name = member_name_ba.data();
    int member_name_length = p.length();

    for (int i=objects.size() - 1; i>=0; --i) {
        QObject *object = objects[i];
        Q_ASSERT(object);
        const QMetaObject *meta_object = object->metaObject();

        // the pointer wrapper is a transparent type so we skip it.
        if (meta_object == &QuickPtrDispatchObject::staticMetaObject)
            continue;

//         printf(" - looking in [%s]\n", meta_object->className());

        // children
        QObjectList children = object->children();
        for (int child_index=0; child_index<children.size(); ++child_index) {
            if (p == children.at(child_index)->objectName()) {
                mem->setWritable(false);
                mem->setIndex(qsa_encode_member_index(ObjectType, i, child_index));
                return true;
            }
        }

        // properties
        int property_index = meta_object->indexOfProperty(member_name);
        if (property_index >= 0) {
            QMetaProperty prop = meta_object->property(property_index);
            mem->setIndex(qsa_encode_member_index(PropertyType, i, property_index));;
            sh->cached_members[p] = *mem;
            return true;
        }

        // slots
        {
            // Look up slots the really hard way since we don't know
            // the exact signature...
            QList<int> slot_indexes;
            const QMetaObject *mo = meta_object;
            while (mo) {
                int first_method = mo->methodOffset();
                int last_method = mo->methodCount() - 1;

                for (int method_index = first_method; method_index <= last_method; ++method_index) {
                    QMetaMethod method = mo->method(method_index);
                    const char *signature = method.signature();
                    if (method.methodType() != QMetaMethod::Signal
                        && strncmp(signature, member_name, member_name_length) == 0
                        && signature[member_name_length] == '(') {
                        slot_indexes << method_index;
                    }
                }
                mo = mo->superClass();
            }

            if (!slot_indexes.isEmpty()) {
                sh->cached_slots[p] = slot_indexes;
                mem->setIndex(qsa_encode_member_index(SlotType, i, 0));
                sh->cached_members[p] = *mem;
                return true;
            }
        }

        // enums
        {
            uint value = 0;
            bool ok = false;
            for (int search_idx = meta_object->enumeratorCount() - 1;
                 search_idx >= 0 && !ok;
                 --search_idx) {
                QMetaEnum metaEnum = meta_object->enumerator(search_idx);
                for (int key_idx=0; key_idx<metaEnum.keyCount() && !ok; ++key_idx) {
                    if (qstrcmp(metaEnum.key(key_idx), member_name) == 0) {
                        ok = true;
                        value = (key_idx & low_bits(14)) | ((search_idx & low_bits(10)) << 14);
                    }
                }
            }

            if (ok) {
                mem->setIndex(qsa_encode_member_index(EnumType, i, value));
                sh->cached_members[p] = *mem;
                return true;
            }
        }
    }

    return QSWritableClass::member(objPtr, p, mem);
}

QSObject QSWrapperClass::fetchValue(const QSObject *o,
                                     const QSMember &mem) const
{
    // member specific to this class ?
    if (mem.type() != QSMember::Custom)
        return QSWritableClass::fetchValue(o, mem);

    QSWrapperShared *sh = shared(o);
    const QVector<QObject *> &objects = sh->objects;

    MemberType type = (MemberType) qsa_decode_index_type(mem.index());
    int object_index = (int) qsa_decode_index_object(mem.index());
    int member_index = (int) qsa_decode_index_member(mem.index());

    QObject *object = objects.at(object_index);
    const QMetaObject *meta_object = object->metaObject();

    switch (type) {

    case PropertyType:
        {
            QMetaProperty property = meta_object->property(member_index);
            if (!property.isReadable())
                return env()->throwError(QString("Property %1 is not readable").arg(mem.name()));
            if (!property.isScriptable())
                return env()->throwError(QString("Property %1 is not scriptable").arg(mem.name()));
            QVariant variant = property.read(object);
            QuickScriptVariant script_variant(interpreter(), variant);
            return script_variant.isNative() ? script_variant.toNative() : script_variant;
        }

    case SlotType:
        return env()->funcRefClass()->createReference(*o, mem);
        break;

    case EnumType:
        {
            int search_idx = (member_index >> 14) & low_bits(10);
            int key_idx = (member_index & low_bits(14));

            QMetaEnum metaEnum = meta_object->enumerator(search_idx);
            return env()->createNumber(metaEnum.value(key_idx));
        }
        break;

    case ObjectType:
        {
            QObject *child = object->children().at(member_index);
            Q_ASSERT(mem.name() == child->objectName());
            return interpreter()->wrap(child);
        }
        break;

    default:
        qWarning("QSWrapperClass::fetchValue(), should not get here\n");
        break;
    }

    return createUndefined();
}

QSObject QSWrapperClass::invoke(QSObject *objPtr, const QSMember &mem) const
{
    if (mem.type() != QSMember::Custom)
        return QSWritableClass::invoke(objPtr, mem);

    Q_ASSERT(mem.owner() == this);
    QSWrapperShared *sh = shared(objPtr);
    const QList<int> &slot_indexes = sh->cached_slots[mem.name()];

    Q_ASSERT(!slot_indexes.isEmpty());
    int object_index = qsa_decode_index_object(mem.index());

    QSObject returned = qsa_execute_slot(env(), sh->objects.at(object_index), slot_indexes);

    return returned;
}

void QSWrapperClass::write(QSObject *objPtr, const QSMember &mem,
                            const QSObject &val) const
{
    Q_ASSERT (mem.isWritable());

    // member specific to this class ?
    if (mem.type() != QSMember::Custom) {
        QSWritableClass::write(objPtr, mem, val);
        return;
    }

    QSWrapperShared *sh = shared(objPtr);
    MemberType type = (MemberType) qsa_decode_index_type(mem.index());
    int object_index = qsa_decode_index_object(mem.index());
    int member_index = qsa_decode_index_member(mem.index());

    QObject *q_object = sh->objects[object_index];
    const QMetaObject *meta_object = q_object->metaObject();

    switch (type) {

    case PropertyType:
        {
            QMetaProperty meta_property = meta_object->property(member_index);
            if (!meta_property.isWritable())
                env()->throwError(QString("Property %1 is not writable").arg(mem.name()));
            if (!meta_property.isScriptable())
                env()->throwError(QString("Property %1 is not scriptable").arg(mem.name()));
            QVariant v = val.toVariant(meta_property.type());
            if (!meta_property.write(q_object, v)) {
                env()->throwError(QString("Failed to write property '%1'")
                                         .arg(mem.name()));
            }
            break;
        }

    default:
        {
            env()->throwError(QString("Writing property '%1' failed, type=%2")
                              .arg(mem.name()).arg(type));
            break;
        }
    }

}

bool QSWrapperClass::toBoolean(const QSObject *obj) const
{
    return !objectVector(obj)->isEmpty();
}

QString QSWrapperClass::toString(const QSObject *obj) const
{
    QVector<QObject *> *objects = objectVector(obj);
    if (objects->isEmpty())
        return QLatin1String("[object QObject]");
    return QLatin1String("[object ")
        + QLatin1String(objects->at(0)->metaObject()->className())
        + QLatin1String("]");
}

QVariant QSWrapperClass::toVariant(const QSObject *obj, QVariant::Type) const
{
    const QVector<QObject *> *objects = objectVector(obj);
    return qVariantFromValue(objects->at(0));
}

/*!
  \reimp
  Handles two special cases: a) a comparison of references to QObjects
  no matter whether the QObjects still or already use a detached wrapper
  class. b) distinct pointer wrapper objects containing identical pointers
*/

QSEqualsResult QSWrapperClass::isEqual(const QSObject &a, const QSObject &b) const
{
    Q_ASSERT(a.objectType() == this);
    if (!b.isA(interpreter()->wrapperClass()) &&
         !b.isA(interpreter()->pointerClass()))
        return EqualsUndefined;

    // check whether the references point to the same QObject
    QSWrapperClass *aclass = (QSWrapperClass*)a.objectType();
    QSWrapperClass *bclass = (QSWrapperClass*)b.objectType();
    QVector<QObject *> *ova = aclass->objectVector(&a);
    QVector<QObject *> *ovb = bclass->objectVector(&b);
    if (ova->isEmpty() && ovb->isEmpty()) // both null pointers ?
        return EqualsIsEqual;
    return (QSEqualsResult) (!ova->isEmpty() && !ovb->isEmpty() && ova->at(0) == ovb->at(0));
}

QSObject QSWrapperClass::wrap(const QVector<QObject *> &objs)
{
    QSWrapperShared *sh = new QSWrapperShared(this);
    sh->objects = objs;
    if (objs.size()) {
        QObject::connect(objs[0], SIGNAL(destroyed(QObject*)),
                         &sh->watcher, SLOT(objectDestroyed(QObject*)));
    }
    return QSObject(this, sh);
}

QSWrapperShared* QSWrapperClass::createShared(QObject *o) const
{
    Q_ASSERT(o);
    QSWrapperShared *shared = new QSWrapperShared(this);

    bool b = interpreter()->queryDispatchObjects(o, &shared->objects);
    Q_UNUSED(b);
    Q_ASSERT(b);

    return shared;
}

QObject *QSWrapperClass::object(const QSObject *obj)
{
    Q_ASSERT(obj);
    Q_ASSERT(obj->objectType());
    Q_ASSERT(obj->objectType()->name() == QLatin1String("QObject"));
    Q_ASSERT(((QSWrapperClass*)obj->objectType())->objectVector(obj)->size() > 0);
    return ((QSWrapperClass*)obj->objectType())->objectVector(obj)->at(0);
}


QSObject QSObjectConstructor::cast(const QSList &) const
{
    env()->throwError(QLatin1String("Unable to perform cast to user defined object type '") + cname + QLatin1String("'"));
    return QSObject();
}

void QSFactoryObjectProxy::finalize()
{
    /* We don't need to destroy the shared data, since it is registered
       in the environment in QuickInterpreter::wrap(), and will be deleted
       by env shortly after env has called finalize on all classes */
    sinst.invalidate();
}


bool QSFactoryObjectProxy::member(const QSObject *, const QString &n,
                                   QSMember *m) const
{
    return sinst.isValid()
        ? sinst.objectType()->member(&sinst, n, m)
        : constr->member(0, n, m);
}

QSMemberMap QSFactoryObjectProxy::members(const QSObject *) const
{
    return sinst.objectType()->members(&sinst);
}

QSObject QSFactoryObjectProxy::fetchValue(const QSObject *,
                                           const QSMember &mem) const
{
    return sinst.isValid()
        ? sinst.objectType()->fetchValue(&sinst, mem)
        : mem.owner()->fetchValue(0, mem);
}

void QSFactoryObjectProxy::write(QSObject *, const QSMember &mem,
                                  const QSObject &val) const
{
    sinst.objectType()->write(&(((QSFactoryObjectProxy*) this)->sinst),
                               mem, val);
}

QSObject QSFactoryObjectProxy::invoke(QSObject *, const QSMember &mem) const
{
    return ((QSWrapperClass*) sinst.objectType())
        ->invoke(&(((QSFactoryObjectProxy*) this)->sinst), mem);
}

QSObjectConstructor::QSObjectConstructor(QSClass *b,
                                          const QString &className, Type t)
    : QSClass(b), QuickEnvClass(b->env()), cname(className), type(t)
{
}

QSObject QSObjectConstructor::construct(const QSList &args) const
{
    if (type == Class) {
        QVector<QObject *> result;
        QList<QVariant> vargs;
        for (int i = 0; i < args.size(); i++)
            vargs.append(args[i].toVariant(QVariant::Invalid));
        bool success = interpreter()->construct(cname, vargs, &result);
        if (success && result.size() >= 1)
            success = interpreter()->queryDispatchObjects(result[0], &result);
        if (result.size() == 0 || !result[0])
            return env()->throwError(GeneralError,
                                      QLatin1String("Could not construct ") + cname +
                                      QLatin1String(". Invalid constructor arguments were specified"));

         // ### this seems wrong... Why would we ever get a dispatch
         // ### object here... They should all be QObject's coming
         // ### from the factories... so the code below should always
         // ### run
        if (!qobject_cast<QuickPtrDispatchObject *>(result[0])) {
            QSObject obj = interpreter()->wrapperClass()->wrap(result);
            if (m_proxy) {
                ((QSWrapperShared *) obj.shVal())->creator = m_proxy;
            }
            return obj;
        }
        return interpreter()->pointerClass()->
            wrapPointer((QuickPtrDispatchObject*)result[ 0 ]);
    }
    return createUndefined();
}


bool QSObjectConstructor::member(const QSObject *, const QString &n, QSMember *m) const
{
    extern const QMetaObject *qsa_query_meta_object(const QByteArray &name);
    const QMetaObject *metaObj = qsa_query_meta_object(cname.toLatin1());

    if (!metaObj)
        metaObj = qsa_query_meta_object((QLatin1String("QS") + cname).toLatin1());

    if (!metaObj)
        return false;

    QByteArray matchName = n.toLatin1();
    for (int i=0; i<metaObj->enumeratorCount(); ++i) {
        QMetaEnum metaEnum = metaObj->enumerator(i);

        for (int j=0; j<metaEnum.keyCount(); ++j) {
            if (qstrcmp(metaEnum.key(j), matchName.constData()) == 0) {
                m->setName(n);
                m->setOwner(this);
                m->setType(QSMember::Custom);
                m->setWritable(false);
                m->setIndex(metaEnum.value(j));
                m->setStatic(true);
                return true;
            }
        }
    }

    return false;
}


QSObject QSObjectConstructor::fetchValue(const QSObject *,
                                          const QSMember &mem) const
{
    Q_ASSERT(mem.type() == QSMember::Custom);
    Q_ASSERT(mem.owner() == this);
    Q_ASSERT(mem.isStatic());

    return createNumber(mem.index());
}


/////////////////// pointer wrapper /////////////////////////////

QSObject QSPointerClass::wrapPointer(QuickPtrDispatchObject *ptr)
{
    QSWrapperShared *sh = new QSWrapperShared(this);
    sh->objects.resize(1);
    sh->objects[0] = ptr;
    return QSObject(this, sh);
}


QSObject QSPointerClass::wrapPointer(const char *n, void *p)
{
    QSWrapperShared *sh = new QSWrapperShared(this);
    sh->objects.resize(1);
    sh->objects[0] = new QuickPtrDispatchObject(n, p);
    interpreter()->queryDispatchObjects(n, p, &sh->objects);
    return QSObject(this, sh);
}

QString QSPointerClass::toString(const QSObject *obj) const
{
    QVector<QObject *> *objects = objectVector(obj);
    if (objects->isEmpty())
        return QLatin1String("[object Pointer]");
    return QSWrapperClass::toString(obj);
}

double QSPointerClass::toNumber(const QSObject *o) const
{
    return (ulong)pointer(o);
}

bool QSPointerClass::toBoolean(const QSObject *o) const
{
    return pointer(o) != 0;
}

QVariant QSPointerClass::toVariant(const QSObject *obj, QVariant::Type) const
{
    // ### ugly
    QString s = QString(QLatin1String("Pointer:%1:Pointer")).arg((ulong)pointer(obj));
    return QVariant(s);
}

void* QSPointerClass::pointer(const QSObject *obj) const
{
    const QVector<QObject *> *objects = objectVector(obj);
    if (objects->isEmpty())
        return 0;
    return ((QuickPtrDispatchObject*)objects->at(0))->pointer();
}

QString QSPointerClass::pointerType(const QSObject *obj) const
{
    const QVector<QObject *> *objects = objectVector(obj);
    if (objects->isEmpty())
        return "void";
    return objects->at(0)->objectName();
}

/////////////////////////////////////////////////////////////////////////////

QSVariantShared::QSVariantShared(const QVariant &v)
    : variant(v)
{
    native = (
              v.type() == QVariant::Bool ||
              v.type() == QVariant::ByteArray ||
              v.type() == QVariant::Color ||
              v.type() == QVariant::Date ||
              v.type() == QVariant::DateTime ||
              v.type() == QVariant::Double ||
              v.type() == QVariant::Font ||
              v.type() == QVariant::List ||
              v.type() == QVariant::Map ||
              v.type() == QVariant::Palette ||
              v.type() == QVariant::Pixmap ||
              v.type() == QVariant::Point ||
              v.type() == QVariant::Rect ||
              v.type() == QVariant::Size ||
              v.type() == QVariant::String ||
              v.type() == QVariant::StringList ||
              v.type() == QVariant::Time ||
              v.type() == QVariant::UInt ||
              v.type() == QVariant::Int
             );
}

void QSVariantShared::invalidate()
{
    iobj.invalidate();
    QSShared::invalidate();
}

QSVariantShared *QSVariantClass::shared(const QSObject *obj) const
{
    Q_ASSERT(obj->objectType() == this);
    return (QSVariantShared*)obj->shVal();
}

QVariant *QSVariantClass::variant(const QSObject *obj) const
{
    return &shared(obj)->variant;
}

QSObject QSVariantClass::construct(const QSList &) const
{
    // there's no "Variant" object available to the user so
    // this function should never get called. Theoretically.
    qWarning("QSVariantClass::construct(const QSList&) called");
    return construct(QVariant());
}

QSObject QSVariantClass::construct(const QVariant &v) const
{
    return QSObject(this, new QSVariantShared(v));
}

QVariant QSVariantClass::toVariant(const QSObject *obj, QVariant::Type) const
{
    return shared(obj)->variant;
}


/////////////////////////////////////////////////////////////////////////////

QuickScriptVariant::QuickScriptVariant(QuickInterpreter *ip, const QVariant &v)
    : QSObject(ip->variantClass(), new QSVariantShared(v))
{
}

QSVariantShared *QuickScriptVariant::shared() const
{
    return (QSVariantShared*)shVal();
}

bool QuickScriptVariant::isNative() const
{
    return (bool)shared()->native;
}

QSObject QuickScriptVariant::toNative() const
{
    return toPrimitive(); // ### Preferred was StringType
}

QVariant QuickScriptVariant::value() const
{
    return shared()->variant;
}

static inline QSObject stringlist_to_qsarray(const QStringList &lst, QSEnv *env)
{
    QSArray array(env);
    int i = 0;
    for (QStringList::ConstIterator it = lst.begin();
          it != lst.end(); ++it)
        array.put(QString::number(i++), env->createString(*it));
    return array;
}

void QSVariantShared::createObject(QuickInterpreter *ip)
{
    // ##### complete for other types
    const QVariant& var = variant;
    QSEnv *env = ip->env();
    switch (var.type()) {
    case QVariant::ByteArray:
        iobj = ip->byteArrayClass()->construct(var.toByteArray());
        break;
#ifndef QSA_NO_GUI
    case QVariant::Pixmap:
        iobj = ip->pixmapClass()->construct(qVariantValue<QPixmap>(var));
        break;
    case QVariant::Color:
        iobj = ip->colorClass()->construct(qVariantValue<QColor>(var));
        break;
    case QVariant::Font:
        iobj = ip->fontClass()->construct(qVariantValue<QFont>(var));
        break;
#endif
    case QVariant::Date: {
        QSList l;
        l.append(env->createNumber(var.toDate().year()));
        l.append(env->createNumber(var.toDate().month() + 1));
        l.append(env->createNumber(var.toDate().day()));
        iobj = ip->env()->dateClass()->construct(l);
        break;
    }
    case QVariant::Time: {
        QSList l;
        l.append(env->createNumber(QDate::currentDate().year()));
        l.append(env->createNumber(QDate::currentDate().month() + 1));
        l.append(env->createNumber(QDate::currentDate().day()));
        l.append(env->createNumber(var.toTime().hour()));
        l.append(env->createNumber(var.toTime().minute()));
        l.append(env->createNumber(var.toTime().second()));
        l.append(env->createNumber(var.toTime().msec()));
        iobj = ip->env()->dateClass()->construct(l);
        break;
    }
    case QVariant::DateTime: {
        QSList l;
        l.append(env->createNumber(var.toDateTime().date().year()));
        l.append(env->createNumber(var.toDateTime().date().month() + 1));
        l.append(env->createNumber(var.toDateTime().date().day()));
        l.append(env->createNumber(var.toDateTime().time().hour()));
        l.append(env->createNumber(var.toDateTime().time().minute()));
        l.append(env->createNumber(var.toDateTime().time().second()));
        l.append(env->createNumber(var.toDateTime().time().msec()));
        iobj = ip->env()->dateClass()->construct(l);
        break;
    }
    case QVariant::Point:
        iobj = ip->pointClass()->construct(var.toPoint());
        break;
    case QVariant::Size:
        iobj = ip->sizeClass()->construct(var.toSize());
        break;
    case QVariant::Rect:
        iobj = ip->rectClass()->construct(var.toRect());
        break;
    case QVariant::StringList: {
        iobj = stringlist_to_qsarray(var.toStringList(), env);
        break;
    }
    case QVariant::List: {
        QList<QVariant> lst = var.toList();
        QSArray array(env);
        int i = 0;
        for (QList<QVariant>::ConstIterator it = lst.begin(); it != lst.end(); ++it) {
            QuickScriptVariant qsvar =  QuickScriptVariant(ip, *it);
            array.put(QString::number(i++), qsvar.isNative() ? qsvar.toNative() : qsvar);
        }
        iobj = array;
        break;
    }
    case QVariant::Map: {
        QMap<QString, QVariant> map = var.toMap();
        QSObject obj;
        if (qsa_converts_map_to_object)
            obj = env->objectClass()->construct();
        else
            obj = QSArray(env);

        for (QMap<QString, QVariant>::ConstIterator it = map.begin(); it != map.end(); ++it) {
            QuickScriptVariant qsvar =  QuickScriptVariant(ip, *it);
            obj.put(it.key(), qsvar.isNative() ? qsvar.toNative() : qsvar);
        }
        iobj = obj;
        break;
    }
    case QVariant::String:
        iobj = env->createString(var.toString());
        break;
    default:
        iobj = QSObject(); // null !
        break;
    }
}

QSObject QSVariantClass::toPrimitive(const QSObject *obj,
                                      const QSClass *) const
{
    QSObject res;
    QVariant *var = variant(obj);
    QSVariantShared *sh = shared(obj);
    switch (var->type()) {
    case QVariant::Bool:
        return createBoolean(var->toBool());
    case QVariant::Int:
        return createNumber(var->toInt());
    case QVariant::UInt:
        return createNumber(var->toUInt());
    case QVariant::Double:
        return createNumber(var->toDouble());
    case QVariant::String:
        return createString(var->toString());
    case QVariant::DateTime:
        return env()->dateClass()->construct(var->toDateTime());
    case QVariant::Date:
        return env()->dateClass()->construct(QDateTime(var->toDate()));
    case QVariant::Time:
        return env()->dateClass()->construct(QDateTime(QDate::currentDate(), var->toTime()));
    case QVariant::StringList:
    case QVariant::List:
    case QVariant::Map:
    case QVariant::Rect:
    case QVariant::Size:
    case QVariant::Point:
    case QVariant::Color:
    case QVariant::Font:
    case QVariant::ByteArray:
    case QVariant::Pixmap:
        sh->createObject(interpreter());
        return sh->iobj;
#ifndef QSA_NO_GUI
    case QVariant::Palette:
        return interpreter()->paletteClass()->construct(qVariantValue<QPalette>(var));
#endif
    default:
        // fallback for cases not handled above
        return createString(toString(obj));
    }
}

bool QSVariantClass::toBoolean(const QSObject *obj) const
{
    QVariant *var = variant(obj);
    switch (var->type()) {
    case QVariant::String:
        return !var->toString().isEmpty();
    default:
        return var->toBool();
    }
}

double QSVariantClass::toNumber(const QSObject *obj) const
{
    QVariant *var = variant(obj);
    switch (var->type()) {
    case QVariant::Int:
    case QVariant::Bool:
        return double(var->toInt());
    case QVariant::UInt:
        return double(var->toUInt());
    case QVariant::Double:
        return double(var->toDouble());
    case QVariant::String:
        return QString(var->toString()).toDouble();
#ifndef QSA_NO_GUI
    case QVariant::Color:
        return double(qVariantValue<QColor>(var).rgb());
#endif
    case QVariant::Date:
        // QDate -> UTC conversion
        return -1000.0 * QDateTime(var->toDate()).secsTo(QDateTime(QDate(1970, 1, 1)));
    default:
        //qWarning("QuickScriptVariant::toNumber: unhandled QVariant type.");
        return NaN;
    }
}

QString QSVariantClass::toString(const QSObject *obj) const
{
    QVariant *var = variant(obj);
    QString str;
    switch (var->type()) {
    case QVariant::Bool:
        return var->toBool() ? QLatin1String("true") : QLatin1String("false");
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
        return QSString::from(var->toDouble());
    case QVariant::ByteArray:
    case QVariant::String:
        return var->toString();
#ifndef QSA_NO_GUI
    case QVariant::Color:
        return qVariantValue<QColor>(*var).name();
    case QVariant::Font:
        return qVariantValue<QFont>(*var).toString();
#endif
    case QVariant::Point: {
        QPoint point = var->toPoint();
        return QString::fromLatin1("(%1, %2)").arg(point.x()).arg(point.y());
    }
    case QVariant::Rect: {
        QRect rect = var->toRect();
        return QString::fromLatin1("(%1, %2, %3, %4)").arg(rect.x()).arg(rect.y()).
            arg(rect.width()).arg(rect.height());
    }
    case QVariant::Size: {
        QSize size = var->toSize();
        return QString::fromLatin1("(%1, %2)").arg(size.width()).arg(size.height());
    }
    case QVariant::Date:
        return var->toDate().toString();
    case QVariant::Time:
        return var->toTime().toString();
    case QVariant::DateTime:
        return var->toDateTime().toString();
    case QVariant::StringList:
        return var->toStringList().join(QLatin1String(","));
    case QVariant::List: {
        QList<QVariant> lst = var->toList();
        QString str;
        bool first = true;
        for (QList<QVariant>::ConstIterator it = lst.begin();
              it != lst.end(); ++it) {
            if (!first)
                str += QLatin1String(",");
            str += (*it).toString();
            first = false;
        }
        return str;
    }
    case QVariant::Map: {
        QMap<QString, QVariant> map = var->toMap();
        QString str;
        bool first = true;
        for (QMap<QString, QVariant>::ConstIterator it = map.begin();
              it != map.end(); ++it) {
            if (!first)
                str += QLatin1String(",");
            str += it.key() + QLatin1String("=") + (*it).toString();
            first = false;
        }
        return str;
    }
    default:
        return QLatin1String("undefined");
    }
}


bool QSVariantClass::member(const QSObject *objPtr, const QString &n,
                             QSMember *m) const
{
    if(!objPtr)
        return false;
    QSVariantShared *sh = shared(objPtr);
    if (!sh->iobj.isValid()) {
        sh->createObject(interpreter());
        if (!sh->iobj.isValid())
            return false;
    }
    return sh->iobj.objectType()->member(&sh->iobj, n, m);
}

QSObject QSVariantClass::fetchValue(const QSObject *objPtr,
                                     const QSMember &mem) const
{
    QSVariantShared *sh = shared(objPtr);
    Q_ASSERT(sh->iobj.isValid());
    return sh->iobj.objectType()->fetchValue(&sh->iobj, mem);
}

void QSVariantClass::write(QSObject *objPtr, const QSMember &mem,
                            const QSObject &val) const
{
    QSVariantShared *sh = shared(objPtr);
    Q_ASSERT(sh->iobj.isValid());
    sh->iobj.objectType()->write(&sh->iobj, mem, val);
    sh->variant = sh->iobj.toVariant(QVariant::Invalid);
}

QSObject QSVariantClass::invoke(QSObject * objPtr, const QSMember &mem) const
{
    QSVariantShared *sh = shared(objPtr);
    Q_ASSERT(sh->iobj.isValid());
    return sh->iobj.invoke(mem, *env()->arguments());
}

/////////////////////////////////////////////////////////////////////////////

QSApplicationClass::QSApplicationClass(QSClass *b)
    : QSWritableClass(b, AttributeAbstract), QuickEnvClass(b->env())
{
}

QSMemberMap QSApplicationClass::members(const QSObject *o) const
{
    QSMemberMap mmap = QSWritableClass::members(o);

    QObjectList *objects;
    if ((objects = interpreter()->topLevelObjects()) != 0) {
        for (int i=0; i<objects->size(); ++i) {
            QObject *o = objects->at(i);
            QSMember m;
            m.setType(QSMember::Custom);
            m.setOwner(this);
            m.setName(o->objectName());
            m.setWritable(false);
            mmap[o->objectName()] = m;
        }
    }

    return mmap;
}

bool QSApplicationClass::member(const QSObject *o, const QString &n,
                                 QSMember *m) const
{
    if (!interpreter()->children().contains(n)) // a top level object ?
        return QSWritableClass::member(o, n, m);

    m->setType(QSMember::Custom);
    m->setOwner(this);
    m->setName(n);
    m->setWritable(false);
    return true;
}

QSObject QSApplicationClass::fetchValue(const QSObject *objPtr,
                                         const QSMember &mem) const
{
    if (mem.type() != QSMember::Custom)
        return QSWritableClass::fetchValue(objPtr, mem);

    QString n = mem.name();

    QObjectList *objects = interpreter()->topLevelObjects();
    if (objects) {
        for (int i=0; i<objects->size(); ++i) {
            if (n == objects->at(i)->objectName())
                return interpreter()->wrap(objects->at(i));
        }
    }
    return createUndefined();
}

QSUserData::~QSUserData()
{
    if (dat) {
        QSWrapperShared *tmp = dat;
        tmp->invalidate(); // sets our dat to 0
        if (tmp->deref())
            delete tmp;
    }
}


static inline char qsa_first_char(const QSObject &object) {
    if (object.isUndefined())
        return 0;
    if (object.objectType() == object.objectType()->env()->numberClass()) {
        return (char) object.toNumber();
    }
    QString str = object.toString();
    return str.isEmpty() ? 0 : str.at(0).toLatin1();
}

static inline QChar qsa_first_qchar(const QSObject &object) {
    if (object.isUndefined())
        return 0;
    if (object.objectType() == object.objectType()->env()->numberClass()) {
        return (QChar) (ushort) object.toNumber();
    }
    QString str = object.toString();
    return str.isEmpty() ? 0 : str.at(0);
}

// Changes: "ClassName*" -> "ClassName"
QByteArray qsa_strip_stars(const QByteArray &name, int star_count)
{
    if (!star_count)
        return name;
    int first_star = name.length() - star_count;
    if (name.length() >= star_count + 1 && name.at(first_star) == '*')
        return name.mid(0, first_star);
    return name;
}

#define QSA2QT(cache_type, cast_type, value, match_class)                               \
                data[pos] = caching->cache_type((cast_type) value);                     \
                exact_match &= arg.objectType() == env->match_class();                  \
                break

#define QSA2QT_NAN_CHECK(cache_type, cast_type, value, match_class)                     \
                         if (qsaIsNan(value)) { match = false; break; }                 \
                         data[pos] = caching->cache_type((cast_type) value);            \
                         exact_match &= arg.objectType() == env->match_class();         \
                         break

QSObject convert_qt2qsa(QSEnv *env, void *value, const QSATypeInfo &ti, QObject *obj = 0);

static void *qsa_default_value(QSASlotCaching *caching, const QSATypeInfo &ti, const QMetaObject *mo = 0);

static bool qsa_enum_match(int enumValue, const QMetaObject *object,
                           const QByteArray &type_name, bool /*matchValue*/ = true)
{
    Q_ASSERT(object != 0);

    int idx = object->indexOfEnumerator(type_name.constData());
    if (idx >= 0) {
        // commented out code gives type safety for enum values but hinders using them as bitmasks
        //if (!matchValue)
        return true;

        /*QMetaEnum e = object->enumerator(idx);
        const char *hasValue = e.valueToKey(enumValue);
        return (hasValue != 0);*/
    } else {
        int pos = type_name.indexOf("::");

        QByteArray class_name;
        QByteArray enum_name;
        if (pos >= 0) {
            class_name = type_name.left(pos);
            enum_name = type_name.mid(pos + 2);
        }

        extern const QMetaObject *qsa_query_meta_object(const QByteArray &name);
        object = qsa_query_meta_object(class_name);

        if (object != 0)
            return qsa_enum_match(enumValue, object, enum_name);
    }

    return false;
}

// #define QSA_SLOT_DEBUG

QSObject qsa_execute_slot(QSEnv *env, QObject *qobject, const QList<int> &slot_indexes)
{
#ifdef QSA_SLOT_DEBUG
    printf("\nqsa_execute_slot\n");
#endif

    const QSList *arguments = env->arguments();
    const QMetaObject *meta_object = qobject->metaObject();

    QuickInterpreter *interpreter = QuickInterpreter::fromEnv(env);
    QSASlotCaching *caching = interpreter->slotCaching();
    caching->lock();

    void *stored_data[32];
    QList<QSATypeInfo> stored_types;

    void *data[32];
    QList<QSATypeInfo> types;
    int matched_slot = -1;


    bool exact_match = true;
    bool use_stored = false;

    for (int slot_index = 0; slot_index<slot_indexes.size(); ++slot_index) {
        int slot = slot_indexes.at(slot_index);
        QMetaMethod meta_method = meta_object->method(slot);

#ifdef QSA_SLOT_DEBUG
        printf("--- executing slot on object %s %s::%s\n",
               strlen(meta_method.typeName()) == 0 ? "void" : meta_method.typeName(),
               meta_object->className(),
               meta_method.signature());
#endif

        QSAMethodSignature signature(meta_method.signature(), meta_method.typeName());

        types = caching->signature_cache[signature];
        if (types.isEmpty()) {
            QList<QByteArray> type_names;
            type_names += meta_method.typeName();
            type_names += meta_method.parameterTypes();
            for (int i=0; i<type_names.size(); ++i) {
                QSATypeInfo t;
                t.name = type_names.at(i);
                if (t.name.startsWith("const "))
                    t.name = t.name.mid(6);
                t.id = QMetaType::type(t.name);
                if (t.id == QMetaType::Void)
                    t.id = QVariant::nameToType(t.name);
                if (t.id == QMetaType::Void) {
                    if (strcmp(t.name, "QMap<QString,QVariant>") == 0) t.id = QVariant::Map;
                    else if (strcmp(t.name, "QList<QVariant>") == 0) t.id = QVariant::List;
                    else if (strcmp(t.name, "qreal") == 0) t.id = QVariant::Double;
                }
                types << t;
            }
            caching->signature_cache[signature] = types;
        }

#ifdef QSA_SLOT_DEBUG
        for (int si = 0; si<types.size(); ++si) {
            printf("   - id=%d, type='%s'\n", types.at(si).id, (const char *) types.at(si).name);
        }
#endif

        Q_ASSERT(types.size() < 32); // Having more than 31 arguments seems unlikely (0 is return).
        bool match = types.size() == arguments->size() + 1; // +1 is the return value
        exact_match = true;

        data[0] = qsa_default_value(caching, types.at(0), meta_object);

        int pos = 1;
        for (; pos<types.size() && match; ++pos) {
            const QSATypeInfo &ti = ((const QList<QSATypeInfo> &)types).at(pos);
            const QSObject &arg = arguments->at(pos - 1);

            switch (ti.id) {

                // Boolean type
            case QMetaType::Bool: QSA2QT(bools, bool, arg.toBoolean(), booleanClass);

                // Integer types
            case QMetaType::Short: QSA2QT_NAN_CHECK(shorts, short, arg.toNumber(), numberClass);
            case QMetaType::UShort: QSA2QT_NAN_CHECK(ushorts, ushort, arg.toNumber(), numberClass);
            case QMetaType::Int: QSA2QT_NAN_CHECK(ints, int, arg.toNumber(), numberClass);
            case QMetaType::UInt: QSA2QT_NAN_CHECK(uints, uint, arg.toNumber(), numberClass);
            case QMetaType::Long: QSA2QT_NAN_CHECK(longs, long, arg.toNumber(), numberClass);
            case QMetaType::ULong: QSA2QT_NAN_CHECK(ulongs, ulong, arg.toNumber(), numberClass);

                // Float types
            case QMetaType::Float: QSA2QT(floats, float, arg.toNumber(), numberClass);
            case QMetaType::Double: QSA2QT(doubles, double, arg.toNumber(), numberClass);

                // Text and char types
            case QMetaType::Char: QSA2QT(chars, char, qsa_first_char(arg), numberClass);
            case QMetaType::UChar: QSA2QT(uchars, uchar, qsa_first_char(arg), numberClass);
            case QMetaType::QChar: QSA2QT(qchars, QChar, qsa_first_qchar(arg), numberClass);
            case QMetaType::QString: QSA2QT(strings, QString, arg.toString(), stringClass);

                // Pointer types
            case QMetaType::QObjectStar:
                if (arg.isA(interpreter->wrapperClass())) {
                    data[pos] = caching->voidptrs(QSWrapperClass::object(&arg));
                } else if (arg.isNull() || arg.isUndefined()) {
                    data[pos] = caching->voidptrs(0);
                } else {
                    match = false;
                }
                break;
#ifndef QSA_NO_GUI
            case QMetaType::QWidgetStar:
                {
                    if (arg.isA(interpreter->wrapperClass())) {
                        QWidget *w = qobject_cast<QWidget *>(QSWrapperClass::object(&arg));
                        if (w)
                            data[pos] = caching->voidptrs(w);
                        else
                            match = false;
                    } else if (arg.isNull() || arg.isUndefined()) {
                        data[pos] = caching->voidptrs(0);
                    } else {
                        match = false;
                    }
                }
                break;
#endif
            case QMetaType::VoidStar:
                if (arg.isA(interpreter->pointerClass()))
                    data[pos] =
                        caching->voidptrs(((QSPointerClass *)arg.objectType())->pointer(&arg));
                else if (arg.isA(interpreter->wrapperClass()))
                    data[pos] = caching->voidptrs(QSWrapperClass::object(&arg));
                else if (arg.isNull() || arg.isUndefined())
                    data[pos] = caching->voidptrs(0);
                else
                    match = false;
                break;

                // Custom variant types
            case QVariant::Map:
                {
                    const QVariant *v = caching->variants(arg.toVariant(QVariant::Map));
                    data[pos] = const_cast<void *>(v->constData());
                    match = arg.objectType() == env->arrayClass()
                            && v->type() == QVariant::Map;
                }
                break;
            case QVariant::List:
                {
                    const QVariant *v = caching->variants(arg.toVariant(QVariant::List));
                    data[pos] = const_cast<void *>(v->constData());
                    match = arg.objectType() == env->arrayClass()
                            && v->type() == QVariant::List;

                }
                break;

            default:
                {
                    int star_count = ti.name.count('*');
                    QByteArray ti_name = qsa_strip_stars(ti.name, star_count);
                    int ti_id = QVariant::nameToType(ti_name);

                    const QVariant *v = caching->variants(arg.toVariant(QVariant::Type(ti_id)));

                    if (v->isValid() && (int)v->type() == ti_id) {
                        data[pos] = const_cast<void *>(v->constData());

                    } else if (strcmp(ti_name, "QVariant") == 0) {
                        const QVariant *v = caching->variants(arg.toVariant(QVariant::Invalid));
                        data[pos] = (void *)v;

                    } else if (strcmp(ti_name, "QList<int>") == 0) {
                        if (arg.objectType() == env->arrayClass()) {
                            QList<int> tmp_list;
                            int len = QSArrayClass::length(&arg);
                            for (int ai=0; ai<len; ++ai)
                                tmp_list << (int) arg.get(QString::number(ai)).toNumber();
                            data[pos] = caching->intlists(tmp_list);
                        } else {
                            match = false;
                        }
                    } else if (strcmp(ti_name, "QObjectList") == 0) {
                        if (arg.objectType() == env->arrayClass()) {
                            QObjectList tmp_list;
                            int len = QSArrayClass::length(&arg);
                            for (int ai=0; ai<len; ++ai) {
                                QSObject a = arg.get(QString::number(ai));
                                if (a.isA(interpreter->wrapperClass()))
                                    tmp_list << QSWrapperClass::object(&a);
                            }
                            data[pos] = caching->qobjectlists(tmp_list);
                        } else {
                            match = false;
                        }
                    } else if (strcmp(ti_name, "char") == 0) {
                        QByteArray *ba = caching->bytearrays(arg.toString().toLocal8Bit());
                        data[pos] = const_cast<char *>(ba->constData());
                        exact_match &= arg.objectType() == env->stringClass();
                    } else {
                        if (arg.isA(interpreter->wrapperClass())) {
                            QObject *object = QSWrapperClass::object(&arg);
                            if (object->inherits(ti_name)) {
                                data[pos] = object; // assign w/o cache since its done below
                            } else {
                                match = false;
                            }

                        } else if (star_count) {
                            const QByteArray match_name = star_count == 1
                                                          ? ti_name
                                                          : qsa_strip_stars(ti.name, 1);

                            // Check pointer name against original (nonstripped) name
                            if (arg.isA(interpreter->pointerClass())
                                && interpreter->pointerClass()->pointerType(&arg) == match_name) {
                                void *ptr = interpreter->pointerClass()->pointer(&arg);
                                data[pos] = ptr; // assign w/o cache since its done below
                            } else if (arg.isNull() || arg.isUndefined()) {
                                data[pos] = 0; // assign w/o cache since its done below
                            } else {
                                match = false;
                            }
                        } else if (qsa_enum_match(arg.toInteger(), meta_object, ti_name)) {
                            QSA2QT(ints, int, arg.toNumber(), numberClass);
                        } else {
                            match = false;
                        }
                    }


                    if (match) {
                        for (int i=0; i<star_count; ++i)
                            data[pos] = caching->voidptrs(data[pos]);
                    }

                    break;
                }
            }
        }

        if (match) {
            if (exact_match) {
                // Found exact match so exit and call it.
                matched_slot = slot;
                use_stored = false;
                break;
            } else if (matched_slot < 0) {
                matched_slot = slot;
                // keep for later
                if (slot_indexes.size() > 1) {
                    use_stored = true;
                    for (int i=0; i<pos; ++i)
                        stored_data[i] = data[i];
                }
                stored_types = types;
            }

        }

    }

    QSObject return_value;

    if (matched_slot >= 0) {

#ifdef QSA_SLOT_DEBUG
        {
            QMetaMethod meta_method = meta_object->method(matched_slot);
            printf("   - calling %s::%s, exact=%d, use_stored=%d\n",
                   meta_object->className(), meta_method.signature(), exact_match, use_stored);
        }
#endif

        bool ok = qobject->qt_metacall(QMetaObject::InvokeMetaMethod, matched_slot,
                                       use_stored ? stored_data : data);
        Q_UNUSED(ok);
        Q_ASSERT(ok);

        return_value = convert_qt2qsa(env,
                                      (use_stored ? stored_data : data)[0],
                                      (use_stored ? stored_types : types).at(0),
                                      qobject);


    } else {
        QString msg("No matching slot found, available overloads are:\n");

        for (int i=0; i<slot_indexes.size(); ++i) {
            QMetaMethod meta_method = meta_object->method(slot_indexes.at(i));
            const char *return_type = meta_method.typeName();
            msg += QString("    %1 %2::%3\n")
                   .arg(strlen(return_type) ? return_type : "void")
                   .arg(meta_object->className())
                   .arg(meta_method.signature());
        }

        return_value = env->throwError(msg);
    }

    caching->unlock();

    return return_value;
}

static QSObject qsa_pod_value(QSEnv *env, QMetaType::Type type, void *value)
{
    switch (type) {
    case QMetaType::Bool: return env->createNumber(*((bool *) value));
    case QMetaType::Short: return env->createNumber(*((short *) value));
    case QMetaType::UShort: return env->createNumber(*((ushort *) value));
    case QMetaType::Int: return env->createNumber(*((int *) value));
    case QMetaType::UInt: return env->createNumber(*((uint *) value));
    case QMetaType::Float: return env->createNumber(*((float *) value));
    case QMetaType::Double: return env->createNumber(*((double *) value));
    case QMetaType::Char: return env->createString(QString("%1").arg(*((char *) value)));
    case QMetaType::UChar: return env->createString(QByteArray(1, ((uchar *)value)[0]));
    case QMetaType::Long: return env->createNumber(*((long *) value));
    case QMetaType::ULong: return env->createNumber(*((ulong *) value));
    default: return QSObject();
    };
}

static QSObject qsa_script_variant(QuickInterpreter *ip, QVariant::Type type, void *value)
{
    QVariant var(type, value);

    if (var.isValid()) {
        QuickScriptVariant variant(ip, var);
        return variant.isNative() ? variant.toNative() : variant;
    }

    return QSObject();
}

QSObject convert_qt2qsa(QSEnv *env, void *value, const QSATypeInfo &ti, QObject *qobj)
{
#ifdef QSA_SLOT_DEBUG
    printf("convert_qt2qsa, '%s' type=%d\n", (const char *) ti.name, ti.id);
#endif

    // Special case the meta types that are not known to QVariant
    QSObject ret_val = qsa_pod_value(env, QMetaType::Type(ti.id), value);
    if (ret_val.isValid())
        return ret_val;

    // The case for void functions.
    if (ti.id == QMetaType::Void && ti.name.isEmpty())
        return env->createUndefined();

    QuickInterpreter *ip = QuickInterpreter::fromEnv(env);
    ret_val = qsa_script_variant(ip, QVariant::Type(ti.id), value);

    if (ti.id < int(QVariant::UserType) && ret_val.isValid()) {
        return ret_val;

    } else if (ti.name == "QVariant") {
        // We could have casted directly, but we choose to use
        // the common code path
        QVariant *var = ((QVariant *) value);
        if (var->isNull())
            return env->createUndefined();
        return qsa_script_variant(ip, var->type(), (void *) var->constData());

    } else if (ti.name == "QList<int>") {
        QList<int> *l = (QList<int> *) value;
        QSArray array(env);
        for (int i=0; i<l->size(); ++i)
            array.put(QString::number(i), env->createNumber(l->at(i)));
        return array;

    } else if (ti.name == "QObjectList") {
        QObjectList *l = (QObjectList *) value;
        QSArray array(env);
        for (int i=0; i<l->size(); ++i)
            array.put(QString::number(i), ip->wrap(l->at(i)));
        return array;

    } else {
        int star_count = ti.name.count('*');
        if (star_count) {

            // See if it is a QObject subclass first of all
            extern const QMetaObject *qsa_query_meta_object(const QByteArray &name);

            QByteArray stripped_name = qsa_strip_stars(ti.name);
            const QMetaObject *meta_object = qsa_query_meta_object(stripped_name);

            if (meta_object) {
                return ip->wrap(*((QObject **)value));

            } else {

                // Strip away stars to see if it matches a
                // variant type and then treat it as such
                QByteArray stripped = qsa_strip_stars(ti.name, star_count);
                QVariant::Type var_type = QVariant::nameToType(stripped);

                if (var_type != QVariant::Invalid && var_type < QVariant::UserType) {
                    while (star_count--) value = *(void **) value;
                    return qsa_script_variant(ip, var_type, value);
                } else if (stripped == "QVariant") {
                    while (star_count--) value = *(void **) value;
                    QVariant *var = ((QVariant *) value);
                    return qsa_script_variant(ip, var->type(), (void *) var->constData());
                } else if (var_type == QVariant::UserType) {
                    // Handle pointers to PODs not automatically handled by QVariant. Should we do
                    // char* as a special case to get strings?
                    int meta_type = QMetaType::type(stripped);

                    while (star_count--) value = *(void **) value;

                    ret_val = qsa_pod_value(env, QMetaType::Type(meta_type), value);
                    if (ret_val.isValid())
                        return ret_val;
                } else if (qobj != 0) {
                    QByteArray qualified_name = QByteArray(qobj->metaObject()->className());
                    int pos = qualified_name.lastIndexOf("::");
                    if (pos >= 0) {
                        qualified_name = qualified_name.left(pos + 2);
                        qualified_name += stripped_name;

                        if (qsa_query_meta_object(qualified_name))
                            return ip->wrap(*((QObject **)value));
                    }
                }

                // if all else fails, wrap it as a pointer.
                if (*(void **)value) {
                    return ip->pointerClass()->wrapPointer(star_count == 1
                                                           ? stripped
                                                           : qsa_strip_stars(ti.name, 1),
                                                           *((void **)value));
                } else {
                    return env->createUndefined();
                }
            }
        } else {
            if (qobj != 0 && value != 0 && qsa_enum_match(0, qobj->metaObject(), ti.name, false)) {
                int i = *(int *) value;
                return env->createNumber(i);
            }
        }
        printf("::convert_qt2qsa(), unhandled type id=%d, name='%s'\n",
               ti.id, (const char *) ti.name);
    }

    return env->createUndefined();
}

static void *qsa_default_value(QSASlotCaching *caching, const QSATypeInfo &ti, const QMetaObject *mo)
{
    const QVariant *v = caching->variants(QVariant::Type(ti.id));
    void *const_data = (void *) v->constData();

    if (v->type() == QVariant::Invalid) {
        if (ti.name == "QVariant") {
            return caching->variants(QVariant());
        } else if (ti.name == "QList<int>")
            return caching->intlists(QList<int>());
        else if (ti.name == "QObjectList")
            return caching->qobjectlists(QObjectList());
        else if (mo != 0 && qsa_enum_match(0, mo, ti.name, false))
            return caching->ints(0);
        else if (ti.name.count('*') == 0 && ti.name.length() > 0)
            return 0;
    }

    return const_data;
}

