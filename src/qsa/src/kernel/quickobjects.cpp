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

#include "quickobjects.h"

#include <qsfunction.h>
#include <qsoperations.h>
#include <qsenv.h>
#include <qsinternal.h>
#include <qsdate_object.h>
#include "quickinterpreter.h"
#include "quickdispatchobject.h"
#include <qmainwindow.h>
#include <private/qucomextra_p.h>
#include "quickbytearrayobject.h"
#include "quickpixmapobject.h"
#include "quickcolorobject.h"
#include "quickfontobject.h"
#include "quickcoordobjects.h"
#include <qsarray_object.h>
#include <qdatetime.h>
#include <qwidgetfactory.h>
#include "../engine/qsfuncref.h"
#include <qapplication.h>
#include <qmetaobject.h>
#include <qobjectlist.h>
#include <qptrdict.h>
#include <private/qcom_p.h>
#include <private/qpluginmanager_p.h>

#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
#include <qthread.h>
extern Q_EXPORT Qt::HANDLE qt_get_application_thread_id();
#endif

using namespace QS;

class qs_ptr_ref;
struct qs_method_info;

static QSObject uObjectToQS( QuickInterpreter *ip,
			     QUObject *o, const void *extra=0, QObject *qobj = 0 );
static bool qsToUObject(  QUObject *o,
			  const QSObject &v,
			  QUType *t,
			  const void *extra,
			  QPtrList<qs_ptr_ref> *allocs,
                          qs_method_info *);

static QValueList<QuickMetaData> getSlots( QObject* o, const char *s, bool super )
{
    QValueList<QuickMetaData> mds;
    const QMetaObject* meta = o->metaObject();
    QStrList slotList = meta->slotNames( super );
    int slen = qstrlen( s );
    int index = -1;
    const char *sl = slotList.first();
    for ( ; sl; sl = slotList.next() ) {
	index++;
	int pos = strchr( sl, '(' ) - sl;
	if ( slen == pos && !qstrncmp( sl, s, pos ) )  { // param less match
	    if ( sl ) { // ### why this check ? (Harri)
		const QMetaData* md = meta->slot( index, super );
		if ( md && md->access != QMetaData::Private )
		    mds.append( QuickMetaData( md, index + (super?0:meta->slotOffset()) ) );
	    }
	}
    }

    return mds;
}

QObjectList *extraChildren( QObject *wid )
{
    QObjectList *extra = 0;
    QObject *o;
    if ( wid->isA( "QMainWindow" ) ) {
	extra = new QObjectList;
	QWidget *w = ( (QMainWindow*)wid )->centralWidget();
	if ( w && w->children() ) {
	    QObjectListIt it( *w->children() );
	    while ( it.current() ) {
		extra->append( it.current() );
		++it;
	    }
	}
	QObjectList *l = wid->queryList( "QToolBar" );
	if ( l ) {
	    QObjectListIt it( *l );
	    while ( it.current() ) {
		extra->append( it.current() );
		++it;
	    }
	}
	delete l;
	l = wid->queryList( "QPopupMenu" );
	if ( l ) {
	    QObjectListIt it( *l );
	    while ( it.current() ) {
		extra->append( it.current() );
		++it;
	    }
	}
	delete l;
    } else if ( ( o = wid->child( 0, "QLayoutWidget" ) ) ) {
	if ( o->children() ) {
	    if ( !extra )
		extra = new QObjectList;
	    QObjectListIt it( *o->children() );
	    while ( it.current() ) {
		extra->append( it.current() );
		++it;
	    }
	}
    }
    return extra;
}

QObject *objectChild( QObject *wid, const char *objName, const char *inheritsClass, bool recursiveSearch )
{
    QObject *o = wid->child( objName, inheritsClass, recursiveSearch );
    if ( o )
	return o;
    if ( !o ) {
	if ( wid->isA( "QMainWindow" ) && ( objName || inheritsClass ) ) {
	    QObjectList *l = extraChildren( wid );
	    if ( l ) {
		QObjectListIt it( *l );
		while ( it.current() ) {
		    if ( ( !objName || qstrcmp( objName, it.current()->name() ) == 0 ) &&
			 ( !inheritsClass || it.current()->inherits( inheritsClass ) ) ) {
			delete l;
			return it.current();
		    }
		    ++it;
		}
	    }
	    delete l;
	} else if ( ( o = wid->child( 0, "QLayoutWidget", FALSE ) ) ) {
	    return o->child( objName, inheritsClass, FALSE );
	}
    }
    // ### not nice, but try to be as good as possible
    return wid->child( objName, inheritsClass, TRUE );
}

/////////////////// internal functions /////////////////////////////

class qs_ptr_ref
{
public:
    enum Type { FloatPtr, CharStar, UIntPtr, Variant, LongPtr, ULongPtr, UCharPtr,
		ShortPtr, UShortPtr, IntListPtr, ObjectListPtr, VariantListPtr,
                StringListPtr };
    qs_ptr_ref(char *str) : charStar( str ), type( CharStar ) { }
    qs_ptr_ref(float *ptr) : floatPtr( ptr ), type( FloatPtr ) { }
    qs_ptr_ref(uint *ptr) : uintPtr(ptr), type(UIntPtr) { }
    qs_ptr_ref(long *ptr) : longPtr(ptr), type(LongPtr) { }
    qs_ptr_ref(ulong *ptr) : ulongPtr(ptr), type(ULongPtr) { }
    qs_ptr_ref(QVariant *ptr) : varPtr(ptr), type(Variant) { }
    qs_ptr_ref(uchar *ptr) : ucharPtr(ptr), type(UCharPtr) { }
    qs_ptr_ref(short *ptr) : shortPtr(ptr), type(ShortPtr) { }
    qs_ptr_ref(ushort *ptr) : ushortPtr(ptr), type(UShortPtr) { }
    qs_ptr_ref(QValueList<int> *ptr) : intListPtr(ptr), type(IntListPtr) { }
    qs_ptr_ref(QObjectList *ptr) : objListPtr(ptr), type(ObjectListPtr) { }
    qs_ptr_ref(QValueList<QVariant> *ptr) : varListPtr(ptr), type(VariantListPtr) { }
    qs_ptr_ref(QStringList *ptr) : stringListPtr(ptr), type(StringListPtr) { }

    ~qs_ptr_ref()
    {
	switch( type ) {
	case FloatPtr:
	    delete floatPtr;
	    break;
	case CharStar:
	    delete [] charStar;
	    break;
	case UIntPtr:
	    delete uintPtr;
	    break;
	case Variant:
	    delete varPtr;
	    break;
	case LongPtr:
	    delete longPtr;
	    break;
	case ULongPtr:
	    delete ulongPtr;
	    break;
	case UCharPtr:
	    delete ucharPtr;
	    break;
	case ShortPtr:
	    delete shortPtr;
	    break;
	case UShortPtr:
	    delete ushortPtr;
	    break;
	case IntListPtr:
	    delete intListPtr;
	    break;
	case ObjectListPtr:
	    delete objListPtr;
	    break;
	case VariantListPtr:
	    delete varListPtr;
	    break;
        case StringListPtr:
            delete stringListPtr;
	}
    }
    union {
	char *charStar;
	float *floatPtr;
	uint *uintPtr;
	long *longPtr;
	ulong *ulongPtr;
	QVariant *varPtr;
	uchar *ucharPtr;
	short *shortPtr;
	ushort *ushortPtr;
	QValueList<int> *intListPtr;
	QObjectList *objListPtr;
	QValueList<QVariant> *varListPtr;
        QStringList *stringListPtr;
    };
    Type type;
};

struct qs_method_info
{
    const char *name;
    int no;

    bool isCurrentReference() const
    {
        Q_ASSERT(name);

        int pos = 0;

        // find (
        while (name[pos] && name[pos] != '(') ++pos;

        Q_ASSERT(name[pos]);
        ++pos;

        bool inTemplate = false;
        int arg = 0;
        while (name[pos] && arg <= no) {
            if (name[pos] == '<')
                inTemplate = true;
            else if (name[pos] == '>')
                inTemplate = false;
            else if (!inTemplate && name[pos] == ',')
                ++arg;
            else if (arg == no && name[pos] == '&')
                return TRUE;
            ++pos;
        }
        return FALSE;
    }
};

#define QSA_SLOT_ARGUMENT_LIMIT 16

static QSObject qsa_execute_slot_no_cast(QSEnv *env, QObject *qobj,
                                         const QValueList<QuickMetaData> &mds, bool *matched)
{
    const QString str_qobject = QString::fromLatin1("QObject");
    const QString str_pointer = QString::fromLatin1("Pointer");

    const QSList &args = *env->arguments();

    QuickInterpreter *ip = QuickInterpreter::fromEnv(env);
    QSObject foo = ip->env()->createNumber(23);

    QUObject slotargs[QSA_SLOT_ARGUMENT_LIMIT];

    for (QValueList<QuickMetaData>::ConstIterator it = mds.begin(); it != mds.end(); ++it) {
        const QUMethod *m = (*it).method;

        if (m->count > QSA_SLOT_ARGUMENT_LIMIT)
            continue;

        Q_ASSERT(m);
	int offset = m->count && m->parameters[0].inOut == QUParameter::Out;

	if (args.size() != (m->count - offset))
	    continue;

        bool matching = true;

        const QUParameter *p = m->parameters;

        for (int i=0; i<m->count - offset && matching; ++i) {

            const QSObject &arg = args.at(i);

            if (QUType::isEqual(p[i + offset].type, &static_QUType_QString)
                && arg.objectType() == env->stringClass()) {
                static_QUType_QString.set(&slotargs[i+1], arg.toString());

            } else if (QUType::isEqual(p[i + offset].type, &static_QUType_bool)
                       && arg.objectType() == env->booleanClass()) {
                static_QUType_bool.set(&slotargs[i+1], arg.toBoolean());

            } else if (QUType::isEqual(p[i + offset].type, &static_QUType_double)
                       && arg.objectType() == env->numberClass()) {
                static_QUType_double.set(&slotargs[i+1], arg.toNumber());

#if 0
            } else if (QUType::isEqual(p[i + offset].type, &static_QUType_int)
                       && arg.objectType() == env->numberClass()) {
                static_QUType_int.set(&slotargs[i + 1], arg.toNumber());
#endif

            } else if (QUType::isEqual(p[i + offset].type, &static_QUType_ptr)) {

                bool ok = false;

                // QObject subclasses
                if (arg.isA(ip->wrapperClass())) {
                    QObject *qobject = QSWrapperClass::object(&arg);
                    Q_ASSERT(qobject);
                    if (qobject->inherits((const char *)p[i+offset].typeExtra)) {
                        static_QUType_ptr.set(&slotargs[i+1], qobject);
                        ok = true;
                    }

                // Pointer types.
                } else  if (arg.isA(ip->pointerClass())) {
                    if (qstrcmp(ip->pointerClass()->pointerType(&arg),
                                (const char *)p[i+offset].typeExtra) == 0) {
                        static_QUType_ptr.set(&slotargs[i+1],
                                              ip->pointerClass()->pointer(&arg));
                        ok = true;
                    }
                }

                matching = ok;

            } else {
                matching = false;
            }

        } // for (int i=offset ...

        // This function seems to be a perfect match, invoke it.
        if (matching) {

            qobj->qt_invoke((*it).id, slotargs);

            Q_ASSERT(matched);
            *matched = true;

            // has return value
            if (offset) {
                QSObject ret = uObjectToQS(ip, &slotargs[0], m->parameters[0].typeExtra,
                    qobj);
                return ret;
            } else {
                return QSObject();
            }
        }

    }

    Q_ASSERT(matched);
    *matched = false;
    return QSObject();
}

static QSObject executeSlot( QSEnv *env, QObject *qobj,
                             const QValueList<QuickMetaData> &mds )
{
    bool exactMatch = false;
    QSObject obj = qsa_execute_slot_no_cast(env, qobj, mds, &exactMatch);
    if (exactMatch) {
        return obj;
    }

    const QSList &args = *env->arguments();
    const QUMethod *m = 0;
    bool firstIsReturn = FALSE;

    int returnOffset = 0;
    bool ok = FALSE;
    QUObject *uo = 0;
    QPtrList<qs_ptr_ref> pointers;
    pointers.setAutoDelete( TRUE );
    QValueList<QuickMetaData>::ConstIterator md = mds.begin();
    while (md != mds.end()) {
        m = (*md).method;
        if ( m && m->count && m->parameters[0].inOut == QUParameter::Out )
            firstIsReturn = TRUE;
        if ( args.size() != ( m->count - ( firstIsReturn ? 1 : 0 ) ) ) {
            ++md;
            m = 0;
            firstIsReturn = FALSE;
            continue;
        }

        returnOffset = firstIsReturn ? 0 : 1;

        uo = new QUObject[ m->count + ( firstIsReturn ? 0 : 1 ) ];
        const QUParameter *p = m->parameters;
        ok = TRUE;

        qs_method_info methodInfo = { (*md).name, 0 };

        for ( int i = 0; i < m->count; i++ ) {
            if ( firstIsReturn && !i )
                continue;

            methodInfo.no = i - (firstIsReturn ? 1 : 0);

            if ( !qsToUObject( &uo[ i + ( firstIsReturn ? 0 : 1 ) ],
                               args[ i - ( firstIsReturn ? 1 : 0 ) ],
                               p[ i ].type, p[ i ].typeExtra, &pointers,
                               &methodInfo ) ) {
                if ( QUType::isEqual( p[ i ].type, &static_QUType_QVariant ) ) {
                    QVariant var = args[ methodInfo.no ].
                                   toVariant( (QVariant::Type)*(char*)p[i].typeExtra );
                    if ( var.type() != QVariant::Invalid )
                        static_QUType_QVariant.set( &uo[ i + returnOffset ],
                                                    var );
                } else if ( QUType::isEqual( p[ i ].type, &static_QUType_varptr ) ) {
                    QVariant var = args[ methodInfo.no ].
                                   toVariant( (QVariant::Type)*(char*)p[i].typeExtra );
                    ok = var.type() == (QVariant::Type)*(char*)p[i].typeExtra;
                    if ( ok )
                        static_QUType_varptr.set( &uo[ i + returnOffset ],
                                                  var.rawAccess( 0, QVariant::Invalid, TRUE ) );
                } else {
                    ok = FALSE;
                    break;
                }
            }
        }
        if ( ok ) {
            break;
        } else {
            ++md;
            m = 0;
            firstIsReturn = FALSE;
            delete [] uo;
        }
    }

    if ( !ok ) {
        QString msg = QString::fromLatin1( "No matching overload found. Following overloads are possible:\n" );
        md = mds.begin();
        while ( md != mds.end() ) {
            msg += QString::fromLatin1("    ") + QString::fromLatin1( (*md).name ) + QString::fromLatin1("\n");
            ++md;
        }
        msg.remove( msg.length() - 1, 1 );
        return env->throwError( TypeError, msg );
    }

    qobj->qt_invoke( (*md).id, uo );
    QuickInterpreter *ip = QuickInterpreter::fromEnv( env );
    QSObject ret = firstIsReturn ?
                   uObjectToQS( ip, uo, m->parameters[0].typeExtra, qobj ) :
                   env->createUndefined();
    delete [] uo;
    return ret;
}

class QSVariantShared : public QSShared
{
public:
    QSVariantShared( const QVariant &v, const QMetaProperty *m );
    virtual void invalidate();
    void createObject( QuickInterpreter * );

    QVariant variant;
    const QMetaProperty *mp;
    QSObject iobj;
    uint native : 1;
};

void QSWrapperSharedWatcher::objectDestroyed(QObject *obj) {
    if (!parent->objects.isEmpty() && parent->objects[0] == obj)
        parent->objects.data()[0] = 0;
}


QSWrapperShared::QSWrapperShared( const QSWrapperClass *cl )
    : QSInstanceData( 0, cl->createUndefined() ), cls( cl ),
      udata( 0 ), objTyp( FactoryObject )
{
    watcher.parent = this;
    creator = 0;
}

QSWrapperShared::~QSWrapperShared()
{
    if (!cls->env()->isShuttingDown() && isConnected())
        cls->env()->removeShared(this);

    for( QMap<QString,QuickScriptProperty>::Iterator cached = propertyCache.begin();
         cached != propertyCache.end(); ++cached )
        if( (*cached).type == QSOT::Property ) {
            QuickScriptVariant *variant = (*cached).id.var;
            variant->setVal( (QSShared*) 0 );
            variant->invalidate();
            delete variant;
        }

    invalidateWrapper();
    Q_ASSERT( objects.isEmpty() && receivers.isEmpty() );
}

void QSWrapperShared::invalidate()
{
    invalidateWrapper();
    if (cls->env()->isShuttingDown())
        QSInstanceData::invalidate();
}

void QSWrapperShared::invalidateWrapper()
{
    QuickScriptReceiver *r;
    QMap<QObject*, QuickScriptReceiver*>::Iterator it = receivers.begin();
    while ( it != receivers.end() ) {
        r = *it;
        r->invalidate();
        ++it;
        delete r;
    }
    receivers.clear();
    // let QObject lose the reference to us
    if ( udata ) {
        udata->invalidate();
        udata = 0;
    }
    for ( uint i = 1; i < objects.count(); i++ )
        delete objects[ i ];

    if( objects.count() > 0
        && objTyp == FactoryObject
        && objects.at(0)
        && !objects.at( 0 )->parent() ) {
        delete objects[ 0 ];
    }

    objects.resize( 0 );
}

EventId QSWrapperShared::findEventId( const QString &event )
{
    QString ev = event.left( event.find( '(' ) );
    const char *e = ev.latin1();
    int l = ev.length();
    for ( int i = (int)objects.count()-1; i >= 0; --i ) {
        const QMetaObject *meta = objects[ i ]->metaObject();
        int n = meta->numSignals( TRUE );
        for ( int j = n - 1; j >= 0; --j ) {
            const QMetaData *sig = meta->signal( j, TRUE );
            int pos = strchr( sig->name, '(' ) - sig->name;
            if ( l == pos && !qstrncmp( sig->name, e, pos ) ) {
                const QUMethod *m = sig->method;
                int mc = m->count;
                if ( m->count && m->parameters[0].inOut == QUParameter::Out )
                    mc--;
                QString ee( event.mid(ev.length()+1, event.findRev(')')-ev.length()-1));
                ee = ee.simplifyWhiteSpace();
                QStringList lst = QStringList::split( ',', ee );
                if ( (int)lst.count() != mc )
                    continue;
                QStringList::ConstIterator sit = lst.begin();
                bool ok = TRUE;
                for ( int k = ( mc == m->count ? 0 : 1); k < mc; ++k, ++sit ) {
                    QString arg = *sit;
                    int asterix = arg.find( '*' );
                    if ( asterix != -1 )
                        arg.remove( asterix, 1 );
                    int ampersand = arg.find( '&' );
                    if ( ampersand != -1 )
                        arg.remove( ampersand, 1 );
                    int cnst = arg.find( QString::fromLatin1("const") );
                    if ( cnst != -1 ) {
                        arg.remove( 0, cnst+5 );
                        arg = arg.simplifyWhiteSpace();
                    }
                    QStringList al = QStringList::split( ':', arg );
                    arg = al[(int)al.count() - 1].simplifyWhiteSpace();
                    QuickInterpreter::cleanTypeRev( arg );
                    QString type = QString::fromLatin1(m->parameters[k].type->desc());
                    bool isPtrType = type == QString::fromLatin1("ptr")
                           || type == QString::fromLatin1("varptr");
                    if ( type == QString::fromLatin1("ptr") )
                        type = (const char*)m->parameters[k].typeExtra;
                    else if ( type == QString::fromLatin1("varptr") )
                        type = QVariant::typeToName( (QVariant::Type)*(char*)m->parameters[k].typeExtra );
                    if ( arg != type && !( isPtrType
                                           && ( arg == QString::fromLatin1("ptr")
                                                || arg == QString::fromLatin1("varptr") ) ) ) {
                        ok = FALSE;
                        break;
                    }
                }
                if ( !ok )
                    continue;
                return EventId( j, i );
            }
        }
    }
    return EventId();
}

bool QSWrapperShared::setEventHandler( QuickInterpreter *ip, const QString &event,
                                       QObject *ctx, const QString &func, QSObject qsctx )
{
    EventId evid = findEventId( event );
    if ( evid.id == -1 )
        return FALSE;
    QMap<QObject*, QuickScriptReceiver*>::ConstIterator it = receivers.find( objects[ evid.obj ] );
    QuickScriptReceiver *r = 0;
    if ( it == receivers.end() ) {
        r = new QuickScriptReceiver( objects[ evid.obj ] );
        receivers.insert( objects[ evid.obj ], r );
    } else {
        r = *it;
    }
    r->setEventHandler( ip, evid.id, ctx, func, qsctx );
    return TRUE;
}

bool QSWrapperShared::removeEventHandler( const QString &event,
                                          QObject *ctx, const QString &func, QSObject qsctx )
{
    EventId evid = findEventId( event );
    if ( evid.id == -1 )
        return FALSE;
    QMap<QObject*, QuickScriptReceiver*>::Iterator it = receivers.find( objects[ evid.obj ] );
    if ( it == receivers.end() )
        return FALSE;
    (*it)->removeEventHandler( evid.id, ctx, func, qsctx );
    return TRUE;
}

QSWrapperClass::QSWrapperClass( QSClass *b )
    : QSWritableClass( b ), QuickEnvClass( b->env() )
{
}

QSWrapperClass::~QSWrapperClass()
{
    invalidate();
}

QSWrapperShared *QSWrapperClass::shared( const QSObject *obj ) const
{
    Q_ASSERT( obj->isA( this ) );
    return (QSWrapperShared*)obj->shVal();
}

QPtrVector<QObject>& QSWrapperClass::objectVector( const QSObject *obj ) const
{
    return shared( obj )->objects;
}

void QSWrapperClass::invalidate()
{
//     QPtrDictIterator<QSWrapperShared> it( *dict );
//     QSWrapperShared *sh;
//     while ( ( sh = it.current() ) ) {
// 	QObject::disconnect( (QObject*)it.currentKey(), SIGNAL( destroyed( QObject* ) ),
// 			     this, SLOT( objectDestroyed( QObject* ) ) );
// 	++it;
// 	sh->invalidate( this );
// 	// sh is deleted by reference counting
//     }
//     dict->clear();
}

void QSWrapperClass::deref( QSObject *o ) const
{
#ifndef QS_LEAK
    o->shVal()->deref();
    if ( o->shVal()->count==0 ) {
        ((QSWrapperClass*)this)->invalidate();
        // Removed from environment by the destructor... Do it there
        // since it can also be removed as an effect of the userdata
        // being deleted...
        delete o->shVal();
        o->setVal( (QSShared*)0 );
    }
#endif
}

bool QSWrapperClass::member( const QSObject *objPtr, const QString &p,
                             QSMember *mem ) const
{
    if ( !objPtr )
        return QSWritableClass::member( objPtr, p, mem );

    mem->setType( QSMember::Custom );
    mem->setOwner( this );
    mem->setName( p );

    QSWrapperShared *sh = shared( objPtr );
    if( !sh )
        return FALSE;
    const QPtrVector<QObject> &objects = sh->objects;

    if ( objects.isEmpty() )
        return QSWritableClass::member( objPtr, p, mem );

    QString key;
    // cache lookup
    QMap<QString, QSOT::QuickScriptObjectType>::ConstIterator it2
        = sh->hasPropCache.find( p );
    if ( it2 != sh->hasPropCache.end() ) {
        if ( *it2 == QSOT::Object ) {
            for ( int i = (uint)objects.count()-1; i >= 0; i-- ) {
                QObject *obj = objectChild( objects[ i ], p.ascii(), "QObject", FALSE );
                if ( obj )
                    return TRUE;
            }
        } else if ( *it2 != QSOT::Unknown ) {
            key = p;
            QMap<QString,QuickScriptProperty>::Iterator it =
                sh->propertyCache.find( key );
            if ( it != sh->propertyCache.end() ) {
                switch ( (*it).type ) {
                case QSOT::Property: {
                    QVariant v;
                    const QMetaProperty *mp = (*it).id.var->metaProperty();
                    objects[ (*it).objNum ]->qt_property( mp->id(), 1, &v );
                    if ( !mp->writable() )
                        mem->setWritable( FALSE );
                    if ( mp->isEnumType() ) {
                        return TRUE;
                    } else {
                        if ( v != (*it).id.var->value() ) {
                            /* Don't allocate a new one, change existing instead. Solves
                               memory issue and should be sligtly more optimal */
                            ( (QSVariantShared*) (*it).id.var->shVal() )->variant = v;
                        }
                        return TRUE;
                    }
                }
                case QSOT::Slot:
                    mem->setExecutable( TRUE );
                    mem->setWritable( FALSE );
                    return TRUE;
                case QSOT::Enum:
                    mem->setWritable( FALSE );
                    return TRUE;
                default:
                    break;
                }
                return QSWritableClass::member( objPtr, p, mem );
            } else {
                int i;
                switch ( *it2 ) {
                case QSOT::Property: {
                    for ( i = (uint)objects.count()-1; i >= 0; i-- ) {
                        const QMetaProperty *mp =
                            objects[ i ]->metaObject()->
                            property(objects[i]->metaObject()->findProperty(p.ascii(), TRUE), TRUE);
                        if ( mp ) {
                            QVariant v;
                            objects[ i ]->qt_property( mp->id(), 1, &v );
                            if ( !mp->writable() )
                                mem->setWritable( FALSE );
                            if ( mp->isEnumType() ) {
                                return TRUE;;
                            } else {
                                QuickScriptVariant *var =
                                    new QuickScriptVariant( interpreter(), v, mp );
                                Q_ASSERT( !sh->propertyCache.contains( key ) );
                                sh->propertyCache.
                                    replace( key, QuickScriptProperty( QSOT::Property, var, i ) );
                                return TRUE;
                            }
                        }
                    }
                } break;
                case QSOT::Slot: {
                    for ( i = int(objects.count())-1; i >= 0; i-- ) {
                        QValueList<QuickMetaData> mds = getSlots( objects[ i ], p.ascii(), TRUE);
                        if ( !mds.isEmpty() ) {
                            mem->setExecutable( TRUE );
                            mem->setWritable( FALSE );
                            QuickScriptObjectFunc *func = 0; // new QuickScriptObjectFunc( objects[ i ], mds );
                            sh->propertyCache.
                                replace( key, QuickScriptProperty( QSOT::Slot, func, i ) );
                            return TRUE;
                        }
                    }
                case QSOT::Enum:
                    for ( i = int(objects.count())-1; i >= 0; i-- ) {
                        QStrList enums = objects[ i ]->metaObject()->enumeratorNames(TRUE);
                        for ( int k = 0; k < (int)enums.count(); ++k ) {
                            const QMetaEnum *me =
                                objects[ i ]->metaObject()->enumerator( enums.at( k ),TRUE);
                            for ( int l = 0; l < (int)me->count; ++l ) {
                                if ( qstrcmp( me->items[l].key, p.ascii() ) == 0 ) {
                                    mem->setWritable( FALSE );
                                    sh->propertyCache.
                                        replace( key,
                                                 QuickScriptProperty( QSOT::Enum,
                                                                      me->items[l].value, i ) );
                                    return TRUE;
                                }
                            }
                        }
                    }
                } break;
                default:
                    break;
                }
            }
        } else {
            return QSWritableClass::member( objPtr, p, mem );
        }
    }

    // no cache hit. doing a real search now starting with enums.
    if (!objects[0]) {
        env()->throwError(QString::fromLatin1("Object has been deleted"));
        return FALSE;
    }

    QStrList enums = objects[ 0 ]->metaObject()->enumeratorNames( TRUE );
    for ( int k = 0; k < (int)enums.count(); ++k ) {
        const QMetaEnum *me = objects[ 0 ]->metaObject()->enumerator( enums.at( k ), TRUE );
        for ( int l = 0; l < (int)me->count; ++l ) {
            if ( qstrcmp( me->items[l].key, p.ascii() ) == 0 ) {
                mem->setWritable( FALSE );
                sh->hasPropCache.replace( p, QSOT::Property );
                sh->propertyCache.
                    replace( p, QuickScriptProperty( QSOT::Enum, me->items[l].value, 0 ) );
                return TRUE;
            }
        }
    }

    // search QObject properties, childs etc.
    for ( int i = int(objects.count())-1; i >= 0; i-- ) {
        QObject *o = objects[ i ];
        const QMetaObject *meta = o->metaObject();
        const QMetaProperty *mp = meta->
                                  property( meta->findProperty( p.ascii(), TRUE ), TRUE );
        if ( mp ) {
            if( !mp->scriptable() ) {
                return FALSE;
            }
            if ( key.isEmpty() )
                key = p;
            if ( !mp->writable() )
                mem->setWritable( FALSE );
            sh->hasPropCache.replace( p, QSOT::Property );
            QVariant v;
            o->qt_property( mp->id(), 1, &v );
            if ( mp->isEnumType() ) {
                return TRUE;
            } else {
                QuickScriptVariant *var =
                    new QuickScriptVariant( interpreter(), v, mp );
                Q_ASSERT( !sh->propertyCache.contains( key ) );
                sh->propertyCache.
                    replace( key, QuickScriptProperty( QSOT::Property, var, i ) );
                return TRUE;
            }
        }

        QObject *cobj = objectChild( o, p.ascii(), "QObject", FALSE );
        if ( cobj ) {
            sh->hasPropCache.replace( p, QSOT::Object );
            return TRUE;
        }

        QValueList<QuickMetaData> mds = getSlots( o, p.ascii(), TRUE );
        if ( !mds.isEmpty() ) {
            if ( key.isEmpty() )
                key = p;
            mem->setExecutable( TRUE );
            mem->setWritable( FALSE );
            QuickScriptObjectFunc *func = 0; // new QuickScriptObjectFunc( objects[ i ], mds );
            sh->propertyCache.
                replace( key, QuickScriptProperty( QSOT::Slot, func, i ) );
            sh->hasPropCache.replace( p, QSOT::Slot );
            return TRUE;
        }

        QStrList enums = meta->enumeratorNames( TRUE );
        for ( int k = 0; k < (int)enums.count(); ++k ) {
            const QMetaEnum *me = meta->enumerator( enums.at( k ), TRUE );
            for ( int l = 0; l < (int)me->count; ++l ) {
                if ( qstrcmp( me->items[l].key, p.ascii() ) == 0 ) {
                    sh->propertyCache.
                        replace( p, QuickScriptProperty( QSOT::Enum, me->items[l].value, i ) );
                    return TRUE;
                }
            }
        }
    }

    sh->hasPropCache.replace( p, QSOT::Unknown );

    return QSWritableClass::member( objPtr, p, mem );
}

QSObject QSWrapperClass::fetchValue( const QSObject *o,
                                     const QSMember &mem ) const
{
    // member specific to this class ?
    if ( mem.type() != QSMember::Custom )
        return QSWritableClass::fetchValue( o, mem );

    QSWrapperShared *sh = shared( o );
    const QPtrVector<QObject> &objects = sh->objects;
    QString p = mem.name();

    QMap<QString, QSOT::QuickScriptObjectType>::ConstIterator it2
        = sh->hasPropCache.find( p );
    Q_ASSERT( it2 != sh->hasPropCache.end() ); // filled my member()
    Q_ASSERT( *it2 != QSOT::Unknown );

    if ( *it2 == QSOT::Object ) {
        for ( int i = int(objects.count())-1; i >= 0; i-- ) {
            QObject *obj = objectChild( objects[ i ], p.ascii(), "QObject", FALSE );
            if ( obj )
                return interpreter()->wrap( obj );
        }
    } else {
        QMap<QString,QuickScriptProperty>::Iterator it =
            sh->propertyCache.find( p );
        Q_ASSERT( it != sh->propertyCache.end() );  // filled my member()
        switch ( (*it).type ) {
        case QSOT::Property: {
            QVariant v;
            const QMetaProperty *mp = (*it).id.var->metaProperty();
            objects[ (*it).objNum ]->qt_property( mp->id(), 1, &v );
            if ( mp->isEnumType() ) {
                return createString( QString::fromLatin1(mp->valueToKey( v.toInt() )) );
            } else {
                if ( v != (*it).id.var->value() ) {
                    /* Reuse old object instead of creating new. */
                    ( (QSVariantShared*) (*it).id.var->shVal() )->variant = v;
                }
                if ( (*it).id.var->isNative() )
                    return (*it).id.var->toNative();
                return  *(*it).id.var;
            }
        }
        case QSOT::Slot:
            return env()->funcRefClass()->createReference( *o, mem );
        case QSOT::Enum:
            return createNumber( (*it).id.enumValue );
        default:
            break;
        }
    }

    qWarning( "QSWrapperClass::write: shouldn't get here" );
    return createUndefined();
}

QSObject QSWrapperClass::invoke( QSObject *objPtr, const QSMember &mem ) const
{
    if ( mem.type() != QSMember::Custom )
        return QSWritableClass::invoke( objPtr, mem );

    Q_ASSERT( mem.owner() == this );
    QSWrapperShared *sh = shared( objPtr );
    const QPtrVector<QObject> &objects = sh->objects;
    const char *asc = mem.name().ascii();

    for ( int i = int(objects.count())-1; i >= 0; i-- ) {
        QValueList<QuickMetaData> mds = getSlots( objects[ i ], asc, TRUE );
        if ( !mds.isEmpty() ) {
            return executeSlot( env(), objects[ i ], mds );
        }
    }

    qWarning( "QSWraperClass::invoke: shouldn't get here" );
    return createUndefined();
}

void QSWrapperClass::write( QSObject *objPtr, const QSMember &mem,
                            const QSObject &val ) const
{
    Q_ASSERT ( mem.isWritable() );

    // member specific to this class ?
    if ( mem.type() != QSMember::Custom ) {
        QSWritableClass::write( objPtr, mem, val );
        return;
    }

    QSWrapperShared *sh = shared( objPtr );
    const QPtrVector<QObject> &objects = sh->objects;
    const char *n = mem.name().ascii();

    for ( int i = objects.size()-1; i >= 0; --i ) {
        QObject *obj = objects[ i ];
        const QMetaObject *meta = obj->metaObject();
        const QMetaProperty *prop =
            meta->property( meta->findProperty( n, TRUE ), TRUE );
        if ( !prop )
            continue;

        // shouldn't happen since writable is set to false by member()
        Q_ASSERT( prop->writable() );

        QVariant var = val.toVariant( QVariant::nameToType( prop->type() ) );

        if ( prop->isEnumType() ) {
            if ( var.type() == QVariant::Double )
                var.asInt();
            if ( !obj->setProperty( n, var ) ) {
                qWarning( "QuickScriptObject::put( %s ) setting enum failed.", n );
                // ### provide more descriptive message. Like 'invalid type x',
                // or 'unkown enum. possible enums are x, y and z'.
            }
            return;
        }
        bool b = obj->setProperty( n, var );
        if ( !b ) {
            qWarning( "QuickScriptObject::put( %s ) failed.", n );
        }
        return;
    }
    QSWritableClass::write( objPtr, mem, val );
}

bool QSWrapperClass::toBoolean( const QSObject *obj ) const
{
    return !objectVector( obj ).isEmpty();
}

QString QSWrapperClass::toString( const QSObject *obj ) const
{
    QPtrVector<QObject> &objects = objectVector( obj );
    if ( objects.isEmpty() )
        return QString::fromLatin1("[object QObject]");
    return QString::fromLatin1("[object ") + QString::fromLatin1( objects[0]->className() ) + QString::fromLatin1("]");
}

QVariant QSWrapperClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    const QPtrVector<QObject>& objects = objectVector( obj );
    QString s = QString::fromLatin1("Pointer:%1:QObject"); // ### ugly
    s = s.arg( QString::number( (ulong)(objects[0] ) ) );
    return QVariant( s );
}

QString QSWrapperClass::debugString( const QSObject *obj ) const
{
    QPtrVector<QObject> &objects = objectVector( obj );
    QString s = QString::fromLatin1("{");
    bool first = TRUE;
    uint i;
    for ( i = 0; i < objects.count(); i++ ) {
        if ( objects[i]->isA( "QuickNamespace" ) )
            continue;
        const QObjectList *ch = objects[i]->children();
        if ( ch ) {
            QObjectListIt it( *ch );
            while ( it.current() ) {
                QObject *o = it.current();
                if ( o->inherits( "QLayoutWidget" ) ||
                     o-> inherits( "QWidgetStack" ) ||
                     o->inherits( "Spacer" ) ||
                     o->inherits( "QSizeGrip" ) ||
                     o->inherits( "QWidgetStack" ) ) {
                    ++it;
                    continue;
                }
                QString n = QString::fromLatin1(o->name());
                if ( n.find( QString::fromLatin1(" ") ) != -1
                     || n.find( QString::fromLatin1("qt_") ) != -1
                     || n.find( QString::fromLatin1("unnamed") ) != -1 ) {
                    ++it;
                    continue;
                }
                QSObject wrp = interpreter()->wrap( o );
                if ( !first )
                    s += QString::fromLatin1(",");
                first = FALSE;
                s += wrp.objectType()->identifier() + QString::fromLatin1("=") + wrp.debugString();
                ++it;
            }
        }
    }

    for ( i = 0; i < objects.count(); i++ ) {
        if ( objects[i]->isA( "QuickNamespace" ) )
            continue;
        const QMetaObject *meta = objects[i]->metaObject();
        for ( int j = 0; j < meta->numProperties( TRUE ); ++j ) {
            const QMetaProperty *mp = meta->property( j, TRUE );
            if ( i != 0 && qstrcmp( mp->name(), "name" ) == 0 )
                continue;
            if ( !first )
                s += QString::fromLatin1(",");
            first = FALSE;
            s += QString::fromLatin1(mp->name());
            s += QString::fromLatin1("=");
            s += objects[i]->property( mp->name() ).toString();
            s += QString::fromLatin1(":");
            QString t = QString::fromLatin1(mp->type());
            QuickInterpreter::cleanType( t );
            s += t;
        }
    }
    s += QString::fromLatin1("}:") + QString::fromLatin1( objects[0]->className() );
    return s;
}

/*!
  \reimp
  Handles two special cases: a) a comparison of references to QObjects
  no matter whether the QObjects still or already use a detached wrapper
  class. b) distinct pointer wrapper objects containing identical pointers
*/

QSEqualsResult QSWrapperClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    Q_ASSERT( a.objectType() == this );
    if ( !b.isA( interpreter()->wrapperClass() ) &&
         !b.isA( interpreter()->pointerClass() ) )
        return EqualsUndefined;

    // check whether the references point to the same QObject
    QSWrapperClass *aclass = (QSWrapperClass*)a.objectType();
    QSWrapperClass *bclass = (QSWrapperClass*)b.objectType();
    QPtrVector<QObject>& ova = aclass->objectVector( &a );
    QPtrVector<QObject>& ovb = bclass->objectVector( &b );
    if ( ova.isEmpty() && ovb.isEmpty() ) // both null pointers ?
        return EqualsIsEqual;
    return ( QSEqualsResult ) ( !ova.isEmpty() && !ovb.isEmpty() && ova[0] == ovb[0] );
}

QSObject QSWrapperClass::wrap( const QPtrVector<QObject> &objs )
{
    QSWrapperShared *sh = new QSWrapperShared( this );
    sh->objects = objs;
    if (objs.size()) {
        QObject::connect(objs[0], SIGNAL(destroyed(QObject*)),
                         &sh->watcher, SLOT(objectDestroyed(QObject*)));
    }
    return QSObject( this, sh );
}

QSWrapperShared* QSWrapperClass::createShared( QObject *o ) const
{
    Q_ASSERT( o );
    QSWrapperShared *shared = new QSWrapperShared( this );

    bool b = interpreter()->queryDispatchObjects( o, shared->objects );
    Q_ASSERT( b );

    return shared;
}

QObject *QSWrapperClass::object(const QSObject *obj)
{
    Q_ASSERT(obj);
    Q_ASSERT(obj->objectType());
    Q_ASSERT(obj->objectType()->name() == QString::fromLatin1("QObject"));
    Q_ASSERT(((QSWrapperClass*)obj->objectType())->objectVector(obj).size() > 0);
    return ((QSWrapperClass*)obj->objectType())->objectVector(obj).at(0);
}


class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static QCString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};


QSObject QSObjectConstructor::cast(const QSList &) const
{
    env()->throwError(QString::fromLatin1("Unable to perform cast to user defined object type '") + cname + QString::fromLatin1("'"));
    return QSObject();
}

void QSFactoryObjectProxy::finalize()
{
    /* We don't need to destroy the shared data, since it is registered
       in the environment in QuickInterpreter::wrap(), and will be deleted
       by env shortly after env has called finalize on all classes */
    sinst.invalidate();
}


bool QSFactoryObjectProxy::member( const QSObject *, const QString &n,
                                   QSMember *m ) const
{
    return sinst.isValid()
        ? sinst.objectType()->member( &sinst, n, m )
        : constr->member( 0, n, m );
}

QSMemberMap QSFactoryObjectProxy::members( const QSObject * ) const
{
    return sinst.objectType()->members( &sinst );
}

QSObject QSFactoryObjectProxy::fetchValue( const QSObject *,
                                           const QSMember &mem ) const
{
    return sinst.isValid()
        ? sinst.objectType()->fetchValue( &sinst, mem )
        : mem.owner()->fetchValue( 0, mem );
}

void QSFactoryObjectProxy::write( QSObject *, const QSMember &mem,
                                  const QSObject &val ) const
{
    sinst.objectType()->write( &( ( (QSFactoryObjectProxy*) this )->sinst ),
                               mem, val );
}

QSObject QSFactoryObjectProxy::invoke( QSObject *, const QSMember &mem ) const
{
    return ( (QSWrapperClass*) sinst.objectType() )
        ->invoke( &( ( (QSFactoryObjectProxy*) this )->sinst ), mem );
}

QSObjectConstructor::QSObjectConstructor( QSClass *b,
                                          const QString &className, Type t )
    : QSClass( b ), QuickEnvClass( b->env() ), cname( className ), type( t )
{
}

QSObject QSObjectConstructor::construct( const QSList &args ) const
{
    if ( type == Class ) {
        QPtrVector<QObject> result;
        QValueList<QVariant> vargs;
        for ( int i = 0; i < args.size(); i++ )
            vargs.append( args[i].toVariant( QVariant::Invalid ) );
        bool success = interpreter()->construct( cname, vargs, result );
        if ( success && result.size() >= 1 )
            success = interpreter()->queryDispatchObjects( result[0], result );
        if ( result.size() == 0 || !result[0] )
            return env()->throwError( GeneralError,
                                      QString::fromLatin1("Could not construct ") + cname +
                                      QString::fromLatin1(". Invalid constructor arguments were specified") );

         // ### this seems wrong... Why would we ever get a dispatch
         // ### object here... They should all be QObject's coming
         // ### from the factories... so the code below should always
         // ### run
        if (!result[0]->inherits("QuickPtrDispatchObject")) {
            QSObject obj = interpreter()->wrapperClass()->wrap(result);
            if (m_proxy) {
                ((QSWrapperShared *) obj.shVal())->creator = m_proxy;
            }
            return obj;
        }
        return interpreter()->pointerClass()->
            wrapPointer( (QuickPtrDispatchObject*)result[ 0 ] );
    }
#if 0
    else if ( type == Form ) {
        QWidget *parentWidget = 0;
        const char *name = 0;
        if ( args.size() > 0 ) {
            if ( args[0].isA( "QObject" ) ) {
                QSObject lo = args[0];
                QObject *o = interpreter()->wrapperClass()->
                             objectVector( &lo )[0];
                if ( o->isWidgetType() )
                    parentWidget = (QWidget*)o;
            }
            if ( args.size() > 1 && args[1].isString() ) {
                name = qstrdup( args[1].toString().latin1() );
            }
        }

        for ( QMap<QWidget*, QString>::ConstIterator it = qwf_forms->begin(); it != qwf_forms->end(); ++it ) {
            if ( cname == QString( it.key()->name() ) ) {
                QPtrVector<QObject> result;
                qwf_language = new QString( "Qt Script" );
                QWidget *w = QWidgetFactory::create( *it, 0, parentWidget, name );
                delete qwf_language;
                qwf_language = 0;
                int idx = result.size();
                result.resize( idx + 1 );
                result.insert( idx, w );
                return interpreter()->wrapperClass()->wrap( result );
            }
        }
    }
#endif
    return createUndefined();
}


bool QSObjectConstructor::member( const QSObject *,
                                  const QString &n,
                                  QSMember *m ) const
{
    QMetaObject *metaObj = QMetaObject::metaObject( cname.latin1() );
    if( !metaObj )
        metaObj = QMetaObject::metaObject( QString(QString::fromLatin1("QS") + cname).latin1() );

    if( !metaObj )
        return FALSE;

    const char *matchName = n.latin1();
    QStrList names = metaObj->enumeratorNames( FALSE );
    uint enumCount = metaObj->numEnumerators();
    const QMetaEnum *metaEnum;
    for( uint i=0; i<enumCount; ++i ) {
        metaEnum = metaObj->enumerator( names.at( i ), FALSE );
        Q_ASSERT( metaEnum );
        for( uint j=0; j<metaEnum->count; ++j ) {
            if( qstrcmp( metaEnum->items[j].key, matchName ) == 0 ) {
                m->setName( n );
                m->setOwner( this );
                m->setType( QSMember::Custom );
                m->setWritable( FALSE );
                m->setIndex( metaEnum->items[j].value );
                m->setStatic( TRUE );
                return TRUE;
            }
        }
    }

    return FALSE;
}


QSObject QSObjectConstructor::fetchValue( const QSObject *,
                                          const QSMember &mem ) const
{
    Q_ASSERT( mem.type() == QSMember::Custom );
    Q_ASSERT( mem.owner() == this );
    Q_ASSERT( mem.isStatic() );

    return createNumber( mem.index() );
}


/////////////////// pointer wrapper /////////////////////////////

QSObject QSPointerClass::wrapPointer( QuickPtrDispatchObject *ptr )
{
    QSWrapperShared *sh = new QSWrapperShared( this );
    sh->objects.resize( 1 );
    sh->objects.insert( 0, ptr );
    return QSObject( this, sh );
}


QSObject QSPointerClass::wrapPointer( const char *n, void *p )
{
    QSWrapperShared *sh = new QSWrapperShared( this );
    sh->objects.resize( 1 );
    sh->objects.insert( 0, new QuickPtrDispatchObject( n, p ) );
    interpreter()->queryDispatchObjects( n, p, sh->objects );
    return QSObject( this, sh );
}

QString QSPointerClass::toString( const QSObject *obj ) const
{
    const QPtrVector<QObject> &objects = objectVector( obj );
    if ( objects.isEmpty() )
        return QString::fromLatin1("[object Pointer]");
    return QSWrapperClass::toString( obj );
}

double QSPointerClass::toNumber( const QSObject *o ) const
{
    return (ulong)pointer( o );
}

bool QSPointerClass::toBoolean( const QSObject *o ) const
{
    return pointer( o ) != 0;
}

QVariant QSPointerClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    // ### ugly
    QString s = QString::fromLatin1( "Pointer:%1:Pointer" ).arg( (ulong)pointer( obj ) );
    return QVariant( s );
}

QString QSPointerClass::debugString( const QSObject *obj ) const
{
    const QPtrVector<QObject> &objects = objectVector( obj );
    if ( objects.isEmpty() )
        return QString::fromLatin1("{}:Pointer");
    else
        return QSWrapperClass::debugString( obj );
}

void* QSPointerClass::pointer( const QSObject *obj ) const
{
    const QPtrVector<QObject> &objects = objectVector( obj );
    if ( objects.isEmpty() )
        return 0;
    return ((QuickPtrDispatchObject*)objects[ 0 ])->pointer();
}

const char* QSPointerClass::pointerType( const QSObject *obj ) const
{
    const QPtrVector<QObject> &objects = objectVector( obj );
    if ( objects.isEmpty() )
        return "void";
    return objects[ 0 ]->name();
}

/////////////////////////////////////////////////////////////////////////////

QSVariantShared::QSVariantShared( const QVariant &v, const QMetaProperty *m )
    : variant( v ),
      mp( m )
{
    native = (
              v.type() == QVariant::Bool ||
              v.type() == QVariant::ByteArray ||
              v.type() == QVariant::Color ||
              v.type() == QVariant::ColorGroup ||
              v.type() == QVariant::CString ||
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

QSVariantShared *QSVariantClass::shared( const QSObject *obj ) const
{
    Q_ASSERT( obj->objectType() == this );
    return (QSVariantShared*)obj->shVal();
}

QVariant *QSVariantClass::variant( const QSObject *obj ) const
{
    return &shared( obj )->variant;
}

QSObject QSVariantClass::construct( const QSList & ) const
{
    // there's no "Variant" object available to the user so
    // this function should never get called. Theoretically.
    qWarning( "QSVariantClass::construct( const QSList& ) called" );
    return construct( QVariant(), 0 );
}

QSObject QSVariantClass::construct( const QVariant &v,
                                    const QMetaProperty *m ) const
{
    return QSObject( this, new QSVariantShared( v, m ) );
}

QVariant QSVariantClass::toVariant( const QSObject *obj, QVariant::Type ) const
{
    return shared( obj )->variant;
}


/////////////////////////////////////////////////////////////////////////////

QuickScriptVariant::QuickScriptVariant( QuickInterpreter *ip,
                                        const QVariant &v,
                                        const QMetaProperty *m )
    : QSObject( ip->variantClass(), new QSVariantShared( v, m ) )
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

const QMetaProperty *QuickScriptVariant::metaProperty() const
{
    return shared()->mp;
}

static inline QSObject stringlist_to_qsarray(const QStringList &lst, QSEnv *env)
{
    QSArray array( env );
    int i = 0;
    for ( QStringList::ConstIterator it = lst.begin();
          it != lst.end(); ++it )
        array.put( QString::number( i++ ), env->createString( *it ) );
    return array;
}

void QSVariantShared::createObject( QuickInterpreter *ip )
{
    // ##### complete for other types
    const QVariant& var = variant;
    QSEnv *env = ip->env();
    switch ( var.type() ) {
    case QVariant::ByteArray:
        iobj = ip->byteArrayClass()->construct( var.toByteArray() );
        break;
    case QVariant::Pixmap:
        iobj = ip->pixmapClass()->construct( var.toPixmap() );
        break;
    case QVariant::Date: {
        QSList l;
        l.append( env->createNumber( var.toDate().year() ) );
        l.append( env->createNumber( var.toDate().month() + 1 ) );
        l.append( env->createNumber( var.toDate().day() ) );
        iobj = ip->env()->dateClass()->construct( l );
        break;
    }
    case QVariant::Time: {
        QSList l;
        l.append( env->createNumber( QDate::currentDate().year() ) );
        l.append( env->createNumber( QDate::currentDate().month() + 1 ) );
        l.append( env->createNumber( QDate::currentDate().day() ) );
        l.append( env->createNumber( var.toTime().hour() ) );
        l.append( env->createNumber( var.toTime().minute() ) );
        l.append( env->createNumber( var.toTime().second() ) );
        l.append( env->createNumber( var.toTime().msec() ) );
        iobj = ip->env()->dateClass()->construct( l );
        break;
    }
    case QVariant::DateTime: {
        QSList l;
        l.append( env->createNumber( var.toDateTime().date().year() ) );
        l.append( env->createNumber( var.toDateTime().date().month() + 1 ) );
        l.append( env->createNumber( var.toDateTime().date().day() ) );
        l.append( env->createNumber( var.toDateTime().time().hour() ) );
        l.append( env->createNumber( var.toDateTime().time().minute() ) );
        l.append( env->createNumber( var.toDateTime().time().second() ) );
        l.append( env->createNumber( var.toDateTime().time().msec() ) );
        iobj = ip->env()->dateClass()->construct( l );
        break;
    }
    case QVariant::Color:
        iobj = ip->colorClass()->construct( var.toColor() );
        break;
    case QVariant::Font:
        iobj = ip->fontClass()->construct( var.toFont() );
        break;
    case QVariant::Point:
        iobj = ip->pointClass()->construct( var.toPoint() );
        break;
    case QVariant::Size:
        iobj = ip->sizeClass()->construct( var.toSize() );
        break;
    case QVariant::Rect:
        iobj = ip->rectClass()->construct( var.toRect() );
        break;
    case QVariant::StringList: {
        iobj = stringlist_to_qsarray(var.toStringList(), env);
        break;
    }
    case QVariant::List: {
        QValueList<QVariant> lst = var.toList();
        QSArray array( env );
        int i = 0;
        for ( QValueList<QVariant>::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
            QuickScriptVariant qsvar =  QuickScriptVariant( ip, *it, 0 );
            array.put(QString::number( i++ ), qsvar.isNative() ? qsvar.toNative() : qsvar);
        }
        iobj = array;
        break;
    }
    case QVariant::Map: {
        QMap<QString, QVariant> map = var.toMap();
        QSArray array( env );
        for ( QMap<QString, QVariant>::ConstIterator it = map.begin(); it != map.end(); ++it ) {
            QuickScriptVariant qsvar =  QuickScriptVariant( ip, *it, 0 );
            array.put( it.key(), qsvar.isNative() ? qsvar.toNative() : qsvar);
        }
        iobj = array;
        break;
    }
    case QVariant::String:
    case QVariant::CString:
        iobj = env->createString( var.toString() );
        break;
    default:
        iobj = QSObject(); // null !
        break;
    }
}

QSObject QSVariantClass::toPrimitive( const QSObject *obj,
                                      const QSClass * ) const
{
    QSObject res;
    QVariant *var = variant( obj );
    QSVariantShared *sh = shared( obj );
    switch ( var->type() ) {
    case QVariant::Bool:
        return createBoolean( var->toBool() );
    case QVariant::Int:
        return createNumber( var->toInt() );
    case QVariant::UInt:
        return createNumber( var->toUInt() );
    case QVariant::Double:
        return createNumber( var->toDouble() );
    case QVariant::CString:
    case QVariant::String:
        return createString( var->toString() );
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
        sh->createObject( interpreter() );
        return sh->iobj;
    case QVariant::ColorGroup:
        return interpreter()->colorGroupClass()->construct(var->toColorGroup());
    case QVariant::Palette:
        return interpreter()->paletteClass()->construct(var->toPalette());
    default:
        // fallback for cases not handled above
        return createString( toString( obj ) );
    }
}

bool QSVariantClass::toBoolean( const QSObject *obj ) const
{
    QVariant *var = variant( obj );
    switch ( var->type() ) {
    case QVariant::CString:
        return !var->toCString().isEmpty();
    case QVariant::String:
        return !var->toString().isEmpty();
    default:
        return var->toBool();
    }
}

double QSVariantClass::toNumber( const QSObject *obj ) const
{
    QVariant *var = variant( obj );
    switch ( var->type() ) {
    case QVariant::Int:
    case QVariant::Bool:
        return double( var->toInt() );
    case QVariant::UInt:
        return double( var->toUInt() );
    case QVariant::Double:
        return double( var->toDouble() );
    case QVariant::String:
        return QString( var->toString() ).toDouble();
    case QVariant::Color:
        return double( var->toColor().rgb() );
    case QVariant::Date:
        // QDate -> UTC conversion
        return -1000.0 * QDateTime( var->toDate() ).
            secsTo( QDate( 1970, 1, 1 ) );
    default:
        //qWarning( "QuickScriptVariant::toNumber: unhandled QVariant type." );
        return NaN;
    }
}

QString QSVariantClass::toString( const QSObject *obj ) const
{
    QVariant *var = variant( obj );
    QString str;
    switch ( var->type() ) {
    case QVariant::Bool:
        return var->toBool() ? QString::fromLatin1("true") : QString::fromLatin1("false");
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
        return QSString::from( var->toDouble() );
    case QVariant::CString:
    case QVariant::String:
        return var->toString();
    case QVariant::Color:
        return var->toColor().name();
    case QVariant::ByteArray: {
        QString str;
        QByteArray ba = var->toByteArray();
        for ( int i = 0; i < (int)ba.size() ; ++i )
            str += ba.data()[i];
        return str;
    }
    case QVariant::Point: {
        QPoint point = var->toPoint();
        return QString::fromLatin1( "(%1, %2)" ).arg( point.x() ).arg( point.y() );
    }
    case QVariant::Rect: {
        QRect rect = var->toRect();
        return QString::fromLatin1( "(%1, %2, %3, %4)" ).arg( rect.x() ).arg( rect.y() ).
            arg( rect.width() ).arg( rect.height() );
    }
    case QVariant::Size: {
        QSize size = var->toSize();
        return QString::fromLatin1( "(%1, %2)" ).arg( size.width() ).arg( size.height() );
    }
    case QVariant::Date:
        return var->toDate().toString();
    case QVariant::Time:
        return var->toTime().toString();
    case QVariant::Font:
        return var->toFont().toString();
    case QVariant::DateTime:
        return var->toDateTime().toString();
    case QVariant::StringList:
        return var->toStringList().join( QString::fromLatin1(",") );
    case QVariant::List: {
        QValueList<QVariant> lst = var->toList();
        QString str;
        bool first = TRUE;
        for ( QValueList<QVariant>::ConstIterator it = lst.begin();
              it != lst.end(); ++it ) {
            if ( !first )
                str += QString::fromLatin1(",");
            str += (*it).toString();
            first = FALSE;
        }
        return str;
    }
    case QVariant::Map: {
        QMap<QString, QVariant> map = var->toMap();
        QString str;
        bool first = TRUE;
        for ( QMap<QString, QVariant>::ConstIterator it = map.begin();
              it != map.end(); ++it ) {
            if ( !first )
                str += QString::fromLatin1(",");
            str += it.key() + QString::fromLatin1("=") + (*it).toString();
            first = FALSE;
        }
        return str;
    }
    default:
        return QString::fromLatin1("undefined");
    }
}

QString QSVariantClass::debugString( const QSObject *obj ) const
{
    QVariant *var = variant( obj );
    switch ( var->type() ) {
    case QVariant::Bool:
        return var->toBool() ? QString::fromLatin1("true:Boolean") : QString::fromLatin1("false:Boolean");
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
        return QSString::from( var->toDouble() ) + QString::fromLatin1(":Number");
    case QVariant::CString:
    case QVariant::String:
        return var->toString() + QString::fromLatin1(":String");
    case QVariant::Color:
        return var->toColor().name() + QString::fromLatin1(":Color");
    case QVariant::Point: {
        QPoint point = var->toPoint();
        return QString::fromLatin1( "{x=%1:Number,y=%2:Number}:Point" ).
            arg( point.x() ).arg( point.y() );
    };
    case QVariant::Rect: {
        QRect rect = var->toRect();
        return QString::fromLatin1( "{x=%1:Number,y=%2:Number,width=%3:Number,"
                        "height=%4:Number}:Rect" ).arg( rect.x() ).
            arg( rect.y() ). arg( rect.width() ).arg( rect.height() );
    };
    case QVariant::Size: {
        QSize size = var->toSize();
        return QString::fromLatin1( "{width=%1:Number=,height=%2:Number}:Size" ).
            arg( size.width() ).arg( size.height() );
    };
    case QVariant::Date:
        return var->toDate().toString() + QString::fromLatin1(":Date");
    case QVariant::Time:
        return var->toTime().toString() + QString::fromLatin1(":Date");
    case QVariant::DateTime:
        return var->toDateTime().toString() + QString::fromLatin1(":Date");
    case QVariant::Font: {
        QFont font = var->toFont();
        return QString::fromLatin1( "{family=%1:String,pointSize=%2:Number,bold=%3:Boolean,italic=%4:Boolean,underline=%5:Boolean}:Font" ).
            arg( font.family() ).arg( font.pointSize() ).
            arg( QString::number( (uint)font.bold() ) ).
            arg( QString::number( (uint)font.italic() ) ).
            arg( QString::number( (uint)font.underline() ) );
    };
    case QVariant::StringList:
        return var->toStringList().join( QString::fromLatin1(",") )
            + QString::fromLatin1(":StringList");
    case QVariant::List: {
        QValueList<QVariant> lst = var->toList();
        QString str = QString::fromLatin1("{");
        bool first = TRUE;
        for ( QValueList<QVariant>::ConstIterator it = lst.begin();
              it != lst.end(); ++it ) {
            if ( !first )
                str += QString::fromLatin1(",");
            str += (*it).toString();
            str += QString::fromLatin1(":");
            str += QString::fromLatin1((*it).typeName());
            first = FALSE;
        }
        return str + QString::fromLatin1("}:List");
    }
    case QVariant::Map: {
        QMap<QString, QVariant> map = var->toMap();
        QString str = QString::fromLatin1("{");
        bool first = TRUE;
        for ( QMap<QString, QVariant>::ConstIterator it = map.begin();
              it != map.end(); ++it ) {
            if ( !first )
                str += QString::fromLatin1(",");
            str += it.key() + QString::fromLatin1("=") + (*it).toString() + QString::fromLatin1(":") + QString::fromLatin1((*it).typeName());
            first = FALSE;
        }
        return str + QString::fromLatin1("}:Map");
    }
    default:
        // fallback
        return toString( obj );
    }
}

bool QSVariantClass::member( const QSObject *objPtr, const QString &n,
                             QSMember *m ) const
{
    if( !objPtr )
        return FALSE;
    QSVariantShared *sh = shared( objPtr );
    if ( !sh->iobj.isValid() ) {
        sh->createObject( interpreter() );
        if ( !sh->iobj.isValid() )
            return FALSE;
    }
    return sh->iobj.objectType()->member( &sh->iobj, n, m );
}

QSObject QSVariantClass::fetchValue( const QSObject *objPtr,
                                     const QSMember &mem ) const
{
    QSVariantShared *sh = shared( objPtr );
    Q_ASSERT( sh->iobj.isValid() );
    return sh->iobj.objectType()->fetchValue( &sh->iobj, mem );
}

void QSVariantClass::write( QSObject *objPtr, const QSMember &mem,
                            const QSObject &val ) const
{
    QSVariantShared *sh = shared( objPtr );
    Q_ASSERT( sh->iobj.isValid() );
    sh->iobj.objectType()->write( &sh->iobj, mem, val );
    sh->variant = sh->iobj.toVariant( QVariant::Invalid );
}

QSObject QSVariantClass::invoke( QSObject * objPtr, const QSMember &mem ) const
{
    QSVariantShared *sh = shared( objPtr );
    if (sh->iobj.isValid())
        return sh->iobj.invoke( mem, *env()->arguments() );
    else
        return QSSharedClass::invoke(objPtr, mem);
}

/////////////////////////////////////////////////////////////////////////////


class TimerObject : public QObject {
public:
    TimerObject( QObject *parent=0, const char *name=0 ) :
        QObject( parent, name ) { }
    static int setTimer( const QSObject &interval, const QSObject &func );
    static void stopTimer( const QSObject &id );
    static void stopTimers();

protected:
    void timerEvent( QTimerEvent * );

private:
    static TimerObject *timer;
    QMap<int, EventTarget> eventMap;
};

TimerObject *TimerObject::timer = 0;

/**
 * Starts a timer with the specified \a interval that will call
 * a function named \a handler each time it fires.
 * This function returns an identifier for the started timer or -1
 * if the arguments were invalid.
 */

int TimerObject::setTimer( const QSObject &interval, const QSObject &func )
{
    if ( !timer ) {
        timer = new TimerObject();
        // ### install clean up handler
    }

    if ( !interval.isDefined() )
        return -1;
    double iv = interval.toNumber();
    if ( isNaN( iv ) )
        return -1;
    int id = timer->startTimer( int(iv) );
    if ( id == 0 )
        return -1;
    Q_ASSERT( func.isExecutable() );
    EventTarget target;	// slight misuse
    target.eng = QuickInterpreter::fromEnv( interval.env() );
    target.targets << EventTarget::Target( 0, func, QString::fromLatin1("") );
    timer->eventMap.insert( id, target );
    return id;
}

void TimerObject::stopTimer( const QSObject &id )
{
    if ( timer ) {
        double d = id.toNumber();
        if ( !isNaN( d ) ) {
            timer->killTimer( int(d) );
            timer->eventMap.remove( int(d) );
        }
    }
}

void TimerObject::stopTimers()
{
    if ( timer ) {
        timer->killTimers();
        timer->eventMap.clear();
    }
}

// execute the function bound to this timer. passed the id as first argument.
void TimerObject::timerEvent( QTimerEvent *e )
{
    if ( !QuickInterpreter::timersEnabled() )
        return;
    QMap<int, EventTarget>::ConstIterator it = eventMap.find( e->timerId() );
    if ( it == eventMap.end() )
        return;
    QSList args;
    Q_ASSERT( (*it).eng->env() );
    args.append( (*it).eng->env()->createNumber( e->timerId() ) );
    QSObject func = (*it).targets.first().qsctx;
    Q_ASSERT( func.isExecutable() );
    (*it).eng->reinit();
    func.invoke( QSMember(), args );
}

QSApplicationClass::QSApplicationClass( QSClass *b )
    : QSWritableClass( b, AttributeAbstract ), QuickEnvClass( b->env() )
{
    if ( qApp ) {
        QSArray args( b->env() );
        for ( int i = 0; i < qApp->argc(); ++i )
            args.put( QString::number( i ), createString( QString::fromLatin1(qApp->argv()[ i ] )) );
        addStaticVariableMember( QString::fromLatin1("argv"), args, AttributeNone );
    }
}

bool QSApplicationClass::member( const QSObject *o, const QString &n,
                                 QSMember *m ) const
{
    if ( !interpreter()->children().contains( n ) ) // a top level object ?
        return QSWritableClass::member( o, n, m );

    m->setType( QSMember::Custom );
    m->setOwner( this );
    m->setName( n );
    m->setWritable( FALSE );
    return TRUE;
}

QSObject QSApplicationClass::fetchValue( const QSObject *objPtr,
                                         const QSMember &mem ) const
{
    if ( mem.type() != QSMember::Custom )
        return QSWritableClass::fetchValue( objPtr, mem );

    QObjectListIt it( *(interpreter()->topLevelObjects()) );
    QCString n = mem.name().ascii();
    while ( it.current() ) {
        if ( n == (*it)->name() )
            return interpreter()->wrap( *it );
        ++it;
    }
    qWarning( "QSApplicationClass::fetchValue: child widget disappeared" );
    return createUndefined();
}

void QSApplicationClass::exit( QSEnv * )
{
    QApplication::exit(); // ### modify behaviour in devel mode
}

/*!
  Start a timer that will fire in a number of milliseconds specified
  in the first parameter. Upon firing of the timer the function specified
  in the second parameter will be executed with the id of the timer as
  argument. Returns the id of the timer.
*/
QSObject qsStartTimer( QSEnv *env )
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    if (qt_get_application_thread_id() != QThread::currentThread())
        return env->throwError(GeneralError,
                               QString::fromLatin1("Cannot start timer outside of the GUI thread"));
#endif
    QSObject hnd = env->arg( 1 );
    if ( hnd.isFunction() ) {
        int id = TimerObject::setTimer( env->arg( 0 ), hnd );
        return env->createNumber( id );
    }
    QString msg = QString::fromLatin1("Can only install functions as event handler");
    return env->throwError( TypeError, msg );
}

void qsKillTimer( QSEnv *env )
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    if (qt_get_application_thread_id() != QThread::currentThread()) {
        env->throwError(GeneralError,
                        QString::fromLatin1("Cannot start timer outside of the GUI thread"));
        return;
    }
#endif
    Q_UNUSED(env);
    TimerObject::stopTimer( env->arg( 0 ) );
}

void qsKillTimers( QSEnv *env )
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    if (qt_get_application_thread_id() != QThread::currentThread()) {
        env->throwError(GeneralError,
                        QString::fromLatin1("Cannot start timer outside of the GUI thread"));
        return;
    }
#endif
    TimerObject::stopTimers();
}

////////////////////////////////////////////////////////////////////

QSObject uObjectToQS( QuickInterpreter *ip, QUObject *o, const void *extra,
                      QObject *qobj )
{
    if ( QUType::isEqual( o->type, &static_QUType_double ) )
        return ip->env()->createNumber( static_QUType_double.get( o ) );
    else if ( QUType::isEqual( o->type, &static_QUType_int ) )
        return ip->env()->createNumber( static_QUType_int.get( o ) );
    else if ( QUType::isEqual( o->type, &static_QUType_bool ) )
        return ip->env()->createBoolean( static_QUType_bool.get( o ) );
    else if ( QUType::isEqual( o->type, &static_QUType_QString ) )
        return ip->env()->createString( static_QUType_QString.get( o ) );
    else if ( QUType::isEqual( o->type, &static_QUType_charstar ) )
        return ip->env()->createString( QString::fromLatin1(static_QUType_charstar.get( o )) );
    else  if (QUType::isEqual(o->type, &static_QUType_enum))
        return ip->env()->createNumber(static_QUType_enum.get(o));
    else if ( QUType::isEqual( o->type, &static_QUType_QVariant ) ) {
        QVariant var = static_QUType_QVariant.get( o );
        QuickScriptVariant v( ip, var, 0 );
        if ( v.isNative() )
            return v.toNative();
        return ip->variantClass()->construct( var, 0 );
    } else if ( QUType::isEqual( o->type, &static_QUType_varptr ) ) {
        QVariant var;
        var.rawAccess( o->payload.ptr, (QVariant::Type)*(char*)extra, TRUE );
        QuickScriptVariant v( ip, var, 0 );
        if ( v.isNative() )
            return v.toNative();
        return ip->variantClass()->construct( var, 0 );
    } else if ( QUType::isEqual( o->type, &static_QUType_ptr ) ) {
        if ( QMetaObject::hasMetaObject( (char*)extra ) )
            return ip->wrap( (QObject*)static_QUType_ptr.get( o ) );
        else if (qstrcmp((char*)extra, "QRect") == 0)
            return ip->rectClass()->construct(*((QRect*) static_QUType_ptr.get(o)));
        else if (qstrcmp((char*)extra, "QPoint") == 0)
            return ip->pointClass()->construct(*((QPoint*) static_QUType_ptr.get(o)));
        else if (qstrcmp((char*)extra, "QSize") == 0)
            return ip->sizeClass()->construct(*((QSize*) static_QUType_ptr.get(o)));
        else if (qstrcmp((char*)extra, "QColor") == 0)
            return ip->colorClass()->construct(*((QColor*) static_QUType_ptr.get(o)));
        else if (qstrcmp((char*)extra, "QPixmap") == 0)
            return ip->pixmapClass()->construct(*((QPixmap*) static_QUType_ptr.get(o)));
        else if (qstrcmp((char*)extra, "QByteArray") == 0)
            return ip->byteArrayClass()->construct(*((QByteArray*) static_QUType_ptr.get(o)));
        else if (qstrcmp((char*)extra, "QFont") == 0)
            return ip->fontClass()->construct(*((QFont*) static_QUType_ptr.get(o)));
        else if (qstrcmp((char*)extra, "QStringList") == 0)
            return stringlist_to_qsarray(*(QStringList*) static_QUType_ptr.get(o), ip->env());
        else if (qstrcmp((char*)extra, "int") == 0)
            return ip->env()->createNumber( *(int*) static_QUType_ptr.get(o) );
        else if (qstrcmp((char*)extra, "uint") == 0)
            return ip->env()->createNumber( *(uint*) static_QUType_ptr.get(o) );
        else if (qobj != 0) { // Attempt to find it in the current object's namespace
            QCString s = QCString(qobj->className());

            int pos = s.findRev( "::" );
            if ( pos >= 0 ) {
                s = s.left( pos + 2 );
                s += QCString( (char*)extra );
                if ( QMetaObject::hasMetaObject( s ) )
                    return ip->wrap( (QObject*)static_QUType_ptr.get( o ) );
            }
        }
        return ip->pointerClass()->
            wrapPointer((char*)extra, static_QUType_ptr.get( o ) );
    }
    return ip->env()->createUndefined();
}

#define VARIANT ( v.toVariant( QVariant::nameToType( t->desc() ) ) )

bool qsToUObject( QUObject *o, const QSObject &v,
                  QUType *t,
                  const void *extra,
                  QPtrList<qs_ptr_ref> *allocs,
                  qs_method_info *methodInfo )
{
    Q_ASSERT( v.isValid() );
    QuickInterpreter *ip = QuickInterpreter::fromEnv( v.env() );
    if( QUType::isEqual( t, &static_QUType_QString ) ) {
        static_QUType_QString.set( o, VARIANT.toString() );
    } else if( QUType::isEqual( t, &static_QUType_int ) ) {
        static_QUType_int.set( o, VARIANT.toInt() );
    } else if( QUType::isEqual( t, &static_QUType_double ) ) {
        static_QUType_double.set( o, VARIANT.toDouble() );
    } else if( QUType::isEqual( t, &static_QUType_bool ) ) {
        static_QUType_bool.set( o, VARIANT.toBool() );
    } else if( QUType::isEqual( t, &static_QUType_charstar ) ) {
        QString qstr = v.toString();
        char *str = new char[qstr.length()+1];
        qstrcpy( str, qstr.ascii() );
        allocs->append( new qs_ptr_ref( str ) );
        static_QUType_charstar.set( o, str );
    } else if( QUType::isEqual( t, &static_QUType_QVariant ) ) {
        static_QUType_QVariant.set( o, VARIANT );
    } else if (QUType::isEqual(t, &static_QUType_ptr)) {
        if ( v.isA( ip->pointerClass() ) ) {
            const char *ts = ip->pointerClass()->pointerType( &v );
            void *vptr = ip->pointerClass()->pointer( &v );
            if ( qstrcmp( ts, (const char*)extra ) != 0 ) {
                qWarning( "Parameter type mismatch. Needed %s, got %s",
                          (const char*)extra, ts );
                return FALSE;
            }
            static_QUType_ptr.set( o, vptr );
        } else if ( v.isA( ip->wrapperClass() ) ) {
            QObject *qobj = ip->wrapperClass()->objectVector( &v ).at( 0 );
            if ( qobj->inherits( (const char *)extra ) )
                static_QUType_ptr.set( o, qobj );
            else
                return FALSE;
        } else if (qstrcmp((const char*) extra, "float") == 0) {
            float *f = new float( v.toNumber() );
            allocs->append( new qs_ptr_ref( f ) );
            static_QUType_ptr.set( o, f );
        } else if (qstrcmp((const char*) extra, "long") == 0) {
            long *l = new long( (long)v.toNumber() );
            allocs->append( new qs_ptr_ref( l ) );
            static_QUType_ptr.set( o, l );
        } else if (qstrcmp((const char*) extra, "ulong") == 0) {
            ulong *l = new ulong( (ulong)v.toNumber() );
            allocs->append( new qs_ptr_ref( l ) );
            static_QUType_ptr.set( o, l );
        } else if (qstrcmp((const char*) extra, "QVariant") == 0) {
            QVariant *var = new QVariant(v.toVariant(QVariant::Invalid));
            allocs->append(new qs_ptr_ref(var));
            static_QUType_ptr.set(o, var);
        } else if (qstrcmp((const char*) extra, "uchar") == 0) {
            uchar *uc = new uchar( v.isString()
                                   ? (uchar) *v.toString().latin1()
                                   : (uchar) v.toNumber() );
            allocs->append( new qs_ptr_ref(uc) );
            static_QUType_ptr.set(o, uc);
        } else if (qstrcmp((const char*) extra, "char") == 0) {
            char *c = new char( v.isString()
                                ? (uchar) *v.toString().latin1()
                                : (uchar) v.toNumber() );
            allocs->append( new qs_ptr_ref(c) );
            static_QUType_ptr.set(o, c);
        } else if (qstrcmp((const char*) extra, "short") == 0) {
            short *c = new short( (short)v.toNumber() );
            allocs->append( new qs_ptr_ref(c) );
            static_QUType_ptr.set(o, c);
        } else if (qstrcmp((const char*) extra, "ushort") == 0) {
            ushort *c = new ushort( (ushort)v.toNumber() );
            allocs->append( new qs_ptr_ref(c) );
            static_QUType_ptr.set(o, c);
        } else if (qstrcmp((const char*) extra, "QValueList<int>") == 0 && v.isA("Array")) {
            QValueList<int> *intList = new QValueList<int>;
            int len = QSArrayClass::length(&v);
            for ( int i=0; i<len; ++i )
                intList->append((int) v.get( QString::number( i ) ).toNumber());
            allocs->append( new qs_ptr_ref(intList) );
            static_QUType_ptr.set(o, intList);
        } else if (qstrcmp((const char*) extra, "QObjectList") == 0 && v.isA("Array")) {
            QObjectList *objList = new QObjectList;
            int len = QSArrayClass::length(&v);
            for ( int i=0; i<len; ++i ) {
                QSObject tmp = v.get( QString::number( i ) );
                if ( tmp.isA( ip->wrapperClass() ) )
                    objList->append(((QSWrapperClass*)tmp.objectType())->objectVector(&tmp).at(0));
            }
            allocs->append( new qs_ptr_ref(objList) );
            static_QUType_ptr.set(o, objList);
        } else if (v.isUndefined() && !methodInfo->isCurrentReference()) {
            static_QUType_ptr.set(o, 0);
        }
    } else if( QUType::isEqual( t, &static_QUType_enum ) ) {
        static_QUType_enum.set( o, int( v.toNumber() ) );
    } else if(QUType::isEqual(t, &static_QUType_varptr)) {
        if ((int)(*((char*)extra)) == QVariant::UInt) {
            uint *ui = new uint((uint)v.toNumber());
            allocs->append(new qs_ptr_ref(ui));
            static_QUType_varptr.set(o, ui);
        } else if ((int)(*((char*)extra)) == QVariant::Palette
                   && v.objectType()->name() == QString::fromLatin1("Palette")) {
            static_QUType_varptr.set(o, QSPaletteClass::palette(&v));
        } else if ((int)(*((char*)extra)) == QVariant::Font
                   && v.objectType()->name() == QString::fromLatin1("Font")) {
            static_QUType_varptr.set(o, ((QSFontClass *)v.objectType())->font(&v));
        } else if ((int)(*((char*)extra)) == QVariant::Color
                   && v.objectType()->name() == QString::fromLatin1("Color")) {
            static_QUType_varptr.set(o, ((QSColorClass *)v.objectType())->color(&v));
        } else if ((int)(*((char*)extra)) == QVariant::ColorGroup
                   && v.objectType()->name() == QString::fromLatin1("ColorGroup")) {
            static_QUType_varptr.set(o, QSColorGroupClass::colorGroup(&v));
        } else if ((int)(*((char*)extra)) == QVariant::List && v.isA("Array")) {
            QValueList<QVariant> *varList = new QValueList<QVariant>;
            int len = QSArrayClass::length(&v);
            for ( int i=0; i<len; ++i )
                varList->append(v.get(QString::number(i)).toVariant(QVariant::Invalid));
            allocs->append( new qs_ptr_ref(varList) );
            static_QUType_ptr.set(o, varList);
        } else if ((int)(*((char*)extra)) == QVariant::StringList && v.isA("Array")) {
            QStringList *stringList = new QStringList;
            int len = QSArrayClass::length(&v);
            for ( int i=0; i<len; ++i )
                stringList->append(v.get(QString::number(i)).toString());
            allocs->append( new qs_ptr_ref(stringList) );
            static_QUType_ptr.set(o, stringList);
        }
        else if (v.isUndefined() && !methodInfo->isCurrentReference()) {
            static_QUType_varptr.set(o, 0);
        }

    }
    return QUType::check( o, t );
}

////////////////////////////////////////////////////////////////////

QuickScriptReceiver::QuickScriptReceiver( QObject *o )
    : qobj( o ), handler( 0 )
{
}

QuickScriptReceiver::~QuickScriptReceiver()
{
    delete handler;
}

void QuickScriptReceiver::setEventHandler( QuickInterpreter *ip, int id,
                                           QObject *ctx, const QString &func, QSObject qsctx )
{
    if ( !handler )
        handler = new QuickScriptEventMap;
    if ( handler->find( id ) == handler->end() )
        QObject::connectInternal( qobj, id, this, QSLOT_CODE, id );
    (*handler)[id].eng = ip;
    (*handler)[id].targets << EventTarget::Target( ctx, qsctx, func );
}

void QuickScriptReceiver::removeEventHandler( int id, QObject *ctx, const QString &func, QSObject qsctx )
{
    if ( !handler )
        return;
    if ( handler->find( id ) == handler->end() )
        return;
    if ( (*handler)[id].targets.count() == 1 )
        QObject::disconnectInternal( qobj, id, this, QSLOT_CODE, id );
    QValueList<EventTarget::Target>::Iterator it = (*handler)[id].targets.begin();
    while ( it != (*handler)[id].targets.end() ) {
        QValueList<EventTarget::Target>::Iterator it2 = it;
        ++it;
        if ( (*it2).func == func &&
             ( (*it2).ctx == ctx || (*it2).qsctx.equals( qsctx ) ) )
            (*handler)[id].targets.remove( it2 );
    }
    if ( (*handler)[id].targets.count() == 0 )
        (*handler).remove( id );
}

bool QuickScriptReceiver::qt_invoke( int id, QUObject *o )
{
    const QMetaData* qmd = qobj->metaObject()->signal( id, TRUE );
    QuickMetaData md = QuickMetaData( qmd, id );
    QSList args;
    QuickScriptEventMap::Iterator it = handler->find( id );
    if ( it == handler->end() )
        return FALSE;
    const QUParameter *param = md.method->parameters;
    QuickInterpreter *ip = (*it).eng;
    // 0th element contains return value
    for ( int i = 1; i < md.method->count+1; i++, param++ )
        args.append( uObjectToQS( ip, &o[ i ], param->typeExtra, qobj ) );
    for ( QValueList<EventTarget::Target>::ConstIterator sit = (*it).targets.begin(); sit != (*it).targets.end(); ++sit ) {
        if ( (*sit).ctx )
            ip->call( (*sit).ctx, (*sit).func, args );
        else
            ip->call( (*sit).qsctx, (*sit).func, args );
    }

    return TRUE;
}


void QuickScriptReceiver::invalidate()
{
    for( QuickScriptEventMap::Iterator it = handler->begin();
         it != handler->end(); ++it ) {
        for( QValueList<EventTarget::Target>::Iterator targets = (*it).targets.begin();
             targets != (*it).targets.end(); ++targets ) {
            (*targets).qsctx.invalidate();
        }
    }
}

void stopAllTimers()
{
    TimerObject::stopTimers();
}

QSUserData::~QSUserData()
{
    if ( dat ) {
        QSWrapperShared *tmp = dat;
        tmp->invalidate(); // sets our dat to 0
        if (tmp->deref())
            delete tmp;
    }
}
