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

#include "qsfuncref.h"

#include "qslexer.h"
#include "qsnodes.h"
#include "qserror_object.h"

extern int qsyyparse();

/*! \reimp */
QSObject QSFuncRefClass::cast( const QSList &args ) const
{
    return construct( args );
}

QSObject QSFuncRefClass::construct( const QSList &args ) const
{
    // ###
  QString p = QString::fromLatin1("");
  QString body;
  int argsSize = args.size();
  if (argsSize == 0) {
    body = "";
  } else if (argsSize == 1) {
    body = args[0].toString();
  } else {
    p = args[0].toString();
    for (int k = 1; k < argsSize - 1; k++)
      p += QString::fromLatin1(",") + args[k].toString();
    body = args[argsSize-1].toString();
  }

  QSLexer::lexer()->setCode( body, -1 );
  if ( qsyyparse() ) {
    /* TODO: free nodes */
    return env()->throwError( SyntaxError, QString::fromLatin1("Syntax error in function body") );
  }

  QSFunctionScopeClass *scope =
      new QSFunctionScopeClass( env()->objectClass() );
  QSFunctionBodyNode * bodyNode = QSProgramNode::last();
  bodyNode->setScopeDefinition( scope );
  scope->setFunctionBodyNode(bodyNode);
  QSMember mem( bodyNode );

  // parse parameter list. throw syntax error on illegal identifiers
  int len = p.length();
  const QChar *c = p.unicode();
  int i = 0, params = 0;
  QString param;
  while (i < len) {
      while (*c == ' ' && i < len)
	  c++, i++;
      if ( QSLexer::isIdentLetter( c->unicode() ) ) {  // else error
	  param = QString(c, 1);
	  c++, i++;
	  while (i < len && ( QSLexer::isIdentLetter( c->unicode() ) ||
			      QSLexer::isDecimalDigit( c->unicode() ) ) ) {
	      param += QString(c, 1);
	      c++, i++;
	  }
	  while (i < len && *c == ' ')
	      c++, i++;
	  if (i == len) {
	      int index = scope->addVariableMember( param, AttributeNone );
	      Q_ASSERT( params==index );
	      params++;
	      break;
	  } else if (*c == ',') {
	      int index = scope->addVariableMember( param, AttributeNone );
	      Q_ASSERT( params==index );
	      params++;
	      c++, i++;
	      continue;
	  } // else error
      }
      return env()->throwError( SyntaxError, QString::fromLatin1("Syntax error in parameter list") );
  }

  scope->setNumArguments( params );
  return createReference( env()->currentScope(), mem );

}

bool QSFuncRefClass::toBoolean( const QSObject * ) const
{
    return TRUE;
}

double QSFuncRefClass::toNumber( const QSObject * ) const
{
    return NaN;
}

QString QSFuncRefClass::toString( const QSObject * ) const
{
    return QString::fromLatin1("function"); // ###
}

class QSReferenceData : public QSShared {
public:
    QSReferenceData( const ScopeChain &ctx, const QSObject &bs, const QSMember &mem )
	: context( ctx ), base( bs ), member( mem )
    {
// 	printf("QSReferenceData::QSReferenceData()\n");
	if (member.type() == QSMember::ScriptFunction)
	    member.scriptFunction->ref();
    }

    ~QSReferenceData()
    {
// 	printf("QSReferenceData::~QSReferenceData()\n");
	if (member.type() == QSMember::ScriptFunction) {
	    if (member.scriptFunction->deref())
		delete member.scriptFunction;
	}
    }
    ScopeChain context;
    QSObject base;
    QSMember member;

    void invalidate()
    {
	base.invalidate();
	ScopeChain::Iterator it = context.begin();
	while( it!=context.end() ) {
	    (*it).invalidate();
	    it++;
	}
    }
};


QSObject QSFuncRefClass::invoke( QSObject * objPtr,
				   const QSMember & ) const
{
    Q_ASSERT( objPtr->isA( this ) );
    QSReferenceData * data = (QSReferenceData*) objPtr->shVal();
    QSObject base = data->base;
    QSMember bmem = data->member;

    env()->pushScopeBlock();
    ScopeChain::const_iterator it = data->context.begin();
    while( it!=data->context.end() ) {
	env()->pushScope( *it );
	it++;
    }

    QSObject retVal = base.invoke( bmem, *(env()->arguments()) );

    env()->popScopeBlock();
    return retVal;
}

QSObject QSFuncRefClass::createReference( const QSObject &base,
					    const QSMember &mem ) const
{
    ScopeChain chain = env()->scope();
    // Reversed order for simplified iteration later.
    ScopeChain context;
    ScopeChain::const_iterator it = chain.begin();
    while( it!=chain.end() ) {
	context.push_front( *it );
	it++;
    }
    return env()->createShared( this, new QSReferenceData( context, base, mem ) );
}

QSMember QSFuncRefClass::refMember( const QSObject &ref )
{
    if( ref.isFunction() ) {
	return ( (QSReferenceData*) ref.shVal() )->member;
    }
    qWarning( "QSFuncRefClass::refMember() - not a reference" );
    return QSMember();
}

QSObject QSFuncRefClass::refBase( const QSObject &ref )
{
    if( ref.isFunction() ) {
	return ( (QSReferenceData*) ref.shVal() )->base;
    }
    qWarning( "QSFuncRefClass::refBase() - not a reference" );
    return ref.env()->createUndefined();
}

ScopeChain QSFuncRefClass::refScope( const QSObject &ref )
{
    if( ref.isFunction() ) {
	return ( (QSReferenceData*) ref.shVal() )->context;
    }
    qWarning( "QSFuncRefClass::refScope() - not a reference" );
    return ScopeChain();
}

QSEqualsResult QSFuncRefClass::isEqual( const QSObject &a, const QSObject &b ) const
{
    if( a.objectType() == b.objectType() )
	return ( QSEqualsResult )
	    ( refBase( a ).equals( refBase( b ) )
	      && refMember( a ) == refMember( b ) );
    return EqualsNotEqual;
}

