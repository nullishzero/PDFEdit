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

#ifndef QSOBJECT_H
#define QSOBJECT_H

#include "qsglobal.h"
#include "../kernel/dlldefs.h"
#include <qstring.h>
#include <qvariant.h>

class QSList;
class QSClass;
class QSBooleanClass;
class QSForInNode;
class QSEngine;
class QSEnv;

/**
   * Error codes.
   */
enum ErrorType { NoError = 0,
		 GeneralError,
		 EvalError,
		 RangeError,
		 ReferenceError,
		 SyntaxError,
		 TypeError,
		 URIError,
		 ThrowError };

enum ValueType {
    TypeValue,
    TypeShared,
    TypeClass,
    TypeOther
};

// Reggie: This is a really ugly hack to get QSA link on windows
double QUICKCORE_EXPORT NaN();
double QUICKCORE_EXPORT Inf();
#define NaN NaN()
#define Inf Inf()

// forward declarations
class Imp;
class QSShared;
struct QSProperty;
typedef QMap<QString, QSProperty> QSPropertyMap;
class PropList;
class QSMember;

union QUICKCORE_EXPORT Value {
    int i;
    double d;
    bool b;
    QStringData *str;
    QSShared *sh;
    QSClass *cl;
    void *other;
};

/**
   * @short Main base class for every Qt Script object.
   */


class QSElementNode;
class QSObject;

struct QSFakeQString{ QStringData* d;};
inline QStringData *qsStringData( const QString &s ) { return ((QSFakeQString*)&s)->d; }

class QUICKCORE_EXPORT QSObject {
    friend class QSStringClass;
    friend class QSEnv;
public:
    /**
     * Constructor.
     */
    QSObject();
    QSObject( const QSClass *c );
    QSObject( const QSClass *c, QSShared *s );
    /**
     * Copy constructor.
     */
    QSObject( const QSObject & );

    /*
     * Assignment operator
     */
    QSObject& operator=( const QSObject & );

    bool operator==( const QSObject &a ) const;

    /**
     * Destructor.
     */
    ~QSObject();
    bool isValid() const;
    void invalidate() { clss = 0; }
    /**
     * @return True if this objects is of any other value than Undefined.
     */
    bool isDefined() const;
    /**
     * @return the type of the object. One of the @ref Type enums.
     */
    QString typeName() const;

    ValueType valueType() const;

    /*! Return the object's most specific type
     */
    inline const QSClass *objectType() const;
    void setType( QSClass *c );

    bool isA( const QSClass *c ) const;
    /**
     * Check whether object is of a certain type. Allows checking of
     * host objects, too.
     * @param type name (Number, Boolean etc.)
     */
    bool isA( const char *s ) const;

    bool isObject() const;
    bool isNull() const;
    bool isUndefined() const;
    bool isNumber() const;
    bool isString() const;
    bool isBoolean() const;
    bool isFunction() const;
    bool isPrimitive() const;

    bool equals( const QSObject &other ) const;
    bool strictEquals( const QSObject &other ) const;
    QSCompareResult compareTo( const QSObject &other ) const;

    /*!
     * Returns the environment this object belongs to.
     */

    QSEnv *env() const;
    /**
     * Examine the inheritance structure of this object.
     * @param t Name of the base class.
     * @return True if object is of type t or a derived from such a type.
     */
    //     bool derivedFrom(const char *s) const;

    /**
     * @return Conversion to primitive type (Undefined, Boolean, Number
     * or String)
     * @param preferred Optional hint. Either StringType or NumberType.
     */
    QSObject toPrimitive( const QSClass *preferred=0 ) const; // ECMA 9.1
    /**
     * @return Conversion to Boolean type.
     */
    bool toBoolean() const; // ECMA 9.2
    /**
     * @return Conversion to Number type.
     */
    double toNumber() const; // ECMA 9.3
    QVariant toVariant( QVariant::Type t ) const;
    /**
     * @return Conversion to double. 0.0 if conversion failed.
     */
    double round() const;
    /**
     * @return Conversion to Number type containing an integer value.
     */
    int toInteger() const; // ECMA 9.4
    /**
     * @return Conversion to signed integer value.
     */
    int toInt32() const; // ECMA 9.5
    /**
     * @return Conversion to unsigned integer value.
     */
    unsigned int toUInt32() const; // ECMA 9.6
    /**
     * @return Conversion to unsigned short value.
     */
    unsigned short toUInt16() const; // ECMA 9.7
    /**
     * @return Conversion to String type.
     */
    QString toString() const; // ECMA 9.8

    /*
      Gets a qualified name in this object. E.g. if name is
      a.b.c, c is returned based on this.a.b.c
    */
    QSObject getQualified(const QString &name) const;

    // Properties
    /**
     * The internal [[Get]] method.
     * @return The value of property p.
     */
    QSObject get( const QString &p ) const;
    /**
     * The internal [[HasProperty]] method.
     * @param p Property name.
     * @return Boolean value indicating whether the object already has a
     * member with the given name p.
     */
    bool hasProperty( const QString &p ) const;
    void put( const QString &p, const QSObject& v );
    /**
     * The internal [[Delete]] method. Removes the specified property from
     * the object.
     * @param p Property name.
     * @return True if the property was deleted successfully or didn't exist
     * in the first place. False if the DontDelete attribute was set.
     */
    bool deleteProperty( const QString &p );
    /**
     * Convenience function for adding a Number property constructed from
     * a double value.
     */
    void put( const QString &p, double d );
    /**
     * Convenience function for adding a Number property constructed from
     * an integer value.
     */
    void put( const QString &p, int i );
    /**
     * Convenience function for adding a Number property constructed from
     * an unsigned integer value.
     */
    void put( const QString &p, unsigned int u );
    bool isExecutable() const;
    /**
     * Execute function implemented via the @ref Function::execute() method.
     *
     * Note: check availability via @ref implementsCall() beforehand.
     * @param thisV Object serving as the 'this' value.
     * @param args Pointer to the list of arguments or null.
     * @return Result of the function call.
     */
    QSObject executeCall( QSObject *thisV,
			  const QSList *args );

    QSObject execute( const QSList &args );

    QSObject invoke( const QSMember &mem, const QSList &args );

    QSObject fetchValue( const QSMember &mem );
    void write( const QSMember &mem, const QSObject &val );

    void mark();

    const QSClass * resolveMember( const QString &name, QSMember *mem, const QSClass *owner=0, int *offset=0 ) const;

#ifdef QSDEBUG_MEM
    /**
     * @internal
     */
    static int count;
#endif
    int iVal() const { return val.i; }
    void setVal( int v ) { val.i = v; }

    double dVal() const { return val.d; }
    void setVal( double v ) { val.d = v; }

    bool bVal() const { return val.b; }
    void setVal( bool v ) { val.b = v; }

    QString sVal() const;
    void setVal( const QString &v ) { val.str = qsStringData(v); val.str->ref(); }

    void *oVal() const { return val.other; }
    void setVal( void *v ) { val.other = v; }

    QSShared *shVal() const { return val.sh; }
    void setVal( QSShared *s );

    QString debugString() const;
protected:
    const QSClass *clss;

private:
#if QS_DEBUG_MEM >= 1
    void checkShared() const;
#endif

    Value val;
}; // end of QSObject

inline QString QSObject::sVal() const {
    return *(QString*)( &val.str );
}

inline void QSObject::setVal( QSShared *s )
{
#if QS_DEBUG_MEM >= 1
    checkShared();
#endif
    val.sh = s;
}

class QUICKCORE_EXPORT QSShared : public QShared {
    friend class QSEnv;
public:
    QSShared() : next( 0 ), prev( 0 ) { }
    virtual ~QSShared() { }
    virtual void invalidate() { }
    bool isConnected() const { return next!=0 || prev!=0; }

private:
    QSShared *next;
    QSShared *prev;
};

class QUICKCORE_EXPORT QSWritable : public QSShared {
public:
    QSWritable();
    ~QSWritable();

    void setProperty( const QString &n, const QSProperty &p );
    QSProperty *reference( const QString &n ) const;
    bool hasProperty( const QString &n ) const;
    QSPropertyMap *properties() const { return props; }

    void invalidate();

private:
    QSPropertyMap *props;
};

struct QSProperty {
    QSProperty() { }
    QSProperty( const QSObject &o ) : object( o ) { }
    void invalidate() { object.invalidate(); }
    QSObject object;
};

class QSEngineImp;
/**
 * The Global object represents the global namespace. It holds the native
 * objects like String and functions like eval().
   *
   * It also serves as a container for variables created by the user, i.e.
   * the statement
   * <pre>
   *   var i = 2;
   * </pre>
   * will basically perform a Global::current().put("i", Number(2)); operation.
   *
   * @short Unique global object containing initial native properties.
   */
class QUICKCORE_EXPORT Global : public QSObject {
    friend class QSEngineImp;
public:
private:
    Global( QSEngine *e );
    void init();
    void clear();
    QSEngine *eng;
};

inline const QSClass *QSObject::objectType() const
{
    Q_ASSERT( clss );
    return clss;
}

#endif
