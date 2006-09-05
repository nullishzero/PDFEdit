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

#ifndef QUICKINTERPRETER_H
#define QUICKINTERPRETER_H

#ifndef QT_H
#include <qobject.h>
#include <qobjectlist.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qptrvector.h>
#include <qmap.h>
#include "dlldefs.h"
#include <qptrlist.h>
#endif // QT_H

#include "quickdispatchobject.h"
#include "qsengine.h"
#include <qstypes.h>
#include <qsargument.h>

class QuickDebugger;
class QuickDispatchObjectFactory;
class QSWrapperClass;
class QSPointerClass;
class QSVariantClass;
class QSMapClass;
class QSPointClass;
class QSSizeClass;
class QSRectClass;
class QSColorClass;
class QSFontClass;
class QSByteArrayClass;
class QSPixmapClass;
class QSApplicationClass;
class QSWrapperShared;
class QSPaletteClass;
class QSColorGroupClass;

typedef QPtrList<QSWrapperShared> QSWrapperSharedList;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class QUICKCORE_EXPORT QMap<int, QObject *>;
template class QUICKCORE_EXPORT QMap<QString, QObject*>;
template class QUICKCORE_EXPORT QMap<QString, bool>;
template class QPtrList<QuickDispatchObjectFactory>;
template class QUICKCORE_EXPORT QMap<QString, QPtrList<QuickDispatchObjectFactory> >;
template class QUICKCORE_EXPORT QMap<QObject*, QSWrapperClass*>;
//template class QUICKCORE_EXPORT QStringList;
// MOC_SKIP_END
#endif

#if defined( Q_TEMPLATEDLL ) && QT_VERSION<0x030200
// MOC_SKIP_BEGIN
// template class QUICKCORE_EXPORT QMap<QString, QString>;
template class QUICKCORE_EXPORT QMap<int, QString>;
// MOC_SKIP_END
#endif

class QUICKCORE_EXPORT QuickInterpreter : public QSEngine {
    Q_OBJECT
public:
    QuickInterpreter( bool deb=FALSE );
    virtual ~QuickInterpreter();

    void addTopLevelObject( QObject *o );
    void setTopLevelObjects( QObjectList *l );
    bool hasTopLevelParent( QObject *o );
    QStringList children() const { return kids; }
    QObjectList *topLevelObjects() const { return toplevel; }

    bool checkSyntax( const QString &code );

    QSArgument execute( QObject *_this, const QString &code,
			const QString &scriptName );
    QSArgument call( QObject *ctx, const QString &func, const QSList &args );
    QSArgument call( QSObject ctx, const QString &func, const QSList &args );
    QSArgument call( QObject *ctx, const QString &func, const QSArgumentList &args );

    void clear();
    void stop();

    bool hadError() const;

    void warn( const QString &msg, int l );

    QuickDebugger *debuggerEngine() const { return debugger; }

    struct Property
    {
	Property() {}
	Property( const QString &n, const QString &t ) : name( n ), type( t ) {}
	QString name;
	QString type;
	bool operator==( const Property &p ) const { return name == p.name && type == p.type; }
    };

    static void enableTimers( bool b );
    static bool timersEnabled();

    void reinit();

    void clearSourceIdMap();
    void addSourceId( int id, QObject *o );
    QObject *objectOfSourceId( int id ) const;
    int sourceIdOfObject( QObject *o ) const;

    QString nameOfSourceId( int id ) const;
    int sourceIdOfName( const QString &name ) const;

    bool queryDispatchObjects( QObject *obj, QPtrVector<QObject> &result );
    bool queryDispatchObjects( const QCString &name, void *ptr,
			       QPtrVector<QObject> &result );
    bool construct( const QString &className, const QValueList<QVariant> &args,
		    QPtrVector<QObject> &result );

    // use with caution
    static QuickInterpreter *fromEnv( QSEnv *e );

    static void cleanType( QString &type );
    static void cleanTypeRev( QString &type );

    void emitToggleDebugger( bool enable ) { emit toggleDebugger( enable ); }

    QStringList staticGlobalObjects() const { return staticGlobals; }

    QuickDispatchObjectFactory *dispatchObjectFactory() const { return factory; }

    QSWrapperClass *wrapperClass() const { return wrpClass; }
    QSPointerClass *pointerClass() const { return ptrClass; }
    QSVariantClass *variantClass() const { return varClass; }
    QSMapClass *mapClass() const { return mpClass; }
    QSPointClass *pointClass() const { return pntClass; }
    QSSizeClass *sizeClass() const { return sizClass; }
    QSRectClass *rectClass() const { return rctClass; }
    QSColorClass *colorClass() const { return colClass; }
    QSFontClass *fontClass() const { return fntClass; }
    QSByteArrayClass *byteArrayClass() const { return baClass; }
    QSPixmapClass *pixmapClass() const { return pixClass; }
    QSApplicationClass *applicationClass() const { return appClass; }
    QSColorGroupClass *colorGroupClass() const { return colGrpClass; }
    QSPaletteClass *paletteClass() const { return palClass; }
    QSObject wrap( QObject *o );

    QSArgument convertToArgument( const QSObject &object );

    bool hasFunction( const QString &function ) const;
    bool hasVariable( const QString &variable ) const;
    bool hasClass( const QString &className ) const;

    QStringList functionsOf( QSObject &obj, bool includeSignature,
			     bool includeNative = FALSE,
			     bool includeMemberFunctions = FALSE ) const;
    QStringList classes() const;
    QStringList classesOf( QSObject &obj ) const;
    QStringList variablesOf( QSObject &obj, bool includeStatic = FALSE,
			     bool includeCustom = FALSE,
			     bool includeMemberVariables = FALSE ) const;
    QSObject object( const QString &name ) const;
    const QSClass *classOf( const QSObject &obj ) const;

    int uniqueId() const { return id; }
    int userDataId() const { return usrDataId; }

    void setVariable(QObject *context, const QString &name, const QSArgument &value);
    QSArgument variable(QObject *context, const QString &name);

signals:
    void runtimeError();
    void parseError();
    void warning( const QString &msg, int line );
    void runProject();
    void stopProject();
    void queryGlobalFunctions( QStringList &funcs );
    void toggleDebugger( bool );

private slots:
    void topLevelDestroyed( QObject *o );

private:
    friend class QuickScriptObject;
    friend class QuickScript;
#if defined(Q_DISABLE_COPY)
    QuickInterpreter( const QuickInterpreter & );
    QuickInterpreter& operator=( const QuickInterpreter & );
#endif

    void init();
    void invalidateWrappers();

    uint shuttingDown:1;
    QuickDebugger *debugger;
    QObjectList *toplevel;
    QStringList kids;
    QMap<int, QObject *> sourceIdMap;
    QMap<int, QString> sourceIdNames;
    QStringList staticGlobals;

    QSWrapperClass *wrpClass;
    QSPointerClass *ptrClass;
    QSVariantClass *varClass;
    QSMapClass *mpClass;
    QSPointClass *pntClass;
    QSSizeClass *sizClass;
    QSRectClass *rctClass;
    QSColorClass *colClass;
    QSColorGroupClass *colGrpClass;
    QSPaletteClass *palClass;
    QSFontClass *fntClass;
    QSByteArrayClass *baClass;
    QSPixmapClass *pixClass;
    QSApplicationClass *appClass;
    QSWrapperSharedList *wrapperShared;
    QuickDispatchObjectFactory *factory;

    int id;
    int usrDataId;

};

#endif
