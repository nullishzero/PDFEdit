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

#ifndef QUICKINTERPRETER_H
#define QUICKINTERPRETER_H

#ifndef QT_H
#include <QObject>
#include <QObjectList>
#include <QStringList>
#include <QList>
#include <QVector>
#include <QMap>
#include <QHash>
#endif // QT_H

#include "quickdispatchobject.h"
#include "qsengine.h"
#include <qstypes.h>
#include <QByteArray>

#ifdef QT3_SUPPORT
#include <Q3ValueList>
#endif

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
class QSMetaObject;

typedef QList<QSWrapperShared *> QSWrapperSharedList;

static int qsa_keeparound_grow_factor = 8;

template <typename T> class QSATypeBuffer
{
public:
    QSATypeBuffer() : m_data(0), m_capacity(0), m_size(0) { }
    ~QSATypeBuffer() { delete [] m_data; }

    void reset() { m_size = 0; }

    T *operator() (T t) {
        if (m_size == m_capacity) {
            m_capacity += qsa_keeparound_grow_factor;
            m_data = new T[m_capacity];
        }
        m_data[m_size++] = t;
        return &m_data[m_size-1];
    }

private:
    T *m_data;
    int m_capacity;
    int m_size;
};

struct QSATypeInfo {
    QByteArray name;
    int id;
};

struct QSAMethodSignature {
    QSAMethodSignature(const char *s, const char *r)
        : signature(s), return_type(r) { while (*++signature != '('); }
    const char *signature;              // (paramType1, paramType2, ...)
    const char *return_type;            // returnType
};

uint qHash(const QSAMethodSignature &method);

bool operator == (const QSAMethodSignature &a, const QSAMethodSignature &b);


struct QSASlotCaching
{
    QSASlotCaching() : lockLevel(0), m_skip_reset(16) { };

    QSATypeBuffer<bool> bools;
    QSATypeBuffer<short> shorts;
    QSATypeBuffer<ushort> ushorts;
    QSATypeBuffer<int> ints;
    QSATypeBuffer<uint> uints;
    QSATypeBuffer<long> longs;
    QSATypeBuffer<ulong> ulongs;

    QSATypeBuffer<float> floats;
    QSATypeBuffer<double> doubles;

    QSATypeBuffer<char> chars;
    QSATypeBuffer<uchar> uchars;
    QSATypeBuffer<QChar> qchars;
    QSATypeBuffer<QString> strings;

    QSATypeBuffer<QVariant> variants;
    QSATypeBuffer<QStringList> stringlists;
    QSATypeBuffer<QList<int> > intlists;
    QSATypeBuffer<QObjectList> qobjectlists;
    QSATypeBuffer<void *> voidptrs;
    QSATypeBuffer<QByteArray> bytearrays;

#ifdef QT3_SUPPORT
    QSATypeBuffer<Q3ValueList<int> > q3intlists;
#endif

    void reset() {
        if (--m_skip_reset < 0 && lockLevel == 0) {
            m_skip_reset = 16;

            bools.reset();

            shorts.reset();
            ushorts.reset();
            ints.reset();
            uints.reset();
            longs.reset();
            ulongs.reset();

            floats.reset();
            doubles.reset();

            chars.reset();
            uchars.reset();
            qchars.reset();
            strings.reset();

            variants.reset();
            stringlists.reset();
            intlists.reset();
            qobjectlists.reset();
            voidptrs.reset();
            bytearrays.reset();

#ifdef QT3_SUPPORT
            q3intlists.reset();
#endif
        }
    }

    void lock() { ++lockLevel; }
    void unlock() { --lockLevel; }

    int lockLevel;
    int m_skip_reset;
    QHash<QSAMethodSignature, QList<QSATypeInfo> > signature_cache;
};


struct QSAConnection
{
    QSAConnection() : sender(0), signal(0), function_ref() { }
    QSAConnection(QObject *send, const char *sig, QSObject ref)
        : sender(send), signal(QLatin1String(sig)), function_ref(ref) { }
    QObject *sender;
    QString signal;
    QSObject function_ref;
};

bool operator == (const QSAConnection &a, const QSAConnection &b);



class QuickInterpreter : public QSEngine {
    Q_OBJECT
public:
    QuickInterpreter(bool deb=false);
    virtual ~QuickInterpreter();

    void addTopLevelObject(QObject *o);
    void setTopLevelObjects(QObjectList *l);
    bool hasTopLevelParent(QObject *o);
    QStringList children() const { return kids; }
    QObjectList *topLevelObjects() const { return toplevel; }

    bool checkSyntax(const QString &code);

    QVariant execute(QObject *_this, const QString &code,
			const QString &scriptName);
    QVariant call(QObject *ctx, const QString &func, const QSList &args);
    QVariant call(QSObject ctx, const QString &func, const QSList &args);
    QVariant call(QObject *ctx, const QString &func, const QVariantList &args);

    void clear();
    void stop();

    bool hadError() const;

    void timerEvent(QTimerEvent *e);

    void warn(const QString &msg, int l);

    QuickDebugger *debuggerEngine() const { return debugger; }

    struct Property
    {
	Property() {}
	Property(const QString &n, const QString &t) : name(n), type(t) {}
	QString name;
	QString type;
	bool operator==(const Property &p) const { return name == p.name && type == p.type; }
    };

    static void enableTimers(bool b);
    static bool timersEnabled();

    void reinit();

    void clearSourceIdMap();
    void addSourceId(int id, QObject *o);
    QObject *objectOfSourceId(int id) const;
    int sourceIdOfObject(QObject *o) const;

    QString nameOfSourceId(int id) const;
    int sourceIdOfName(const QString &name) const;

    bool queryDispatchObjects(QObject *obj, QVector<QObject *> *result);
    bool queryDispatchObjects(const QByteArray &name, void *ptr, QVector<QObject *> *result);
    bool construct(const QString &className, const QVariantList &args, QVector<QObject *> *result);

    // use with caution
    static QuickInterpreter *fromEnv(QSEnv *e);

    static void cleanType(QString &type);
    static void cleanTypeRev(QString &type);

    void emitToggleDebugger(bool enable) { emit toggleDebugger(enable); }

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
    QSObject wrap(QObject *o);

    QVariant convertToArgument(const QSObject &object);

    bool hasFunction(const QString &function) const;
    bool hasVariable(const QString &variable) const;
    bool hasClass(const QString &className) const;

    QStringList functionsOf(QSObject &obj, bool includeSignature,
			     bool includeNative = false,
			     bool includeMemberFunctions = false) const;
    QStringList classes() const;
    QStringList classesOf(QSObject &obj) const;
    QStringList variablesOf(QSObject &obj, bool includeStatic = false,
			     bool includeCustom = false,
			     bool includeMemberVariables = false) const;
    QSObject object(const QString &name) const;
    const QSClass *classOf(const QSObject &obj) const;

    int uniqueId() const { return id; }
    int userDataId() const { return usrDataId; }

    void setVariable(QObject *context, const QString &name, const QVariant &value);
    QVariant variable(QObject *context, const QString &name);

    QSASlotCaching *slotCaching() {
        m_slotCaching.reset();
        return &m_slotCaching;
    }

    bool addConnection(QObject *object, const char *signal, const QSObject &function_ptr);
    bool removeConnection(QObject *object, const char *signal, const QSObject &function_ptr);

    QSAConnection scriptSlot(int id) const { return m_script_connections.value(id); }

    QHash<int, QSObject> *timers() { return &m_timers; }

signals:
    void runtimeError();
    void parseError();
    void warning(const QString &msg, int line);
    void runProject();
    void stopProject();
    void queryGlobalFunctions(QStringList &funcs);
    void toggleDebugger(bool);

private slots:
    void topLevelDestroyed(QObject *o);

private:
    friend class QuickScriptObject;
    friend class QuickScript;
#if defined(Q_DISABLE_COPY)
    QuickInterpreter(const QuickInterpreter &);
    QuickInterpreter& operator=(const QuickInterpreter &);
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

    QSASlotCaching m_slotCaching;
    QSMetaObject *m_dynamic_slots;
    QHash<int, QSAConnection> m_script_connections;
    QHash<int, QSObject> m_timers;

};

#endif
