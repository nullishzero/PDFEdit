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

#ifndef QSCLASS_H
#define QSCLASS_H

#include "qsvalues.h"
#include "qstypes.h"
#include "qsenv.h"
#include "qsmember.h"
#include "qsobjectlist.h"

#if defined(Q_TEMPLATEDLL) && (defined(Q_CC_MSVC_NET) || defined(Q_CC_MSVC))
Q_TEMPLATE_EXTERN template class QUICKCORE_EXPORT QValueList<int>;
Q_TEMPLATE_EXTERN template class QUICKCORE_EXPORT QValueList<bool>;
Q_TEMPLATE_EXTERN template class QUICKCORE_EXPORT QMap<QString,QString>;
#endif

class QSClassClass;
class QSClassDefNode;
class QSInstanceData;
class QSFunctionBodyNode;
class QSNode;
class QSNodeList;

/*! Abstract base class for all objects.
 */
class QUICKCORE_EXPORT QSClass {
    friend class QSObject;
public:
    QSClass( QSEnv *e, int a = AttributeNone );
    QSClass( QSClass *b, int a = AttributeNone );
    virtual ~QSClass();
    virtual void clear();

    QSEnv *env() const { return en; }
    QSClass *base() const { return bclass; }
    void setBase(QSClass *base) { bclass = base; }
    virtual QString name() const = 0;
    virtual QString identifier() const { return name(); }
    QSClassClass *asClass() const;
    bool inherits( const QSClass *c ) const;

    QSClass * enclosingClass() const { return encClass; }
    void setEnclosingClass( QSClass * cl ) { encClass = cl; }

    bool isFinal() const { return attrs & AttributeFinal; }
    bool isExecutable() const { return attrs&AttributeExecutable; }
    bool isAbstract() const { return attrs&AttributeAbstract; }

    virtual QSCompareResult compare( const QSObject &a, const QSObject &b ) const;

    virtual void ref( QSObject *o ) const;
    virtual void deref( QSObject *o ) const;
    virtual void mark( QSObject *o ) const;

    virtual bool toBoolean( const QSObject *obj ) const;
    virtual double toNumber( const QSObject *obj ) const;
    virtual QString toString( const QSObject *obj ) const;
    virtual QSObject toPrimitive( const QSObject *obj,
				  const QSClass *preferred=0 ) const;
    virtual QVariant toVariant( const QSObject *obj, QVariant::Type pref ) const;

    virtual QString debugString( const QSObject *obj ) const;

    bool hasProperty( const QSObject *obj, const QString &prop ) const;
    QSObject get( const QSObject *obj, const QString &prop ) const;
    void put( QSObject *obj, const QString &prop, const QSObject &v ) const;

    virtual bool deleteProperty( QSObject *obj, const QSMember &mem ) const;
    virtual bool member( const QSObject *o, const QString &n,
			 QSMember *m ) const;
    virtual QSObject fetchValue( const QSObject *objPtr,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;
    void write( QSObject * objPtr, int index, const QSObject &val ) const;

    void addFunctionMember( const QString &n, QSFunctionBodyNode *f,
			    int attributes );
    int addVariableMember( const QString &n, int attributes );
    void addStaticVariableMember( const QString &n, const QSObject &v,
				  int attr );
    void addMember( const QString &n, const QSMember &m );
    virtual void addMember( const QString &n, const QSMember &m,
			    const QSObject &stVal );
    virtual bool deleteMember( const QString &n );
    void replaceMember( const QString &n, QSMember *m );
    virtual void replaceMember( const QString &n, QSMember *m,
				const QSObject &stVal );
    void setNumVariables( int count ) { numVars = count; }
    int numVariables() const { return numVars; }
    int numStaticVariables() const { return numStaticVars; }

    virtual QSObject construct( const QSList &args ) const;
    virtual QSObject execute( const QSObject *o, QSObject *thisValue, const QSList &args ) const;
    virtual QSObject cast( const QSList &args ) const;

    virtual QSMemberMap * definedMembers() const { return mmap; }
    virtual QSMemberMap members( const QSObject *obj ) const;

    void setStaticMember( int idx, const QSObject &val );
    QSObject staticMember( int idx ) const;

    int attributes() const { return attrs; }

    void throwError( ErrorType e, const QString &msg ) const;

    QSObject createString( const QString &s ) const;
    QSObject createNumber( double d ) const;
    QSObject createBoolean( bool b ) const;
    QSObject createUndefined() const;
    QSObject createNull() const;

    virtual ValueType valueType() const { return TypeOther; }

    virtual void finalize();

protected:
    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;
    virtual QSEqualsResult isStrictEqual( const QSObject &a,
					  const QSObject &b ) const;
    virtual QSObject invoke( QSObject * objPtr, const QSMember &mem ) const;

private:
    void init();
    void removeStaticVar( const QSMember &old );
    void fillMemberVarIndex( QSMember * member );

private:
    QSEnv *en;
    QSClass *bclass;
    QSClass *encClass;
    int attrs;
    QSMemberMap *mmap;
    QSObjectList staticMembers;
    QValueList<int> replacedVars;
    int numVars;
    int numStaticVars;
};

/*! Object class. Has no parents and is the base for Number etc.
 */

class QUICKCORE_EXPORT QSObjectClass : public QSClass {
public:
    QSObjectClass( QSEnv *e );
    QSObjectClass( QSClass *c ) : QSClass( c ) { }
    void init();
    QString name() const { return QString::fromLatin1("Object"); }

    bool toBoolean( const QSObject * ) const { return TRUE; } // ####
    double toNumber( const QSObject * ) const { return NaN; } // ####
    QString toString( const QSObject *obj ) const;

    QSObject construct() const;
    QSObject construct( const QSList &args ) const;
    virtual QSObject cast( const QSList &args ) const;

    static QSObject toStringScript( QSEnv *env );
    static QSObject valueOf( QSEnv *env );
};

class QUICKCORE_EXPORT QSUndefinedClass : public QSClass {
public:
    QSUndefinedClass( QSClass *b ) : QSClass( b ) { }
    QString name() const { return QString::fromLatin1("Undefined"); }

    bool toBoolean( const QSObject *obj ) const;
    double toNumber( const QSObject *obj ) const;
    QString toString( const QSObject * ) const;
    QSObject toPrimitive( const QSObject *obj,
			  const QSClass *preferred=0 ) const;

protected:
    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;
};

class QUICKCORE_EXPORT QSNullClass : public QSClass {
public:
    QSNullClass( QSClass *b ) : QSClass( b ) { }
    QString name() const { return QString::fromLatin1("Null"); }

    bool toBoolean( const QSObject *obj ) const;
    double toNumber( const QSObject *obj ) const;
    QString toString( const QSObject * ) const;
    QSObject toPrimitive( const QSObject *obj,
			  const QSClass *preferred=0 ) const;

protected:
    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;
};

class QUICKCORE_EXPORT QSBooleanClass : public QSClass {
public:
    QSBooleanClass( QSClass *b );
    void init();
    QString name() const { return QString::fromLatin1("Boolean"); }

    bool toBoolean( const QSObject *obj ) const;
    double toNumber( const QSObject *obj ) const;
    QString toString( const QSObject * ) const;
    QSObject toPrimitive( const QSObject *obj,
			  const QSClass *preferred=0 ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;

    QSObject construct( bool b ) const;
    QSObject construct( const QSList &args ) const;
    virtual QSObject cast( const QSList &args ) const;

    virtual ValueType valueType() const { return TypeValue; }

protected:
    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;
};

class QUICKCORE_EXPORT QSNumberClass : public QSClass {
    friend class QSBooleanClass;
public:
    QSNumberClass( QSClass *b );
    void init();
    QString name() const { return QString::fromLatin1("Number"); }

    bool toBoolean( const QSObject *obj ) const;
    double toNumber( const QSObject *obj ) const;
    QString toString( const QSObject * ) const;
    QSObject toPrimitive( const QSObject *obj,
			  const QSClass *preferred=0 ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;

    QSObject construct( const QSList &args ) const;
    virtual QSObject cast( const QSList &args ) const;

    static QSObject toStringScript( QSEnv * );
    static QSObject valueOf( QSEnv * );

    virtual ValueType valueType() const { return TypeValue; }

protected:
    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;
};

class QUICKCORE_EXPORT QSStringClass : public QSClass {
public:
    QSStringClass( QSClass *b );
    void init();
    QString name() const { return QString::fromLatin1("String"); }
    virtual void ref( QSObject *o ) const;
    virtual void deref( QSObject *o ) const;

    virtual QSObject fetchValue( const QSObject *objPtr,
				 const QSMember &mem ) const;

    bool toBoolean( const QSObject *obj ) const;
    double toNumber( const QSObject *obj ) const;
    QString toString( const QSObject * ) const;
    QSObject toPrimitive( const QSObject *obj,
			  const QSClass *preferred=0 ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;

    QSObject construct( const QSList &args ) const;
    virtual QSObject cast( const QSList &args ) const;

    virtual ValueType valueType() const { return TypeValue; }

    static QSObject fromCharCode( QSEnv * );

    static QSObject toStringScript( QSEnv * );
    static QSObject charAt( QSEnv * );
    static QSObject charCodeAt( QSEnv * );
    static QSObject indexOf( QSEnv * );
    static QSObject lastIndexOf( QSEnv * );
    static QSObject match( QSEnv * );
    static QSObject replace( QSEnv * );
    static QSObject split( QSEnv * );
    static QSObject substr( QSEnv * );
    static QSObject toLowerCase( QSEnv * );
    static QSObject toUpperCase( QSEnv * );
    // QString API
    static QSObject isEmpty( QSEnv * );
    static QSObject left( QSEnv * );
    static QSObject mid( QSEnv * );
    static QSObject right( QSEnv * );
    static QSObject find( QSEnv * );
    static QSObject findRev( QSEnv * );
    static QSObject startsWith( QSEnv * );
    static QSObject endsWith( QSEnv * );

    static QSObject argInt( QSEnv * );
    static QSObject argDec( QSEnv * );
    static QSObject argStr( QSEnv * );
    static QSObject arg( QSEnv * );

protected:
    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;
};

class QUICKCORE_EXPORT QSCharacterClass : public QSClass {
public:
    QSCharacterClass( QSClass *b ) : QSClass( b ) { }
    QString name() const { return QString::fromLatin1("Character"); }

    bool toBoolean( const QSObject *obj ) const;
    double toNumber( const QSObject *obj ) const;
    QString toString( const QSObject * ) const;

    virtual ValueType valueType() const { return TypeValue; }
};

class QSSharedClass : public QSClass {
public:
    QSSharedClass( QSEnv *e, int attr = AttributeNone )
	: QSClass( e, attr ) { }
    QSSharedClass( QSClass *b, int attr = AttributeNone )
	: QSClass( b, attr ) { }

    virtual void ref( QSObject *o ) const;
    virtual void deref( QSObject *o ) const;

    virtual ValueType valueType() const { return TypeShared; }
};

class QSClassClass : public QSSharedClass {
public:
    QSClassClass( QSClass *b, int a, const QString &n );
    ~QSClassClass();
    QString name() const { return QString::fromLatin1("Class"); }
    QString identifier() const { return cname; }

    bool toBoolean( const QSObject *obj ) const;
    double toNumber( const QSObject *obj ) const;
    QString toString( const QSObject * ) const;

    QSObject construct( const QSList &args ) const;

    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;

    static QSInstanceData* data( QSObject *obj );
    static const QSInstanceData* data( const QSObject *obj );

    bool hasDefaultConstructor() const { return defaultCtor; }
    void setHasDefaultConstructor( bool d ) { defaultCtor = d; }

    void addMemberInitializer( QSNode * node );
    void addStaticInitializer( QSNode * node );
    void setClassBodyNode( QSFunctionBodyNode *node );
    QSFunctionBodyNode *classBodyNode() const { return bodyNode; }

    void setClassDefNode( QSClassDefNode *node ) { clDefNode = node; }
    QSClassDefNode *classDefNode() const { return clDefNode; }

    void executeBlock( QSEnv *env );

private:
    int initVariables( QSInstanceData *data ) const;

private:
    QString cname;
    uint defaultCtor:1;
    QSNodeList *memberInit;
    QSNodeList *staticInit;
    QSFunctionBodyNode * bodyNode;
    QSClassDefNode *clDefNode;
};

class QUICKCORE_EXPORT QSWritableClass : public QSSharedClass {
public:
    QSWritableClass( QSClass *b, int a = AttributeNone ) : QSSharedClass( b, a ) { }
    void mark( QSObject *o ) const;
    virtual bool member( const QSObject *o, const QString &n,
			 QSMember *m ) const;
    virtual QSObject fetchValue( const QSObject *objPtr,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;

    virtual bool deleteProperty( QSObject *obj, const QSMember &mem ) const;

    virtual QSMemberMap members( const QSObject *obj ) const;

    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;

    QSObject createWritable() const;

    QSPropertyMap *properties( const QSObject *obj ) const;
protected:
    static QSWritable *data( QSObject *obj );
    static const QSWritable *data( const QSObject *obj );
    virtual QSObject invoke( QSObject * objPtr, const QSMember &mem ) const;
};

class QSFuncDeclNode;

class QUICKCORE_EXPORT QSFunctionScopeClass : public QSWritableClass
{
public:
    QSFunctionScopeClass( QSClass *b, QSFuncDeclNode * func );

    QSFunctionScopeClass( QSClass *b )
	: QSWritableClass( b ), numArgs( 0 ), body_node(0) { }

    virtual QString name() const { return QString::fromLatin1("FunctionScope"); }
    virtual QString identifier() const;
    virtual QSObject construct( const QSList & ) const;
    virtual void clear();
    void setNumArguments( int num ) { numArgs = num; }
    int numArguments() const { return numArgs; }

    void setFunctionBodyNode(QSFunctionBodyNode *bn) { body_node = bn; }
    QSFunctionBodyNode *functionBodyNode() const { return body_node; }

private:
    QString ident;
    int numArgs;
    QSFunctionBodyNode *body_node;
};

class QUICKCORE_EXPORT QSEvalScopeClass: public QSWritableClass
{
public:
    QSEvalScopeClass( QSClass *b ) : QSWritableClass( b ) { }
    virtual QString name() const { return QString::fromLatin1("EvalScope"); }
    virtual QSObject construct( const QSList & ) const;
};

class QUICKCORE_EXPORT QSBlockScopeClass: public QSSharedClass
{
public:
    QSBlockScopeClass( QSEnv *e ) : QSSharedClass( e ) { }
    void activateScope() const;
    void deactivateScope() const;
    virtual QString name() const { return QString::fromLatin1("BlockScope"); }
    virtual QSMemberMap members( const QSObject *obj ) const;


};

// doesn't add much value. just a concrete class for anonymous
// objects.
class QUICKCORE_EXPORT QSDynamicClass : public QSWritableClass {
public:
    QSDynamicClass( QSClass *b ) : QSWritableClass( b ) { }
    QString name() const { return QString::fromLatin1("Object"); }
};

class QUICKCORE_EXPORT QSTypeClass : public QSSharedClass {
public:
    QSTypeClass( QSClass *b ) : QSSharedClass( b, AttributeExecutable ) { }
    QString name() const { return QString::fromLatin1("Type"); }

    virtual bool member( const QSObject *o, const QString &n,
			 QSMember *m ) const;
    virtual QSMemberMap members( const QSObject *obj ) const;
    QSMemberMap allMembers( const QSObject *obj ) const;

    virtual QSObject fetchValue( const QSObject *o,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;

    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;
    virtual ValueType valueType() const { return TypeClass; }

    static QSClass *classValue(const QSObject *obj);

    QSObject createType(QSClass *cl ) const;
    QSShared *createTypeShared(QSClass *cl) const;

protected:
    virtual QSObject invoke( QSObject * objPtr, const QSMember &mem ) const;

};

class QUICKCORE_EXPORT QSGlobalClass : public QSWritableClass {
public:
    QSGlobalClass( QSClass *b );
    QString name() const { return QString::fromLatin1("Global"); } // invisible to the user
};

class QUICKCORE_EXPORT QSDebugClass : public QSClass
{
public:
    QSDebugClass( QSClass *base );

    QString name() const { return QString::fromLatin1("Debug"); }

    static void dumpObject( QSEnv * env );
    static void dumpScope( QSEnv * env );
    static void dumpType( QSEnv * env );
};

class QUICKCORE_EXPORT QSSystemClass : public QSClass
{
public:
    QSSystemClass( QSClass *base );

    QString name() const { return QString::fromLatin1("System"); }

    static void print( QSEnv * env );
    static void println( QSEnv * env );
    static QSObject getenv( QSEnv *env );
    static void setenv( QSEnv *env );
};

class QUICKCORE_EXPORT QSAbstractBaseClass : public QSClass
{
public:
    QSAbstractBaseClass(QSClass *base, const QString &id)
	: QSClass(base), ident(id) { };

    QString identifier() const { return ident; }
    QString name() const { return QString::fromLatin1("AbstractBase"); }

    void replace(QSClassClass *classDef);

private:
    QString ident;
};

class QSInstanceData : public QSWritable {
public:
    QSInstanceData( int count, const QSObject &def );
    ~QSInstanceData()
    {
	delete [] vals;
    }

    int size() const { return sz; }
    void resize( int count, const QSObject &def );
    void ensureSize( int count, const QSObject &def );

    QSObject * value( int index ) const
    {
	Q_ASSERT( index>=0 && index<sz );
	return &vals[index];
    }

    void setValue( int index, const QSObject &value )
    {
	Q_ASSERT( index>=0 && index<sz );
	vals[index] = value;
    }

    virtual void invalidate();

private:
    // disabled
    QSInstanceData( const QSInstanceData & );
    QSInstanceData& operator=( const QSInstanceData & );

    int sz;
    QSObject *vals;
};

inline void QSClass::replaceMember( const QString &n, QSMember *m )
{
    replaceMember( n, m, createUndefined() );
}

inline void QSClass::addMember( const QString &n, const QSMember &m )
{
    addMember( n, m, createUndefined() );
}



#endif
