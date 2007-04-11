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

#include "qsclass.h"
#include "qsenv.h"
#include "qsoperations.h"
#include "qstypes.h"
#include "qsnodes.h"
#include "qsfuncref.h"


#include <stdlib.h>
#if defined(Q_OS_WIN32)
#include <qt_windows.h>
#endif

using namespace QS;

QSClass::QSClass( QSEnv *e, int a )
    : en( e ),
      bclass( 0 ),
      encClass( 0 ),
      attrs( a )
{
    Q_ASSERT( e );
    init();
}

QSClass::QSClass( QSClass *b, int a )
    : bclass( b ),
      encClass( 0 ),
      attrs( a )
{
    Q_ASSERT( b && b->env() );
    en = b->env();
    init();
}

QSClass::~QSClass()
{
}

void QSClass::clear()
{
    for (QSMemberMap::ConstIterator it = mmap->begin(); it!=mmap->end(); ++it) {
        if ((*it).type() == QSMember::ScriptFunction) {
            if ((*it).scriptFunction->deref()) {
                delete (*it).scriptFunction;
            }
        }
    }
    delete mmap;
    mmap = 0;
    staticMembers.clear();
}


void QSClass::init()
{
    mmap = new QSMemberMap();
    numVars = base() ? base()->numVariables() : 0;
    numStaticVars = 0;
    en->registerClass( this );
}

/*!
  Checks if the two objects are equal. Returns positive if equal, 0 if
  not equal and negative if the class is unable to determine.
*/
QSEqualsResult QSClass::isEqual( const QSObject &a, const QSObject &/*b*/ ) const
{
    Q_ASSERT( a.isA( this ) );
//     qDebug( "QSClass::isEqual( %s, %s )",
// 	    a.typeName().ascii(), b.typeName().ascii() );
    return EqualsUndefined;
}

/*!
  Checks if two objects are strictly equal, meaning that they are
  exactly the same object.
*/
QSEqualsResult QSClass::isStrictEqual( const QSObject &a, const QSObject &b ) const
{
    Q_ASSERT( a.isA( this ) );
    if ( a.objectType() != b.objectType() )
	return EqualsNotEqual;
    if ( a.isUndefined() || a.isNull() )
	return EqualsIsEqual;
    if ( a.isNumber() ) {
	double doubA = a.toNumber();
	if ( isNaN( doubA ) )
	    return EqualsNotEqual;
	double doubB = b.toNumber();
	if ( isNaN( doubB ) )
	    return EqualsNotEqual;
	if ( doubA == doubB
	     || ( doubA==0 && doubB==-0 )
	     || ( doubA==-0 && doubB==0 ) )
	    return EqualsIsEqual;
	return EqualsNotEqual;
    } else if ( a.isString() ) {
        return (QSEqualsResult) (a.toString() == b.toString()
                                 || (a.sVal().isEmpty() && b.sVal().isEmpty()));
    } else if ( a.isBoolean() ) {
	return ( QSEqualsResult ) ( a.toBoolean() == b.toBoolean() );
    }
    return ( QSEqualsResult ) ( a.shVal() == b.shVal() );
}

QSCompareResult QSClass::compare( const QSObject &a, const QSObject &b ) const
{
//     qDebug( "\nQSClass::compare" );
//     qDebug( "a: %s\nb: %s", a.toString().latin1(), b.toString().latin1() );

    QSObject primA = a.toPrimitive( env()->numberClass() );
    QSObject primB = b.toPrimitive( env()->numberClass() );

    if( primA.isString() && primB.isString() ) {
        QString strA = primA.toString();
        QString strB = primB.toString();
        if (strA.isEmpty() && strB.isEmpty())
            return CompareEqual;
	int ret = QString::compare(strA, strB);
	if( ret==0 )
	    return CompareEqual;
	else if( ret<0 )
	    return CompareLess;
	else
	    return CompareGreater;
    }
    double doubA = primA.toNumber();
    double doubB = primB.toNumber();

//     qDebug( "a=%f,   b=%f", doubA, doubB );

    if( isNaN( doubA ) || isNaN( doubB ) ) {
	return CompareUndefined;
    }

    // ### +0 vs -0 cases...

    if( doubA==doubB ) {
	return CompareEqual;
    } else if( doubA < doubB ) {
	return CompareLess;
    } else {
	return CompareGreater;
    }

    return CompareUndefined;
}

void QSClass::finalize()
{
#if 0 // ### required to avoid double deletions
    QSObjectList::iterator it = staticMembers.begin();
    QSObjectList::iterator end = staticMembers.end();
    while ( it != end ) {
	(*it).invalidate();
	++it;
    }
#else
    staticMembers.clear();
#endif
    for (QSMemberMap::ConstIterator it = mmap->begin(); it!=mmap->end(); ++it)
	if ((*it).type() == QSMember::ScriptFunction) {
            if ((*it).scriptFunction->scopeDefinition())
                (*it).scriptFunction->scopeDefinition()->setFunctionBodyNode(0);
	    (*it).scriptFunction->setScopeDefinition(0);
        }
}


QSClassClass* QSClass::asClass() const
{
    return name() == QString::fromLatin1("Class") ? (QSClassClass*)this : 0;
}

QSObject QSClass::execute( const QSObject *, QSObject *, const QSList & ) const
{
    throwError( TypeError, QString::fromLatin1("Cannot invoke objects of type %1 as function").arg(name()) );
    return createUndefined();
}


/*!
  Returns TRUE if this class is a subclass of \c; FALSE otherwise.
  A class is defined to be a subclass of itself.
 */
bool QSClass::inherits( const QSClass *c ) const
{
    const QSClass *b = this;
    while ( b && b != c )
	b = b->base();

    return b == c;
}

/*!
 * Mark \a o and its sub-properties as referenced. This is used
 * for the mark & sweep garbage collection.
 *
 * The default implementation does nothing but you should reimplement
 * this function if objects of this class contain other objects
 * or references to them.
 */

void QSClass::mark( QSObject * /*o*/ ) const
{
    // do nothing
}

void QSClass::ref( QSObject * /*o*/ ) const
{
}

void QSClass::deref( QSObject * /*o*/ ) const
{
}

/*!
 * Returns \a obj converted to a boolean value.
 *
 * The default implementation returns TRUE.
 */

bool QSClass::toBoolean( const QSObject * /*obj*/ ) const
{
    return TRUE;
}

/*!
 * Return \a obj converted to a floating point number; NaN if
 * the conversion failed.
 *
 * The default implementation returns NaN.
 */

double QSClass::toNumber( const QSObject * ) const
{
    return NaN;
}

/*!
 * Return \a obj converted to a string.
 *
 * The default implementation returns "[object N]" where N is
 * the name of this class as retrieved by name().
 */

QString QSClass::toString( const QSObject * ) const
{
    return QString::fromLatin1("[object ") + name() + QString::fromLatin1("]");
}

QSObject QSClass::toPrimitive( const QSObject *obj,
			       const QSClass *preferred ) const
{
    if( preferred != env()->numberClass() )
	return createString( toString( obj ) );
    else
	return createNumber( toNumber( obj ) );
}

/*! Convert \a obj to an equivalent QVariant. The default implementation
  returns in invalid QVariant which may also be the case where no mapping
  for an equivalent type exists.
 */

QVariant QSClass::toVariant( const QSObject * /*obj*/, QVariant::Type ) const
{
    // ### how about respecting the prefered type ?
    return QVariant();
}

#if QS_MAX_STACK>0
static int debugStringRecursionDepth = 0;
#endif

QString QSClass::debugString( const QSObject *obj ) const
{
#if QS_MAX_STACK>0
    if( ++debugStringRecursionDepth==QS_MAX_STACK ) {
	Q_ASSERT( obj->isValid() );
	obj->env()->throwError( RangeError,
                                QString::fromLatin1("Internal recursion level maxed out in: "
                                                    "QSArrayClass::joinInternal"), -1 );
	--debugStringRecursionDepth;
	return QString::null;
    }
#endif
    QString retVal = QString::null;
    if ( obj->isPrimitive() ) {
	retVal = toString( obj ) + QString::fromLatin1(":") + name();
    } else {
	QSMemberMap m = members( obj );
	if (  m.isEmpty() ) {
	    retVal = toString( obj ) + QString::fromLatin1(":") + name();
	} else {
	    QSMemberMap::ConstIterator it = m.begin();
	    retVal = "{";
	    for ( ;; ) {
		QSObject p = env()->resolveValue( it.key() );
		if ( !p.isValid() ) {
		    // ### should never happen (but sometimes does)
		    ++it;
		    if ( it == m.end() )
			break;
		    else
			continue;
		}
		retVal += it.key() + QString::fromLatin1("=") + p.debugString();
		++it;
		if ( it == m.end() )
		    break;
		else
		    retVal += QString::fromLatin1(",");
	    }
	    retVal += QString::fromLatin1("}:") + identifier();
	}
    }
#if QS_MAX_STACK>0
    --debugStringRecursionDepth;
#endif
    return retVal;
}


bool QSClass::deleteProperty( QSObject *, const QSMember & ) const
{
    return FALSE;
}

/*!
  Retrieves a pointer to the class member \a n; 0 if no such member exists.
*/

bool QSClass::member( const QSObject *, const QString &n, QSMember *m ) const
{
//     qDebug( "QSClass::member() class = %s, name = %s", name().latin1(), n.latin1() );
    Q_ASSERT( !n.isEmpty() );
    Q_ASSERT( m );
    Q_ASSERT( mmap );

    QSMemberMap::Iterator it = mmap->find( n );
    if( it == mmap->end() ) {
	return FALSE;
    } else {
	*m = it.data();
	return TRUE;
    }
}


QSObject QSClass::fetchValue( const QSObject *objPtr,
			      const QSMember &mem ) const
{
//     qDebug( "fetching from: "  + identifier() + ", " + mem );
    if( !mem.isReadable() ) {
	qDebug( "QSClass:fetchValue() - not readable: %s", mem.name().latin1() );
	return createUndefined();
    }

    if( mem.type()==QSMember::Variable ) {
	if ( !mem.isStatic() ) {
	    QSInstanceData *data = (QSInstanceData*)objPtr->shVal();
	    if ( mem.idx >= data->size() ) {
		// ### could throw error in member()
// 		qWarning( "QSClass::fetchValue: non-resized array access" );
		return createUndefined();
	    }
	    QSObject * ptr = data->value( mem.idx );

	    if ( !ptr->isValid() ) {
//  		qWarning( "QSMember::fetch: Accessed uninitialized variable" );
		return createUndefined();
	    }
	    return *ptr;
	} else {
	    return staticMember( mem.idx );
	}
    } else if ( mem.isExecutable() ) {
	return env()->funcRefClass()->createReference( *objPtr, mem );
    }

    return createUndefined();
}

void QSClass::write( QSObject *objPtr, const QSMember &mem,
		     const QSObject &val ) const
{
    Q_ASSERT( mem.isWritable() );

    if (mem.type() != QSMember::Variable) {
        env()->throwError(ReferenceError,
                          QString::fromLatin1("Member '%1' cannot be overwritten in '%2'")
                          .arg(mem.name()).arg(name()));
        return;

    }

    if ( mem.isWritable() && mem.type()==QSMember::Variable ) {
	if ( !mem.isStatic() ) {
	    QSInstanceData *data = (QSInstanceData*)objPtr->shVal();
	    if ( mem.idx >= data->size() ) {
		qWarning( "QSClass::write(), index=%d greater than array size=%d",
			  mem.idx, data->size() );
		// ### could throw error in member()
		return;
	    }
	    data->setValue( mem.idx, val );
	} else {
	    QSClass * cl = (QSClass*) this;
	    cl->setStaticMember( mem.idx, val );
	}
    }
}

/*!
  Only use for objPtr's that absolutly have QSInstanceData
*/
void QSClass::write( QSObject * objPtr, int index, const QSObject &val ) const
{
    QSInstanceData *idata = (QSInstanceData*)objPtr->shVal();
    idata->setValue( index, val );
}

void QSClass::setStaticMember( int idx, const QSObject &val )
{
    Q_ASSERT( idx>=0 && idx<numStaticVars );
    staticMembers[idx] = val;
}

QSObject QSClass::staticMember( int idx ) const
{
    Q_ASSERT( idx>=0 && idx<numStaticVars );
    return staticMembers[idx];
}

static bool compareScopes( const QSObject &a, const QSObject &b )
{
    return a.objectType()==b.objectType() && a.shVal()==b.shVal();
}

QSObject QSClass::invoke( QSObject * objPtr, const QSMember &mem ) const
{
    Q_ASSERT( mem.isExecutable() );
    Q_ASSERT( objPtr->objectType() == this );
    switch( mem.type() ) {
    case QSMember::NativeFunction:
	return (*mem.nativeFunction)( env() );
    case QSMember::NativeVoidFunction:
	(*mem.nativeVoidFunction)( env() );
	return createUndefined();
    case QSMember::NativeMemberFunction:
	Q_ASSERT( !mem.isStatic() );
	qWarning( "This should never be called!!" );
	return createUndefined();
    case QSMember::ScriptFunction:
	{
	    Q_ASSERT( mem.scriptFunction );
	    const QSList *args = env()->arguments();

#ifdef QSDEBUGGER
	    Debugger *dbg = env()->engine()->debugger();
	    // arguments as string for call stack info
	    QString argStr = QString::fromLatin1("");
	    for ( int j = 0; j < args->size(); j++ ) {
		if ( j > 0 )
		    argStr += QString::fromLatin1(", ");
		QSObject a = args->at( j );
		argStr += a.toString() + QString::fromLatin1(" : ") + a.typeName();
	    }
	    QString n = mem.scriptFunction->scopeDefinition()->identifier();
	    if( dbg )
		dbg->callEvent( n, argStr );
	    // debugger has to be told that we are potentially
	    // jumping to script code in a different source unit
	    int oldSourceId = -1;
	    if ( dbg ) {
		oldSourceId = dbg->sourceId();
		dbg->setSourceId( mem.scriptFunction->sourceId() );
	    }
#endif
	    QSFunctionScopeClass *scopeDef = mem.scriptFunction->scopeDefinition();

// 	    qDebug( "Calling function:  " + scopeDef->identifier() );
// 	    qDebug( "objPtr is:         " + objPtr->objectType()->identifier() );
// 	    qDebug( "currentScope is:   " + env()->currentScope().objectType()->identifier() );
// 	    env()->printScopeChain();

	    // Use invalid object for scopes that don't have variables..
	    QSObject scope = scopeDef->construct( *args );

	    QSObject returnValue;

	    if ( compareScopes( *objPtr, env()->currentScope() ) ) {
//  		qDebug( "Push scope type 1" );
		env()->pushScope( scope );
		returnValue = mem.scriptFunction->execute( env() );
		env()->popScope();
	    } else if( objPtr->objectType()->enclosingClass()==env()->currentScope().objectType() ) {
//  		qDebug( "Push scope type 1b" );
		env()->pushScope( *objPtr );
		env()->pushScope( scope );
		returnValue = mem.scriptFunction->execute( env() );
		env()->popScope();
		env()->popScope();
	    } else if ( objPtr->objectType()->enclosingClass() == 0 ) {
// 		qDebug( "Push scope type 1c" );
		env()->pushScopeBlock();
		env()->pushScope( env()->globalObject() );
		env()->pushScope( *objPtr );
		env()->pushScope( scope );
//   		env()->printScopeChain();
		returnValue = mem.scriptFunction->execute( env() );
		env()->popScopeBlock();
            } else if ( env()->currentScope().objectType() == env()->globalObject().objectType() ) {
                // object has an enclosing class, but current scope is only global
//                 qDebug( "Push scope type 1d" );
                env()->pushScopeBlock();
                env()->pushScope( env()->globalObject() );
                env()->pushScope( *objPtr );
                env()->pushScope( scope );
                returnValue = mem.scriptFunction->execute( env() );
                env()->popScopeBlock();
	    } else {
//  		qDebug( "Push scope type 2" );
		// ### This loop is a duplicate of the one already done in resolvenode
		ScopeChain chain = env()->scope();
		ScopeChain::Iterator it = chain.begin();

		bool pushObj = FALSE;

		while( it!=chain.end() ) {
		    if( compareScopes( *it, *objPtr ) ) {
			break;
		    } else if ( (*it).objectType() ==
				objPtr->objectType()->enclosingClass() ) {
			pushObj = TRUE;
			break;
		    }
		    else {
			it = chain.remove( it );
		    }
		}

		env()->pushScopeBlock();
		while( chain.size()>0 ) {
		    env()->pushScope( chain.back() );
		    chain.pop_back();
		}
		if( pushObj ) env()->pushScope( *objPtr );
		env()->pushScope( scope );
//   		env()->printScopeChain();
		returnValue = mem.scriptFunction->execute( env() );
		env()->popScopeBlock();
	    }
#ifdef QSDEBUGGER
	    if ( dbg )
		dbg->returnEvent();
	    // restore previous source id
	    if (dbg)
		dbg->setSourceId(oldSourceId);
#endif
	    return env()->isReturnValueMode() ?  returnValue : createUndefined();
	}
    case QSMember::Variable: {
	QSObject o = fetchValue( objPtr, mem );
	if ( o.objectType()->valueType() == TypeClass )
	    return QSTypeClass::classValue(&o)->cast( *env()->arguments() );
	qFatal( "QSClass::invoke: Unhandled variable type" );
	break;
    }
    default:
	qFatal( "QSClass::invoke: Unhandled switch case %d", mem.type() );
    }
    return createUndefined();
}

void QSClass::addFunctionMember( const QString &n, QSFunctionBodyNode * f, int attributes )
{
    addMember( n, QSMember( f, attributes ), createUndefined() );
}

int QSClass::addVariableMember( const QString &n, int attributes )
{
    addMember( n, QSMember( QSMember::Variable, attributes ), createUndefined() );
    return attributes & AttributeStatic ? numStaticVars - 1 : numVars-1;
}

void QSClass::addStaticVariableMember( const QString &name, const QSObject &value, int attr )
{
    addMember( name, QSMember( QSMember::Variable, attr | AttributeStatic ), value );
}

/*!
  Add member \a member with name \a n to this class. \stVal contains the value
  if the member is a static variable.
 */
void QSClass::addMember( const QString &n, const QSMember &member, const QSObject &stVal )
{
    Q_ASSERT( !mmap->contains( n ) );

    QSMember m = member;
    m.setName( n );
    m.setOwner( this );

    switch (m.type()) {
    case QSMember::Variable:
	if( m.isStatic() ) {
	    m.setIndex( numStaticVars++ );
	    staticMembers.append( stVal );
	} else {
	    m.setIndex( numVars++ );
	}
	break;
    case QSMember::ScriptFunction:
	m.scriptFunction->ref(); // Since it is stored by member.
        break;
    default:
        break;
    }
    mmap->insert( n, m );
}

/* Factored out from replace member */
void QSClass::removeStaticVar( const QSMember &old )
{
    staticMembers.remove( staticMembers.at(old.idx) );
    QSMemberMap::iterator it = mmap->begin();
    while( it!=mmap->end() ) {
	QSMember &cur = *it;
	if( cur.type()==QSMember::Variable && cur.isStatic() && cur.idx>old.idx )
	    cur.idx--;
	it++;
    }
    numStaticVars--;
}


/* Factored out from replaceMember */
void QSClass::fillMemberVarIndex( QSMember *member )
{
    if( !replacedVars.isEmpty() ) { // Reuse old varspace if possible.
	member->idx = replacedVars[0];
	replacedVars.pop_front();
    } else {
	member->idx = numVars++;
    }
}


/*!
  Replaces the member \name with \member. \stVal can contain the value if the
  member is a static variable.
*/
void QSClass::replaceMember( const QString &name, QSMember *member,
			     const QSObject &stVal )
{
//     qDebug( "QSClass::replaceMember(%s)", name.latin1() );
    Q_ASSERT( mmap->contains( name ) );
    QSMember old = *(mmap->find( name ));
    QSMember &m = *member;
    m.setName( name );
    m.setOwner( this );

    // Delete old function implementation.
    if (old.type() == QSMember::ScriptFunction) {
        if (old.scriptFunction->deref()) { // will delete
            delete old.scriptFunction;
            old.scriptFunction = 0;
        } else {
            if (old.scriptFunction->scopeDefinition())
                old.scriptFunction->setScopeDefinition(0);
            old.scriptFunction->setScopeDefinition(0);
        }
    }
    // Ref new one...
    if (m.type() == QSMember::ScriptFunction)
	m.scriptFunction->ref();

    if( old.type()==QSMember::Variable && m.type()==QSMember::Variable ) {
	if( old.isStatic() == m.isStatic() ) { // both static or both nonstatic
	    m.idx = old.idx;
	    if( old.isStatic() )  // replace value if static
		staticMembers[m.idx] = stVal;
	} else if( old.isStatic() ) {
	    removeStaticVar( old );
	    fillMemberVarIndex( &m );
	} else if( m.isStatic() ) {
	    m.idx = numStaticVars++;
	    staticMembers.append( stVal );
	    replacedVars.append( old.idx );
	}

    } else if( (old.type()==QSMember::ScriptFunction ||
		old.type()==QSMember::NativeFunction ||
		old.type()==QSMember::NativeMemberFunction) &&
	       (m.type()==QSMember::ScriptFunction ||
		m.type()==QSMember::NativeFunction ||
		m.type()==QSMember::NativeMemberFunction) ) {
	// Replace only...

    } else if ( old.type()==QSMember::Variable ) { // Variable -> function
	if( old.isStatic() ) { // Delete and update member indexes
	    removeStaticVar( old );
	} else { // Store index for reuse later.
	    replacedVars.append( old.idx );
	}

    } else if ( m.type()==QSMember::Variable ) {
	if( m.isStatic() ) {
	    m.idx = numStaticVars++;
	    staticMembers.append( stVal );
	} else {
	    fillMemberVarIndex( &m );
	}
    } else {
	qFatal( "QSClass::replaceMember() -- Unhandled case" );
    }

    mmap->replace( name, m );
}

/*!
  Deletes the member under name \name. If deletion was not possible, FALSE
  is returned
*/
bool QSClass::deleteMember( const QString &name ) {
    if( !mmap->contains( name ) ) {
	return FALSE;
    }
    // ### What do we do about variable indexes??
    mmap->remove( name );
    return TRUE;
}

bool QSClass::hasProperty( const QSObject *obj,
			   const QString &p ) const
{
    // standard class property ?
    QSMember m;
    if ( member( obj, p, &m ) && m.type() != QSMember::Identifier )
	return TRUE;
//     // dynamic property
//     return data( obj )->hasProperty( p );
    return FALSE;
}

QSObject QSClass::get( const QSObject *objPtr, const QString &p ) const
{
    QSMember mem;
    if ( !member( objPtr, p, &mem ) || mem.type() == QSMember::Identifier )
	return createUndefined();
    return fetchValue( objPtr, mem );
}

void QSClass::put( QSObject *objPtr, const QString &p,
		   const QSObject &v ) const
{
    QSMember mem;
    if ( !member( objPtr, p, &mem  ) && mem.type() != QSMember::Identifier ) {
	qWarning( "QSClass::put: refused write of %s", p.ascii() );
	return;
    }
    mem.setName( p );
    write( objPtr, mem, v );
}

QSObject QSClass::construct( const QSList & /* args */ ) const
{
    return createUndefined();
}

QSObject QSClass::cast( const QSList &args ) const
{
    return args.size() > 0 ? args[0] : createUndefined() ;
}

/*!  Returns the map of members of \a obj or class members if \a obj
  is 0.
  The default implementation will list all members pre-defined
  via the addMember() function or one of its specializations. Class
  inherting from QSClass can reimplement this function to also give
  information about custom properties.
 */

QSMemberMap QSClass::members( const QSObject *obj ) const
{
    Q_ASSERT( mmap );
    if ( obj )
	return *mmap;
    QSMemberMap m;
    QSMemberMap::const_iterator it = mmap->begin();
    for ( ; it != mmap->end(); ++it )
	if ( (*it).isStatic() )
	    m.insert( it.key(), it.data() );
    return m;
}

/*!
  Convenience function to throw an error of type \a a with the user
  visible message \a msg.
*/

void QSClass::throwError( ErrorType e, const QString &msg ) const
{
    (void)env()->throwError( e, msg, -1 );
}

QSObject QSClass::createString( const QString &s ) const
{
    return en->createString( s );
}

QSObject QSClass::createNumber( double d ) const
{
    return en->createNumber( d );
}

QSObject QSClass::createBoolean( bool b ) const
{
    return en->createBoolean( b );
}

QSObject QSClass::createUndefined() const
{
    return en->createUndefined();
}

QSObject QSClass::createNull() const
{
    return en->createNull();
}

bool QSUndefinedClass::toBoolean( const QSObject * ) const
{
    return FALSE;
}

double QSUndefinedClass::toNumber( const QSObject * ) const
{
    return NaN;
}

QString QSUndefinedClass::toString( const QSObject * ) const
{
    return QString::fromLatin1("undefined");
}

QSObject QSUndefinedClass::toPrimitive( const QSObject *obj,
					const QSClass * ) const
{
    return *obj;
}

QSEqualsResult QSUndefinedClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    Q_ASSERT( a.isA( this ) );
    if ( b.isUndefined() || b.isNull() )
	return EqualsIsEqual;
    else
	return EqualsUndefined;
}

bool QSNullClass::toBoolean( const QSObject * ) const
{
    return FALSE;
}

double QSNullClass::toNumber( const QSObject * ) const
{
    return 0.0;
}

QString QSNullClass::toString( const QSObject * ) const
{
    return QString::fromLatin1("null");
}

QSObject QSNullClass::toPrimitive( const QSObject *obj,
				   const QSClass * ) const
{
    return *obj;
}

QSEqualsResult QSNullClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    Q_ASSERT( a.isA( this ) );
    if( b.isNull() || b.isUndefined() )
	return EqualsIsEqual;
    return EqualsUndefined;
}


bool QSCharacterClass::toBoolean( const QSObject *obj ) const
{
    return !obj->sVal()[0].isNull();
}

double QSCharacterClass::toNumber( const QSObject *obj ) const
{
    return QSString::toDouble( obj->sVal() );
}

QString QSCharacterClass::toString( const QSObject *obj ) const
{
    return obj->sVal();
}

void QSSharedClass::ref( QSObject * o ) const
{
#ifndef QS_LEAK
    o->shVal()->ref();
#endif
}

void QSSharedClass::deref( QSObject * o ) const
{
#ifndef QS_LEAK
    o->shVal()->deref();
    if( o->shVal()->count==0 ) {
	env()->removeShared( o->shVal() );
	delete o->shVal();
	o->setVal( (QSShared*)0 ); // for debugging purposes only
    }
#endif
}

QSClassClass::QSClassClass( QSClass *b, int a, const QString &n )
    : QSSharedClass( b, a ), cname( n ), defaultCtor( FALSE ), bodyNode( 0 ), clDefNode(0)
{
    memberInit = new QSNodeList();
    staticInit = new QSNodeList();
}

QSClassClass::~QSClassClass()
{
    // If shut down, we cannot allow other classes to be deleted as this will
    // mess up the iterators calling finalize, clear and delete in QSEnv::clear().
    if (env()->isShuttingDown()) {
        if (bodyNode->scopeDefinition())
            bodyNode->scopeDefinition()->setFunctionBodyNode(0);
	bodyNode->setScopeDefinition(0);

    }

    clDefNode->setClassDefinition(0);

    if (clDefNode->deref()) {
	delete clDefNode;
	bodyNode = 0;
	clDefNode = 0;
    }

    delete memberInit;
    delete staticInit;
}

bool QSClassClass::toBoolean( const QSObject * ) const
{
    return TRUE;
}

double QSClassClass::toNumber( const QSObject * ) const
{
    return NaN;
}

QString QSClassClass::toString( const QSObject * ) const
{
    return QString::fromLatin1("[class ") + cname + QString::fromLatin1("]");
}

QSInstanceData* QSClassClass::data( QSObject *obj )
{
    return (QSInstanceData*)obj->shVal();
}

const QSInstanceData* QSClassClass::data( const QSObject *obj )
{
    return (const QSInstanceData*)obj->shVal();
}

/*!
  \reimp
  Construct an instance of the class described by this object.
 */

QSObject QSClassClass::construct( const QSList &args ) const
{
    /* Look for non QSClassClass in parent chain. Can only be object class. If anything
       else, it must be a QSAbstractBaseClass, which is an error at this point. */
    QSClass *baseChain = base();
    while (baseChain && baseChain->asClass())
	baseChain = baseChain->base();
    if (baseChain && baseChain->name() == QString::fromLatin1("AbstractBase")) {
	return env()->throwError(QString(QString::fromLatin1("class '%1' is %2derived from undefined class '%3'"))
				 .arg(cname)
				 .arg(baseChain == base()
                                      ? QString::fromLatin1("")
                                      : QString::fromLatin1("indirectly "))
				 .arg(baseChain->identifier()));
    }

    // Initialize all entries to undefined
    QSInstanceData *data = new QSInstanceData( numVariables(),
					       createUndefined() );
    for( int i=0; i < numVariables(); i++ )
	data->setValue( i, createUndefined() );
    QSObject inst = env()->createShared( this, data );

    // Set up scope
    ScopeChain chain = env()->scope();
    ScopeChain::Iterator sit = chain.begin();
    while( sit!=chain.end() ) {
	if( (*sit).objectType()==enclosingClass() ) {
	    break;
	}
	sit = chain.remove( sit );
    }
    // Fill up scope chain.
    env()->pushScopeBlock();
    while( chain.size()>0 ) {
	env()->pushScope( chain.back() );
	chain.pop_back();
    }
    env()->pushScope( inst );

    initVariables( data );

    // Clean up scope
    env()->popScopeBlock();

    if ( hasDefaultConstructor() && !env()->isExceptionMode() ) {
	QSObject ctor = get( &inst, cname );
	Q_ASSERT( ctor.isExecutable() );
	ctor.invoke( QSMember(), args );
	// ### do something with the return value/type ?
    }

    return inst;
}


QSEqualsResult QSClassClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    if( b.objectType() == this ) {
	return ( QSEqualsResult ) ( b.shVal() == a.shVal() );
    }
    return EqualsNotEqual;
}

void QSClassClass::addMemberInitializer( QSNode * node )
{
    memberInit->append( node );
}

void QSClassClass::addStaticInitializer( QSNode * node )
{
    staticInit->append( node );
}

void QSClassClass::setClassBodyNode( QSFunctionBodyNode * node )
{
    bodyNode = node;
}

/*!
  Execute statements contained in this class' block.
*/

void QSClassClass::executeBlock( QSEnv *env )
{
    // Set up scope
    ScopeChain chain = env->scope();
    ScopeChain::Iterator sit = chain.begin();
    while( sit!=chain.end() ) {
	if( (*sit).objectType()==enclosingClass() ) {
	    break;
	}
	sit = chain.remove( sit );
    }
    // Fill up scope chain.
    env->pushScopeBlock();
    while( chain.size()>0 ) {
	env->pushScope( chain.back() );
	chain.pop_back();
    }
    // Push the type object...
    env->pushScope( env->globalObject().get(cname) );

   // Call initializers
    QPtrListIterator<QSNode> it( *staticInit );
    for ( uint j = 0; j<staticInit->count(); j++ ) {
	QSNode *init = it();
	if ( init ) {
	    setStaticMember( j, init->rhs( env ) );
	    // abort if init code caused an error
	    if ( env->isExceptionMode() )
		break;
	}
    }

    if( bodyNode )
	bodyNode->execute( env );

    // Clean up scope
    env->popScopeBlock();
}

/*!
  \internal
  Runs the initializers on the variables declared in this class. This function
  assumes that scopes are set up correctly and can only be called from
  within the QSClassClass::construct function.
*/
int QSClassClass::initVariables( QSInstanceData * data ) const
{
    int offset = 0;
    QSClassClass *cl = base() ? base()->asClass() : 0;
    if( cl )
	offset = cl->initVariables( data );

    // Call initializers
    QPtrListIterator<QSNode> it( *memberInit );
    for ( uint j = 0; j<memberInit->count(); j++ ) {
	QSNode *init = it();
	if ( init ) {
	    data->setValue( offset + j, init->rhs( env() ) );
	    // abort if init code caused an error
	    if ( env()->isExceptionMode() )
		break;
	}
    }
    return offset + memberInit->count();
}

/*!
  \reimp
*/

void QSWritableClass::mark( QSObject * /*o*/ ) const
{
}

bool QSWritableClass::member( const QSObject *o, const QString &n,
			      QSMember *m ) const
{
    //    qDebug( "QSWritableClass::member() class = %s, name = %s", name().latin1(), n.latin1() );
    Q_ASSERT( /* o &&*/ !n.isEmpty() );
    Q_ASSERT( m );

    if( !o || !o->isDefined() ) {
	return QSClass::member( o, n, m );
    }

    if( !o->shVal() ) {
	return QSClass::member( 0, n, m );
    }


    // The error here is that the object is a dummy!!!
    const QSWritable *w = (QSWritable*) o->shVal(); //data( o );

    if ( !w->hasProperty( n ) ) {
	if ( QSClass::member( o, n, m ) )
	    return TRUE;
	// property doesn't exit, yet. We'll offer to create a new one
	m->setType( QSMember::Identifier );
	m->setName( n );
	m->setOwner( this );
	return FALSE;
    }

    m->setType( QSMember::Object );
    m->obj = &w->reference( n )->object;
    m->setName( n );
    m->setOwner( this );
    return TRUE;
}

QSObject QSWritableClass::fetchValue( const QSObject *objPtr,
				      const QSMember &mem ) const
{
//     qDebug( "QSWritableClass::fetchValue() -> mem.type = %s", mem.typeName().latin1() );
    if( mem.type()==QSMember::Object ) {
	return *mem.obj;
    }
    return QSClass::fetchValue( objPtr, mem );
}

void QSWritableClass::write( QSObject *objPtr, const QSMember &mem,
			     const QSObject &val ) const
{
//     qDebug( "QSWritableClass::write() -> mem.type = %s", mem.typeName().latin1() );
    if( mem.type()==QSMember::Object ) {
	*mem.obj = val;
    } else if( mem.type()==QSMember::Identifier ) {
// 	qDebug( "Writing to QSMember::Identifier: name = %s", mem.str.latin1() );
	data( objPtr )->setProperty( mem.name(), QSProperty( val ) );
    } else {
	QSClass::write( objPtr, mem, val );
    }
}

bool QSWritableClass::deleteProperty( QSObject *obj, const QSMember &mem ) const
{
    if ( mem.type()==QSMember::Object ) {
	properties( obj )->remove( mem.name() );
	return TRUE;
    }
    return FALSE;
}

QSObject QSWritableClass::invoke( QSObject * objPtr, const QSMember &mem ) const
{
    if( mem.type()==QSMember::Object ) {
	Q_ASSERT( mem.obj->isValid() );
	return objPtr->invoke( mem, *env()->arguments() );
    }
    return QSClass::invoke( objPtr, mem );
}

QSWritable *QSWritableClass::data( QSObject *obj )
{
    return (QSWritable*)obj->shVal();
}

const QSWritable *QSWritableClass::data( const QSObject *obj )
{
    return (const QSWritable*)obj->shVal();
}

/*!
  Create an empty, writable object of this class.
*/

QSObject QSWritableClass::createWritable() const
{
    return QSObject( this, new QSWritable() );
}

QSPropertyMap *QSWritableClass::properties( const QSObject *obj ) const
{
    return data( obj )->properties();
}

/*!
  \reimp
  Returns the pre-defined members plus dynamic properties of \a obj.
*/

QSMemberMap QSWritableClass::members( const QSObject *obj ) const
{
    QSMemberMap map = QSClass::members( obj );
    if ( obj ) {
	QSPropertyMap *pmap = properties( obj );
	if ( pmap ) {
	    QSPropertyMap::ConstIterator it = pmap->begin();
	    while ( it != pmap->end() ) {
		QSMember mem( QSMember::Object, AttributeEnumerable );
		mem.setName( it.key() );
		mem.setExecutable( it.data().object.isExecutable() );
		map.insert( it.key(), mem );
		++it;
	    }
	}
    }
    return map;
}

QSEqualsResult QSWritableClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    if( b.objectType() == this ) {
	return ( QSEqualsResult ) ( b.shVal() == a.shVal() );
    }
    return EqualsNotEqual;
}

QSFunctionScopeClass::QSFunctionScopeClass( QSClass *b, QSFuncDeclNode * func )
    : QSWritableClass( b ), ident(func->identifier()), numArgs( 0 ), body_node(0)
{
}

QString QSFunctionScopeClass::identifier() const
{
    return ident.isNull() ? QString(QString::fromLatin1("[anonymous]")) : ident;
}

void QSFunctionScopeClass::clear()
{
    if (body_node)
        body_node->setScopeDefinition(0);
    body_node = 0;
    QSWritableClass::clear();
}

QSObject QSFunctionScopeClass::construct( const QSList &args ) const
{
//   qDebug( "Creating functions scope for: " + identifier() + ", %d", numVariables() );
    QSInstanceData *dat = new QSInstanceData( numVariables(),
					      createUndefined() );
    QSObject scope = env()->createShared( this, dat );

    // fill slots for passed arguments
    QSListIterator it = args.begin();
    int i = 0;
    while ( it != args.end() && i < numArguments() ) {
	dat->setValue( i, *it );
	it++;
	i++;
    }
    // intialize remaining ones with "undefined"
    while ( i < numArguments() )
	dat->setValue( i++, createUndefined() );

    QSArray argObj( env() );
    it = args.begin();
    for ( i = 0; it != args.end(); ++i, ++it )
	argObj.put( QString::number( i ), *it );
    scope.put( QString::fromLatin1("arguments"), argObj );

    return scope;
}

QSObject QSEvalScopeClass::construct( const QSList & ) const
{
    return env()->createShared( this,
				new QSInstanceData( numVariables(),
						    createUndefined() ) );
}

void QSBlockScopeClass::activateScope() const
{
    QSObject scope( this );
    scope.setVal( env()->currentScope().shVal() );
    ref( &scope );
    env()->pushScope( scope );
}

void QSBlockScopeClass::deactivateScope() const
{
    env()->popScope();
}

QSMemberMap QSBlockScopeClass::members( const QSObject *obj ) const
{
    QSMemberMap newMap( *definedMembers() );
    QSMemberMap encMap = enclosingClass()->members( obj );
    QSMemberMap::ConstIterator it = encMap.begin();
    while( it!=encMap.end() ) {
	newMap[ it.key() ] = it.data();
	it++;
    }
    return newMap;
}

class QSTypeClassShared : public QSShared
{
public:
    QSTypeClassShared(QSClass *cl)
	: classValue(cl)
    {
    }

    ~QSTypeClassShared()
    {
	// Delete the class when it is cleared.
	if (!classValue->env()->isShuttingDown()) {
	    classValue->env()->unregisterClass(classValue);
	    classValue->clear();
	    delete classValue;
	}
    }

    QSClass *classValue;
};

QSShared *QSTypeClass::createTypeShared(QSClass *cl) const
{
    return new QSTypeClassShared(cl);
}

QSObject QSTypeClass::createType(QSClass *cl) const
{
    return QSObject(this, new QSTypeClassShared(cl));
}

QSClass *QSTypeClass::classValue(const QSObject *obj)
{
    Q_ASSERT(obj->objectType()->inherits(obj->objectType()->env()->typeClass()));
    return ((QSTypeClassShared *)obj->shVal())->classValue;
}

bool QSTypeClass::member( const QSObject *o, const QString &n,
			  QSMember *m ) const
{
    if( !o )
	return FALSE;
    Q_ASSERT( o->isA( this ) );
    QSClass *tcl = classValue(o);
    return tcl->member( 0, n, m );
}

QSMemberMap QSTypeClass::members( const QSObject *obj ) const
{
    Q_ASSERT( obj->isA( this ) );
    if ( ( ( const QSClass * ) classValue(obj) ) == this )
	return QSClass::members( obj );
    else
	return classValue(obj)->members( 0 );
}


QSMemberMap QSTypeClass::allMembers( const QSObject *obj ) const
{
    Q_ASSERT( obj->isA( this ) );
    if ( ( ( const QSClass * ) classValue(obj) ) == this )
	return QSClass::members( obj );
    else
	return *( classValue(obj)->definedMembers() );
}


QSObject QSTypeClass::fetchValue( const QSObject *o,
				  const QSMember &mem ) const
{
    Q_ASSERT( o->isA( this ) );
    if ( !mem.hasAttribute( AttributeStatic ) ) {
	throwError( ReferenceError, QString::fromLatin1("Cannot access a non-static member "
                                                        "without an object reference") );
	return createUndefined();
    }
    QSClass *tcl = classValue(o);
    return tcl->fetchValue( o, mem );
}

QSObject QSTypeClass::invoke( QSObject *o, const QSMember &mem ) const
{
    Q_ASSERT( o->objectType()==this );
    // we are not interested in static functions
    if ( mem.isStatic() )
	 return QSClass::invoke( o, mem );
    else if( mem.type()==QSMember::Variable ) // Indirect casting.
	return classValue(o)->cast( *env()->arguments() );

    throwError( ReferenceError, QString::fromLatin1("Cannot invoke a non-static function "
                                                    "without an object reference") );
    return createUndefined();
}

void QSTypeClass::write( QSObject *objPtr, const QSMember &mem,
			 const QSObject &val ) const
{
    Q_ASSERT( mem.isWritable() );
    // Q_ASSERT( mem.type()==QSMember::Variable );

    if ( !mem.hasAttribute( AttributeStatic ) ) {
	throwError( ReferenceError, QString::fromLatin1("Cannot access a non-static member "
                                                        "without an object reference") );
	return;
    }

    QSClass * cl = classValue(objPtr);

    if( mem.type()==QSMember::Variable ) {
	cl->setStaticMember( mem.idx, val );
    }
    else {
	throwError( ReferenceError, QString::fromLatin1("Trying to write to a nonvariable") );
	return;
    }
}

QSEqualsResult QSTypeClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    if( b.objectType()==this ) {
	return ( QSEqualsResult ) ( classValue(&a) == classValue(&b) );
    }
    return EqualsUndefined;
}


static void qs_dumpclass( const QSClass *cl )
{
    printf( "class %s", cl->identifier().latin1() );
    printf( "  - %s\n", cl->isExecutable() ? "executable" : "not executable" );
    printf( "  - %s\n", cl->isFinal() ? "final" : "not final" );
    QSMemberMap::Iterator it = cl->definedMembers()->begin();
    for( ; it!=cl->definedMembers()->end(); it++ ) {
	QSMember mem = *it;
	QString line = QString(QString::fromLatin1("  ")) + mem;
	printf( "%s\n", line.latin1() );
    }
    if( cl->enclosingClass() )
	qs_dumpclass( cl->enclosingClass() );
    if( cl->base() )
	qs_dumpclass( cl->base() );
}


static void qs_dumptype( const QSList &args )
{
    if ( args.size()>=1 &&
	 args[0].objectType()==args[0].objectType()->env()->typeClass() ) {
	printf( "DUMP TYPE::\n" );
	QSObject arg0 = args[0];
	QSClass *cl = QSTypeClass::classValue(&arg0);
	qs_dumpclass( cl );
    }
    printf( "\n" );
}


static void qs_dumpobject( const QSObject &obj )
{
    const QSClass * cl = obj.objectType();
    printf( "DUMP OBJECT:: %p\n", obj.shVal() );
    printf( "class %s :: %s\n",
	    cl->name().latin1(),
	    cl->identifier().latin1() );
    QSMemberMap::Iterator it = cl->definedMembers()->begin();
    for( ; it!=cl->definedMembers()->end(); it++ ) {
	QSMember mem = *it;
	if( mem.isReadable() ) {
	    QSObject value = cl->fetchValue( &obj, mem );
	    if( mem.type()==QSMember::Variable )
		printf( "  %2d: %s = %s\n", mem.index(), mem.name().latin1(), value.toString().latin1() );
	    else
		printf( "      %s = %s\n", mem.name().latin1(), value.toString().latin1() );
	}
    }
}


QSDebugClass::QSDebugClass( QSClass *base )
    : QSClass( base, AttributeAbstract )
{
    addMember( QString::fromLatin1("dumpObject"),
	       QSMember( &dumpObject, AttributeNonWritable|AttributeStatic ) );
    addMember( QString::fromLatin1("dumpScope"),
	       QSMember( &dumpScope, AttributeNonWritable|AttributeStatic ) );
    addMember( QString::fromLatin1("dumpType"),
	       QSMember( &dumpType, AttributeNonWritable|AttributeStatic ) );
}

void QSDebugClass::dumpObject( QSEnv * env )
{
    qs_dumpobject( ( env->numArgs() > 0 ? env->arg( 0 ) : env->createUndefined() ) );
}


void QSDebugClass::dumpScope( QSEnv * env )
{
    ScopeChain chain = env->scope();
    ScopeChain::ConstIterator it = chain.begin();
    qDebug( "\n---------- DUMP SCOPE ----------" );
    while( it!=chain.end() ) {
	qs_dumpobject( *it );
	if( (*it).objectType() == env->typeClass() ) {
            QSList itList( *it );
	    qs_dumptype( itList );
	}
	it++;
    }
    qDebug( "---------- DUMP COMPLETE ----------" );
}


void QSDebugClass::dumpType( QSEnv * env )
{
    qs_dumptype( *env->arguments() );
}


QSSystemClass::QSSystemClass( QSClass *base )
    : QSClass( base, AttributeAbstract )
{
    addMember( QString::fromLatin1("print"),
	       QSMember( &print, AttributeNonWritable | AttributeStatic ) );
    addMember( QString::fromLatin1("println"),
	       QSMember( &println, AttributeNonWritable | AttributeStatic ) );
    addMember( QString::fromLatin1("getenv"),
	       QSMember( &getenv, AttributeNonWritable | AttributeStatic ) );
    addMember( QString::fromLatin1("setenv"),
	       QSMember( &setenv, AttributeNonWritable | AttributeStatic ) );
}


void QSSystemClass::println( QSEnv *env )
{
    printf( "%s\n", env->arg( 0 ).toString().latin1() );
}


void QSSystemClass::print( QSEnv *env )
{
    printf( "%s", env->arg( 0 ).toString().latin1() );
}


QSObject QSSystemClass::getenv( QSEnv *env )
{
    return env->createString( QString::fromLatin1(::getenv( env->arg( 0 ).toString().latin1() )) );
}


void QSSystemClass::setenv( QSEnv *env )
{
#if defined(Q_OS_HPUX) || defined(Q_OS_IRIX) || defined(Q_OS_SOLARIS) || defined( Q_CC_BOR )
    putenv( (char*)( env->arg( 0 ).toString() + "="
		     + env->arg( 1 ).toString() ).latin1() ); // char* on Solaris
#elif defined(Q_OS_WIN32)
    _putenv( QString::fromLatin1("%1=%2")
             .arg(env->arg( 0 ).toString())
             .arg(env->arg( 1 ).toString() ).latin1() );
#else
    ::setenv( (char *)env->arg( 0 ).toString().latin1(),
	      (char *)env->arg( 1 ).toString().latin1(), 1 );
#endif
}


/* Implementation of the QSAbstractBaseClass.
 * Used primarly to support cross referencing of class between files, e.g.
 * declaring a class in one file and deriving from it in another.
 */
void QSAbstractBaseClass::replace(QSClassClass *newBase)
{
    QPtrList<QSClassClass> userClasses;
    QPtrList<QSClass> allClasses = env()->classes();

    // Build a list of the user classes, excluding this one.
    QPtrListIterator<QSClass> it(allClasses);
    QSClass *tmp;
    while ((tmp = it()))
	if (tmp->asClass() && tmp != newBase)
	    userClasses.append((QSClassClass*)tmp);


    // Check if userclasses have this abstract class definition as parent and update
    // member table if so.
    QPtrListIterator<QSClassClass> userIt(userClasses);
    QPtrList<QSClassClass> directChildren;
    QSClassClass *userClass;
    while ((userClass = userIt())) {
	QSClass *baseClass = userClass->base();
	// Directly derived, base pointer must be updated later
	if (userClass->base() == this)
	    directChildren.append(userClass);

	while (baseClass && baseClass != this)
	    baseClass = baseClass->base();

	// update offsets in member table...
	if (baseClass == this) {
	    userClass->setNumVariables(newBase->numVariables() + userClass->numVariables());
	    QSMemberMap *mems = userClass->definedMembers();
	    for (QSMemberMap::Iterator it = mems->begin(); it != mems->end(); ++it) {
		QSMember &m = (*it);
		if (m.type() == QSMember::Variable && !m.isStatic())
		    m.setIndex(m.index()+newBase->numVariables());
	    }

	}
    }

    userIt = QPtrListIterator<QSClassClass>(directChildren);
    while ((userClass = userIt()))
	userClass->setBase(newBase);

    // We no longer serve any purpose, so we disappear...
    env()->unregisterClass(this);
    clear();
    delete this;
};

QSInstanceData::QSInstanceData( int count, const QSObject &def )
{
    vals = new QSObject[count];
    sz = count;
    for( int i=0; i<count; i++ )
      vals[i] = def;
}

void QSInstanceData::resize( int count, const QSObject &def )
{
    QSObject *tmp = vals;
    vals = new QSObject[count];
    for( int i=0; i<sz; i++ ) {
	vals[i] = tmp[i];
    }
    for( int j=sz; j<count; j++ )
      vals[j] = def;
    delete [] tmp;
    sz = count;
}

/*!
  Insure that this object has enough space for \a count objects.
  If that is already the case the array won't be resized.
 */
void QSInstanceData::ensureSize( int count, const QSObject &def )
{
    if ( count > sz )
	resize( count, def );
}

/*!
  Invalidates all the objects in this instance data so that
  it can be destroyed at a later time without any problems without
  further reference counting.
*/
void QSInstanceData::invalidate()
{
    for( int i=0; i<sz; i++ ) {
	vals[i].invalidate();
    }
    QSWritable::invalidate();
}

QString operator+( const QString &a, const QSMember &b )
{
    QString s;
    s.sprintf( "QSMember(%s.%s, %s, %x)",
	       b.owner() ? b.owner()->identifier().latin1() : "(no owner)",
	       b.name().latin1(),
	       b.typeName().latin1(),
	       b.attributes() );
    return a + s;
}

bool operator==( const QSMember &a, const QSMember &b )
{
    return a.type() == b.type() && a.owner() == b.owner() &&
		     !a.name().isEmpty() && a.name() == b.name();
}

