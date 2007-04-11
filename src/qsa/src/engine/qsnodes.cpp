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

#include "qsnodes.h"
#include "qsclass.h"
#include "qsenv.h"
#include "qsengine.h"
#include "qslexer.h"
#include "qsinternal.h"
#include "qsoperations.h"
#include "qsobject_object.h"
#include "qsregexp_object.h"
#include "qsdebugger.h"
#include "qsfuncref.h"
#include "qserror_object.h"
#include <qapplication.h>

#include "quickobjects.h" // from ../kernel

#include <math.h>
#include <assert.h>

// #define QSNODES_ALLOC_DEBUG

using namespace QS;
#ifndef Q_CC_BOR
#define QSA_NUMBER_OPTIMIZATION
#endif


#ifdef QSDEBUGGER
#define QSBREAKPOINT(e) if( !hitStatement( e ) ) return QSObject();
#define QSABORTPOINT(e) if( abortStatement( e ) ) return QSObject();
#else
#define QSBREAKPOINT(e)
#define QSABORTPOINT(e)
#endif

/* Makes the gui responsive while QSA is running. This lets users throw
   exceptions in the interpreter while it is running and thereby kill it.
*/
#define QSA_EVENT_TRIGGER_IMPL if (env->engine()->timeoutTrigger()) \
                                   env->engine()->timeoutTrigger()->fireTimeout();

#define updateErrorline()  				\
    if( env->isExceptionMode() ) { 			\
	QSObject err = env->exception();		\
	int line = QSErrorClass::errorLine(&err);       \
	if( line==-1 )					\
	    QSErrorClass::setErrorLine(&err, lineNo()); \
    }

/* For convenience and safty when implementing the ref / deref operators.
 */
#define ref_printf( a, b, c, d )
#define QSNODE_REF_IMPL_1(name, node1) 			\
    void name::ref() {                                  \
        ref_printf(#name "::ref(%p) count: %d -> %d\n", this, refCount, refCount+1);	\
        if (node1) node1->ref();			\
        QSNode::ref();					\
    }							\
    bool name::deref() {                                \
        ref_printf(#name "::deref(%p): count: %d -> %d\n", this, refCount, refCount-1); \
        if (node1 && node1->deref()) delete node1;	\
        return QSNode::deref();				\
    }

#define QSNODE_REF_IMPL_2(name, node1, node2) 		\
    void name::ref() {                                  \
        ref_printf(#name "::ref(%p) count: %d -> %d\n", this, refCount, refCount+1);	\
        if (node1) node1->ref();			\
        if (node2) node2->ref();			\
        QSNode::ref();					\
    }							\
    bool name::deref() {                                \
        ref_printf(#name "::deref(%p): count: %d -> %d\n", this, refCount, refCount-1); \
        if (node1 && node1->deref()) delete node1;	\
        if (node2 && node2->deref()) delete node2;	\
        return QSNode::deref();				\
    }

#define QSNODE_REF_IMPL_3(name, node1, node2, node3)	\
    void name::ref() {                                  \
        ref_printf(#name "::ref(%p) count: %d -> %d\n", this, refCount, refCount+1);	\
        if (node1) node1->ref();			\
        if (node2) node2->ref();			\
        if (node3) node3->ref();			\
        QSNode::ref();					\
    }							\
    bool name::deref() {                                \
        ref_printf(#name "::deref(%p): count: %d -> %d\n", this, refCount, refCount-1); \
        if (node1 && node1->deref()) delete node1;	\
        if (node2 && node2->deref()) delete node2;	\
        if (node3 && node3->deref()) delete node3;	\
        return QSNode::deref();				\
    }

#define QSNODE_REF_IMPL_4(name, node1, node2, node3, node4) \
    void name::ref() {                                  \
        ref_printf(#name "::ref(%p) count: %d -> %d\n", this, refCount, refCount+1);	\
        if (node1) node1->ref();			\
        if (node2) node2->ref();			\
        if (node3) node3->ref();			\
        if (node4) node4->ref();			\
        QSNode::ref();					\
    }							\
    bool name::deref() {                                \
        ref_printf(#name "::deref(%p): count: %d -> %d\n", this, refCount, refCount-1); \
        if (node1 && node1->deref()) delete node1;	\
        if (node2 && node2->deref()) delete node2;	\
        if (node3 && node3->deref()) delete node3;	\
        if (node4 && node4->deref()) delete node4;	\
        return QSNode::deref();				\
    }

#ifdef QSNODES_ALLOC_DEBUG
static int qsNodeCount = 0;
#endif

QSNode::QSNode()
    : refCount(1)
{
#ifdef QSNODES_ALLOC_DEBUG
    printf("QSNode::QSNode() :: %p\n", this);
    qsNodeCount++;
#endif
    assert( QSLexer::lexer() );
    line = QSLexer::lexer()->lineNo();
}

QSNode::~QSNode()
{
#ifdef QSNODES_ALLOC_DEBUG
    printf("QSNode::~QSNode() :: %p nodeCount: %d\n", this, qsNodeCount);
#endif
}

QSObject QSNode::evaluate( QSEnv *env )
{
    //    qWarning( "QSNode::evaluate()" );
    return rhs( env );
}

QSObject QSNode::rhs( QSEnv *env ) const
{
    return ((QSNode*)this)->lhs( env ).dereference();
}

QSReference QSNode::lhs( QSEnv *env )
{
    // qWarning( "QSNode::lhs( QSEnv * )" );
    return QSReference( rhs( env ) ); // no real reference
}

QSObject QSNode::throwError( QSEnv *env, ErrorType e, const char *msg ) const
{
    return env->throwError( e, QString::fromLatin1(msg), lineNo() );
}

#ifdef QSDEBUGGER
void QSStatementNode::setLoc( int line0, int line1 )
{
    l0 = line0;
    l1 = line1;
    sid = QSLexer::lexer()->sourceId();
}

bool QSStatementNode::hitStatement( QSEnv *env )
{
    Debugger *deb = env->engine()->debugger();
    if ( !deb )
	return TRUE;
    if ( deb->hit( lineNo(), breakPoint ) &&
	 deb->mode() != Debugger::Stop )
	return TRUE;
    env->setExecutionMode( QSEnv::Normal );
    return FALSE;
}

// return TRUE if the debugger wants us to stop at this point
bool QSStatementNode::abortStatement( QSEnv *env )
{
    Debugger *deb = env->engine()->debugger();
    if ( !deb || deb->mode() != Debugger::Stop )
	return FALSE;
    env->setExecutionMode( QSEnv::Normal );
    return TRUE;
}

/**
 * Try to set or delete a breakpoint depending on the value of set.
 * The call will return TRUE if successful, i.e. if line is inside
 * of the statement's range. Additionally, a breakpoint had to
 * be set already if you tried to delete with set=FALSE.
 */
bool QSStatementNode::setBreakpoint( int id, int line, bool set )
{
    // in our source unit and line range ?
    if ( id != sid || (( line < l0 || line > l1 ) && line >= 0) )
	return FALSE;

    if ( !set && !breakPoint )
	return FALSE;

    breakPoint = set;
    return TRUE;
}
#endif

QSObject QSNullNode::rhs( QSEnv *env ) const
{
    return env->createNull();
}

QSObject QSBooleanNode::rhs( QSEnv *env ) const
{
    return env->createBoolean( value );
}

QSObject QSNumberNode::rhs( QSEnv *env ) const
{
    return env->createNumber( value );
}

QSObject QSStringNode::rhs( QSEnv *env ) const
{
    return env->createString( value );
}

QSObject QSRegExpNode::rhs( QSEnv *env ) const
{
    QSList list;
    QSObject p = env->createString( pattern );
    QSObject f = env->createString( flags );
    list.append( p );
    list.append( f );

    return env->regexpClass()->construct( list );
}

// ECMA 11.1.1
QSObject QSThisNode::rhs( QSEnv * env ) const
{
    return env->thisValue();
}

QSResolveNode::~QSResolveNode()
{
    delete info;
    info = 0;
}

QSObject QSResolveNode::rhs( QSEnv * env ) const
{
    QSA_EVENT_TRIGGER_IMPL
    if( info ) { // Using direct lookup if possible.
	return env->getValueDirect( info->member.index(), info->level );
    }
    QSObject o = env->resolveValue( ident );
    if ( o.isValid() )
	return o;
    else
	return env->throwError( QString::fromLatin1("Use of undefined variable '%1'").arg(ident) );
}

QSReference QSResolveNode::lhs( QSEnv * env )
{
    QSA_EVENT_TRIGGER_IMPL
    if( info ) { // Use direct lookup if possible.
	QSObject base = env->scopeObjectAt( info->level );
	QSReference ref( base, info->member, base.objectType() );
	return ref;
    }
    const ScopeChain scope = env->scope();
    ScopeChain::const_iterator it = scope.begin();
    int offset;
    QSMember mem;
    while( it!=scope.end() ) {
	offset = 0;
	const QSClass * cl = (*it).resolveMember( ident, &mem, (*it).objectType(), &offset );
	if( cl && mem.type()!=QSMember::Identifier ) {
	    while( offset-- )
		it++;
	    Q_ASSERT( (*it).isValid() );
	    return QSReference( *it, mem, cl );
	}
	it++;
    }

    // identifier not found
    QString msg = QString::fromLatin1( "Use of undefined variable %1" ).arg( ident );
    env->engine()->warn( msg, lineNo() );
    mem.setType( QSMember::Identifier );
    mem.setName( ident );
    return QSReference( env->globalObject(), mem, env->globalClass() );
}

// ECMA 11.1.4
QSObject QSArrayNode::rhs( QSEnv *env ) const
{
    QSObject array;
    int length;
    int elisionLen = elision ? elision->rhs( env ).toInt32() : 0;

    if ( element ) {
	array = element->rhs( env );
	length = opt ? array.get( QString::fromLatin1("length") ).toInt32() : 0;
    } else {
	array = QSArray( env );
	length = 0;
    }

    if ( opt )
	array.put( QString::fromLatin1("length"), env->createNumber( elisionLen+length ) );

    return array;
}

// ECMA 11.1.4
QSObject QSElementNode::rhs( QSEnv *env ) const
{
    QSObject array, val;
    int length = 0;
    int elisionLen = elision ? elision->rhs( env ).toInt32() : 0;

    if ( list ) {
	array = list->rhs( env );
	val = node->rhs( env );
	length = array.get( QString::fromLatin1("length") ).toInt32();
    } else {
	array = QSArray( env );
	val = node->rhs( env );
    }

    array.put( QSString::from( elisionLen + length ), val );

    return array;
}

// ECMA 11.1.4
QSObject QSElisionNode::rhs( QSEnv *env ) const
{
    if ( elision )
	return env->createNumber( elision->rhs( env ).toNumber() + 1.0 );
    else
	return env->createNumber( 1 );
}

// ECMA 11.1.5
QSObject QSObjectLiteralNode::rhs( QSEnv *env ) const
{
    if ( list )
	return list->rhs( env );

    return env->objectClass()->construct();
}

// ECMA 11.1.5
QSObject QSPropertyValueNode::rhs( QSEnv *env ) const
{
    QSObject obj;
    if ( list )
	obj = list->rhs( env );
    else
	obj = env->objectClass()->construct();
    QSObject n = name->rhs( env );
    QSObject v = assign->rhs( env );

    obj.put( n.toString(), v );

    return obj;
}

// ECMA 11.1.5
QSObject QSPropertyNode::rhs( QSEnv *env ) const
{
    QSObject s;

    if ( str.isNull() ) {
	s = env->createString( QSString::from(numeric) );
    } else
	s = env->createString( str );

    return s;
}

// ECMA 11.1.6
QSObject QSGroupNode::rhs( QSEnv *env ) const
{
    return group->rhs( env );
}

QSReference QSGroupNode::lhs( QSEnv *env )
{
    return group->lhs( env );
}

QSReference QSAccessorNode1::lhs( QSEnv *env )
{
    QSObject v1 = expr1->rhs( env );
    QSObject v2 = expr2->rhs( env );
    QString s = v2.toString();
    QSMember mem;
    int offset = 0;
    const QSClass *cl = v1.resolveMember( s, &mem, v1.objectType(), &offset );
    Q_ASSERT( !offset );
    if ( !mem.isDefined() ) {
	mem.setWritable( FALSE );
        QSReference ref(v1, mem, cl);
        ref.setIdentifier(v2.toString());
        return ref;
    }
    return QSReference( v1, mem, cl );
}

QSObject QSAccessorNode1::rhs( QSEnv *env ) const
{
    QSObject v1 = expr1->rhs( env );
    QSObject v2 = expr2->rhs( env );

    QString s = v2.toString();
    QSMember mem;
    int offset = 0;
    const QSClass *cl = v1.resolveMember( s, &mem, v1.objectType(), &offset );
    Q_ASSERT( !offset );
    if ( cl && mem.isDefined() ) {
	QSObject obj = cl->fetchValue( &v1, mem );
	if (obj.isUndefined() && mem.type() == QSMember::Identifier)
	    return env->throwError( QString::fromLatin1("Trying to access undefined member '%1'").arg(s) );
	updateErrorline();
	return obj;
    }
    return env->throwError( QString::fromLatin1("Trying to access undefined member '%1'").arg(s) );
}

QSReference QSAccessorNode2::lhs( QSEnv *env )
{
    QSObject v = expr->rhs( env );
    QSMember mem;
    int offset = 0;
    const QSClass *cl = v.resolveMember( ident, &mem, v.objectType(), &offset );
    Q_ASSERT( !offset );
    if ( !mem.isDefined() ) {
	mem.setWritable( FALSE );
        QSReference ref(v, mem, cl);
        ref.setIdentifier(ident);
        return ref;
    }
    return QSReference( v, mem, cl );
}

QSObject QSAccessorNode2::rhs( QSEnv *env ) const
{
    QSObject v = expr->rhs( env );
    QSMember mem;
    int offset = 0;
    const QSClass *cl = v.resolveMember( ident, &mem, v.objectType(), &offset );
    Q_ASSERT( !offset );
    if ( cl && mem.isDefined() ) {
	QSObject obj = cl->fetchValue( &v, mem );
	if (obj.isUndefined() && mem.type() == QSMember::Identifier)
	    return env->throwError( QString::fromLatin1("Trying to access undefined member '%1'").arg(ident) );
	updateErrorline();
	return obj;
    }
    return env->throwError( QString::fromLatin1("Trying to access undefined member '%1'").arg(ident) );
}

QSObject QSNewExprNode::rhs( QSEnv *env ) const
{
    // ### ???
    return ((QSNode*)this)->evaluate( env );
}

// ECMA 11.2.2
QSObject QSNewExprNode::evaluate( QSEnv *env )
{
    QSObject v = expr->rhs( env );

    QSList *argList = args ? args->evaluateList( env ) : 0;

    const QSClass *cl = v.objectType();
    QSClass *type = 0;
    /* When inside a class, the constructor will be found before the Type class.
     * Traverse the enclosing class structure to find class with same name and use
     * it instead. */
    if ( cl == env->funcRefClass() ) {
	QSMember member = QSFuncRefClass::refMember(v);
        QString funcName = member.name();
        if (member.type() == QSMember::ScriptFunction) {
            type = member.scriptFunction->scopeDefinition();

            while (type) {
                if (type->asClass() && type->identifier() == funcName) {
                    break;
                }
                type = type->enclosingClass();
            }
        }

        // Happens if we have a function in the global scope and try to new it.
        if (!type) {
            delete argList;
            return throwError(env,
                              TypeError,
                              QString::fromLatin1("Cannot instantiate function '%1'")
                              .arg(funcName).latin1());
        }
    } else if ( cl->valueType() != TypeClass ) {
	delete argList;
	return throwError( env,
			   TypeError,
			   QString::fromLatin1( "Cannot instantiate object of type '%1'" )
			   .arg( cl->identifier() ).latin1() );
    }

    if (!type)
	type = QSTypeClass::classValue(&v);

    if ( !argList )
	argList = new QSList;


    QSObject res;
    if( !type->isAbstract() ) {
	res = type->construct( *argList );
    } else {
	res = env->createUndefined();
	env->throwError( TypeError,
			 QString::fromLatin1( "Cannot instantiate abstract class '%1'" )
			 .arg( type->identifier() ),
			 lineNo() );
    }
    updateErrorline();
    delete argList;

    return res;
}



QSObject QSFunctionCallNode::execute( QSEnv *env )
{
    return rhs( env );
}

// ECMA 11.2.3
QSObject QSFunctionCallNode::rhs( QSEnv *env ) const
{
    QSReference ref = expr->lhs( env );

    QSList *argList = args->evaluateList( env );

    // bail out on error
    if ( env->isExceptionMode() ) {
	delete argList;
	return env->createUndefined();
    }

    QSObject base = ref.base();
    const QSClass *cl = base.objectType();
    QSMember mem = ref.member();

    if( cl==env->funcRefClass() ) {
	// ### for the odd construction '( new Function(...))() ' case, a better solution..
    } else if( !mem.isDefined() ) {
	delete argList;
	QString msg =
            QString::fromLatin1("Undefined member function '%1' for object '%2' of type: '%3'")
            .arg(ref.identifier().isEmpty() ? QString::fromLatin1("unknown") : ref.identifier())
            .arg(base.toString())
            .arg(base.objectType() ? base.objectType()->name() : QString::fromLatin1("undefined"));
	return env->throwError( TypeError, msg, lineNo() );
    } else if( !mem.isExecutable() ) {
	base = cl->fetchValue( &base, mem );
	cl = base.objectType();
	if( !cl->isExecutable() ) {
	    delete argList;
	    QString msg =
		QString::fromLatin1( "'%1' undefined or not a function" ).arg( mem.name() );
	    return env->throwError( TypeError, msg, lineNo() );
	}
    }

#if QS_MAX_STACK > 0
    env->incrStackDepth();
    if ( env->stackDepth() > QS_MAX_STACK ) {
        qWarning( "QSFunctionCallNode::execute() stack overflow" );
        delete argList;
        return env->throwError( RangeError, QString::fromLatin1("Stack overflow"), lineNo() );
    }
#endif

#ifdef QSDEBUGGER
    steppingInto( TRUE, env );
#endif

    QSObject result = base.invoke( mem, *argList );
#ifdef QSDEBUGGER
    updateErrorline();
    steppingInto( FALSE, env );
#endif

#if QS_MAX_STACK > 0
    env->decrStackDepth();
#endif

    delete argList;
    return result;
}

#ifdef QSDEBUGGER
void QSFunctionCallNode::steppingInto( bool in, QSEnv *env ) const
{
    Debugger *dbg = env->engine()->debugger();
    if ( !dbg )
	return;
    if (in) {
	// before entering function. Don't step inside if 'Next' is chosen.
	((QSFunctionCallNode*)this)->previousMode = dbg->mode();
	if ( previousMode == Debugger::Next )
	    dbg->setMode( Debugger::Continue );
    } else {
	// restore mode after leaving function
	dbg->setMode( previousMode);
    }
}
#endif

QSObject QSArgumentsNode::rhs( QSEnv * ) const
{
    assert( 0 );
    return QSObject(); // dummy, see evaluateList()
}

// ECMA 11.2.4
QSList* QSArgumentsNode::evaluateList( QSEnv *env )
{
    if ( !list )
	return new QSList();

    return list->evaluateList( env );
}

QSObject QSArgumentListNode::rhs( QSEnv * ) const
{
    assert( 0 );
    return QSObject(); // dummy, see evaluateList()
}

// ECMA 11.2.4
QSList* QSArgumentListNode::evaluateList( QSEnv *env )
{
    QSObject v = expr->rhs( env );

    if ( !list ) {
	QSList *l = new QSList();
	l->append( v );
	return l;
    }

    QSList *l = list->evaluateList( env );
    l->append( v );

    return l;
}

// ECMA 11.8
QSObject QSRelationalNode::rhs( QSEnv *env ) const
{
    QSObject v1 = expr1->rhs( env );
    QSObject v2 = expr2->rhs( env );

#ifdef QSA_NUMBER_OPTIMIZATION
    // Optimization for numbers.
    if (v1.objectType() == env->numberClass() && v2.objectType() == env->numberClass()) {
	switch(oper) {
	case OpLess:
	    return env->createBoolean(v1.dVal() < v2.dVal());
	case OpLessEq:
	    return env->createBoolean(v1.dVal() <= v2.dVal());
	case OpGreater:
	    return env->createBoolean(v1.dVal() > v2.dVal());
	case OpGreaterEq:
	    return env->createBoolean(v1.dVal() >= v2.dVal());
	default:
	    return env->createBoolean(FALSE);
	}
    }
#endif

    bool b = FALSE;
    if ( oper == OpLess || oper == OpGreaterEq ) {
	QSCompareResult r = v1.compareTo( v2 );
	if( r==CompareUndefined )
	    b = FALSE;
	else
	    b = r==CompareLess ? oper==OpLess : oper==OpGreaterEq ;

    } else if ( oper == OpGreater || oper == OpLessEq ) {
	QSCompareResult r = v1.compareTo( v2 );
	if( r==CompareUndefined )
	    b = FALSE;
	else
	    b = r==CompareGreater ? oper==OpGreater : oper==OpLessEq ;

    } else if ( oper == OpIs || oper == OpInstanceOf ) {

	// ### emit runtime warning about deprecated instanceof operator ?
	// ### without prototype support it doesn't do the right thing anyway
        if (v2.objectType()->name() == "FactoryObject") {
            QSWrapperShared *s = (QSWrapperShared *) v1.shVal();
            return env->createBoolean(s->creator == v2.objectType());
        }

	if( v2.objectType() != env->typeClass() ) {
	    return throwError( env, TypeError,
			       "Right side of operator 'is' is not a type" );
	}
	return env->createBoolean( v1.isA( QSTypeClass::classValue(&v2) ) );
    } else if ( oper == OpIn ) {
	/* Is all of this OK for host objects? */
	if ( !v2.isObject() )
	    return throwError( env, TypeError,
			       "Shift expression not an object into IN expression" );
	b = v2.hasProperty( v1.toString() );
    }
    return env->createBoolean( b );
}

// ECMA 11.9
QSObject QSEqualNode::rhs( QSEnv *env ) const
{
    QSObject v1 = expr1->rhs( env );
    QSObject v2 = expr2->rhs( env );

#ifdef QSA_NUMBER_OPTIMIZATION
    if (v1.objectType() == env->numberClass() && v2.objectType() == env->numberClass()) {
	switch(oper) {
	case OpStrEq:
	case OpEqEq:
	    return env->createBoolean(v1.dVal() == v2.dVal());
	case OpStrNEq:
	case OpNotEq:
	    return env->createBoolean(v1.dVal() != v2.dVal());
	default:
	    Q_ASSERT(!"QSEqualNode::rhs() - this is not possible!!");
	}
    }
#endif

    bool result;
    if ( oper == OpEqEq || oper == OpNotEq ) { 	// == and !=
	result = oper == OpEqEq ? v1.equals( v2 ) : !v1.equals( v2 );
    } else if ( oper == OpStrEq || OpStrNEq ) 	// === and !==
	result = ( oper == OpStrEq ? v1.strictEquals( v2 ) : !v1.strictEquals( v2 ) );
    else
	result = false;
    return env->createBoolean( result );
}

// ECMA 11.10
QSObject QSBitOperNode::rhs( QSEnv *env ) const
{
    QSObject v1 = expr1->rhs( env );
    QSObject v2 = expr2->rhs( env );
    int i1 = v1.toInt32();
    int i2 = v2.toInt32();
    int result;
    if ( oper == OpBitAnd )
	result = i1 & i2;
    else if ( oper == OpBitXOr )
	result = i1 ^ i2;
    else
	result = i1 | i2;

    return env->createNumber( result );
}

// ECMA 11.11
QSObject QSBinaryLogicalNode::rhs( QSEnv *env ) const
{
    QSObject v1 = expr1->rhs( env );

    bool b1 = v1.toBoolean();
    if ( (!b1 && oper == OpAnd) || (b1 && oper == OpOr) )
	return v1;

    QSObject v2 = expr2->rhs( env );

    return v2;
}

// ECMA 11.12
QSObject QSConditionalNode::rhs( QSEnv *env ) const
{
    QSObject v = logical->rhs( env );
    bool b = v.toBoolean();

    return b ? expr1->rhs( env ) : expr2->rhs( env );
}

// ECMA 11.13
QSObject QSAssignNode::rhs( QSEnv *env ) const
{
    QSObject v2 = expr->rhs( env );
    if( env->isExceptionMode() )
	return env->createUndefined();
    QSObject v;
    QSReference ref;
    ErrorType err;
    if ( oper == OpEqual ) {
	ref = left->lhs( env );
	if ( !ref.isWritable() )
	    return env->throwError( ReferenceError,
				    QString::fromLatin1("Left hand side value is not writable"),
				    lineNo() );
	ref.assign( v2 );
	updateErrorline();
	return v2;
    } else {
	ref = left->lhs( env );
	if ( !ref.isWritable() )
	    return env->throwError( ReferenceError,
				    QString::fromLatin1("Left hand side value is not writable"),
				    lineNo() );
	QSObject v1 = ref.dereference();
#ifdef QSA_NUMBER_OPTIMIZATION
	bool isNum = v2.objectType() == env->numberClass()
                  && v1.objectType() == env->numberClass();
#endif
	int i1 = v1.toInt32();
	int i2 = v2.toInt32();
	uint ui;
	switch ( oper ) {
	case OpMultEq:
	    v =
#ifdef QSA_NUMBER_OPTIMIZATION
	    isNum ? env->createNumber(v1.dVal() * v2.dVal()) :
#endif
		mult( env, v1, v2, '*' );
	    break;
	case OpDivEq:
	    v =
#ifdef QSA_NUMBER_OPTIMIZATION
		isNum ? env->createNumber(v1.dVal() / v2.dVal()) :
#endif
		mult( env, v1, v2, '/' );
	    break;
	case OpPlusEq:
	    v =
#ifdef QSA_NUMBER_OPTIMIZATION
		isNum ? env->createNumber(v1.dVal() + v2.dVal()) :
#endif
		add( env, v1, v2, '+' );
	    break;
	case OpMinusEq:
	    v =
#ifdef QSA_NUMBER_OPTIMIZATION
		isNum ? env->createNumber(v1.dVal() - v2.dVal()) :
#endif
		add( env, v1, v2, '-' );
	    break;
	case OpLShift:
	    v = env->createNumber( i1 <<= i2 );
	    break;
	case OpRShift:
	    v = env->createNumber( i1 >>= i2 );
	    break;
	case OpURShift:
	    ui = v1.toUInt32();
	    v = env->createNumber( ui >>= i2 );
	    break;
	case OpAndEq:
	    v = env->createNumber( i1 &= i2 );
	    break;
	case OpXOrEq:
	    v = env->createNumber( i1 ^= i2 );
	    break;
	case OpOrEq:
	    v = env->createNumber( i1 |= i2 );
	    break;
	case OpModEq:
	    v = env->createNumber( i1 %= i2 );
	    break;
	default:
	    v = env->createUndefined();
	}
    };

    err = NoError; ref.assign( v );
    updateErrorline();
    if ( err == NoError )
	return v;
    else
	return throwError( env, err, "Invalid reference" );
}

// ECMA 11.3
QSObject QSPostfixNode::rhs( QSEnv *env ) const
{
    QSReference ref = expr->lhs( env );
    if ( !ref.isWritable() )
	return throwError( env, ReferenceError,
			   "Left hand side value is not writable" );
    QSObject v = ref.dereference();
    double n = v.toNumber();
    double newValue = ( oper == OpPlusPlus ) ? n + 1 : n - 1;
    ref.assign( env->createNumber( newValue ) );

    return env->createNumber( n );
}

// ECMA 11.4.1
QSObject QSDeleteNode::rhs( QSEnv *env ) const
{
    QSReference ref = expr->lhs( env );
    return env->createBoolean( ref.deleteProperty() );
}

// ECMA 11.4.2
QSObject QSVoidNode::rhs( QSEnv *env ) const
{
    (void) expr->rhs( env );

    return env->createUndefined();
}

// ECMA 11.4.3
QSObject QSTypeOfNode::rhs( QSEnv *env ) const
{
    QString s;
    QSReference ref = expr->lhs( env );
    QSObject v;
    if ( ref.isReference() ) {
	if ( !ref.isDefined() )
	    return env->createString( QString::fromLatin1("undefined") );
	v = ref.dereference();
    } else {
	v = ref.base();
    }

    const QSClass *cl = v.objectType();
    if( cl == env->undefinedClass() )
	s = "undefined";
    else if( cl == env->nullClass() )
	s = "object";
    else if( cl == env->booleanClass() )
	s = "boolean";
    else if( cl == env->numberClass() )
	s = "number";
    else if( cl == env->stringClass() )
	s = "string";
    else if ( v.isExecutable() )
	s = "function";
    else
	s = "object";

    return env->createString( s );
}

// ECMA 11.4.4 and 11.4.5
QSObject QSPrefixNode::rhs( QSEnv *env ) const
{
    QSReference ref = expr->lhs( env );
    if ( !ref.isWritable() )
	return throwError( env, ReferenceError,
			   "Left hand side value is not writable" );
    QSObject v = ref.dereference();
    double n = v.toNumber();

    double newValue = ( oper == OpPlusPlus ) ? n + 1 : n - 1;
    QSObject n2 = env->createNumber( newValue );

    ref.assign( n2 );

    return n2;
}

// ECMA 11.4.6
QSObject QSUnaryPlusNode::rhs( QSEnv *env ) const
{
    QSObject v = expr->rhs( env );

    return env->createNumber( v.toNumber() );
}

// ECMA 11.4.7
QSObject QSNegateNode::rhs( QSEnv *env ) const
{
    QSObject v = expr->rhs( env );
    double n = v.toNumber();

    return env->createNumber( -n );
}

// ECMA 11.4.8
QSObject QSBitwiseNotNode::rhs( QSEnv *env ) const
{
    QSObject v = expr->rhs( env );
    int i32 = v.toInt32();

    return env->createNumber( ~i32 );
}

// ECMA 11.4.9
QSObject QSLogicalNotNode::rhs( QSEnv *env ) const
{
    QSObject v = expr->rhs( env );
    bool b = v.toBoolean();

    return env->createBoolean( !b );
}

// ECMA 11.5
QSObject QSMultNode::rhs( QSEnv *env ) const
{
    QSObject v1 = term1->rhs( env );
    QSObject v2 = term2->rhs( env );

#if defined  (QSA_NUMBER_OPTIMIZATION)
    if (v1.objectType() == env->numberClass() && v2.objectType() == env->numberClass()) {
	switch(oper) {
	case '*': return env->createNumber( v1.dVal() * v2.dVal() );
	case '/': return env->createNumber( v1.dVal() / v2.dVal() );
	case '%': return env->createNumber( fmod(v1.dVal(), v2.dVal()) );
	default:
	    break;
	}
    }
#endif
    return mult( env, v1, v2, oper );
}

// ECMA 11.7
QSObject QSShiftNode::rhs( QSEnv *env ) const
{
    QSObject v1 = term1->rhs( env );
    QSObject v2 = term2->rhs( env );
    unsigned int i2 = v2.toUInt32();
    i2 &= 0x1f;

    long result;
    switch ( oper ) {
    case OpLShift:
	result = v1.toInt32() << i2;
	break;
    case OpRShift:
	result = v1.toInt32() >> i2;
	break;
    case OpURShift:
	result = v1.toUInt32() >> i2;
	break;
    default:
	assert( !"ShiftNode: unhandled switch case" );
	result = 0L;
    }

    return env->createNumber( double(result) );
}

// ECMA 11.6
QSObject QSAddNode::rhs( QSEnv *env ) const
{
    QSObject v1 = term1->rhs( env );
    QSObject v2 = term2->rhs( env );

#ifdef QSA_NUMBER_OPTIMIZATION
    // Make special case for numbers
    if (v1.objectType() == env->numberClass() && v2.objectType() == env->numberClass())
	return env->createNumber( oper == '+' ? v1.dVal() + v2.dVal() : v1.dVal() - v2.dVal() );
#endif

    return add( env, v1, v2, oper );
}

// ECMA 11.14
QSObject QSCommaNode::rhs( QSEnv *env ) const
{
    (void) expr1->rhs( env ); // ignore return value

    return expr2->rhs( env );
}

QSObject QSScopeNode::execute( QSEnv *env )
{
    if( scope ) {
	scope->activateScope();
	QSObject result = executeStatement( env );
	scope->deactivateScope();
	return result;
    }
    return executeStatement( env );
}


// ECMA 12.1
QSObject QSBlockNode::executeStatement( QSEnv *env )
{
    if ( !statlist )
	return env->createUndefined();

    return statlist->execute( env );
}

// ECMA 12.1
QSObject QSStatListNode::execute( QSEnv *env )
{
    if( !list ) {
	QSObject r = statement->execute( env );
	return r;
    }

    QSObject l = list->execute( env );
    if( env->isExceptionMode() || !env->isNormalMode() ) {
	return l;
    }


    QSObject s = statement->execute( env );
    if( env->isExceptionMode() || !env->isNormalMode() ) {
	return s;
    }

    return QSObject();
}

// ECMA 12.2
QSObject QSAssignExprNode::rhs( QSEnv *env ) const
{
    return expr->rhs( env );
}

// ECMA 12.3
QSObject QSEmptyStatementNode::execute( QSEnv * )
{
//     return QSObject( Normal );
    return QSObject();
}

// ECMA 12.6.3
QSObject QSForNode::executeStatement( QSEnv *env )
{
    QSBREAKPOINT( env );
	;
    if ( expr1 )
	(void)expr1->rhs( env );
    if( env->isExceptionMode() )
	return QSObject();

    QSObject retVal;

    while ( 1 ) {
	QSA_EVENT_TRIGGER_IMPL
	if ( expr2 ) {
	    bool b = expr2->rhs( env ).toBoolean();
	    if ( !b ) {
		break;
	    }
	}
	// bail out on error
	if ( env->isExceptionMode() ) {
	    break;
	}

	QSObject c = stat->execute( env );
	if ( c.isValid() )
	    retVal = c;
	if ( env->isContinueMode() && env->isCurrentLabelValid() ) {
	    env->setExecutionMode( QSEnv::Normal );
	} else if ( env->isBreakMode() && env->isCurrentLabelValid() ) {
	    if( env->currentLabel().isEmpty() )
		env->setExecutionMode( QSEnv::Normal );
	    break;
	} else if ( !env->isNormalMode() ) {
	    return retVal;
	}

	if ( expr3 )
	    (void)expr3->rhs( env );
    }
    return retVal;
}

// ECMA 12.6.4
QSObject QSForInNode::executeStatement( QSEnv *env )
{
    QSObject retval = env->createUndefined();
    QSObject obj = expr->rhs( env );
    QSMemberMap mmap = obj.objectType()->members( &obj );
    QSMemberMap::ConstIterator it = mmap.begin();
    QSObject base;
    QSMember member;
    if ( lexpr ) {
	QSReference ref = lexpr->lhs( env );
	if ( !ref.isWritable() ) {
	    QSObject e = env->throwError( TypeError, QString::fromLatin1("Non-writable index"), lineNo() );
	    return e;
	}
	base = ref.base();
	member = ref.member();
    } else {
	base = env->currentScope();
	member = QSMember( QSMember::Variable, var->index(), AttributeNone );
    }

    if( env->isExceptionMode() ) {
	return QSObject();
    }

    while ( it != mmap.end() ) {
	QSA_EVENT_TRIGGER_IMPL
	const QSMember &mem = *it;
	if ( mem.isEnumberable() ) {
	    base.write( member, env->createString( mem.name() ) );
	    QSObject c = stat->execute( env );
	    if ( c.isValid() )
		retval = c;

	    if ( env->isContinueMode() && env->isCurrentLabelValid() ) {
		env->setExecutionMode( QSEnv::Normal );
	    } else if ( env->isBreakMode() && env->isCurrentLabelValid() ) {
		if( env->currentLabel().isEmpty() )
		    env->setExecutionMode( QSEnv::Normal );
		break;
	    } else if ( !env->isNormalMode() ) {
		return c;
	    }
	}
	++it;
    }
    return retval;
}

// ECMA 12.4
QSObject QSExprStatementNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );

    QSObject v = expr->rhs( env );

    // bail out on error
    if ( env->isExceptionMode() ) {
	return QSObject();
    }

    return v;
}

// ECMA 12.5
QSObject QSIfNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );

    QSObject result;

    QSObject v = expr->rhs( env );
    if (env->isExceptionMode())
        return QSObject();

    bool b = v.toBoolean();

    if ( b )
	result = statement1->execute( env );
    else if( statement2 )
	result = statement2->execute( env );

    return result;
}

// ECMA 12.6.1
QSObject QSDoWhileNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );

    QSObject bv;
    QSObject c;
    QSObject value;
    QSObject result;

    do {
	QSA_EVENT_TRIGGER_IMPL
	// bail out on error
	if ( env->isExceptionMode() ) {
	    result = env->exception();
	    break;
	}

	c = statement->execute( env );
	if ( env->isContinueMode() && env->isCurrentLabelValid() ) {
	    env->setExecutionMode( QSEnv::Normal );
	} else if ( env->isBreakMode() && env->isCurrentLabelValid() ) {
	    if( env->currentLabel().isEmpty() ) // Clean up if no label node is known
		env->setExecutionMode( QSEnv::Normal );
	    result = value;
	    break;
	} else if ( !env->isNormalMode() ) {
	    result = c;
	    break;
	}
	bv = expr->rhs( env );
    } while ( bv.toBoolean() );

    return result;
}

// ECMA 12.6.2
QSObject QSWhileNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );
    QSObject bv;
    QSObject c, result;
    bool b = FALSE;
    QSObject value;

    while ( 1 ) {
	QSA_EVENT_TRIGGER_IMPL
	bv = expr->rhs( env );
	b = bv.toBoolean();

	// bail out on error
	if ( env->isExceptionMode() ) {
	    result = QSObject();
	    break;
	}

	if ( !b )
	    return value;
	c = statement->execute( env );
	if ( c.isValid() )
	    value = c;
	if ( env->isContinueMode() && env->isCurrentLabelValid() ) {
	    env->setExecutionMode( QSEnv::Normal );
	} else if ( env->isBreakMode() && env->isCurrentLabelValid() ) {
	    if( env->currentLabel().isEmpty() ) // Clean up if no label node is known
		env->setExecutionMode( QSEnv::Normal );
	    result = value;
	    break;
	} else if ( !env->isNormalMode() ) {
	    result = c;
	    break;
	}
    }

    return result;
}

// ECMA 12.7
QSObject QSContinueNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );

    env->setExecutionMode( QSEnv::Continue );
    if ( ident.isEmpty() )
 	return QSObject();
    if( !env->containsLabel( ident ) ) {
	return env->throwError( SyntaxError,
				QString::fromLatin1( "Label %1 not found in containing block" )
				  .arg( ident ),
				lineNo() );
    }
    env->setCurrentLabel( ident );
    return QSObject();
}

// ECMA 12.8
QSObject QSBreakNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );

    env->setExecutionMode( QSEnv::Break );
    if ( ident.isEmpty() )
 	return QSObject();
    if( !env->containsLabel( ident ) ) {
	return env->throwError( SyntaxError,
				QString::fromLatin1( "Label %1 not found in containing block" )
				  .arg( ident ),
				lineNo() );
    }
    env->setCurrentLabel( ident );
    return QSObject();
}

// ECMA 12.9
QSObject QSReturnNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );
    QSObject ret = value ? value->rhs( env ) : env->createUndefined();
    if( !env->isExceptionMode() )
	env->setExecutionMode( QSEnv::ReturnValue );
    return ret;
}

// ECMA 12.10
QSObject QSWithNode::executeStatement( QSEnv *env )
{
    QSBREAKPOINT( env );

    QSObject v = expr->rhs( env );
    if( env->isExceptionMode() || v.isUndefined() )
	return QSObject();
    if (!v.isObject()) {
        return env->throwError(ReferenceError,
                               QString::fromLatin1("Evaluation of with statement is not an object, "
                                                   "was '%1' of type %2")
                               .arg(v.toString())
                               .arg(v.isValid() ? v.typeName() : QString::fromLatin1("invalid")),
                               lineNo());
    }
    env->pushScope( v );
    QSObject oldThis = env->thisValue();
    env->setThisValue( v );
    QSObject res = stat->execute( env );
    env->setThisValue( oldThis );
    env->popScope();

    return res;
}

// ECMA QS12.11
QSClauseListNode* QSClauseListNode::append( QSCaseClauseNode *c )
{
    QSClauseListNode *l = this;
    while ( l->nx )
	l = l->nx;
    l->nx = new QSClauseListNode( c );

    return this;
}

// ECMA 12.11
QSObject QSSwitchNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );

    QSObject v = expr->rhs( env );
    QSObject res = block->evalBlock( env, v );

    if ( env->isBreakMode() && ls.contains( env->currentLabel() ) ) {
	env->setExecutionMode( QSEnv::Normal );
	return res;
    }
    else
	return res;
}

// ECMA 12.11
QSObject QSCaseBlockNode::evalBlock( QSEnv *env, const QSObject& input )
{
    QSObject v;
    QSObject res;
    QSClauseListNode *a = list1, *b = list2;
    QSCaseClauseNode *clause;

    if ( a ) {
	while ( a ) {
	    clause = a->clause();
	    a = a->next();
	    v = clause->rhs( env );
	    if ( input.strictEquals( v ) ) {
		res = clause->evalStatements( env );
		if ( !env->isNormalMode() )
		    return res;
		while ( a ) {
		    res = a->clause()->evalStatements( env );
		    if ( !env->isNormalMode() )
			return res;
		    a = a->next();
		}
		break;
	    }
	}
    }

    while ( b ) {
	clause = b->clause();
	b = b->next();
	v = clause->rhs( env );
	if ( input.strictEquals( v ) ) {
	    res = clause->evalStatements( env );
	    if ( !env->isNormalMode() )
		return res;
	    goto step18;
	}
    }

    // default clause
    if ( def ) {
	res = def->evalStatements( env );
	if ( !env->isNormalMode() )
	    return res;
    }
    b = list2;
 step18:
    while ( b ) {
	clause = b->clause();
	res = clause->evalStatements( env );
	if ( !env->isNormalMode() )
	    return res;
	b = b->next();
    }

    return QSObject();
}

// ECMA 12.11
QSObject QSCaseClauseNode::rhs( QSEnv *env ) const
{
    return expr->rhs( env );
}

// ECMA 12.11
QSObject QSCaseClauseNode::evalStatements( QSEnv *env )
{
    if ( list )
	return list->execute( env );
    else {
	env->setExecutionMode( QSEnv::Normal );
	return env->createUndefined();
    }
}

// ECMA 12.12
QSObject QSLabelNode::execute( QSEnv *env )
{
    if ( env->containsLabel( label ) ) {
	env->throwError( SyntaxError,
			 QString::fromLatin1( "Duplicated label %1" ).arg( label ),
			 lineNo() );
	return QSObject(); // Q: What to return here?
    }
    env->pushLabel( label );
    QSObject e = stat->execute( env );
    env->popLabel();

    if ( env->currentLabel() == label )
	env->setExecutionMode( QSEnv::Normal );
    return e;
}

// ECMA 12.13
QSObject QSThrowNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );

    QSObject v = expr->rhs( env );
    if( v.objectType() != env->errorClass() )
	v = env->errorClass()->construct( ThrowError, v.toString(), lineNo() );

    env->setException( v );
    return QSObject();
}

// ECMA 12.14
QSObject QSTryNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );

    QSObject ret;

    if( block )
	ret = block->execute( env );

    if( env->isExceptionMode() && _catch )
	ret = _catch->execute( env );

    if( _final )
	ret = _final->execute( env );

    return ret;
}

QSObject QSCatchNode::executeStatement( QSEnv *env )
{
    // Got the
    env->setValueDirect( index, 1, env->exception() );
    env->setExecutionMode( QSEnv::Normal );
    return block->execute( env );
}

// ECMA 12.14
QSObject QSFinallyNode::execute( QSEnv *env )
{
    return block->execute( env );
}

int QSFunctionBodyNode::count = 0;

QSFunctionBodyNode::QSFunctionBodyNode( QSSourceElementsNode *s )
    : source(s), scopeDef( 0 )
{
    idx = ++count;
#ifdef QSDEBUGGER
    setLoc( -1, -1 );
#endif
}

QSFunctionBodyNode::~QSFunctionBodyNode()
{
    // Will be set to 0 by QSClass::clear during cleanup to avoid cleanup recursion.
    // This is ok since during cleanup everything will be deleted anyway.
    if (scopeDef) {
	scopeDef->env()->unregisterClass(scopeDef);
	scopeDef->clear();
	delete scopeDef;
    }
}

// ECMA 13 + 14 for QSProgramNode
QSObject QSFunctionBodyNode::execute( QSEnv *env )
{
    /* TODO: workaround for empty body which I don't see covered by the spec */
    if ( !source )
	return QSObject();

    QSObject ret = source->execute( env );
#ifdef QSDEBUGGER
    if( env->isExceptionMode() && env->engine()->debugger() ) {
	env->engine()->debugger()->storeExceptionStack();
    }
#endif
    return ret;
}


// ECMA

// ECMA 13
QSObject QSFuncExprNode::rhs( QSEnv *env ) const
{
    QSObject thisValue = env->currentScope();
    if( !thisValue.isDefined() )
	thisValue = env->globalObject();
    QSMember member( body );
    member.setName( body->scopeDefinition()->identifier() );

    return env->funcRefClass()->createReference( thisValue, member );
}


QSParameterNode* QSParameterNode::append( const QString *i, QSTypeNode *t )
{
    QSParameterNode *p = this;
    while ( p->next )
	p = p->next;

    p->next = new QSParameterNode( i, t );

    return this;
}

// ECMA 13
QSObject QSParameterNode::rhs( QSEnv *env ) const
{
    return env->createUndefined();
}


QSProgramNode* QSProgramNode::prog;

QSProgramNode::QSProgramNode( QSSourceElementsNode *s )
    : QSFunctionBodyNode( s )
{
    prog = this;
}

QSProgramNode::~QSProgramNode()
{
    if (prog == this)
	prog = 0;
}

void QSProgramNode::deleteGlobalStatements()
{
    if ( source )
	source->deleteStatements();
}


// ECMA 14
QSObject QSSourceElementsNode::execute( QSEnv *env )
{
    if ( env->isExceptionMode() )
	return QSObject(); // errorCompletion();

    if ( !elements )
	return element->execute( env );

    QSObject c1 = elements->execute( env );
    if ( env->isExceptionMode() )
	return QSObject();
    if ( !env->isNormalMode() )
	return c1;

    QSObject c2 = element->execute( env );
    if ( env->isExceptionMode() )
	return QSObject();

    return c2;
}

void QSSourceElementsNode::deleteStatements()
{
    element->deleteStatements();

    if ( elements )
	elements->deleteStatements();
}


// ECMA 14
QSObject QSSourceElementNode::execute( QSEnv *env )
{
    return statement->execute( env );
}

void QSSourceElementNode::deleteStatements()
{
    delete statement;
}

QSObject QSClassDefNode::execute( QSEnv *env )
{
    // do nothing if this only a forward declaration
    if ( !body )
	return QSObject();

    if (cldef) {
        // run class initializer code
        cldef->executeBlock( env );
    }

    return QSObject();
}

QSObject QSTypeNode::rhs( QSEnv *env ) const
{
    return env->resolveValue( ident );
}

QSObject QSTypedVarNode::rhs( QSEnv *env ) const
{
    if ( type ) {
	QSObject cl = type->rhs(env);
	if (!cl.isValid())
	    return env->throwError(QString::fromLatin1("Type '%1' is undefined").arg(type->identifier()));
        QSList empty;
	return QSTypeClass::classValue(&cl)->construct( empty );
    }
    return env->createUndefined();  	// unused, therefore 0L is okay
}

void QSVarBindingNode::declare( QSEnv *env ) const
{
    // Member variables (static/nonstatic) should not be initialized
    if ( index() < 0 )
	return;
    QSObject scope = env->currentScope(); // Context::current()->variableObject();
    QSObject val = var->rhs( env );
    if( assign && val.isValid() ) {
	QSObject ass = assign->rhs( env );
	val = ass;
    }

    if( !env->isExceptionMode() )
	scope.objectType()->write( &scope, index(), val );
}

void QSVarBindingListNode::declare( QSEnv *env ) const
{
    if ( list )
	list->declare( env );
    binding->declare( env );
}

QSObject QSVarDefNode::execute( QSEnv *env )
{
    QSBREAKPOINT( env );

    list->declare( env );
    return QSObject();
}

#if 0
void QSPackageNode::processFuncDecl( QSEnv * )
{
//     Context *ctx = Context::current();
//     QSObject oldVar = ctx->variableObject();
//     QSObject var = oldVar;
//     QString p = package;
//     // break up dotted notation like P1.P2.P3
//     while ( !p.isEmpty() ) {
// 	int pos = p.find( '.' );
// 	if ( pos < 0 )
// 	    pos = p.length();
// 	// don't overwrite any existing objects, reuse them
// 	QSObject o = var.get( p.left( pos ) );
// 	if ( o.isDefined() ) {
// 	    var = o;
// 	} else {
// 	    QSObject pkg = env->objectClass()->construct();
// 	    var.put( p.left( pos ), pkg, DontDelete );
// 	    var = pkg;
// 	}
// 	p = p.mid( pos + 1 );
//     }
//     ctx->setVariableObject( var );
//     ctx->pushScope( var );
//     (void)block->execute( env );
//     ctx->popScope();
//     ctx->setVariableObject( oldVar );
}
#endif

QSObject QSImportNode::execute( QSEnv *env )
{
    QString errMsg;
    env->engine()->requestPackage( package, errMsg );
    if ( errMsg.isNull() )
	return QSObject();
    else {
	env->setExecutionMode( QSEnv::Throw );
	env->throwError( GeneralError, errMsg, lineNo() );
	return QSObject();
    }
}

// bool QSProgramNode::deref()
// {
//     if (QSFunctionBodyNode::deref()) {
// 	if (this == prog) // A little safty measure when the prog thingy is deleted...
// 	    prog = 0;
// 	return TRUE;
//     }
//     return FALSE;
// }

QSNODE_REF_IMPL_1(QSAccessorNode2, expr)
QSNODE_REF_IMPL_1(QSArgumentsNode, list)
QSNODE_REF_IMPL_1(QSAssignExprNode, expr)
QSNODE_REF_IMPL_1(QSBitwiseNotNode, expr)
QSNODE_REF_IMPL_1(QSBlockNode, statlist)
QSNODE_REF_IMPL_1(QSCatchNode, block)
QSNODE_REF_IMPL_1(QSDeleteNode, expr)
QSNODE_REF_IMPL_1(QSElisionNode, elision)
QSNODE_REF_IMPL_1(QSEmitNode, expr)
QSNODE_REF_IMPL_1(QSExprStatementNode, expr)
QSNODE_REF_IMPL_1(QSFinallyNode, block)
QSNODE_REF_IMPL_1(QSFunctionBodyNode, source)
QSNODE_REF_IMPL_1(QSGroupNode, group)
QSNODE_REF_IMPL_1(QSLabelNode, stat)
QSNODE_REF_IMPL_1(QSLogicalNotNode, expr)
QSNODE_REF_IMPL_1(QSNegateNode, expr)
QSNODE_REF_IMPL_1(QSObjectLiteralNode, list)
QSNODE_REF_IMPL_1(QSPackageNode, block)
QSNODE_REF_IMPL_1(QSPostfixNode, expr)
QSNODE_REF_IMPL_1(QSPrefixNode, expr)
QSNODE_REF_IMPL_1(QSReturnNode, value)
QSNODE_REF_IMPL_1(QSSourceElementNode, statement)
QSNODE_REF_IMPL_1(QSThrowNode, expr)
QSNODE_REF_IMPL_1(QSTypeOfNode, expr)
QSNODE_REF_IMPL_1(QSTypedVarNode, type)
QSNODE_REF_IMPL_1(QSUnaryPlusNode, expr)
QSNODE_REF_IMPL_1(QSVoidNode, expr)
QSNODE_REF_IMPL_2(QSAccessorNode1, expr1, expr2)
QSNODE_REF_IMPL_2(QSAddNode, term1, term2)
QSNODE_REF_IMPL_2(QSArgumentListNode, list, expr)
QSNODE_REF_IMPL_2(QSArrayNode, element, elision)
QSNODE_REF_IMPL_2(QSAssignNode, left, expr)
QSNODE_REF_IMPL_2(QSBinaryLogicalNode, expr1, expr2)
QSNODE_REF_IMPL_2(QSBitOperNode, expr1, expr2)
QSNODE_REF_IMPL_2(QSCaseClauseNode, expr, list)
QSNODE_REF_IMPL_2(QSClauseListNode, cl, nx)
QSNODE_REF_IMPL_2(QSCommaNode, expr1, expr2)
QSNODE_REF_IMPL_2(QSDoWhileNode, statement, expr)
QSNODE_REF_IMPL_2(QSElementNode, list, elision)
QSNODE_REF_IMPL_2(QSEqualNode, expr1, expr2)
QSNODE_REF_IMPL_2(QSFuncExprNode, param, body)
QSNODE_REF_IMPL_2(QSFunctionCallNode, expr, args)
QSNODE_REF_IMPL_2(QSMultNode, term1, term2)
QSNODE_REF_IMPL_2(QSNewExprNode, expr, args)
QSNODE_REF_IMPL_2(QSParameterNode, typ, next)
QSNODE_REF_IMPL_2(QSRelationalNode, expr1, expr2)
QSNODE_REF_IMPL_2(QSShiftNode, term1, term2)
QSNODE_REF_IMPL_2(QSSourceElementsNode, element, elements)
QSNODE_REF_IMPL_2(QSStatListNode, statement, list)
QSNODE_REF_IMPL_2(QSSwitchNode, expr, block)
QSNODE_REF_IMPL_2(QSVarBindingListNode, list, binding)
QSNODE_REF_IMPL_2(QSVarBindingNode, var, assign)
QSNODE_REF_IMPL_2(QSVarDefNode, list, attrs)
QSNODE_REF_IMPL_2(QSWhileNode, expr, statement)
QSNODE_REF_IMPL_2(QSWithNode, expr, stat)
QSNODE_REF_IMPL_3(QSCaseBlockNode, list1, def, list2)
QSNODE_REF_IMPL_3(QSClassDefNode, type, body, attrs)
QSNODE_REF_IMPL_3(QSConditionalNode, expr1, expr2, logical)
QSNODE_REF_IMPL_3(QSIfNode, expr, statement1, statement2)
QSNODE_REF_IMPL_3(QSPropertyValueNode, name, assign, list)
QSNODE_REF_IMPL_3(QSTryNode, block, _catch, _final)
QSNODE_REF_IMPL_4(QSForInNode, lexpr, expr, var, stat)
QSNODE_REF_IMPL_4(QSForNode, expr1, expr2, expr3, stat)
QSNODE_REF_IMPL_4(QSFuncDeclNode, rtype, param, attrs, body)
