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

#include "quickinterpreter.h"

#include "quickobjects.h"
#include "quickdebugger.h"
#include "quickdispatchobject.h"
#include "quickbytearrayobject.h"
#include "quickpixmapobject.h"
#include "quickcolorobject.h"
#include "quickfontobject.h"
#include "quickcoordobjects.h"
#include "quickobjects.h"
#include "qsenv.h"
#include "qserrors.h"
#include "qsfunction.h"
#include "qsfuncref.h"
#include <qobjectlist.h>
#include <qmetaobject.h>
#include <qapplication.h>
#include <qdom.h>
#include <private/qucomextra_p.h>
#include <qdir.h>
#include <qsclass.h>
#include <qsnodes.h>
#include <qvaluevector.h>

#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
#include <qthread.h>
extern Q_EXPORT Qt::HANDLE qt_get_application_thread_id();
#endif

static bool eTimers = TRUE;
static int static_id = 0;

////////////////////////////////////////////////////////////////////

static QSObject qsConnectCommon( QSEnv *env,
				 QSObject &arg0,
				 QSObject &arg2,
				 QSWrapperShared *&sendObj,
				 QSWrapperShared *&recObj,
				 const QPtrVector<QObject> *&sendIfaces,
				 const QPtrVector<QObject> *&recIfaces,
				 QString &sig,
				 QString &sl,
				 int &signal_index,
				 QObject *&sender,
				 const QString &func )
{
    const QSList &args = *env->arguments();
    QuickInterpreter *ip = QuickInterpreter::fromEnv( env );
    const QString overloads =
	QString::fromLatin1( "Following overloads are possible:\n"
		 "%1( sender : QObject, signal : String, receiver : QObject, slot : String )\n"
		 "%2( sender : QObject, signal : String, function : DeclaredFunction )" ).
	arg( func ).arg( func );

    if ( args.size() < 3 || args.size() > 4 ) {
	QString msg = QString::fromLatin1("No matching overload found. ") + overloads;
	return env->throwError( SyntaxError, msg );
    }

    QSWrapperClass *wClass = ip->wrapperClass();
    if ( !args[0].isA( wClass ) ) {
	QString msg = QString::fromLatin1("No matching overload found. 'sender' must be of type QObject but is of type ") +
		      args[0].typeName() + QString::fromLatin1("\n") + overloads;
	return env->throwError( TypeError, msg );
    }

    if ( args.size() == 3 && !args[2].isFunction() ) {
	QString msg = QString::fromLatin1("No matching overloads found. Third argument in this overload "
                                          "must be of type function but is of type ") + args[2].typeName() + QString::fromLatin1("\n") + overloads;
	return env->throwError( TypeError, msg );
    }

    arg0 = args[0];
    arg2 = args[2];
    sendObj = wClass->shared( &arg0 );
    recObj =  arg2.isA( wClass ) ? wClass->shared( &arg2 ) : 0;
    sendIfaces = sendObj->interfaceObjects();
    recIfaces = recObj ? recObj->interfaceObjects() : 0;

    // signal and slot in string representation
    sig = args[1].toString();
    sl = ( args.size()>3 ? args[3] : env->createUndefined() ).toString();

    // find sender and signal
    sender = 0;
    signal_index = -1;
    const char *sigName = sig.ascii();
    int i;
    for ( i = (int)sendIfaces->count()-1; i >= 0; --i ) {
	sender = sendIfaces->at( i );
	signal_index = sender->metaObject()->findSignal( sigName, TRUE );
	if ( signal_index > 0 )
	    break;
    }

    if ( signal_index == -1 ) {
	// didn't find the signal- this signature might be in Qt
	// Script syntax, so lets's try to find out if we can connect
	QString sn = sig;
	QString arg = sig.mid( sig.find( '(' ) + 1 );
	arg = arg.left( arg.findRev( ')' ) );
	QStringList args = QStringList::split( ',', arg );
	sn = sig.left( sig.find( '(' ) );
	for ( i = (int)sendIfaces->count()-1; i >= 0; --i ) {
	    sender = sendIfaces->at( i );
	    for ( int j = 0; j < (int)sender->metaObject()->numSignals( TRUE ); ++j ) {
		const QMetaData *md = sender->metaObject()->signal( j, TRUE );
		QString mdn = QString::fromLatin1(md->name);
		mdn = mdn.left( mdn.find( '(' ) );
		if ( mdn != sn )
		    continue;
		const QUMethod *method = md->method;
		bool ok = method->count == (int)args.count();
		for ( int k = 0; k < method->count; ++k ) {
		    QUParameter p = method->parameters[k];
		    QString s = *args.at( k );
		    int sep;
		    if ( ( sep = s.find( ':' ) ) != -1 )
			s = s.mid( sep + 1 );
		    s = s.simplifyWhiteSpace();
		    if ( s == QString::fromLatin1(p.type->desc())
                         || s == QString::fromLatin1((const char*)p.typeExtra))
			continue;
		    if ( s == QString::fromLatin1("Number") &&
			 ( qstrcmp( p.type->desc(), "int" ) == 0 ||
			   qstrcmp( p.type->desc(), "long" ) == 0 ||
			   qstrcmp( p.type->desc(), "double" ) == 0 ||
			   qstrcmp( p.type->desc(), "float" ) == 0 ||
			   qstrcmp( p.type->desc(), "short" ) == 0 ||
			   qstrcmp( p.type->desc(), "uint" ) == 0 ||
			   qstrcmp( p.type->desc(), "ushort" ) == 0 ||
			   qstrcmp( p.type->desc(), "ulong" ) == 0 ||
			   qstrcmp( p.type->desc(), "unsigned int" ) == 0 ||
			   qstrcmp( p.type->desc(), "unsigned short" ) == 0 ||
			   qstrcmp( p.type->desc(), "unsigned long" ) == 0 ) )
			continue;
		    s.prepend( QString::fromLatin1("Q") );
		    if (s == QString::fromLatin1(p.type->desc())
                        || s == QString::fromLatin1((const char*)p.typeExtra))
			continue;
		    ok = FALSE;
		    break;
		}
		if ( !ok )
		    continue;
		signal_index = j;
		sig = sender->metaObject()->signal( j, TRUE )->name;
		break;
	    }
	    if ( signal_index != -1 )
		break;
	}
    }

    return QSObject();
}

#define QS_CONNECT_INIT( func ) \
    QSObject arg0; \
    QSObject arg2; \
    QSWrapperShared *sendObj; \
    QSWrapperShared *recObj; \
    const QPtrVector<QObject> *sendIfaces = 0; \
    const QPtrVector<QObject> *recIfaces = 0; \
    QString sig; \
    QString sl; \
    int signal_index; \
    QObject *sender = 0; \
    QSObject ret = qsConnectCommon( env, arg0, arg2, sendObj, recObj, \
				    sendIfaces, recIfaces, sig, sl, signal_index, \
				    sender, QString::fromLatin1(#func) ); \
    if ( ret.isValid() ) { \
	return ret; }


static QSObject qsConnect( QSEnv *env )
{
    QS_CONNECT_INIT( connect );

    // find receiver and slot
    QObject *receiver = 0;
    int member_index = -1;
    const char *slotName = sl.ascii();
    if ( recIfaces ) {
	for ( int i = (int)recIfaces->count()-1; i >= 0; --i ) {
	    receiver = recIfaces->at( i );
	    member_index = receiver->metaObject()->findSlot( slotName, TRUE );
	    if ( member_index >= 0 && signal_index >= 0 ) {
		// regular signal/slot connection
		QObject::connectInternal( sender, signal_index,
					  receiver, QSLOT_CODE, member_index );
		return env->createUndefined();
	    }
	}
    }

    if ( signal_index == -1 ) {
	QString msg = QString::fromLatin1("Can't find signal named ") + sig;
	return env->throwError( SyntaxError, msg );
    }

    QuickInterpreter *ip = QuickInterpreter::fromEnv( env );
    if ( recIfaces ) {
	sendObj->setEventHandler( ip, sig, recIfaces->at( 0 ), sl.left( sl.find( '(' ) ) );
    } else {
	QSObject base;
	QString name;
	if ( arg2.isFunction() ) {
	    base = QSFuncRefClass::refBase( arg2 );
	    name = QSFuncRefClass::refMember( arg2 ).name();
	} else {
	    base = arg2;
	    if ( base.isPrimitive() )
		return env->throwError( QString::fromLatin1("Invalid receiver object") );
	    name = env->arg( 3 ).toString();
	    if ( name.endsWith( QString::fromLatin1("()") ) )
		name.truncate( name.length() - 2 );
	}
	sendObj->setEventHandler( ip, sig, 0, name, base );
    }

    return env->createUndefined();
}

static QSObject qsDisconnect( QSEnv *env )
{
    QS_CONNECT_INIT( disconnect );

    // find receiver and slot
    QObject *receiver = 0;
    int member_index = -1;
    const char *slotName = sl.ascii();
    if ( recIfaces ) {
	for ( int i = (int)recIfaces->count()-1; i >= 0; --i ) {
	    receiver = recIfaces->at( i );
	    member_index = receiver->metaObject()->findSlot( slotName, TRUE );
	    if ( member_index > 0 && signal_index > 0 ) {
		// regular signal/slot connection
		QObject::disconnectInternal( sender, signal_index, receiver,
					     QSLOT_CODE, member_index );
		return env->createUndefined();
	    }
	}
    }

    if ( signal_index == -1 ) {
	QString msg = QString::fromLatin1("Can't find signal named ") + sig;
	return env->throwError( SyntaxError, msg );
    }

    if ( recIfaces ) {
	sendObj->removeEventHandler( sig, recIfaces->at( 0 ), sl.left( sl.find( '(' ) ) );
    } else {
	QSObject base = QSFuncRefClass::refBase( arg2 );
	QSMember member = QSFuncRefClass::refMember( arg2 );
	sendObj->removeEventHandler( sig, 0, member.name(), base );
    }

    return env->createUndefined();
}

////////////////////////////////////////////////////////////////////

QuickInterpreter::QuickInterpreter( bool deb )
    : toplevel( 0 )
{
    factory = new QuickDispatchObjectFactory( this );
    shuttingDown = FALSE;
    id = static_id++;
    wrapperShared = new QSWrapperSharedList;
    //    wrapperShared->setAutoDelete( TRUE );
    usrDataId = QObject::registerUserData();

    debugger = deb ? new QuickDebugger( this ) : 0;
    init();
}

QuickInterpreter::~QuickInterpreter()
{
    shuttingDown = TRUE;
    stopAllTimers();
    invalidateWrappers();
    delete factory;
    delete toplevel;
    delete debugger;
    Q_ASSERT( wrapperShared->isEmpty() );
    delete wrapperShared;
}

/*!
  Retrieves a pointer to the interpreter class from an environment
  pointer.
 */

QuickInterpreter *QuickInterpreter::fromEnv( QSEnv *e )
{
    return (QuickInterpreter*)e->engine();
}

void QuickInterpreter::reinit()
{
    QSEngine::init();
}

void QuickInterpreter::enableTimers( bool b )
{
    eTimers = b;
}

bool QuickInterpreter::timersEnabled()
{
    return eTimers;
}

QSArgument QuickInterpreter::convertToArgument( const QSObject &o )
{
    if( !o.isValid() )
	return QSArgument();
    const QSClass *cl = o.objectType();
    if( cl->name() == QString::fromLatin1("QObject") ) {
	QSWrapperShared *shared = (QSWrapperShared*) o.shVal();
	if (shared->objects.isEmpty())
            return QSArgument();
	return QSArgument( shared->objects[0] );
    } else if( cl == ptrClass ) {
	Q_ASSERT( ptrClass->pointer( &o ) );
	return QSArgument( ptrClass->pointer( &o ) );
    } else {
	return QSArgument( o.toVariant( QVariant::Invalid ) );
    }
}

QSObject QuickInterpreter::wrap( QObject *o )
{
    if( !o ) {
	QSList list;
	return env()->nullClass()->construct( list );
    }

#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    if (qt_get_application_thread_id() != QThread::currentThread()
	&& o->inherits("QWidget")) {
	qWarning("QuickInterpreter::wrap(), GUI object (%s [%s]) cannot be used in non GUI thread",
		 o->name(), o->className());
	return QSObject();
    }
#endif

    QSUserData *udata = (QSUserData*) o->userData( userDataId() );
    // set user data in QObject if it's not there, yet
    if( !udata ) {
	udata = new QSUserData( 0 );
	o->setUserData( userDataId(), udata );
    }
    QSWrapperShared *shared = udata->data();
    const QSWrapperClass *cl;
    if ( shared ) {
	// wrapper is already there, reuse it
	cl = shared->wrapperClass();
    } else {
	// create & remember wrapper
	cl = new QSWrapperClass( wrapperClass() );
	shared = cl->createShared( o );
	shared->setObjectType( QSWrapperShared::GlobalObject );
	env()->registerShared( shared );
	wrapperShared->append( shared );
    }
    shared->setUserData( udata );
    udata->setData( shared );
    shared->ref(); // additional ref by either QObject or QSObject
    QSObject obj( cl );
    obj.setVal( shared ); // no ownership needs to be transferred anymore
    return obj;
}

void QuickInterpreter::init()
{
    debuggerEngine()->clear();

    staticGlobals.clear();

    staticGlobals << QString::fromLatin1("NaN")
                  << QString::fromLatin1("undefined")
                  << QString::fromLatin1("Infinity")
                  << QString::fromLatin1("Application");

    // add some common objects to the Global object
    QSObject global( env()->globalObject() );
    QSClass *objClass = env()->objectClass();

    wrpClass = new QSWrapperClass( objClass );
    ptrClass = new QSPointerClass( objClass );
    varClass = new QSVariantClass( objClass );
    appClass = new QSApplicationClass( objClass );
    global.put( QString::fromLatin1("Application"), appClass->createWritable() );

    pntClass = new QSPointClass( objClass, this );
    registerType( pntClass );
    sizClass = new QSSizeClass( objClass, this );
    registerType( sizClass );
    rctClass = new QSRectClass( objClass, this );
    registerType( rctClass );
    colClass = new QSColorClass( objClass );
    registerType( colClass );
    fntClass = new QSFontClass( objClass );
    registerType( fntClass );
    baClass = new QSByteArrayClass( objClass );
    registerType( baClass );
    pixClass = new QSPixmapClass( objClass, this );
    registerType( pixClass );
    palClass = new QSPaletteClass(objClass);
    registerType(palClass);
    colGrpClass = new QSColorGroupClass(objClass);
    registerType(colGrpClass);

    enableDebug(); // adds "debug" function which uses qDebug()
    env()->globalClass()->addMember( QString::fromLatin1("connect"), QSMember( qsConnect ) );
    env()->globalClass()->addMember( QString::fromLatin1("disconnect"), QSMember( qsDisconnect ) );
    env()->globalClass()->addMember( QString::fromLatin1("startTimer"), QSMember( qsStartTimer ) );
    env()->globalClass()->addMember( QString::fromLatin1("killTimer"), QSMember( qsKillTimer ) );
    env()->globalClass()->addMember( QString::fromLatin1("killTimers"), QSMember( qsKillTimers ) );

    QMap<QString,QObject*> statDescr = factory->staticDescriptors();
    QMap<QString,QString> instDescr = factory->instanceDescriptors();
    QValueList<QString> features = instDescr.keys();
    for ( QValueList<QString>::ConstIterator it = features.begin();
	  it != features.end(); ++it ) {
        if (env()->globalClass()->definedMembers()->contains(*it)) {
            qWarning("QSObjectFactory: Trying to register existing class: '%s'", (*it).latin1());
            continue;
        }
	QSObject staticInst;
	if( statDescr.contains( *it ) ) { // has static?
	    QObject *sinst = statDescr[ *it ];
	    Q_ASSERT( sinst );
	    staticInst = wrap( sinst );
	    statDescr.remove( *it );
	}
	QSObjectConstructor *constr =
	    new QSObjectConstructor( objClass, *it );
	QSFactoryObjectProxy *ptype =
	    new QSFactoryObjectProxy( env()->typeClass(), staticInst, constr );
        constr->setFactoryObjectProxy(ptype);
	QSObject proxy( ptype, env()->typeClass()->createTypeShared(constr) );
	env()->globalClass()->addStaticVariableMember( constr->identifier(),
						       proxy,
						       AttributeExecutable );
    }

    for( QMap<QString,QObject*>::ConstIterator sit = statDescr.begin();
	 sit != statDescr.end(); ++sit ) {
        if (env()->globalClass()->definedMembers()->contains(sit.key())) {
            qWarning("QSObjectFactory: Trying to register existing class: '%s'", sit.key().latin1());
            continue;
        }
	QSObject staticInst;
	QObject *sinst = statDescr[ sit.key() ];
	Q_ASSERT( sinst );
	staticInst = wrap( sinst );
	env()->globalClass()->addStaticVariableMember( sit.key(), staticInst, AttributeNone );
    }
}

bool QuickInterpreter::hasTopLevelParent( QObject *o )
{
    for ( QObject *p = toplevel->first(); p; p = toplevel->next() ) {
	QObject *c = p->child( o->name(), o->className() );
	if ( c )
	    return TRUE;
    }
    return FALSE;
}

void QuickInterpreter::addTopLevelObject( QObject *o )
{
    QSEngine::init();
    if ( !toplevel )
	toplevel = new QObjectList;
    if ( toplevel->findRef( o ) != -1 )
	return;
    if ( hasTopLevelParent( o ) )
	return;

    for( QObject *cur = toplevel->first(); cur; cur = toplevel->next() ) {
	if( cur == o ) {
	    return;
	} else if ( cur && o && QString::fromLatin1( cur->name() ) == QString::fromLatin1( o->name() ) ) {
	    return;
	}
    }


    toplevel->append( o );

    kids.clear();
    if ( !toplevel )
	return;
    QObject *obj = toplevel->first();
    while ( obj ) {
	kids.append( QString::fromLatin1(obj->name()) );
	obj = toplevel->next();
    }
    connect( o, SIGNAL( destroyed( QObject * ) ), this, SLOT( topLevelDestroyed( QObject * ) ) );

    QSObject global = env()->globalObject();
    const_cast<QSClass *>(global.objectType())->deleteMember(o->name());
    env()->globalObject().put( QString::fromLatin1(o->name()), wrap( o ) );
    staticGlobals << QString::fromLatin1(o->name());
}

void QuickInterpreter::setTopLevelObjects( QObjectList *l )
{
    QSEngine::init();
    if( toplevel )
	for( QObject *o = toplevel->first(); o; o = toplevel->next() )
	    disconnect( o, SIGNAL( destroyed( QObject* ) ),
			this, SLOT( topLevelDestroyed( QObject* ) ) );
    delete toplevel;
    toplevel = new QObjectList;

    kids.clear();
    if ( !l ) {
	toplevel->clear();
	return;
    }
    QSObject global( env()->globalObject() );
    QObject *o = l->first();
    while ( o ) {
	if ( hasTopLevelParent( o ) ) {
	    o = l->next();
	    continue;
	}
	kids.append( QString::fromLatin1(o->name()) );
	connect( o, SIGNAL( destroyed( QObject * ) ),
		 this, SLOT( topLevelDestroyed( QObject * ) ) );
	global.put( QString::fromLatin1(o->name()), wrap( o ) );
	staticGlobals << QString::fromLatin1(o->name());
	toplevel->append( o );
	o = l->next();
    }
    delete l;
}

void QuickInterpreter::topLevelDestroyed( QObject *o )
{
    toplevel->removeRef( o );
}

bool QuickInterpreter::checkSyntax( const QString &c )
{
    QString code = c + QString::fromLatin1("\n");
    return QSEngine::checkSyntax( code );
}

QSArgument QuickInterpreter::execute( QObject *obj, const QString &c,
				      const QString &name )
{
    QString code = c + QString::fromLatin1("\n");

    int sourceId = debugger ? debugger->freeSourceId() : -1;
    if( !name.isNull() && sourceId >= 0 )
	sourceIdNames[sourceId] = name;

    QSObject t, oldThis;
    if ( obj ) {
	if (!name.isNull() && sourceId >= 0)
            addSourceId( sourceId, obj );
	addTopLevelObject( obj );
	t = wrap( obj );
	oldThis = env()->thisValue();
	env()->setThisValue( t );
    }

    QSEngine::evaluate( t, code );

    // restore this value
    if ( obj )
	env()->setThisValue( oldThis );

    if ( hadError() )
	if( errorType() == QSErrParseError )
	    emit parseError();
	else
	    emit runtimeError();

    // Make sure we dereference the engines return value to avoid pooling
    QSArgument a = convertToArgument( returnValue() );
    setReturnValue(QSObject());
    return a;
}

QSArgument QuickInterpreter::call( QSObject ctx, const QString &func,
				 const QSList &args )
{
    if ( shuttingDown )
	return QVariant();

    QSEngine::call( &ctx, func, args );

    if (hadError())
	emit runtimeError();

    // Make sure we dereference the engines return value to avoid pooling
    QSArgument a = convertToArgument( returnValue() );
    setReturnValue(QSObject());
    return a;
}

QSArgument QuickInterpreter::call( QObject *ctx, const QString &func,
				 const QSList &args )
{
    if ( shuttingDown )
	return QVariant();

    QSObject t;
    if ( ctx )
	t = wrap( ctx );

    QSEngine::call( &t, func, args );

    if (hadError())
	emit runtimeError();

    // Make sure we dereference the engines return value to avoid pooling
    QSArgument a = convertToArgument( returnValue() );
    setReturnValue(QSObject());
    return a;
}

QSArgument QuickInterpreter::call( QObject *ctx, const QString &func, const QSArgumentList &args )
{
    QSList l;
    for ( QSArgumentList::ConstIterator it = args.begin(); it != args.end(); ++it ) {
	switch ( (*it).type() ) {
	case QSArgument::Variant: {
            QuickScriptVariant qsvar(this, (*it).variant());
            if (qsvar.isNative())
                l.append(qsvar.toNative());
            else
                l.append(qsvar);
	    break;
	}
        case QSArgument::QObjectPtr:
	    l.append( wrap( (*it).qobject() ) );
	    break;
	case QSArgument::VoidPointer:
	    qWarning( "QuickInterpreter::call: don't know what to do with a "
		      "QSArgument::VoidPointer here..." );
 	    break;
	default:
	    break;
	}
    }
    return call( ctx, func, l );
}

void QuickInterpreter::setVariable( QObject *context, const QString &func, const QSArgument &value )
{
    QSObject val;
    switch(value.type()) {
    case QSArgument::Variant: {
        QuickScriptVariant qsvar(this, value.variant());
        if (qsvar.isNative())
            val = qsvar.toNative();
        else
            val = qsvar;
        break;
    }
    case QSArgument::QObjectPtr:
	val = wrap( value.qobject() );
	break;
    case QSArgument::VoidPointer:
	qWarning( "QuickInterpreter::setVariable: don't know what to do with "
		  "QSArgument::VoidPointer here..." );
	return;
    default:
	return;
    }

    if (context)
	wrap(context).put(func, val);
    else
	env()->globalObject().put(func, val);
}

QSArgument QuickInterpreter::variable( QObject *context, const QString &varName )
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
    sourceIdNames.clear();
    debuggerEngine()->clear();
    stopAllTimers();
    invalidateWrappers();
    if( toplevel )
	toplevel->clear();
    QSEngine::clear();
    QSEngine::init();
    init();
}

void QuickInterpreter::stop()
{
    stopAllTimers();
    debuggerEngine()->clear();
}

bool QuickInterpreter::hadError() const
{
    return !!QSEngine::errorType();
}

/*!
  Emit warning message \a msg for line \a l.
*/

void QuickInterpreter::warn( const QString &msg, int l )
{
    emit warning( msg, l );
}

void QuickInterpreter::clearSourceIdMap()
{
    sourceIdMap.clear();
}

void QuickInterpreter::addSourceId( int id, QObject *o )
{
    sourceIdMap.insert( id, o );
}

QObject *QuickInterpreter::objectOfSourceId( int id ) const
{
    QMap<int, QObject*>::ConstIterator it = sourceIdMap.find( id );
    if ( it == sourceIdMap.end() )
	return 0;
    return *it;
}

int QuickInterpreter::sourceIdOfObject( QObject *o ) const
{
    for ( QMap<int, QObject*>::ConstIterator it = sourceIdMap.begin(); it != sourceIdMap.end(); ++it ) {
	if ( *it == o )
	    return it.key();
	else if( QString( QString::fromLatin1((*it)->name()) ) == QString::fromLatin1(o->name()) )
	    return it.key();
    }
    return -1;
}

QString QuickInterpreter::nameOfSourceId( int id ) const
{
    QMap<int,QString>::ConstIterator it = sourceIdNames.find( id );
    if ( it == sourceIdNames.end() )
	return QString::null;
    return *it;
}

int QuickInterpreter::sourceIdOfName( const QString &name ) const
{
    for ( QMap<int,QString>::ConstIterator it = sourceIdNames.begin();
	  it != sourceIdNames.end(); ++it ) {
	if ( *it == name )
	    return it.key();
    }
    return -1;
}


void QuickInterpreter::cleanType( QString &type )
{
    type = type.simplifyWhiteSpace();
    if ( type.left( 5 ) == QString::fromLatin1("const") )
	type.remove( 0, 5 );
    if ( type[ (int)type.length() - 1 ] == '&' ||
	 type[ (int)type.length() - 1 ] == '*' )
	type.remove( type.length() - 1, 1 );
    type = type.simplifyWhiteSpace();
    if ( type == QString::fromLatin1("QString") )
	type = QString::fromLatin1("String");
    else if ( type == QString::fromLatin1("int") || type == QString::fromLatin1("uint") ||
	      type == QString::fromLatin1("long") || type == QString::fromLatin1("ulong") ||
	      type == QString::fromLatin1("double") || type == QString::fromLatin1("float") )
	type = QString::fromLatin1("Number");
    else if ( type == QString::fromLatin1("bool") )
	type = QString::fromLatin1("Boolean");
}

void QuickInterpreter::cleanTypeRev( QString &type )
{
    if ( type == QString::fromLatin1("String") )
	type = QString::fromLatin1("QString");
    else if ( type == QString::fromLatin1("Number") )
	type = QString::fromLatin1("double");
    else if ( type == QString::fromLatin1("Boolean") )
	type = QString::fromLatin1("bool");
}

bool QuickInterpreter::queryDispatchObjects( QObject *obj,
					     QPtrVector<QObject> &result )
{
    Q_ASSERT( obj );
    QMetaObject *meta = obj->metaObject();
    result.resize( 1 );
    result.insert( 0, obj );
    while ( meta ) {
	factory->constructInterface( meta->className(), (void*)obj, result );
	meta = meta->superClass();
    }
    return TRUE;
}

bool QuickInterpreter::queryDispatchObjects( const QCString &name, void *ptr,
					     QPtrVector<QObject> &result )
{
    return factory->constructInterface( name, ptr, result );
}

bool QuickInterpreter::construct( const QString &className,
				  const QValueList<QVariant> &vargs,
				  QPtrVector<QObject> &result )
{
    return factory->constructInstance( className, vargs, result );
}

QStringList QuickInterpreter::classes() const
{
    QPtrList<QSClass> clsLst = env()->classes();
    QStringList lst;
    for ( QSClass *cls = clsLst.first(); cls; cls = clsLst.next() ) {
	if ( cls->asClass() )
	    lst << cls->asClass()->identifier();
    }
    return lst;
}


QSObject QuickInterpreter::object( const QString &name ) const
{
    QSObject g = env()->globalObject();
    Global *global = (Global*)&g; // ### ugly
    QSObject obj;
    if ( name.isEmpty() ) {
	obj = g;
    } else {
	int p = name.findRev( '.' );
	if ( p == -1 )
	    obj = global->get( name );
	else
	    obj = global->getQualified( name );
    }
    return obj;
}


const QSClass *QuickInterpreter::classOf( const QSObject &obj ) const
{
    return obj.isA( env()->typeClass() ) ? QSTypeClass::classValue(&obj) : obj.objectType();
}

QStringList QuickInterpreter::functionsOf( QSObject &obj, bool includeSignature, bool includeNative, bool includeMemberFunctions ) const
{
    const QSClass *objType = obj.objectType();
    QSMemberMap mmap = objType == env()->typeClass() && includeMemberFunctions
		       ? ( (QSTypeClass*) objType )->allMembers( &obj )
		       : objType->members( &obj );

    QMap<int, QString> functions;
    for ( QSMemberMap::Iterator it = mmap.begin(); it != mmap.end(); ++it ) {
	if ( ( (*it).type() == QSMember::ScriptFunction ||
	       includeNative && ( (*it).type() == QSMember::NativeFunction ||
				  (*it).type() == QSMember::NativeMemberFunction ) )
	     && !(*it).isPrivate() ) {
	    QString func = (*it).name();
	    // ### will break with mix of script and C++ functions
	    int idx = (*it).type() == QSMember::ScriptFunction ?
		      (*it).scriptFunction->index() : functions.size();
	    if ( includeSignature ) {
		if ( (*it).type() == QSMember::NativeFunction ||
		     (*it).type() == QSMember::NativeMemberFunction ) {
		    func += QString::fromLatin1("()");
		} else {
		    QSFunctionScopeClass *fsc = (*it).scriptFunction->scopeDefinition();
		    int args = fsc->numArguments();
		    func += QString::fromLatin1("(");
		    if ( args > 0 ) {
			func += QString::fromLatin1(" ");
			QSMemberMap *members = fsc->definedMembers();
			QValueVector<QString> vec( args );
			for ( QSMemberMap::ConstIterator ait = members->begin();
			      ait != members->end(); ++ait ) {
			    if ( (*ait).index() < args )
				vec[ (*ait).index() ] = (*ait).name();
			}
			for ( int i = 0; i < args; ++i ) {
			    if ( i > 0 )
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
    for ( ; cit != functions.end(); ++cit )
	    lst << *cit;

    return lst;
}

QStringList QuickInterpreter::classesOf( QSObject &obj ) const
{
    const QSClass *cls = classOf( obj );
    QStringList lst;
    for ( int i = 0; i < cls->numStaticVariables(); ++i ) {
	QSObject o = cls->staticMember( i );
	if ( o.isA( env()->typeClass() ) && QSTypeClass::classValue(&o)->asClass() )
	    lst << QSTypeClass::classValue(&o)->identifier();
    }
    return lst;
}

QStringList QuickInterpreter::variablesOf( QSObject &obj, bool includeStatic, bool includeCustom,
					   bool includeMemberVariables ) const
{
    const QSClass *objType = obj.objectType();
    QSMemberMap mmap = objType == env()->typeClass() && includeMemberVariables
		       ? ( (QSTypeClass*) objType )->allMembers( &obj )
		       : objType->members( &obj );
    QStringList lst;
    for ( QSMemberMap::Iterator it = mmap.begin(); it != mmap.end(); ++it ) {
	if ( ( (*it).type() == QSMember::Variable ||
	       includeCustom && (*it).type() == QSMember::Custom ) &&
	     (!(*it).isStatic() || includeStatic) &&
	     !(*it).isExecutable() )
	    lst << (*it).name();
    }
    return lst;
}

static bool hasMember(QSEnv *env, const QString &function, QSMember::Type type)
{
    QSObject o = env->globalObject();
    QSMember member;
    QStringList names = QStringList::split(QString::fromLatin1("."), function);
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
		return FALSE;
	}
    }
    return FALSE;
}

bool QuickInterpreter::hasFunction( const QString &function ) const
{
    return hasMember(env(), function, QSMember::ScriptFunction);
}

bool QuickInterpreter::hasVariable( const QString &variable ) const
{
    return hasMember( env(), variable, QSMember::Variable);
}

bool QuickInterpreter::hasClass( const QString &className ) const
{
    QSObject foo = object(className);
    return foo.objectType() == env()->typeClass();
}
