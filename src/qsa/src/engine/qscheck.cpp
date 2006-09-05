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

#include "qscheck.h"
#include "qsnodes.h"
#include "qsenv.h"

// #define QSDEBUG_CHECK

#ifdef QSDEBUG_CHECK
#include <typeinfo>
#endif

QSCheckData::QSCheckData( QSEnv *e, QSGlobalClass *g )
    : currenv( e ),
      lattrs( AttributeNone ),
      ltype( 0 ),
      vbCount( 0 ),
      switchLevel(0),
      noGlobStatements( FALSE ),
      directLookup( TRUE )
{
    if ( g )
 	scopeStack.push_front( g );
}

QSCheckData::~QSCheckData()
{
    // sanity checks
    Q_ASSERT( lablist.isEmpty() );
}

void QSCheckData::enterClass( QSClass *c )
{

    scopeStack.push_front( QSScopeResolution( c ) );
}

void QSCheckData::leaveClass()
{
    Q_ASSERT( inClass() );
    scopeStack.pop_front();
    //    clstack.removeLast();
}

void QSCheckData::registerType( QSClass *t )
{
    Q_ASSERT( !t->asClass() );
    QSClass * scope = currentScope();
    // ### Uncertain as to use identifier or name here?
    QSMember member;
    Q_ASSERT( !scope->member( 0, t->identifier(), &member ) );
    scope->addStaticVariableMember( t->identifier(),
				    env()->typeClass()->createType(t),
				    AttributeExecutable );
}

QSClass *QSCheckData::typeInfo( const QString &name ) const
{
    QSMember member;
    QSScopeResolutionList::const_iterator it = scopeStack.begin();
    QSObject tmp;
    tmp.setVal( (int) 0 );
    int offset;
    while( it!=scopeStack.end() ) {
	offset = 0;
	tmp.setType( (*it).cl );
	const QSClass *cl;
	if( (*it).cl->name() == QString::fromLatin1("QObject") ) {
	  cl = env()->thisValue().resolveMember( name, &member, tmp.objectType(),
						 &offset );

	} else {
	    cl = tmp.resolveMember( name, &member, tmp.objectType(), &offset );
	    tmp.invalidate();
	}
	if( cl ) {
	    /* Avoid calls to deref for QSWrapperClass, when tmp is destroyed */
	    tmp.invalidate();
	    QSObject o( cl->fetchValue( 0, member ) );
	    if ( o.objectType() == env()->typeClass() )
		return QSTypeClass::classValue(&o);
	    else
		return 0;
	}
	it++;
    }
    return 0;
}


/*!
  Returns a pointer to the innermost class based on the current
  position in the scope. For global scope the global class is returned.
*/
QSClass * QSCheckData::innermostClass() const
{
    QSScopeResolution scope;
    QSScopeResolutionList::const_iterator it = scopeStack.begin();
    while( it!=scopeStack.end() ) {
	scope = *it;
	if( scope.isClassScope() || scope.isGlobalScope() ) {
	    return scope.cl;
	}
	it++;
    }
    Q_ASSERT( 0 /* should never get here */ );
    return 0;
}

/*!
  Returns the current function if the scope stack top is
  a function. 0 otherwise.
*/

QSFunctionScopeClass * QSCheckData::currentFunction() const
{
    if( scopeStack.size()>0 && scopeStack.first().isFunctionScope() ) {
	return (QSFunctionScopeClass*) scopeStack.first().cl;
    }
    return 0;
}

/*!
  Returns the current class if the scope stack top is a class. 0 otherwise
*/
QSClass * QSCheckData::currentClass() const
{
    if( scopeStack.size()>0 && scopeStack.first().isClassScope() ) {
	return (QSClass*) scopeStack.first().cl;
    }
    return 0;
}

/*!
  Returns the class representing the current scope. 0 if there are no
  objects on the scope stack
*/
QSClass * QSCheckData::currentScope() const
{
    Q_ASSERT( !scopeStack.isEmpty() );
    return scopeStack.first().cl;
}

bool QSCheckData::insideClass( const QString &name ) const
{
    QSScopeResolutionList::const_iterator it = scopeStack.begin();
    while( it!=scopeStack.end() ) {
	if( (*it).isClassScope() && (*it).name()==name ) {
	    return TRUE;
	}
	it++;
    }
    return false;
}


/*!
  Resolves the globally qualified name of \name, including
  package / class / function resolution.
*/
QString QSCheckData::globalName( const QString &name ) const
{
    QStringList lst(name);
    QSScopeResolutionList::const_iterator it = scopeStack.begin();
    while( it!=scopeStack.end() ) {
	lst.prepend( (*it).name() );
	it++;
    }
    return lst.join( QString::fromLatin1(".") );
}

void QSCheckData::enterFunction( QSFunctionScopeClass * func )
{
//     qDebug( "QSCheckData::enterFunction( %s ) into %s",
// 	    func->identifier().ascii(),
// 	    currentScope()->identifier().ascii() );
    scopeStack.push_front( func );
}

void QSCheckData::leaveFunction()
{
    Q_ASSERT( inFunction() );
    scopeStack.pop_front();
}

/*!
  Can we have a return statement at this point, i.e. are we inside
  a function ?
 */

bool QSCheckData::canReturn() const
{
    QSScopeResolutionList::const_iterator it = scopeStack.begin();
    while ( it != scopeStack.end() ) {
	if ( (*it).isFunctionScope() )
	    return TRUE;
	else if ( !(*it).isBlockScope() )
	    return FALSE;
	else
	    ++it;
    }
    return FALSE;
}

void QSCheckData::enterBlock( QSBlockScopeClass * cl )
{
    scopeStack.push_front( cl );
}

void QSCheckData::leaveBlock()
{
    scopeStack.pop_front();
}

void QSCheckData::setLabel( const QString &label )
{
    lastlab = label;
}

QString QSCheckData::currentLabel() const
{
    return lastlab;
}

bool QSCheckData::seenLabel( const QString &label ) const
{
    return lablist.contains( label );
}

void QSCheckData::clearLabel()
{
    lastlab = "";
}

void QSCheckData::enterLoop( const QString &label )
{
    lablist.append( label );
}

void QSCheckData::leaveLoop()
{
    Q_ASSERT( inLoop() );
    lablist.remove( --lablist.end() );
}

void QSCheckData::enterSwitch()
{
    ++switchLevel;
}

void QSCheckData::leaveSwitch()
{
    Q_ASSERT( switchLevel>0 );
    --switchLevel;
}


void QSCheckData::enterPackage( const QString &name )
{
    Q_ASSERT( inGlobal() );
    currpack = name;
}

void QSCheckData::leavePackage()
{
    Q_ASSERT( inPackage() );
    currpack = QString::null;
}


bool QSCheckData::inGlobal() const
{
    return scopeStack.size()>0 && scopeStack.first().isGlobalScope();
}

void QSCheckData::addError( const QSNode *node, QSErrorCode code,
			    const QString &msg)
{
    ecodes.append( code );
    elines.append( node->lineNo() );
    emsgs.append( QString::fromLatin1("Error: ") + msg );
}

void QSCheckData::addError( const QSNode *node, const QString &msg )
{
    addError( node, QSErrGeneral, msg );
}

void QSCheckData::addWarning( const QSNode *node, QSErrorCode code,
			    const QString &msg)
{
    ecodes.append( code );
    elines.append( node->lineNo() );
    emsgs.append( QString::fromLatin1("Warning: ") + msg );
}

void QSCheckData::addWarning( const QSNode *node, const QString &msg )
{
    addWarning( node, QSErrGeneral, msg );
}


/////////////////// QSNode::check() implementations //////////////////////////

void QSNode::check( QSCheckData * )
{
    // ### nothing. should be made pure virtual one day
#ifdef QSDEBUG_CHECK
    qDebug( "check() in %s", typeid(*this).name() );
#endif
}

/*! When parsing libraries or form code we don't want to have anything
  but class, function and variable declarations at global level. This
  function is called by all other statements and will result in an error
  if we currently are in global level.
*/

void QSStatementNode::checkIfGlobalAllowed( QSCheckData *c )
{
    if ( c->globalStatementsForbidden() && c->inGlobal() )
	c->addError( this, QSErrPositionGlobal,
		     QString::fromLatin1("Global statements detected. May have unwanted "
                                         "side effects during execution") );
}

void QSProgramNode::check( QSCheckData *c )
{
    if ( source )
	source->check( c );
    QSClass *cl = c->currentScope();
    if( cl->numVariables() < c->varBlockCount() ) {
	cl->setNumVariables( c->varBlockCount() );
    }
    ( (QSInstanceData*) c->env()->currentScope().shVal() )->
	ensureSize( c->currentScope()->numVariables(),
		    c->env()->createUndefined() );
}

void QSFunctionBodyNode::check( QSCheckData *c )
{
    int attrs = c->lastAttributes();
    c->setLastAttributes(AttributeNone);
    if ( source )
	source->check( c );
    c->setLastAttributes(attrs);
}

void QSSourceElementNode::check( QSCheckData *c )
{
    statement->check( c );
}

void QSSourceElementsNode::check( QSCheckData *c )
{
    if ( elements )
	elements->check( c );
    c->clearLabel();
    element->check( c );
}

void QSStatListNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
    if ( list )
	list->check( c );
    c->clearLabel();
    if ( c->hasError() )
	return;
    statement->check( c );
}

void QSScopeNode::check( QSCheckData *c )
{
    QSBlockScopeClass * cl = new QSBlockScopeClass( c->env() );

    cl->setNumVariables( c->currentScope()->numVariables() );
    cl->setEnclosingClass( c->currentScope() );
    c->enterBlock( cl );

    checkStatement( c );

    c->leaveBlock();
    scope = cl;

    if( cl->numVariables()>c->varBlockCount() )
	c->setVarBlockCount( cl->numVariables() );
}


void QSBlockNode::checkStatement( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
    if ( c->hasError() )
	return;
    if ( statlist )
	statlist->check( c );
}

void QSAssignExprNode::check( QSCheckData *c )
{
    expr->check( c );
}

void QSEmptyStatementNode::check( QSCheckData * )
{
    //    checkIfGlobalAllowed( c );
}

void QSExprStatementNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
    expr->check( c );
}

void QSIfNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );

    expr->check( c );
    statement1->check( c );
    if ( statement2 )
	statement2->check( c );
}

void QSDoWhileNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );

    expr->check( c );
    c->enterLoop( c->currentLabel() );
    statement->check( c );
    c->leaveLoop();
}

void QSWhileNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );

    expr->check( c );
    c->enterLoop( c->currentLabel() );
    statement->check( c );
    c->leaveLoop();
}

void QSForNode::checkStatement( QSCheckData *c )
{
    checkIfGlobalAllowed( c );

    if ( expr1 )
	expr1->check( c );
    if ( expr2 )
	expr2->check( c );
    if ( expr3 )
	expr3->check( c );
    if ( c->hasError() )
	return;

    c->enterLoop( c->currentLabel() );
    stat->check( c );
    c->leaveLoop();
}

void QSForInNode::checkStatement( QSCheckData *c )
{
    checkIfGlobalAllowed( c );

    if ( var )
	var->check( c );
    if ( lexpr )
	lexpr->check( c );
    expr->check( c );
    if ( c->hasError() )
	return;

    c->enterLoop( c->currentLabel() );
    stat->check( c );
    c->leaveLoop();
}

void QSContinueNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
    if ( !c->inLoop() )
	c->addError( this, QString::fromLatin1("'continue' can only be used inside of iteration"
                                               " statements") );
    if ( !ident.isEmpty() && !c->seenLabel( ident ) )
	c->addError( this, QString::fromLatin1( "Unknown label '%1'" ).arg( ident ) );
}

void QSBreakNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
    if ( !c->inLoop() && !c->inSwitch() )
	c->addError( this, QString::fromLatin1("'break' can only be used inside of iteration or "
                                               "switch statements") );
    // ### true if nested loops & switch ?
    if ( !ident.isEmpty() && !c->seenLabel( ident ) )
	c->addError( this, QString::fromLatin1( "Unknown label '%1'" ).arg( ident ) );
}

void QSReturnNode::check( QSCheckData *c )
{
    if ( !c->canReturn() )
	c->addError( this, QString::fromLatin1("Can only return from inside a function") );
    if ( value )
	value->check( c );
}

void QSWithNode::checkStatement( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
    bool dlenabled = c->directLookupEnabled();
    c->setDirectLookupEnabled( false );
    expr->check( c );
    stat->check( c );
    c->setDirectLookupEnabled( dlenabled );
}

void QSSwitchNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );

    expr->check( c );

    c->enterSwitch();
    block->check( c );
    c->leaveSwitch();
}

void QSLabelNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );

    c->pushLabel( label );

    if( stat )
	stat->check( c );

    c->popLabel();
}

void QSThrowNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
}

void QSCatchNode::checkStatement( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
    QSClass *cl = c->currentScope();
    index = cl->addVariableMember( ident, AttributeNone );
    block->check( c );
}

void QSFinallyNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
    block->check( c );
}

void QSTryNode::check( QSCheckData *c )
{
    checkIfGlobalAllowed( c );
    block->check( c );
    if ( _catch )
	_catch->check( c );
    if ( _final )
	_final->check( c );
}

void QSTypedVarNode::check( QSCheckData *c )
{
    if ( type )
	type->check( c );
}

void QSVarBindingNode::check( QSCheckData *c )
{
    var->check( c );
    if ( assign )
	assign->check( c );

    if (!c->directLookupEnabled()) {
        c->addError( this, QString::fromLatin1( "QSA does not support declaring variables inside "
                                                "a 'with' block" ));

        return;
    }

    int attrs = c->lastAttributes();
    QSClass * cl = c->currentScope();
    QSMember m;
    if ( cl->member( 0, var->identifier(), &m ) ) {
	if( cl->asClass() ) {
	    c->addError( this, QString::fromLatin1( "Variable '%1' has already been "
					"declared in class '%2'" )
			 .arg( var->identifier() )
			 .arg( cl->identifier() ) );
	    return;
	}
	m = QSMember( QSMember::Variable, 0, attrs );
	cl->replaceMember( var->identifier(), &m );
	idx = m.index();
    } else {
	idx = cl->addVariableMember( var->identifier(), attrs );
    }

    // store pointer to init node if this is a class member
    QSClassClass *clcl = cl->asClass();
    if ( clcl ) {
	if( attrs & AttributeStatic )
	    clcl->addStaticInitializer( assign );
	else
	    clcl->addMemberInitializer( assign );
	idx = -1; // Disable the variable binding node.
    }
}

void QSVarBindingListNode::check( QSCheckData *c )
{
    if ( list )
	list->check( c );
    binding->check( c );
}

void QSVarDefNode::check( QSCheckData *c )
{
    if ( attrs )
	attrs->check( c );
    else
	c->setLastAttributes( AttributeNone );

    if( kind==1 ) // const definition
	c->setLastAttributes( c->lastAttributes()|AttributeNonWritable );

    int as = c->lastAttributes();
    QSClass *cl = c->currentClass();
    if ( cl && !c->inFunction() ) {
	// class member
    } else {
	if ( as & AttributeStatic ) {
	    c->addError( this, QSErrAttrStaticContext,
			 QString::fromLatin1( "Static variables cannot be declared outside "
				  "a class" ) );
	    return;
	}
    }
    list->check( c );

    // unset attributes
    c->setLastAttributes( AttributeNone );
}

void QSParameterNode::check( QSCheckData * c ) {
    Q_ASSERT( c->inFunction() );

    QSClass * cl = c->currentScope();

    if (id == QString::fromLatin1("arguments")) {
        c->addError(this, QString::fromLatin1("Parameter 'arguments' is a predefined value in function '%1'. Use different parameter name.")
                    .arg(cl->identifier()));
        return;
    }

    QSMember m;
    if( cl->member( 0, id, &m ) ) {
	c->addError( this, QString::fromLatin1( "Parameter '%1' already declared in function '%2'" )
		     .arg( id )
		     .arg( cl->identifier() ) );
	return;
    }

    cl->addVariableMember( id, AttributeNone );

    if( next )
	next->check( c );

    // create a member from id
    // add member to current scope
    // next.check();
}

void QSFuncDeclNode::check( QSCheckData *c )
{
//     qDebug( "Function noticed: " + c->globalName(ident) );
    if ( attrs )
	attrs->check( c );
    else
	c->setLastAttributes( AttributeNone );
    int as = c->lastAttributes();
    QSClass * cl = c->currentScope();
    Q_ASSERT( cl );

    if ( (as&AttributeStatic) && cl->name() != QString::fromLatin1("Class") ) {
	c->addError( this, QSErrAttrStaticContext,
		     QString::fromLatin1( "Function '%1' cannot be declared static "
			      "outside a class" ).arg( ident ) );
	return;
    }

    // A bit of magic fail early when trying to overwrite a context.
    if (c->inGlobal()) {
        QSObject object = c->env()->globalObject().get(ident);
        if (object.isValid()) {
            if (object.objectType()->name() == QString::fromLatin1("QObject")) {
                c->addError(this, QString("Cannot declare function '%1', already a global object "
                                          "present with same name").arg(ident));
                return;
            }
        }
    }

    QSMember m;
    m.setPrivate( as&AttributePrivate );

    if ( cl->member( 0, ident, &m ) ) {
        QSMember mem( body, as );
        cl->replaceMember( ident, &mem );
    } else {
	cl->addFunctionMember( ident, body, as );
    }

    int tmpVarBlockCount = c->varBlockCount();
    c->setVarBlockCount( 0 );
    QSFunctionScopeClass * fscope = new QSFunctionScopeClass( c->env()->objectClass(), this );
    fscope->setEnclosingClass( cl );
    body->setScopeDefinition( fscope );
    fscope->setFunctionBodyNode(body);
    c->enterFunction( fscope );
    if( param )
	param->check( c );
    body->check( c );
    c->leaveFunction();

    if( c->varBlockCount()>fscope->numVariables() )
	fscope->setNumVariables( c->varBlockCount() );
    c->setVarBlockCount( tmpVarBlockCount );

    // Calculate the number of arguments
    int count = 0;
    QSParameterNode * node = param;
    while( node ) {
	count++;
	node = node->nextParam();
    }
    fscope->setNumArguments( count );

    // unset attributes
    c->setLastAttributes( AttributeNone );
}

void QSFuncExprNode::check( QSCheckData *c )
{
    // ### Check attributes and refactor code with QSFuncDeclNode
    QSClass * cl = c->currentScope();
    QSFunctionScopeClass * fscope = new QSFunctionScopeClass( c->env()->objectClass()  );
    fscope->setEnclosingClass( cl );
    body->setScopeDefinition( fscope );
    fscope->setFunctionBodyNode(body);
    c->enterFunction( fscope );
    if( param )
	param->check( c );
    body->check( c );
    c->leaveFunction();

    // Calculate the number of arguments
    int count = 0;
    QSParameterNode * node = param;
    while( node ) {
	count++;
	node = node->nextParam();
    }
    fscope->setNumArguments( count );
}

void QSClassDefNode::check( QSCheckData *c )
{
//     qDebug( "Class noticed: " + c->globalName( ident ) );

    // forward declaration ?
    if ( !body ) {
	c->addWarning( this, QString::fromLatin1("Forward declarations not supported, yet") );
	return;
    }

    if ( attrs )
	attrs->check( c );
    else
	c->setLastAttributes( AttributeNone );
    int as = c->lastAttributes();

    // base class
    QSClass *b = c->env()->objectClass();
    if ( type ) {
	type->check( c );
	if ( c->hasError() )
	    return;
	Q_ASSERT( c->lastType() );
	b = c->lastType();
	// try to cast
	if ( b->isFinal() ) {
	    c->addError( this, QSErrClassBaseFinal,
			 QString::fromLatin1("Base class has been declared final") );
	    return;
	}
    }

    // create new class description
    QSClass *scope = c->currentScope();
    if ( scope->name()==QString::fromLatin1("Class") ) {
	if ( !(as&AttributeStatic) ) {
	    c->addError( this, QSErrNestedClass,
			 QString::fromLatin1( "Nested class '%1' in class '%2' must be "
				  "declared static" )
			 .arg( ident ).arg( scope->identifier() ) );
	    return;
	}
    } else if ( as&AttributeStatic ) {
        c->addError( this, QSErrAttributeConflict,
		     QString::fromLatin1( "Static class '%1' cannot be declared outside "
			      "a class" ).arg( ident ) );
	return;
    } else if ( scope->name()==QString::fromLatin1("FunctionScope") ) {
	c->addError( this, QSErrNestedClass,
		     QString::fromLatin1( "Class '%1' cannot be declared inside function '%2'" )
		     .arg( ident ).arg( scope->identifier() ) );
	return;
    }

    QSClass *absBase = c->env()->classByIdentifier(ident);
    bool usesAbstractBase = absBase && absBase->name() == QString::fromLatin1("AbstractBase");

    QSClassClass *cl = new QSClassClass( b, as, ident );
    ref(); // Compensate for setClassDefNode below. Dereffed in QSClassClass destructor.
    cl->setClassDefNode( this );
    cl->setEnclosingClass( scope );

    QSMember member;
    QSObject tobj = c->env()->typeClass()->createType(cl);
    if( scope->member( 0, cl->identifier(), &member ) ) {
	scope->replaceMember( cl->identifier(), &member, tobj );
    } else {
	scope->addStaticVariableMember( cl->identifier(), tobj,
					AttributeExecutable );
    }

    c->enterClass( cl );
    body->check( c );
    c->leaveClass();

    // if there is a function with the same name it becomes the constructor
    QSMember ctor;
    if ( cl->member( 0, ident, &ctor ) ) {
	if ( ctor.isExecutable() ) {
	    cl->setHasDefaultConstructor( TRUE );
	} else {
	    qWarning( "Constructor is no function" ); // pedantic
	}
    }

    cl->setClassBodyNode( body );
    QSFunctionScopeClass *fs = new QSFunctionScopeClass( c->env()->objectClass() );
    fs->setEnclosingClass( cl );
    body->setScopeDefinition( fs );
    fs->setFunctionBodyNode(body);

    // will be used in execute()
    this->cldef = cl;

    // If the classname exists in env and is a previously undefined type, we have to
    // replace it with this one.
    if (usesAbstractBase) {
	((QSAbstractBaseClass*)absBase)->replace(cl);
    }

}

// has to be in sync with QSAttribute enum (qsglobal.h)
static const char * const attrStr[] = {
    "",
    "abstract",
    "final",
    "private",
    "public",
    "static",
    "true",
    "false"
    "constructor"
};

/*! Map a QSAttribute value (2^n) to index n for above string table
 */
static int attrNo( int abit )
{
    if ( abit == 0 )
	return 0;
    int no = -1;
    while ( abit )
	no++, abit >>= 1;
    return no;
}

void QSAttributeNode::check( QSCheckData *c )
{
    int asum = AttributeNone;
    QValueList<QSAttribute>::ConstIterator it = attrs.begin();
    for ( ; it != attrs.end(); ++it ) {
	if ( asum & *it ) {
	    QString msg = QString::fromLatin1( "Duplicate attribute `%1'" )
			  .arg( QString::fromLatin1(attrStr[attrNo(*it)]) );
	    c->addWarning( this, QSErrAttributeDuplication, msg );
	}
	asum |= *it;
    }
    // check for invalid combinations
    if ( ( asum & AttributePrivate && asum & AttributePublic ) ||
	 ( asum & AttributeTrue && asum & AttributeFalse ) ) {
	// ### more checks, more detailed error message
	c->addError( this, QSErrAttributeConflict, QString::fromLatin1("Conflicting attributes") );
    }
    c->setLastAttributes( asum );
}

void QSTypeNode::check( QSCheckData *c )
{
    QSClass *t = c->typeInfo( ident );
    // Create dummy to allow cross referencing classes between files.
    if (!t) {
	t = c->env()->classByIdentifier(ident);
	if (!t)
	    t = new QSAbstractBaseClass(c->env()->objectClass(), ident);
    }
    c->setLastType( t );
}

void QSPackageNode::check( QSCheckData *c )
{
    if ( c->inPackage() )
	c->addError( this, QString::fromLatin1("Cannot defined nested packages") );
    // possibly too strict
    if ( !c->inGlobal() )
	c->addError( this, QString::fromLatin1("Packages can only be defined at global scope") );
    c->enterPackage( package );
    block->check( c );
    c->leavePackage();

}

void QSImportNode::check( QSCheckData *c )
{
    // possibly too strict
    if ( !c->inGlobal() )
	c->addError( this, QString::fromLatin1("Packages can only be imported at global scope") );
}

void QSAssignNode::check( QSCheckData *c )
{
    expr->check( c );
    left->check( c );
}

void QSGroupNode::check( QSCheckData *c )
{
    if( group )
	group->check( c );
}

void QSElisionNode::check( QSCheckData *c )
{
    if( elision )
	elision->check( c );
}

void QSElementNode::check( QSCheckData *c )
{
    if( list )
	list->check( c );
    if( elision )
	elision->check( c );
    if( node )
	node->check( c );
}

void QSArrayNode::check( QSCheckData *c )
{
    if( element )
	element->check( c );
    if( elision )
	elision->check( c );
}

void QSObjectLiteralNode::check( QSCheckData *c )
{
    if( list )
	list->check( c );
}

void QSPropertyValueNode::check( QSCheckData *c )
{
    if( name )
	name->check( c );
    if( assign )
	assign->check( c );
    if( list )
	list->check( c );
}

void QSAccessorNode1::check( QSCheckData *c )
{
    if( expr1 )
	expr1->check( c );
    if( expr2 )
	expr2->check( c );
}

void QSAccessorNode2::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}

void QSNewExprNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
    if( args )
	args->check( c );
}

void QSFunctionCallNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
    if( args )
	args->check( c );
}

void QSDeleteNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}

void QSVoidNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}

void QSTypeOfNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}

void QSPrefixNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}

void QSUnaryPlusNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}

void QSBitwiseNotNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}

void QSLogicalNotNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}

void QSAddNode::check( QSCheckData *c )
{
    if( term1 )
	term1->check( c );
    if( term2 )
	term2->check( c );
}

void QSMultNode::check( QSCheckData *c )
{
    if( term1 )
	term1->check( c );
    if( term2 )
	term2->check( c );
}

void QSShiftNode::check( QSCheckData *c )
{
    if( term1 )
	term1->check( c );
    if( term2 )
	term2->check( c );
}

void QSRelationalNode::check( QSCheckData *c )
{
    if( expr1 )
	expr1->check( c );
    if( expr2 )
	expr2->check( c );
}

void QSEqualNode::check( QSCheckData *c )
{
    if( expr1 )
	expr1->check( c );
    if( expr2 )
	expr2->check( c );
}

void QSBitOperNode::check( QSCheckData *c )
{
    if( expr1 )
	expr1->check( c );
    if( expr2 )
	expr2->check( c );
}

void QSBinaryLogicalNode::check( QSCheckData *c )
{
    if( expr1 )
	expr1->check( c );
    if( expr2 )
	expr2->check( c );
}

void QSConditionalNode::check( QSCheckData *c )
{
    if( logical )
	logical->check( c );
    if( expr1 )
	expr1->check( c );
    if( expr2 )
	expr2->check( c );
}

void QSCommaNode::check( QSCheckData *c )
{
    if( expr1 )
	expr1->check( c );
    if( expr2 )
	expr2->check( c );
}

void QSClauseListNode::check( QSCheckData *c )
{
    if( cl )
	cl->check( c );
    if( nx )
	nx->check( c );
}

void QSCaseBlockNode::check( QSCheckData *c )
{
    if( list1 )
	list1->check( c );
    if( def )
	def->check( c );
    if( list2 )
	list2->check( c );
}

void QSArgumentListNode::check( QSCheckData *c )
{
    if( list )
	list->check( c );
    if( expr )
	expr->check( c );
}

void QSArgumentsNode::check( QSCheckData *c )
{
    if( list )
	list->check( c );
}

void QSNegateNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}

void QSCaseClauseNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
    if( list )
	list->check( c );
}

void QSNumberNode::check( QSCheckData * )
{
}

void QSBooleanNode::check( QSCheckData * )
{
}

void QSStringNode::check( QSCheckData * )
{
}

void QSRegExpNode::check( QSCheckData * )
{
}

void QSResolveNode::check( QSCheckData *c )
{
    if ( !c->directLookupEnabled() )
	return;
    QSClass * cl = c->currentScope();
    QSClass *cont = cl;
    int uplvl = 0;
    int blocks = 0;
    QSMember member;
    while ( cont ) {
	QSMember mem;
	if ( cont->member( 0, ident, &mem ) ) {
	    if ( mem.type()==QSMember::Variable && !mem.isStatic() ) {
		member = mem;
		break;
	    }
	}
	uplvl++;
	cont = cont->enclosingClass();
    }
    if( member.isDefined() ) {

	/* If the containing class has an undefined base, the indexes will be moved
	   when the we call QSAbstractBaseClass::replace() and the lookup info
	   will be crap. Therefore, we don't create it. */
	QSClass *tmp = cont->base();
	while (tmp) {
	    if (tmp->name() == QString::fromLatin1("AbstractBase"))
		return;
	    tmp = tmp->base();
	}

	// Due to special handling of block scopes in the global object...
	if( cont==c->env()->globalClass() )
 	    uplvl+=blocks;
   	info = new QSLookupInfo( uplvl, member );
    }
}

void QSNullNode::check( QSCheckData * )
{
}

void QSThisNode::check( QSCheckData * )
{
}

void QSPostfixNode::check( QSCheckData *c )
{
    if( expr )
	expr->check( c );
}


void QSPropertyNode::check( QSCheckData * )
{
}
