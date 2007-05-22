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

#include "quickinterpreter.h"

#include "quickobjects.h"
#include "quickdebugger.h"
#include "quickdispatchobject.h"
#include "quickbytearrayobject.h"
#ifndef QSA_NO_GUI
#include "quickpixmapobject.h"
#include "quickcolorobject.h"
#include "quickfontobject.h"
#endif
#include "quickcoordobjects.h"
#include "quickobjects.h"
#include "qsenv.h"
#include "qserrors.h"
#include "qsfunction.h"
#include "qsfuncref.h"
#include <qobject.h>
#include <qmetaobject.h>
#include <qmutex.h>
#include "qsmetaobject.h"
#include "qsslot.h"
#include <qcoreevent.h>

#include <QList>
#include <QByteArray>
#include <QDir>
#include <QVector>
#include <qdir.h>
#include <qsclass.h>
#include <qsnodes.h>

#ifndef QSA_NO_GUI
#include <qapplication.h>
#endif

extern bool qsa_is_non_gui_thread();

static QSObject qsStartTimer(QSEnv *env);
static void qsKillTimer(QSEnv *env);
static void qsKillTimers(QSEnv *env);

static QSObject qsa_connect(QSEnv *env);
static QSObject qsa_disconnect(QSEnv *env);

static void qsa_setup_meta_objects();

static bool eTimers = true;
static int static_id = 0;

QuickInterpreter::QuickInterpreter(bool deb)
    : toplevel(0)
{
    factory = new QuickDispatchObjectFactory(this);
    shuttingDown = false;
    id = static_id++;
    wrapperShared = new QSWrapperSharedList;
    usrDataId = QObject::registerUserData();

    debugger = deb ? new QuickDebugger(this) : 0;
    init();
}

QuickInterpreter::~QuickInterpreter()
{
    shuttingDown = true;
    qsKillTimers(env());
    invalidateWrappers();
    delete factory;
    delete toplevel;
    delete debugger;
    Q_ASSERT(wrapperShared->isEmpty());
    delete wrapperShared;
}

/*!
  Retrieves a pointer to the interpreter class from an environment
  pointer.
 */

QuickInterpreter *QuickInterpreter::fromEnv(QSEnv *e)
{
    return (QuickInterpreter*)e->engine();
}

void QuickInterpreter::reinit()
{
    QSEngine::init();
}

void QuickInterpreter::enableTimers(bool b)
{
    eTimers = b;
}

bool QuickInterpreter::timersEnabled()
{
    return eTimers;
}

void QuickInterpreter::timerEvent(QTimerEvent *e)
{
    int id = e->timerId();
    QSObject function = m_timers[id];

    Q_ASSERT(function.isValid());
    Q_ASSERT(function.isFunction());

    QSList arguments;
    arguments.append(env()->createNumber(id));

    QSObject func_base = QSFuncRefClass::refBase(function);
    QString func_name = QSFuncRefClass::refMember(function).name();

   call(func_base, func_name, arguments);
}

QVariant QuickInterpreter::convertToArgument(const QSObject &o)
{
    if(!o.isValid())
	return QVariant();
    const QSClass *cl = o.objectType();
    if(cl->name() == QString::fromLatin1("QObject")) {
	QSWrapperShared *shared = (QSWrapperShared*) o.shVal();
	if (shared->objects.isEmpty())
            return QVariant();
        QVariant v(QMetaType::QObjectStar, static_cast<QObject *>(0));
        qVariantSetValue<QObject *>(v, shared->objects[0]);
        return v;
    } else if(cl == ptrClass) {
	Q_ASSERT(ptrClass->pointer(&o));
        QVariant v(QMetaType::VoidStar, static_cast<void *>(0));
        qVariantSetValue<void *>(v, ptrClass->pointer(&o));
        return v;
    } else {
	return QVariant(o.toVariant(QVariant::Invalid));
    }
}

QSObject QuickInterpreter::wrap(QObject *o)
{
    if(!o) {
	QSList list;
	return env()->nullClass()->construct(list);
    }

    if (qsa_is_non_gui_thread() && o->isWidgetType()) {
	qWarning("QuickInterpreter::wrap(), GUI object (%s [%s]) cannot be used in non GUI thread",
		 o->objectName().toLatin1().constData(), o->metaObject()->className());
	return QSObject();
    }

    QSUserData *udata = (QSUserData*) o->userData(userDataId());
    // set user data in QObject if it's not there, yet
    if(!udata) {
	udata = new QSUserData(0);
	o->setUserData(userDataId(), udata);
    }
    QSWrapperShared *shared = udata->data();
    const QSWrapperClass *cl;
    if (shared) {
	// wrapper is already there, reuse it
	cl = shared->wrapperClass();
    } else {
	// create & remember wrapper
	cl = new QSWrapperClass(wrapperClass());
	shared = cl->createShared(o);
	shared->setObjectType(QSWrapperShared::GlobalObject);
	env()->registerShared(shared);
	wrapperShared->append(shared);
    }
    shared->setUserData(udata);
    udata->setData(shared);
    shared->ref(); // additional ref by either QObject or QSObject
    QSObject obj(cl);
    obj.setVal(shared); // no ownership needs to be transferred anymore
    return obj;
}

void QuickInterpreter::init()
{
    m_dynamic_slots = new QSMetaObject(this);

    qsa_setup_meta_objects();
    debuggerEngine()->clear();

    staticGlobals.clear();

    staticGlobals << QString::fromLatin1("NaN")
                  << QString::fromLatin1("undefined")
                  << QString::fromLatin1("Infinity")
                  << QString::fromLatin1("Application");

    // add some common objects to the Global object
    QSObject global(env()->globalObject());
    QSClass *objClass = env()->objectClass();

    wrpClass = new QSWrapperClass(objClass);
    ptrClass = new QSPointerClass(objClass);
    varClass = new QSVariantClass(objClass);
    appClass = new QSApplicationClass(objClass);
    global.put(QString::fromLatin1("Application"), appClass->createWritable());

    pntClass = new QSPointClass(objClass, this);
    registerType(pntClass);
    sizClass = new QSSizeClass(objClass, this);
    registerType(sizClass);
    rctClass = new QSRectClass(objClass, this);
    registerType(rctClass);
    baClass = new QSByteArrayClass(objClass);
    registerType(baClass);
#ifndef QSA_NO_GUI
    colClass = new QSColorClass(objClass);
    registerType(colClass);
    fntClass = new QSFontClass(objClass);
    registerType(fntClass);
    pixClass = new QSPixmapClass(objClass, this);
    registerType(pixClass);
    palClass = new QSPaletteClass(objClass);
    registerType(palClass);
    colGrpClass = new QSColorGroupClass(objClass);
    registerType(colGrpClass);
#endif

    enableDebug(); // adds "debug" function which uses qDebug()
//     //
    env()->globalClass()->addMember(QString::fromLatin1("connect"), QSMember(qsa_connect));
    env()->globalClass()->addMember(QString::fromLatin1("disconnect"), QSMember(qsa_disconnect));
    env()->globalClass()->addMember(QString::fromLatin1("startTimer"), QSMember(qsStartTimer));
    env()->globalClass()->addMember(QString::fromLatin1("killTimer"), QSMember(qsKillTimer));
    env()->globalClass()->addMember(QString::fromLatin1("killTimers"), QSMember(qsKillTimers));

    QMap<QString,QObject*> statDescr = factory->staticDescriptors();
    QMap<QString,QString> instDescr = factory->instanceDescriptors();

    QList<QString> features = instDescr.keys();
    for (QList<QString>::ConstIterator it = features.begin();
	  it != features.end(); ++it) {
        if (env()->globalClass()->definedMembers()->contains(*it)) {
            qWarning("QSObjectFactory: Trying to register existing class: '%s'", (*it).toLatin1().constData());
            continue;
        }
	QSObject staticInst;
	if(statDescr.contains(*it)) { // has static?
	    QObject *sinst = statDescr[ *it ];
	    Q_ASSERT(sinst);
	    staticInst = wrap(sinst);
	    statDescr.remove(*it);
	}
	QSObjectConstructor *constr =
	    new QSObjectConstructor(objClass, *it);
	QSFactoryObjectProxy *ptype =
	    new QSFactoryObjectProxy(env()->typeClass(), staticInst, constr);
        constr->setFactoryObjectProxy(ptype);
	QSObject proxy(ptype, env()->typeClass()->createTypeShared(constr));
	env()->globalClass()->addStaticVariableMember(constr->identifier(),
						       proxy,
						       AttributeExecutable);
    }

    for(QMap<QString,QObject*>::ConstIterator sit = statDescr.begin();
	 sit != statDescr.end(); ++sit) {
        if (env()->globalClass()->definedMembers()->contains(sit.key())) {
            qWarning("QSObjectFactory: Trying to register existing class: '%s'", sit.key().toLatin1().constData());
            continue;
        }
	QSObject staticInst;
	QObject *sinst = statDescr[ sit.key() ];
	Q_ASSERT(sinst);
	staticInst = wrap(sinst);
	env()->globalClass()->addStaticVariableMember(sit.key(), staticInst, AttributeNone);
    }
}

bool QuickInterpreter::hasTopLevelParent(QObject *o)
{
    if (!toplevel)
        return false;
    for (int i=0; i<toplevel->size(); ++i) {
        QObject *p = toplevel->at(i);
	QObject *c = 0;
    QList<QObject *> cs = qFindChildren<QObject *>(p, o->objectName());
    for (int i=0; i<cs.size(); ++i) {
        if (cs.at(i)->inherits(o->metaObject()->className()))
            c = cs.at(i);
    }

	if (c)
	    return true;
    }
    return false;
}

void QuickInterpreter::addTopLevelObject(QObject *o)
{
    QSEngine::init();
    if (!toplevel)
	toplevel = new QObjectList;
    if (toplevel->indexOf(o) != -1)
	return;

    Q_ASSERT(!hasTopLevelParent(o));

    for(int i=0; i<toplevel->size(); ++i) {
        QObject *cur = toplevel->at(i);
	if(cur == o) {
	    return;
	} else if (cur && o
                   && cur->objectName() == o->objectName()) {
	    return;
	}
    }


    toplevel->append(o);

    kids.clear();
    if (!toplevel)
	return;
    for (int i=0; i<toplevel->size(); ++i) {
        QObject *obj = toplevel->at(i);
	kids.append(obj->objectName());
    }
    connect(o, SIGNAL(destroyed(QObject *)), this, SLOT(topLevelDestroyed(QObject *)));

    QSObject global = env()->globalObject();
    const_cast<QSClass *>(global.objectType())->deleteMember(o->objectName());
    QString context_name = o->objectName();
    if (context_name.isEmpty()) {
        context_name = "unnamed";
    } else {
        staticGlobals << context_name;
    }
    env()->globalObject().put(context_name, wrap(o));
}

void QuickInterpreter::setTopLevelObjects(QObjectList *l)
{
    QSEngine::init();
    if(toplevel)
        for (int i=0; i<toplevel->size(); ++i) {
            QObject *o = toplevel->at(i);
	    disconnect(o, SIGNAL(destroyed(QObject*)),
			this, SLOT(topLevelDestroyed(QObject*)));
        }
    delete toplevel;
    toplevel = new QObjectList;

    kids.clear();
    if (!l) {
	toplevel->clear();
	return;
    }
    QSObject global(env()->globalObject());

    for (int i=0; i<toplevel->size(); ++i) {
        QObject *o = toplevel->at(i);
	if (hasTopLevelParent(o)) {
	    continue;
	}
	kids.append(o->objectName());
	connect(o, SIGNAL(destroyed(QObject *)),
		 this, SLOT(topLevelDestroyed(QObject *)));
	global.put(o->objectName(), wrap(o));
	staticGlobals << o->objectName();
	toplevel->append(o);
    }
    delete l;
}

void QuickInterpreter::topLevelDestroyed(QObject *o)
{
    QHash<int, QSAConnection>::iterator it;
    for (it=m_script_connections.begin(); it!=m_script_connections.end();) {
        if (it->sender == o)
            it = m_script_connections.erase(it);
        else
            ++it;
    }
    toplevel->removeAll(o);
}

bool QuickInterpreter::checkSyntax(const QString &c)
{
    QString code = c + QString::fromLatin1("\n");
    return QSEngine::checkSyntax(code);
}

QVariant QuickInterpreter::execute(QObject *obj, const QString &c,
				      const QString &name)
{
    QString code = c + QString::fromLatin1("\n");

    int sourceId = debugger ? debugger->freeSourceId() : -1;
    if(!name.isNull() && sourceId >= 0)
	sourceIdNames[sourceId] = name;

    QSObject t, oldThis;
    if (obj) {
	if (!name.isNull() && sourceId >= 0)
            addSourceId(sourceId, obj);

	if (!hasTopLevelParent(obj))
            addTopLevelObject(obj);

	t = wrap(obj);
	oldThis = env()->thisValue();
	env()->setThisValue(t);
    }

    QSEngine::evaluate(t, code);

    // restore this value
    if (obj)
	env()->setThisValue(oldThis);

    if (hadError())
	if(errorType() == QSErrParseError)
	    emit parseError();
	else
	    emit runtimeError();

    // Make sure we dereference the engines return value to avoid pooling
    QVariant a = convertToArgument(returnValue());
    setReturnValue(QSObject());
    return a;
}

QVariant QuickInterpreter::call(QSObject ctx, const QString &func,
				 const QSList &args)
{
    if (shuttingDown)
	return QVariant();

    QSEngine::call(&ctx, func, args);

    if (hadError())
	emit runtimeError();

    // Make sure we dereference the engines return value to avoid pooling
    QVariant a = convertToArgument(returnValue());
    setReturnValue(QSObject());
    return a;
}

QVariant QuickInterpreter::call(QObject *ctx, const QString &func,
				 const QSList &args)
{
    if (shuttingDown)
	return QVariant();

    QSObject t;
    if (ctx)
	t = wrap(ctx);

    QSEngine::call(&t, func, args);

    if (hadError())
	emit runtimeError();

    // Make sure we dereference the engines return value to avoid pooling
    QVariant a = convertToArgument(returnValue());
    setReturnValue(QSObject());
    return a;
}

QVariant QuickInterpreter::call(QObject *ctx, const QString &func, const QVariantList &args)
{
    QSList l;
    for (int i=0; i<args.size(); ++i) {
        const QVariant &v = args.at(i);
	switch (v.type()) {
        case QMetaType::QObjectStar:
	    l.append(wrap(qvariant_cast<QObject *>(v)));
	    break;
	case QMetaType::VoidStar:
	    qWarning("QuickInterpreter::call: don't know what to do with a "
		      "QVariant::VoidPointer here...");
 	    break;
	default:
            {
                QuickScriptVariant qsvar(this, v);
                if (qsvar.isNative())
                    l.append(qsvar.toNative());
                else
                    l.append(qsvar);
                break;
            }
	}
    }
    return call(ctx, func, l);
}

void QuickInterpreter::setVariable(QObject *context, const QString &func, const QVariant &value)
{
    QSObject val;
    switch(value.type()) {
    case QMetaType::QObjectStar:
	val = wrap(qvariant_cast<QObject *>(value));
	break;
    case QMetaType::VoidStar:
	qWarning("QuickInterpreter::setVariable: don't know what to do with "
		  "QVariant::VoidPointer here...");
	return;
    default:
        {
            QuickScriptVariant qsvar(this, value);
            if (qsvar.isNative())
                val = qsvar.toNative();
            else
                val = qsvar;
            break;
        }
    }

    if (context)
	wrap(context).put(func, val);
    else
	env()->globalObject().put(func, val);
}

QVariant QuickInterpreter::variable(QObject *context, const QString &varName)
{
    QSObject obj = context ? wrap(context) : env()->globalObject();
    return convertToArgument(obj.getQualified(varName));
}

void QuickInterpreter::invalidateWrappers()
{
    wrapperShared->clear();
}

void QuickInterpreter::clear()
{
//     printf("QuickInterpreter::clear()\n");
    // Clear all connections
    while (m_script_connections.size()) {
        QHash<int, QSAConnection>::Iterator it = m_script_connections.begin();
//         printf(" -> removing connection: %d\n", it.key());
        removeConnection(it->sender, it->signal.toLatin1(), it->function_ref);
    }

    // clean out slots
    delete m_dynamic_slots;
    m_dynamic_slots = 0;

    sourceIdNames.clear();
    debuggerEngine()->clear();
    qsKillTimers(env());
    invalidateWrappers();

    if(toplevel)
	toplevel->clear();
    QSEngine::clear();
    QSEngine::init();
    init();
}

void QuickInterpreter::stop()
{
    qsKillTimers(env());
    debuggerEngine()->clear();
}

bool QuickInterpreter::hadError() const
{
    return !!QSEngine::errorType();
}

/*!
  Emit warning message \a msg for line \a l.
*/

void QuickInterpreter::warn(const QString &msg, int l)
{
    emit warning(msg, l);
}

void QuickInterpreter::clearSourceIdMap()
{
    sourceIdMap.clear();
}

void QuickInterpreter::addSourceId(int id, QObject *o)
{
    sourceIdMap.insert(id, o);
}

QObject *QuickInterpreter::objectOfSourceId(int id) const
{
    QMap<int, QObject*>::ConstIterator it = sourceIdMap.find(id);
    if (it == sourceIdMap.end())
	return 0;
    return *it;
}

int QuickInterpreter::sourceIdOfObject(QObject *o) const
{
    for (QMap<int, QObject*>::ConstIterator it = sourceIdMap.begin(); it != sourceIdMap.end(); ++it) {
	if (*it == o)
	    return it.key();
	else if ((*it)->objectName() == o->objectName())
	    return it.key();
    }
    return -1;
}

QString QuickInterpreter::nameOfSourceId(int id) const
{
    QMap<int,QString>::ConstIterator it = sourceIdNames.find(id);
    if (it == sourceIdNames.end())
	return QString::null;
    return *it;
}

int QuickInterpreter::sourceIdOfName(const QString &name) const
{
    for (QMap<int,QString>::ConstIterator it = sourceIdNames.begin();
	  it != sourceIdNames.end(); ++it) {
	if (*it == name)
	    return it.key();
    }
    return -1;
}


void QuickInterpreter::cleanType(QString &type)
{
    type = type.simplified();
    if (type.left(5) == QString::fromLatin1("const"))
	    type.remove(0, 5);
    if (type.length() > 0 && (type[ (int)type.length() - 1 ] == '&' ||
	                          type[ (int)type.length() - 1 ] == '*'))
	    type.remove(type.length() - 1, 1);
    type = type.simplified();
    if (type == QString::fromLatin1("QString"))
	    type = QString::fromLatin1("String");
    else if (type == QString::fromLatin1("int") || type == QString::fromLatin1("uint") ||
	         type == QString::fromLatin1("long") || type == QString::fromLatin1("ulong") ||
	         type == QString::fromLatin1("double") || type == QString::fromLatin1("float"))
	    type = QString::fromLatin1("Number");
    else if (type == QString::fromLatin1("bool"))
	    type = QString::fromLatin1("Boolean");
}

void QuickInterpreter::cleanTypeRev(QString &type)
{
    if (type == QString::fromLatin1("String"))
	type = QString::fromLatin1("QString");
    else if (type == QString::fromLatin1("Number"))
	type = QString::fromLatin1("double");
    else if (type == QString::fromLatin1("Boolean"))
	type = QString::fromLatin1("bool");
}

bool QuickInterpreter::queryDispatchObjects(QObject *obj, QVector<QObject *> *result)
{
    Q_ASSERT(obj);
    const QMetaObject *meta = obj->metaObject();
    result->resize(1);
    result->replace(0, obj);
    while (meta) {
	factory->constructInterface(meta->className(), (void*)obj, result);
	meta = meta->superClass();
    }
    return true;
}

bool QuickInterpreter::queryDispatchObjects(const QByteArray &name, void *ptr,
                                            QVector<QObject *> *result)
{
    return factory->constructInterface(name, ptr, result);
}

bool QuickInterpreter::construct(const QString &className, const QVariantList &vargs,
                                 QVector<QObject *> *result)
{
    return factory->constructInstance(className, vargs, result);
}

QStringList QuickInterpreter::classes() const
{
    const QList<QSClass *> &classList = env()->classes();
    QStringList lst;
    for (int i=0; i<classList.size(); ++i) {
        const QSClass *c = classList.at(i);
	if (c->asClass())
	    lst << c->asClass()->identifier();
    }
    return lst;
}


QSObject QuickInterpreter::object(const QString &name) const
{
    QSObject g = env()->globalObject();
    Global *global = (Global*)&g; // ### ugly
    QSObject obj;
    if (name.isEmpty()) {
	obj = g;
    } else {
	int p = name.lastIndexOf('.');
	if (p == -1)
	    obj = global->get(name);
	else
	    obj = global->getQualified(name);
    }
    return obj;
}


const QSClass *QuickInterpreter::classOf(const QSObject &obj) const
{
    return obj.isA(env()->typeClass()) ? QSTypeClass::classValue(&obj) : obj.objectType();
}

QStringList QuickInterpreter::functionsOf(QSObject &obj, bool includeSignature, bool includeNative, bool includeMemberFunctions) const
{
    const QSClass *objType = obj.objectType();
    QSMemberMap mmap = objType == env()->typeClass() && includeMemberFunctions
		       ? ((QSTypeClass*) objType)->allMembers(&obj)
		       : objType->members(&obj);

    QMap<int, QString> functions;
    for (QSMemberMap::Iterator it = mmap.begin(); it != mmap.end(); ++it) {
	if (((*it).type() == QSMember::ScriptFunction ||
	       includeNative && ((*it).type() == QSMember::NativeFunction ||
				  (*it).type() == QSMember::NativeMemberFunction))
	     && !(*it).isPrivate()) {
	    QString func = (*it).name();
	    // ### will break with mix of script and C++ functions
	    int idx = (*it).type() == QSMember::ScriptFunction ?
		      (*it).scriptFunction->index() : functions.size();
	    if (includeSignature) {
		if ((*it).type() == QSMember::NativeFunction ||
		     (*it).type() == QSMember::NativeMemberFunction) {
		    func += QString::fromLatin1("()");
		} else {
		    QSFunctionScopeClass *fsc = (*it).scriptFunction->scopeDefinition();
		    int args = fsc->numArguments();
		    func += QString::fromLatin1("(");
		    if (args > 0) {
			func += QString::fromLatin1(" ");
			QSMemberMap *members = fsc->definedMembers();
			QVector<QString> vec(args);
			for (QSMemberMap::ConstIterator ait = members->begin();
			      ait != members->end(); ++ait) {
			    if ((*ait).index() < args)
				vec[ (*ait).index() ] = (*ait).name();
			}
			for (int i = 0; i < args; ++i) {
			    if (i > 0)
				func += QString::fromLatin1(", ");
			    func += vec[i];
			}
			func += QString::fromLatin1(" ");
		    }
		    func += QString::fromLatin1(")");
		}
	    }
	    functions[idx] = func;
	}
    }

    // assemble to list sorted by original index
    QStringList lst;
    QMap<int, QString>::const_iterator cit = functions.begin();
    for (; cit != functions.end(); ++cit)
	    lst << *cit;

    return lst;
}

QStringList QuickInterpreter::classesOf(QSObject &obj) const
{
    const QSClass *cls = classOf(obj);
    QStringList lst;
    for (int i = 0; i < cls->numStaticVariables(); ++i) {
	QSObject o = cls->staticMember(i);
	if (o.isA(env()->typeClass()) && QSTypeClass::classValue(&o)->asClass())
	    lst << QSTypeClass::classValue(&o)->identifier();
    }
    return lst;
}

QStringList QuickInterpreter::variablesOf(QSObject &obj, bool includeStatic, bool includeCustom,
					   bool includeMemberVariables) const
{
    const QSClass *objType = obj.objectType();

    QSMemberMap mmap = objType == env()->typeClass() && includeMemberVariables
		       ? ((QSTypeClass*) objType)->allMembers(&obj)
		       : objType->members(&obj);
    QStringList lst;
    for (QSMemberMap::Iterator it = mmap.begin(); it != mmap.end(); ++it) {
	if (((*it).type() == QSMember::Variable ||
	       includeCustom && (*it).type() == QSMember::Custom) &&
	     (!(*it).isStatic() || includeStatic) &&
	     !(*it).isExecutable())
	    lst << (*it).name();
    }
    return lst;
}

static bool hasMember(QSEnv *env, const QString &function, QSMember::Type type)
{
    QSObject o = env->globalObject();
    QSMember member;
    QStringList names = function.split(QString::fromLatin1("."));
    int nameCount = names.count();
    for (QStringList::ConstIterator it = names.begin(); it != names.end(); ++it, --nameCount) {
	if (nameCount==1) {
	    if (o.objectType() == env->typeClass())
		return QSTypeClass::classValue(&o)->member(0, *it, &member)
		    && member.type() == type;
	    else if (o.objectType()->member(&o, *it, &member))
		return o.objectType()->member(0, *it, &member) && member.type() == type;
	} else {
	    o = o.get(*it);
	    if (!o.isValid())
		return false;
	}
    }
    return false;
}

bool QuickInterpreter::hasFunction(const QString &function) const
{
    return hasMember(env(), function, QSMember::ScriptFunction);
}

bool QuickInterpreter::hasVariable(const QString &variable) const
{
    return hasMember(env(), variable, QSMember::Variable);
}

bool QuickInterpreter::hasClass(const QString &className) const
{
    QSObject foo = object(className);
    return foo.objectType() == env()->typeClass();
}

bool operator == (const QSAConnection &a, const QSAConnection &b)
{
    return a.sender == b.sender
        && a.signal == b.signal
  && a.function_ref == b.function_ref;
}

void qsa_slot_callback(QObject *_this, int callId, int paramCount,
                       const QMetaType::Type *types,
                       const QByteArray *typeNames,
                       void *argumentList[])
{
//     printf("qsa_slot_callback, id=%d\n", callId);

    QuickInterpreter *ip = static_cast<QuickInterpreter *>(_this);
    QSAConnection connection = ip->scriptSlot(callId);

    Q_ASSERT(connection.sender);
    Q_ASSERT(connection.signal.length());
    Q_ASSERT(connection.function_ref.isValid());

    QSEnv *env = ip->env();
    QSList arguments;

    for (int i=1; i<paramCount; ++i) {
        QSATypeInfo info;
        info.name = typeNames[i];
        info.id = types[i];

        QSObject val = convert_qt2qsa(env, argumentList[i], info, _this);
        Q_ASSERT(val.isValid());

        arguments.append(val);
    }

    QSObject func_base = QSFuncRefClass::refBase(connection.function_ref);
    QString func_name = QSFuncRefClass::refMember(connection.function_ref).name();

    ip->call(func_base, func_name, arguments);
}

bool QuickInterpreter::addConnection(QObject *object,
                                     const char *signal,
                                     const QSObject &function_ptr)
{
    int id = m_dynamic_slots->registerSlot(this, signal+1, qsa_slot_callback);

    if (id < 0)
        return false;

    bool ok = m_dynamic_slots->dynamicConnect(object, signal, id);
    if (!ok)
        return false;

    QSAConnection connection(object, signal, function_ptr);
    m_script_connections[id] = connection;

//     printf("QuickInterpreter::addConnection(), '%s' id=%d\n",
//            qPrintable(QSFuncRefClass::refMember(function_ptr).name()),
//            id);

    return true;
}


bool QuickInterpreter::removeConnection(QObject *object,
                                        const char *signal,
                                        const QSObject &function_ptr)
{
    QSAConnection connection(object, signal, function_ptr);

    int id = -1;
    QHash<int, QSAConnection>::ConstIterator it = m_script_connections.begin();
    while (it != m_script_connections.end()) {
        if (*it == connection) {
            id = it.key();
            break;
        }
        ++it;
    }

    if (id < 0) {
#ifndef QT_NO_DEBUG
        qWarning("QuickInterpreter::removeConnection(), no matching connection found\n");
#endif
        return false;
    }

//     printf("QuickInterpreter::removeConnection(), '%s' id=%d\n",
//            qPrintable(QSFuncRefClass::refMember(function_ptr).name()), id);

    m_dynamic_slots->dynamicDisconnect(object, signal, id);
    bool ok = m_script_connections.remove(id);
    return ok;
}


uint qHash(const QSAMethodSignature &method) {
    return qHash(method.return_type) ^ qHash(method.signature);
}

bool operator == (const QSAMethodSignature &a, const QSAMethodSignature &b)
{
    return !strcmp(a.signature, b.signature)
        && !strcmp(a.return_type, b.return_type);
}

/*******************************************************************************
 * Connect/Disconnect stuff
 */

static QSObject qsa_connect(QSEnv *env)
{
    const QSList *args = env->arguments();
    QuickInterpreter *ip = QuickInterpreter::fromEnv(env);

    QObject *sender = 0;
    QByteArray signal;
    if (args->size() >= 2) {
        QSObject qsSender = args->at(0);
        sender = qsSender.isA(ip->wrapperClass()) ? QSWrapperClass::object(&qsSender) : 0;
        if (!sender)
            return env->throwError("connection failed, sender is not of type QObject");
        QString signal_string = "2" + args->at(1).toString();
        signal = QMetaObject::normalizedSignature(signal_string.toLatin1());
    }

    bool ok = false;

    if (args->size() == 3) {
        // C++ -> Script connection
        QSObject function_ref = args->at(2);

        // resolve function as a name
        if (!function_ref.isFunction())
            function_ref = env->resolveValue(args->at(2).toString());

        if (!function_ref.isValid())
            return env->throwError("connection failed, argument 3 is not a function");

        ok = ip->addConnection(sender, signal, function_ref);

        if (!ok) {
            const QMetaObject *meta_object = sender->metaObject();
            if (meta_object->indexOfSignal(signal) < 0) {
                return env->throwError(QString("connection failed, no such signal %1::%2")
                                       .arg(meta_object->className())
                                       .arg(signal.constData() + 1));
            }
        }

    } else if (args->size() == 4) {
        // C++ -> C++ connection
        QSObject qsReceiver = args->at(2);
        QObject *receiver = qsReceiver.isA(ip->wrapperClass())
                            ? QSWrapperClass::object(&qsReceiver)
                            : 0;
        if (!receiver)
            return env->throwError("connection failed, reciver is not of type QObject");
        QString slot_string = "1" + args->at(3).toString();
        QByteArray slot = QMetaObject::normalizedSignature(slot_string.toLatin1());

        ok = QObject::connect(sender, signal, receiver, slot);
        if (!ok) {
            if (!QMetaObject::checkConnectArgs(signal, slot))
                return env->throwError("connection failed, parameters does not match");
            else
                return env->throwError("connection failed, unknown error");
        }
    } else {
        return env->throwError("connection failed, expected 3 or 4 arguments");
    }

    if (!ok)
        return env->throwError("connection failed");

    return env->createBoolean(true);
}

static QSObject qsa_disconnect(QSEnv *env)
{
    const QSList *args = env->arguments();
    QuickInterpreter *ip = QuickInterpreter::fromEnv(env);

    QObject *sender = 0;
    QByteArray signal;

    if (args->size() >= 2) {
        QSObject qsSender = args->at(0);
        sender = qsSender.isA(ip->wrapperClass()) ? QSWrapperClass::object(&qsSender) : 0;
        if (!sender)
            return env->throwError("disconnection failed, sender is not of type QObject");
        QString signal_string = "2" + args->at(1).toString();
        signal = QMetaObject::normalizedSignature(signal_string.toLatin1());
    }

    bool ok = false;

    if (args->size() == 3) {
        // C++ -> Script connection
        QSObject function_ref = args->at(2);

        // resolve function as a name
        if (!function_ref.isFunction())
            function_ref = env->resolveValue(args->at(2).toString());

        if (!function_ref.isValid())
            return env->throwError("connection failed, argument 3 is not a function");

        ok = ip->removeConnection(sender, signal, function_ref);

        if (!ok) {
            const QMetaObject *meta_object = sender->metaObject();
            if (meta_object->indexOfSignal(signal) < 0) {
                return env->throwError(QString("connection failed, no such signal %1::%2")
                                       .arg(meta_object->className())
                                       .arg(signal.constData() + 1));
            }
        }

    } else if (args->size() == 4) {
        // C++ -> C++ connection
        QSObject qsReceiver = args->at(2);
        QObject *receiver = qsReceiver.isA(ip->wrapperClass())
                            ? QSWrapperClass::object(&qsReceiver)
                            : 0;

        if (!receiver)
            return env->throwError("disconnection failed, reciver is not of type QObject");

        QString slot_string = "1" + args->at(3).toString();
        QByteArray slot = QMetaObject::normalizedSignature(slot_string.toLatin1());

        ok = QObject::disconnect(sender, signal, receiver, slot);
        if (!ok) {
            if (!QMetaObject::checkConnectArgs(signal, slot))
                return env->throwError("disconnection failed, parameters does not match");
            else
                return env->throwError("disconnection failed, unknown error");
        }
    }

    if (!ok)
        return env->throwError("disconnection failed");

    return env->createBoolean(true);
}


/*******************************************************************************
 * Timers
 */
QSObject qsStartTimer( QSEnv *env )
{
    QSObject hnd = env->arg( 1 );
    if ( hnd.isFunction() ) {
        QuickInterpreter *ip = QuickInterpreter::fromEnv(env);

        int interval = (int) env->arg(0).toNumber();
        int id = ip->startTimer(interval);

        QHash<int, QSObject> *timers = ip->timers();
        timers->insert(id, hnd);

        return env->createNumber(id);
    }

    QString msg = QString::fromLatin1("Can only install functions as event handler");
    return env->throwError( TypeError, msg );
}


void qsKillTimer( QSEnv *env )
{
    QuickInterpreter *ip = QuickInterpreter::fromEnv(env);
    int id = (int) env->arg(0).toNumber();

    ip->timers()->remove(id);
    ip->killTimer(id);
}

void qsKillTimers( QSEnv *env )
{
    QuickInterpreter *ip = QuickInterpreter::fromEnv(env);

    QHash<int, QSObject> *timers = ip->timers();
    for (QHash<int, QSObject>::ConstIterator it = timers->begin(); it != timers->end(); ++it) {
        ip->killTimer(it.key());
    }
    timers->clear();
}




/*******************************************************************************
 * Registring the meta objects since we do not have a global map as we did
 * in Qt 3.
 */

#ifndef QSA_NO_GUI
#include <QtGui>
#endif

#define INSERT_QOBJECT(object) qsa_meta_objects()->insert(#object, &object::staticMetaObject);

typedef QHash<QByteArray, const QMetaObject *> QMetaObjectMap;
Q_GLOBAL_STATIC(QMetaObjectMap, qsa_meta_objects);

static QMutex qsa_meta_object_reg_mutex;

void qsa_register_meta_object(const QMetaObject *meta_object)
{
    QMutexLocker locker(&qsa_meta_object_reg_mutex);
    qsa_meta_objects()->insert(meta_object->className(), meta_object);
}

const QMetaObject *qsa_query_meta_object(const QByteArray &name)
{
    return qsa_meta_objects()->value(name, 0);
}

static void qsa_setup_meta_objects()
{
    QMutexLocker locker(&qsa_meta_object_reg_mutex);
    static bool already_set_up = false;
    if (already_set_up)
        return;
    already_set_up = true;

#ifndef QSA_NO_GUI
    // Qt 4 Main Classes
    INSERT_QOBJECT(QApplication);
    INSERT_QOBJECT(QButtonGroup);
    INSERT_QOBJECT(QCheckBox);
    INSERT_QOBJECT(QColorDialog);
    INSERT_QOBJECT(QComboBox);
    INSERT_QOBJECT(QCoreApplication);
    INSERT_QOBJECT(QDateEdit);
    INSERT_QOBJECT(QDesktopWidget);
    INSERT_QOBJECT(QDial);
    INSERT_QOBJECT(QDialog);
    INSERT_QOBJECT(QDockWidget);
    INSERT_QOBJECT(QDoubleSpinBox);
    INSERT_QOBJECT(QDoubleValidator);
    INSERT_QOBJECT(QFile);
    INSERT_QOBJECT(QFileDialog);
    INSERT_QOBJECT(QFocusFrame);
    INSERT_QOBJECT(QFontDialog);
    INSERT_QOBJECT(QFrame);
    INSERT_QOBJECT(QHeaderView);
    INSERT_QOBJECT(QInputDialog);
    INSERT_QOBJECT(QIntValidator);
    INSERT_QOBJECT(QIODevice);
    INSERT_QOBJECT(QLabel);
    INSERT_QOBJECT(QLCDNumber);
    INSERT_QOBJECT(QLineEdit);
    INSERT_QOBJECT(QListView);
    INSERT_QOBJECT(QListWidget);
    INSERT_QOBJECT(QMainWindow);
    INSERT_QOBJECT(QMenu);
    INSERT_QOBJECT(QMenuBar);
    INSERT_QOBJECT(QMessageBox);
    INSERT_QOBJECT(QMovie);
    INSERT_QOBJECT(QObject);
    INSERT_QOBJECT(QPageSetupDialog);
    INSERT_QOBJECT(QProcess);
    INSERT_QOBJECT(QProgressBar);
    INSERT_QOBJECT(QProgressDialog);
    INSERT_QOBJECT(QPushButton);
    INSERT_QOBJECT(QRadioButton);
    INSERT_QOBJECT(QRubberBand);
    INSERT_QOBJECT(QScrollArea);
    INSERT_QOBJECT(QScrollBar);
    INSERT_QOBJECT(QSettings);
    INSERT_QOBJECT(QSlider);
    INSERT_QOBJECT(QSound);
    INSERT_QOBJECT(QSpinBox);
    INSERT_QOBJECT(QSplashScreen);
    INSERT_QOBJECT(QSplitter);
    INSERT_QOBJECT(QStackedWidget);
    INSERT_QOBJECT(QStatusBar);
    INSERT_QOBJECT(QTabBar);
    INSERT_QOBJECT(QTableView);
    INSERT_QOBJECT(QTableWidget);
    INSERT_QOBJECT(QTabWidget);
    INSERT_QOBJECT(QTemporaryFile);
    INSERT_QOBJECT(QTextEdit);
    INSERT_QOBJECT(QThread);
    INSERT_QOBJECT(QTimer);
    INSERT_QOBJECT(QToolBar);
    INSERT_QOBJECT(QToolBox);
    INSERT_QOBJECT(QToolButton);
    INSERT_QOBJECT(QTranslator);
    INSERT_QOBJECT(QTreeView);
    INSERT_QOBJECT(QTreeWidget);
    INSERT_QOBJECT(QWidget);
    INSERT_QOBJECT(QWorkspace);
#endif
}

