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

#include "quickclassparser.h"

bool QuickClass::operator==( const QuickClass &other ) const
{
    return name == other.name &&
	inherits == other.inherits &&
	functions == other.functions &&
	variables == other.variables &&
	connections == other.connections;
}

QString QuickClassParser::legalChars = QString::fromLatin1("abcdefghijklmnopqrstufvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_/*");

QuickClassParser::QuickClassParser()
{
}

void QuickClassParser::parse( const QString &code )
{
    formCode = code;
    QString expr;
    globalClass.name = gname;
    globalClass.type = QuickClass::Global;
    currClass = &globalClass;
    static int functionLength = strlen( "function" );
    static int constructorLength = strlen( "constructor" );

    for ( pos = 0; pos < (int)formCode.length(); ++pos ) {
	if ( legalChars.indexOf( formCode[ pos ] ) == -1 )
	    expr = "";
	else
	    expr += formCode[ pos ];
	if ( expr == QString::fromLatin1("var")
             || expr == QString::fromLatin1("const")
             || expr == QString::fromLatin1("static") ) {
	    int i = pos + 1;
	    while ( formCode[ i ] == ' ' || formCode[ i ] == '\t' )
		++i;
	    if ( formCode.mid( i, 8 ) != QString::fromLatin1("function") ) {
		++pos;
		parseVariableDecl( expr );
	    }
	    expr = "";
	    continue;
	} else if ( expr == QString::fromLatin1("private")
                    || expr == QString::fromLatin1("protected")
                    || expr == QString::fromLatin1("public") ) {
	    lastAccess = expr;
	    expr = QString::fromLatin1("");
	    continue;
	} else if ( expr == QString::fromLatin1("function")
                    || expr == QString::fromLatin1("constructor") ) {
	    ++pos;
	    parseFunction( expr == QString::fromLatin1("function")
                           ? functionLength
                           : constructorLength );
	    expr = "";
	    continue;
	} else if ( expr == QString::fromLatin1("connect") ) {
	    ++pos;
	    parseConnections();
	    expr = "";
	    continue;
	} else if ( expr == QString::fromLatin1("//") ) {
	    pos--;
	    parseCppComment();
	    expr = "";
	    continue;
	} else if ( expr == QString::fromLatin1("/*") ) {
	    pos--;
	    parseCComment();
	    expr = "";
	    continue;
	} else if ( expr == QString::fromLatin1("class") ) {
	    if ( currClass != &globalClass )
		clsses.append( *currClass );
	    currClass = new QuickClass;
	    currClass->type = QuickClass::Class;
	    ++pos;
	    parseClassStart();
	    expr = "";
	} else if (  formCode[ pos ] == '}' ) {
	    if ( currClass != &globalClass )
		clsses.append( *currClass );
	    currClass = &globalClass;
	}
    }
    if ( currClass != &globalClass )
	clsses.append( *currClass );
    clsses.append( globalClass );
}


void QuickClassParser::parseConnections()
{
    enum { InSender,
	   BeforeSignal,
	   InSignal,
	   AfterSignal,
	   InReceiver,
	   BeforeSlot,
	   InSlot
    } state = InSender;

    QString sender, signal, receiver, slot;

    for ( ; pos < (int)formCode.length(); ++pos ) {
	QChar c = formCode[pos];
	if ( c == ';' )
	    return;

	switch ( state ) {
	case InSender:
	    if ( c == ',' ) {
		state = BeforeSignal;
		break;
	    }
	    if ( c != '(' )
		sender += c;
	    break;
	case  BeforeSignal:
	    if ( c == '\"' )
		state = InSignal;
	    break;
	case InSignal:
	    if ( c == '\"' ) {
		state = AfterSignal;
		break;
	    }
	    signal += c;
	    break;
	case AfterSignal:
	    if ( c == ',' )
		state = InReceiver;
	    break;
	case InReceiver:
	    if ( c == ',' ) {
		state = BeforeSlot;
		break;
	    }
	    receiver += c;
	    break;
	case  BeforeSlot:
	    if ( c == '\"' )
		state = InSlot;
	    break;
	case InSlot:
	    if ( c == '\"' )
		goto out_of_loop;
	    slot += c;
	    break;
	}
    }

    return;

 out_of_loop:
    LanguageInterface::Connection conn;
    conn.sender = sender.simplified();
    conn.signal = signal.simplified();
    conn.slot = slot.simplified();

    currClass->connections.append( conn );
}

void QuickClassParser::parseFunction( int functionLength )
{
    QString name;
    QString body;
    QString type;
    int open = 0;
    enum { Name, Args, Type, Body } state = Name;
    enum { Normal, CppComment, CComment, String } bodyState = Normal;
    state = Name;

    for ( ; pos < (int)formCode.length(); ++pos ) {
	if ( formCode[ pos ] == '{' ) {
	    if ( state != Body ) {
		state = Body;
		bodyState = Normal;
	    }
	    open++;
	} else if ( formCode[ pos ] == '}' ) {
	    if ( state != Body || bodyState == Normal )
		open--;
	} else if ( state == Name ) {
	    if ( formCode[ pos ] == '(' ) {
		state = Args;
	    } else if ( formCode[ pos ] == ':' ) {
		state = Type;
		continue; // do not add the ':' somewhere
	    }
	} else if ( state == Name ) {
	   if ( formCode[ pos ] == ')' )
	       state = Name;
	}

	switch ( state ) {
	case Name:
	case Args:
	    name += formCode[ pos ];
	    break;
	case Type:
	    type += formCode[ pos ];
	    break;
	case Body:
	    body += formCode[ pos ];
	    switch ( bodyState ) {
	    case Normal:
		if ( formCode[ pos ] == '\"' ) {
		    bodyState = String;
		} else if ( formCode[ pos ] == '/' ) {
		    if ( pos > 0 && formCode[ pos - 1 ] == '/' )
			bodyState = CppComment;
		} else if ( formCode[ pos ] == '*' ) {
		    if ( pos > 0 && formCode[ pos - 1 ] == '/' )
			bodyState = CComment;
		}
		break;
	    case CppComment:
		if ( formCode[ pos ] == '\n' )
		    bodyState = Normal;
		break;
	    case CComment:
		if ( formCode[ pos ] == '/' &&
		     pos > 0 && formCode[ pos - 1 ] == '*' )
		    bodyState = Normal;
		break;
	    case String:
		if ( formCode[ pos ] == '\"' )
		    bodyState = Normal;
	    }
	    break;
	}

	if ( state == Body ) {
	    if ( !open ) {
		break;
	    } else if ( bodyState == Normal
                        && body.right( functionLength ) == QString::fromLatin1("function") ) {
		// no nested functions supported at the moment
		body.remove( body.length() - functionLength, functionLength );
		pos -= functionLength;
		break;
	    }
	}
    }

    LanguageInterface::Function func;
    func.name = name.trimmed();
    func.returnType = type.trimmed();
    func.body = body;
    if ( lastAccess.isEmpty() )
	func.access = "public";
    else
	func.access = lastAccess;
    lastAccess = QString::null;
    currClass->functions << func;
}

void QuickClassParser::parseVariableDecl( const QString &expr )
{
    QString var;
    for ( ; pos < (int)formCode.length(); ++pos ) {
	if ( formCode[ pos ] == ';' ) {
	    var = var.simplified();
	    if ( var[ (int)var.length() - 1 ] != ';' )
		var += QString::fromLatin1(";");
	    currClass->variables << QString( expr + QString::fromLatin1(" ") + var );
	    return;
	}
	var += formCode[ pos ];
    }
}

void QuickClassParser::parseCComment()
{
    QChar lastChr = formCode[ pos ];
    for ( ; pos < (int)formCode.length(); ++pos ) {
	if ( lastChr == '*' && formCode[ pos ] == '/' )
	    break;
	lastChr = formCode[ pos ];
    }
}

void QuickClassParser::parseCppComment()
{
    QString comment;
    for ( ; pos < (int)formCode.length() && formCode[ pos ] != '\n'; ++pos )
	;
}

void QuickClassParser::parseClassStart()
{
    QString name;
    QString inherits;
    enum { Name, Inherits } state;
    state = Name;

    for ( ; pos < (int)formCode.length(); ++pos ) {
	if ( formCode[ pos ] == '{' ) {
	    currClass->name = name.trimmed();
	    currClass->inherits = inherits.trimmed();
	    if ( lastAccess.isEmpty() )
		currClass->access = "public";
	    else
		currClass->access = lastAccess;
	    lastAccess = QString::null;
	    return;
	} else if ( state == Name && formCode[ pos ] == ':' ) {
	    state = Inherits;
	    continue; // do not add the ':' somewhere
	}

	switch ( state ) {
	case Name:
	    name += formCode[ pos ];
	    break;
	case Inherits:
	    inherits += formCode[ pos ];
	    break;
	}
    }
}
