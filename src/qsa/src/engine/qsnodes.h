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

#ifndef QSNODES_H
#define QSNODES_H

#include "qsmember.h"
#include "qsglobal.h"
#include "qsinternal.h"
#include "qsobject.h"
#include "qsreference.h"
#include "qstypes.h"
#include "qsdebugger.h"
#include <qstring.h>
#include <qptrlist.h>

class QSObject;
class QSCheckData;
class RegExp;
class QSSourceElementsNode;
class QSProgramNode;
class QSTypeNode;
class QSLookupInfo;

enum Operator { OpEqual,
		OpEqEq,
		OpNotEq,
		OpStrEq,
		OpStrNEq,
		OpPlusEq,
		OpMinusEq,
		OpMultEq,
		OpDivEq,
		OpPlusPlus,
		OpMinusMinus,
		OpLess,
		OpLessEq,
		OpGreater,
		OpGreaterEq,
		OpAndEq,
		OpXOrEq,
		OpOrEq,
		OpModEq,
		OpAnd,
		OpOr,
		OpBitAnd,
		OpBitXOr,
		OpBitOr,
		OpLShift,
		OpRShift,
		OpURShift,
		OpIs,
		OpIn,
		OpInstanceOf
};

class QSNode {
public:
    QSNode();
    virtual ~QSNode();
    virtual QSObject evaluate( QSEnv * );
    virtual QSObject rhs( QSEnv * ) const = 0;
    virtual QSReference lhs( QSEnv * );
    int lineNo() const { return line; }
    virtual void check( QSCheckData * ) = 0;
#ifdef QSDEBUGGER
    virtual bool setBreakpoint( int, int, bool ) { return FALSE; }
#endif
    virtual bool deref() { Q_ASSERT(refCount>0); return !--refCount; }
    virtual void ref() { ++refCount; }
protected:
    QSObject throwError( QSEnv *env, ErrorType e, const char *msg ) const;
    int refCount;
private:
    // disallow assignment and copy-construction
    QSNode( const QSNode & );
    QSNode& operator=( const QSNode& );
    int line;
};

class QSStatementNode : public QSNode {
public:
#ifdef QSDEBUGGER
    QSStatementNode() : l0( -1 ), l1( -1 ), sid( -1 ), breakPoint( FALSE ) { }
    void setLoc( int line0, int line1 );
    int firstLine() const { return l0; }
    int lastLine() const { return l1; }
    int sourceId() const { return sid; }
    bool hitStatement( QSEnv *env );
    bool abortStatement( QSEnv *env );
    virtual bool setBreakpoint( int id, int line, bool set );
#endif
    virtual QSObject execute( QSEnv * ) = 0;
    void check( QSCheckData * ) = 0;
    void checkIfGlobalAllowed( QSCheckData * );
    QSObject errorCompletion();
    void pushLabel( const QString *id ) {
	if ( id ) ls.push( *id );
    }
protected:
    LabelStack ls;
private:
    QSObject evaluate( QSEnv *env ) { return env->createUndefined(); }
    QSObject rhs( QSEnv *env ) const { return env->createUndefined(); }
#ifdef QSDEBUGGER
    int l0, l1;
    int sid;
    bool breakPoint;
#endif
};

class QSAttributeNode : public QSNode {
public:
    QSAttributeNode( QSAttribute a ) { add( a ); }
    void add( QSAttribute a ) { attrs.append( a ); }
    QSObject rhs( QSEnv *env ) const { return env->createUndefined(); }
    void check( QSCheckData * );

private:
    QValueList<QSAttribute> attrs;
};

class QSNullNode : public QSNode {
public:
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
};

class QSBooleanNode : public QSNode {
public:
    QSBooleanNode( bool v ) : value( v ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
private:
    bool value;
};

class QSNumberNode : public QSNode {
public:
    QSNumberNode( double v ) : value( v ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
private:
    double value;
};

class QSStringNode : public QSNode {
public:
    QSStringNode( const QString *v ) { value = *v; }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
private:
    QString value;
};

class QSRegExpNode : public QSNode {
public:
    QSRegExpNode( const QString &p, const QString &f )
	: pattern( p ), flags( f ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
private:
    QString pattern, flags;
};

class QSThisNode : public QSNode {
public:
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
};

class QSResolveNode : public QSNode {
public:
    QSResolveNode( const QString *s ) : ident( *s ), info( 0 ) { }
    ~QSResolveNode();
    QSObject rhs( QSEnv * ) const;
    QSReference lhs( QSEnv * );
    void assign( const QSObject &val ) const;
    virtual void check( QSCheckData * );
private:
    QString ident;
    QSLookupInfo *info;
};

class QSGroupNode : public QSNode {
public:
    QSGroupNode( QSNode *g ) : group( g ) { }
    QSObject rhs( QSEnv * ) const;
    QSReference lhs( QSEnv *env );
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *group;
};

class QSElisionNode : public QSNode {
public:
    QSElisionNode( QSElisionNode *e ) : elision( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSElisionNode *elision;
};

class QSElementNode : public QSNode {
public:
    QSElementNode( QSElisionNode *e, QSNode *n ) :
	list( 0 ), elision( e ), node( n ) { }
    QSElementNode( QSElementNode *l, QSElisionNode *e, QSNode *n )
	: list(  l), elision( e ), node( n ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSElementNode *list;
    QSElisionNode *elision;
    QSNode *node;
};

class QSArrayNode : public QSNode {
public:
    QSArrayNode( QSElisionNode *e )
	: element( 0 ), elision( e ), opt( TRUE ) { }
    QSArrayNode( QSElementNode *ele )
	: element( ele ), elision( 0 ), opt( FALSE ) { }
    QSArrayNode( QSElisionNode *eli, QSElementNode *ele )
	: element( ele ), elision( eli ), opt( TRUE ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSElementNode *element;
    QSElisionNode *elision;
    bool opt;
};

class QSObjectLiteralNode : public QSNode {
public:
    QSObjectLiteralNode( QSNode *l ) : list( l ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *list;
};

class QSPropertyValueNode : public QSNode {
public:
    QSPropertyValueNode( QSNode *n, QSNode *a, QSNode *l = 0 )
	: name( n ), assign( a ), list( l ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *name, *assign, *list;
};

class QSPropertyNode : public QSNode {
public:
    QSPropertyNode( double d ) : numeric( d ) { }
    QSPropertyNode( const QString *s ) : str( *s ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
private:
    double numeric;
    QString str;
};

class QSAccessorNode1 : public QSNode {
public:
    QSAccessorNode1( QSNode *e1, QSNode *e2 ) : expr1( e1 ), expr2( e2 ) { }
    QSReference lhs( QSEnv * );
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr1;
    QSNode *expr2;
};

class QSAccessorNode2 : public QSNode {
public:
    QSAccessorNode2( QSNode *e, const QString *s) : expr( e ), ident( *s ) { }
    QSReference lhs( QSEnv * );
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
    QString ident;
};

class QSArgumentListNode : public QSNode {
public:
    QSArgumentListNode( QSNode *e )
	: list( 0 ), expr( e ) { }
    QSArgumentListNode( QSArgumentListNode *l, QSNode *e )
	: list( l ), expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    QSList *evaluateList( QSEnv * );
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSArgumentListNode *list;
    QSNode *expr;
};

class QSArgumentsNode : public QSNode {
public:
    QSArgumentsNode( QSArgumentListNode *l ) : list( l ) { }
    QSObject rhs( QSEnv * ) const;
    QSList *evaluateList( QSEnv * );
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSArgumentListNode *list;
};

class QSNewExprNode : public QSNode {
public:
    QSNewExprNode( QSNode *e) : expr( e ), args( 0 ) { }
    QSNewExprNode( QSNode *e, QSArgumentsNode *a ) : expr( e ), args( a ) { }
    QSObject rhs( QSEnv * ) const;
    QSObject evaluate( QSEnv * );
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
    QSArgumentsNode *args;
};

class QSFunctionCallNode : public QSStatementNode {
public:
    QSFunctionCallNode( QSNode *e, QSArgumentsNode *a )
	: expr( e ), args( a ) { }
    QSObject rhs( QSEnv * ) const;
#ifdef QSDEBUGGER
    void steppingInto( bool in, QSEnv *env ) const;
    Debugger::Mode previousMode;
#endif
    virtual void check( QSCheckData * );
    virtual QSObject execute( QSEnv * );
    bool deref();
    void ref();
private:
    QSNode *expr;
    QSArgumentsNode *args;
};

class QSEmitNode : public QSNode {
public:
    QSEmitNode( QSNode *e, QSArgumentsNode *a )
	: expr( e ), args( a ) { }
  QSObject rhs( QSEnv * ) const { return QSObject(); }
    virtual void check( QSCheckData * ) { };
    bool deref();
    void ref();
private:
    QSNode *expr;
    QSArgumentsNode *args;
};

class QSPostfixNode : public QSNode {
public:
    QSPostfixNode( QSNode *e, Operator o) : expr( e ), oper( o ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData* );
    bool deref();
    void ref();
private:
    QSNode *expr;
    Operator oper;
};

class QSDeleteNode : public QSNode {
public:
    QSDeleteNode( QSNode *e ) : expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSVoidNode : public QSNode {
public:
    QSVoidNode( QSNode *e ) : expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSTypeOfNode : public QSNode {
public:
    QSTypeOfNode( QSNode *e ) : expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSPrefixNode : public QSNode {
public:
    QSPrefixNode( Operator o, QSNode *e ) : oper( o ), expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    Operator oper;
    QSNode *expr;
};

class QSUnaryPlusNode : public QSNode {
public:
    QSUnaryPlusNode( QSNode *e) : expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSNegateNode : public QSNode {
public:
    QSNegateNode( QSNode *e ) : expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSBitwiseNotNode : public QSNode {
public:
    QSBitwiseNotNode( QSNode *e ) : expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSLogicalNotNode : public QSNode {
public:
    QSLogicalNotNode( QSNode *e ) : expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSMultNode : public QSNode {
public:
    QSMultNode( QSNode *t1, QSNode *t2, char op )
	: term1( t1 ), term2( t2 ), oper( op ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *term1, *term2;
    char oper;
};

class QSAddNode : public QSNode {
public:
    QSAddNode( QSNode *t1, QSNode *t2, char op )
	: term1( t1 ), term2( t2 ), oper( op ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *term1, *term2;
    char oper;
};

class QSShiftNode : public QSNode {
public:
    QSShiftNode( QSNode *t1, Operator o, QSNode *t2 )
	: term1( t1 ), term2( t2 ), oper( o ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *term1, *term2;
    Operator oper;
};

class QSRelationalNode : public QSNode {
public:
    QSRelationalNode( QSNode *e1, Operator o, QSNode *e2 )
	: expr1( e1 ), expr2( e2 ), oper( o ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr1, *expr2;
    Operator oper;
};

class QSEqualNode : public QSNode {
public:
    QSEqualNode( QSNode *e1, Operator o, QSNode *e2 )
	: expr1( e1 ), expr2( e2 ), oper( o ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr1, *expr2;
    Operator oper;
};

class QSBitOperNode : public QSNode {
public:
    QSBitOperNode( QSNode *e1, Operator o, QSNode *e2 )
	: expr1( e1 ), expr2( e2 ), oper( o ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr1, *expr2;
    Operator oper;
};

class QSBinaryLogicalNode : public QSNode {
public:
    QSBinaryLogicalNode( QSNode *e1, Operator o, QSNode *e2 )
	: expr1( e1 ), expr2( e2 ), oper( o ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr1, *expr2;
    Operator oper;
};

class QSConditionalNode : public QSNode {
public:
    QSConditionalNode( QSNode *l, QSNode *e1, QSNode *e2 )
	: logical( l ), expr1( e1 ), expr2( e2 ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *logical, *expr1, *expr2;
};

class QSAssignNode : public QSNode {
public:
    QSAssignNode( QSNode *l, Operator o, QSNode *e )
	: left( l ), oper( o ), expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *left;
    Operator oper;
    QSNode *expr;
};

class QSCommaNode : public QSNode {
public:
    QSCommaNode( QSNode *e1, QSNode *e2 ) : expr1( e1 ), expr2( e2 ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr1, *expr2;
};

class QSStatListNode : public QSStatementNode {
public:
    QSStatListNode( QSStatementNode *s ) : statement( s ), list( 0 ) { }
    QSStatListNode( QSStatListNode *l, QSStatementNode *s )
	: statement( s ), list( l ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSStatementNode *statement;
    QSStatListNode *list;
};

class QSAssignExprNode : public QSNode {
public:
    QSAssignExprNode( QSNode *e ) : expr( e ) { }
    QSObject rhs( QSEnv * ) const;
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSScopeNode : public QSStatementNode {
public:
    QSScopeNode() : scope(0) {}
    virtual void check( QSCheckData * );
    virtual QSObject execute( QSEnv * );
    virtual void checkStatement( QSCheckData * ) = 0;
    virtual QSObject executeStatement( QSEnv * ) = 0;
protected:
    QSBlockScopeClass *scope;
};

class QSBlockNode : public QSScopeNode {
public:
    QSBlockNode( QSStatListNode *s ) : statlist( s ) { }
    QSObject executeStatement( QSEnv * );
    void checkStatement( QSCheckData * );
    bool deref();
    void ref();
private:
    QSStatListNode *statlist;
};

class QSEmptyStatementNode : public QSStatementNode {
public:
    QSEmptyStatementNode() { } // debug
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
};

class QSExprStatementNode : public QSStatementNode {
public:
    QSExprStatementNode( QSNode *e ) : expr( e ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSIfNode : public QSStatementNode {
public:
    QSIfNode( QSNode *e, QSStatementNode *s1, QSStatementNode *s2 )
	: expr( e ), statement1( s1 ), statement2( s2 ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
    QSStatementNode *statement1, *statement2;
};

class QSDoWhileNode : public QSStatementNode {
public:
    QSDoWhileNode( QSStatementNode *s, QSNode *e )
	: statement( s ), expr( e ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSStatementNode *statement;
    QSNode *expr;
};

class QSWhileNode : public QSStatementNode {
public:
    QSWhileNode( QSNode *e, QSStatementNode *s )
	: expr( e ), statement( s ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
    QSStatementNode *statement;
};

class QSForNode : public QSScopeNode {
public:
    QSForNode( QSNode *e1, QSNode *e2, QSNode *e3, QSStatementNode *s ) :
	expr1( e1 ), expr2( e2 ), expr3( e3 ), stat( s ) { }
    QSObject executeStatement( QSEnv * );
    void checkStatement( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr1, *expr2, *expr3;
    QSStatementNode *stat;
};

class QSVarBindingNode;

class QSForInNode : public QSScopeNode {
public:
    QSForInNode( QSNode *l, QSNode *e, QSStatementNode *s )
	: var( 0 ), lexpr( l ), expr( e ), stat( s ) { }
    QSForInNode( QSVarBindingNode *v,
		 QSNode *e, QSStatementNode *s )
	: var( v ), lexpr( 0 ), expr( e ), stat( s ) { }
    QSObject executeStatement( QSEnv * );
    void checkStatement( QSCheckData * );
    bool deref();
    void ref();
private:
    QSVarBindingNode *var;
    QSNode *lexpr, *expr;
    QSStatementNode *stat;
};

class QSContinueNode : public QSStatementNode {
public:
    QSContinueNode() { }
    QSContinueNode( const QString *i ) : ident( *i ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
private:
    QString ident;
};

class QSBreakNode : public QSStatementNode {
public:
    QSBreakNode() { }
    QSBreakNode( const QString *i ) : ident( *i ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
private:
    QString ident;
};

class QSReturnNode : public QSStatementNode {
public:
    QSReturnNode( QSNode *v ) : value( v ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *value;
};

class QSWithNode : public QSScopeNode {
public:
    QSWithNode( QSNode *e, QSStatementNode *s ) : expr( e ), stat( s ) { }
    QSObject executeStatement( QSEnv * );
    void checkStatement( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
    QSStatementNode *stat;
};

class QSCaseClauseNode: public QSNode {
public:
    QSCaseClauseNode( QSNode *e, QSStatListNode *l ) : expr( e ), list( l ) { }
    QSObject rhs( QSEnv * ) const;
    QSObject evalStatements( QSEnv *env);
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
    QSStatListNode *list;
};

class QSClauseListNode : public QSNode {
public:
    QSClauseListNode( QSCaseClauseNode *c ) : cl( c ), nx( 0 ) { }
    QSClauseListNode* append( QSCaseClauseNode *c );
    QSCaseClauseNode *clause() const { return cl; }
    QSClauseListNode *next() const { return nx; }
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSObject evaluate( QSEnv * ) { /* should never be called */ return QSObject(); }
    QSObject rhs( QSEnv *env ) const { /* should never be called */ return env->createUndefined(); }
    QSCaseClauseNode *cl;
    QSClauseListNode *nx;
};

class QSCaseBlockNode: public QSNode {
public:
    QSCaseBlockNode( QSClauseListNode *l1, QSCaseClauseNode *d,
		     QSClauseListNode *l2 )
	: list1( l1 ), def( d ), list2( l2 ) { }
    QSObject evalBlock( QSEnv *env, const QSObject& input );
    virtual void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSObject evaluate( QSEnv * ) { /* should never be called */ return QSObject(); }
    QSObject rhs( QSEnv *env ) const { /* should never be called */ return env->createUndefined(); }
    QSClauseListNode *list1;
    QSCaseClauseNode *def;
    QSClauseListNode *list2;
};

class QSSwitchNode : public QSStatementNode {
public:
    QSSwitchNode( QSNode *e, QSCaseBlockNode *b ) : expr( e ), block( b ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
    QSCaseBlockNode *block;
};

class QSLabelNode : public QSStatementNode {
public:
    QSLabelNode( const QString *l, QSStatementNode *s )
	: label( *l ), stat( s ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QString label;
    QSStatementNode *stat;
};

class QSThrowNode : public QSStatementNode {
public:
    QSThrowNode( QSNode *e ) : expr( e ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSNode *expr;
};

class QSCatchNode : public QSScopeNode {
public:
    QSCatchNode( QString *i, QSStatementNode *b ) : ident( *i ), block( b ) { }
    QSObject executeStatement( QSEnv *env );
    void checkStatement( QSCheckData * );
    void setException( const QSObject &e ) { exception = e; }
    bool deref();
    void ref();
private:
    QString ident;
    QSStatementNode *block;
    int index;
    QSObject exception;
};

class QSFinallyNode : public QSStatementNode {
public:
    QSFinallyNode( QSStatementNode *b ) : block( b ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSStatementNode *block;
};

class QSTryNode : public QSStatementNode {
public:
    QSTryNode( QSStatementNode *b, QSNode *c = 0, QSNode *f = 0 )
	: block( b ), _catch( (QSCatchNode*)c ),
	  _final( (QSFinallyNode*)f ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSStatementNode *block;
    QSCatchNode *_catch;
    QSFinallyNode *_final;
};

class QSParameterNode : public QSNode {
public:
    QSParameterNode( const QString *i, QSTypeNode *t )
	: id( *i ), typ( t ), next( 0 ) { }
    QSParameterNode *append( const QString *i, QSTypeNode *t );
    QSObject rhs( QSEnv * ) const;
    QString ident() { return id; }
    QSParameterNode *nextParam() { return next; }
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QString id;
    QSTypeNode *typ;
    QSParameterNode *next;
};

// inherited by QSProgramNode
class QSFunctionBodyNode : public QSStatementNode {
public:
    QSFunctionBodyNode( QSSourceElementsNode *s );
    ~QSFunctionBodyNode();
    void check( QSCheckData * );
    QSObject execute( QSEnv * );
    void setScopeDefinition( QSFunctionScopeClass * s ) { scopeDef = s; }
    QSFunctionScopeClass * scopeDefinition() const { return scopeDef; }
    int index() const { return idx; }
    void setIndex( int i ) { idx = i; }
    bool deref();
    void ref();
 protected:
    QSSourceElementsNode *source;
    QSFunctionScopeClass * scopeDef;
    int idx;
private:
    static int count;
};

class QSFuncDeclNode : public QSStatementNode {
public:
    QSFuncDeclNode( const QString *i, QSParameterNode *p,
		    QSTypeNode *t, QSFunctionBodyNode *b )
	: ident( *i ), param( p ), rtype( t ), body( b ), attrs( 0 ) { }
    void setAttributes( QSAttributeNode *a ) { attrs = a; }
    QSObject execute( QSEnv * ) { /* empty */ return QSObject(); }
    void check( QSCheckData * );
    const QString identifier() const { return ident; }
    bool deref();
    void ref();
private:
    QString ident;
    QSParameterNode *param;
    QSTypeNode *rtype;
    QSFunctionBodyNode *body;
    QSAttributeNode *attrs;
};

class QSFuncExprNode : public QSNode {
public:
    QSFuncExprNode( QSParameterNode *p, QSFunctionBodyNode *b )
	: param( p ), body( b ) { }
    QSObject rhs( QSEnv * ) const;
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSParameterNode *param;
    QSFunctionBodyNode *body;
};

class QSClassDefNode;

class QSSourceElementNode : public QSStatementNode {
public:
    QSSourceElementNode( QSStatementNode *s ) : statement( s ) { }
    void check( QSCheckData * );
    QSObject execute( QSEnv * );
    void deleteStatements();
    bool deref();
    void ref();
private:
    QSStatementNode *statement;
};

class QSSourceElementsNode : public QSStatementNode {
public:
    QSSourceElementsNode( QSSourceElementNode *s1 ) {
	element = s1; elements = 0; }
    QSSourceElementsNode( QSSourceElementsNode *s1, QSSourceElementNode *s2 )
    { elements = s1; element = s2; }
    void check( QSCheckData * );
    QSObject execute( QSEnv * );
    void deleteStatements();
    bool deref();
    void ref();
private:
    QSSourceElementNode *element;
    QSSourceElementsNode *elements;
};

class QSProgramNode : public QSFunctionBodyNode {
public:
    QSProgramNode( QSSourceElementsNode *s );
    ~QSProgramNode();
    void check( QSCheckData * );
    void deleteGlobalStatements();
    static QSProgramNode* last() { return prog; }
private:
    static QSProgramNode *prog;
};

class QSClassDefNode : public QSStatementNode {
public:
    QSClassDefNode( const QString *i, QSTypeNode *t, QSFunctionBodyNode *b )
	: ident( *i ), type( t ), body( b ), cldef( 0 ), attrs( 0 ) { }
    void setAttributes( QSAttributeNode *a ) { attrs = a; }
    void check( QSCheckData * );
    QSObject execute( QSEnv * );
    QString identifier() const { return ident; }
    void setClassDefinition(QSClassClass *cl) { cldef = cl; }
    bool deref();
    void ref();
private:
    QString ident;
    QSTypeNode *type;
    QSFunctionBodyNode *body;
    QSClassClass *cldef;
    QSAttributeNode *attrs;
};

class QSTypeNode : public QSNode {
public:
    QSTypeNode( const QString *i ) : ident( *i ) { }
    void check( QSCheckData * );
    QSObject rhs( QSEnv * ) const;
    QString identifier() const { return ident; }
private:
    QString ident;
};

class QSTypedVarNode : public QSNode {
public:
    QSTypedVarNode( const QString *i, QSTypeNode *t )
	: ident( *i ), type( t ) { }
    QSObject rhs( QSEnv * ) const;
    QString identifier() const { return ident; }
    QSTypeNode *typeNode() const { return type; }
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QString ident;
    QSTypeNode *type;
};

class QSVarBindingNode : public QSNode {
public:
    QSVarBindingNode( QSTypedVarNode *v, QSNode *a )
	: var( v ), assign( (QSAssignNode*)a ), idx( 0 ) { }
    void declare( QSEnv *env ) const;
    void check( QSCheckData * );
    int index() const { return idx; }
    bool deref();
    void ref();
private:
    QSObject rhs( QSEnv *env ) const { declare( env ); return QSObject(); }
    QSTypedVarNode *var;
    QSAssignNode *assign;
    int idx;
};

class QSVarBindingListNode : public QSNode {
public:
    QSVarBindingListNode( QSVarBindingListNode *l, QSVarBindingNode *b )
	: list( l ), binding( b ) { }
    void declare( QSEnv *env ) const;
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QSObject rhs( QSEnv *env ) const { declare( env ); return QSObject(); }
    QSVarBindingListNode *list;
    QSVarBindingNode *binding;
};

class QSVarDefNode : public QSStatementNode {
public:
    QSVarDefNode( int k, QSVarBindingListNode *l )
	: kind( k ), list( l ), attrs( 0 ) { }
    void setAttributes( QSAttributeNode *a ) { attrs = a; }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    int kind;
    QSVarBindingListNode *list;
    QSAttributeNode *attrs;
};

class QSPackageNode : public QSStatementNode {
public:
    QSPackageNode( const QString *s, QSStatementNode *b )
	: package( *s ), block( b ) { }
    QSObject execute( QSEnv * ) { return QSObject(); }
    void check( QSCheckData * );
    bool deref();
    void ref();
private:
    QString package;
    QSStatementNode *block;
};

class QSImportNode : public QSStatementNode {
public:
    QSImportNode( const QString *s ) : package( *s ) { }
    QSObject execute( QSEnv * );
    void check( QSCheckData * );
private:
    QString package;
};


/*!
  Utility class for use with resolvenode's by index lookup.
*/
class QSLookupInfo {
public:
    QSLookupInfo( int lvl, const QSMember &mem )
	: level(lvl), member(mem) { }
    int level;
    QSMember member;
};

class QSNodeList : public QPtrList<QSNode>
{
public:
    QSNodeList() : QPtrList<QSNode>() { }
};

#endif
