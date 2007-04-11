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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qslexer.h"
#include "qsengine.h"
#include "qsnodes.h"
#include "qslookup.h"
#include "qsinternal.h"
#include "grammar.h"
#include "qslexer.lut.h"

#include <qlocale.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define shiftWindowsLineBreak() if( current == '\r' && next1 == '\n' ) shift( 1 );


#ifdef QSDEBUGGER
extern YYLTYPE yylloc;	// global bison variable holding token info
#endif

QSLexer *QSLexer::lx = 0;
int QSLexer::sid;

// a bridge for yacc from the C world to C++
int qsyylex()
{
    return QSLexer::lexer()->lex();
}



QSLexer::QSLexer()
    : yylineno( 1 ),
      size8( 128 ), size16( 128 ), restrKeyword( FALSE ),
      stackToken( -1 ), pos( 0 ),
      code( 0 ), length( 0 ),
      bol( TRUE ),
      current( 0 ), next1( 0 ), next2( 0 ), next3( 0 )
{
    Q_ASSERT( lx == 0 );
    lx = this;

    // allocate space for read buffers
    buffer8 = new char[size8];
    buffer16 = new QChar[size16];

}

QSLexer::~QSLexer()
{
    delete [] buffer8;
    delete [] buffer16;

    if ( lx == this )
	lx = 0;
}

/*!
  Sets the code to be parsed.
  \a lineno specifies which line which is to be interpreted
  as line number zero. \a id specifies the unique identification
  number for this source code unit.
*/
void QSLexer::setCode( const QString &c, int id, int lineno )
{
    errmsg = QString::null;
    yylineno = -lineno;
    sid = id;
    restrKeyword = FALSE;
    delimited = FALSE;
    stackToken = -1;
    pos = 0;
    code = c.unicode();
    length = c.length();
#ifndef QTSCRIPT_PURE_ECMA
    bol = TRUE;
#endif

    // read first characters
    current = ( length > 0 ) ? code[0].unicode() : 0;
    next1 = ( length > 1 ) ? code[1].unicode() : 0;
    next2 = ( length > 2 ) ? code[2].unicode() : 0;
    next3 = ( length > 3 ) ? code[3].unicode() : 0;
}

void QSLexer::shift( uint p )
{
    while ( p-- ) {
	pos++;
	current = next1;
	next1 = next2;
	next2 = next3;
	next3 = ( pos + 3 < length ) ? code[pos+3].unicode() : 0;
    }
}

void QSLexer::setDone( State s )
{
    state = s;
    done = TRUE;
}

int QSLexer::lex()
{
    int token = 0;
    state = Start;
    ushort stringType = 0; // either single or double quotes
    pos8 = pos16 = 0;
    done = FALSE;
    terminator = FALSE;

    // did we push a token on the stack previously ?
    // (after an automatic semicolon insertion)
    if ( stackToken >= 0 ) {
	setDone(Other);
	token = stackToken;
	stackToken = 0;
    }

    while ( !done ) {
	switch ( state ) {
	case Start:
	    if ( isWhiteSpace() ) {
		// do nothing
	    } else if ( current == '/' && next1 == '/' ) {
		shift( 1 );
		state = InSingleLineComment;
	    } else if ( current == '/' && next1 == '*' ) {
		shift(1 );
		state = InMultiLineComment;
	    } else if ( current == 0 ) {
		if ( !terminator && !delimited ) {
		    // automatic semicolon insertion if program incomplete
		    token = ';';
		    stackToken = 0;
		    setDone( Other );
		} else
		    setDone( Eof );
	    } else if ( isLineTerminator() ) {
		shiftWindowsLineBreak();
		yylineno++;
		bol = TRUE;
		terminator = TRUE;
		if ( restrKeyword ) {
		    token = ';';
		    setDone( Other );
		}
	    } else if ( current == '"' || current == '\'' ) {
		state = InString;
		stringType = current;
	    } else if ( isIdentLetter( current ) ) {
		record16( current );
		state = InIdentifier;
	    } else if ( current == '0' ) {
		record8( current );
		state = InNum0;
	    } else if ( isDecimalDigit( current ) ) {
		record8( current );
		state = InNum;
	    } else if ( current == '.' && isDecimalDigit( next1 ) ) {
		record8( current );
		state = InDecimal;
	    } else {
		token = matchPunctuator( current, next1, next2, next3 );
		if ( token != -1 )
		    setDone( Other );
		else {
		    setDone( Bad );
		    errmsg = "Illegal character";
		}
	    }
	    break;
	case InString:
	    if ( current == stringType ) {
		shift( 1 );
		setDone( String );
	    } else if ( current == 0 || isLineTerminator() ) {
		setDone( Bad );
		errmsg = "Unclosed string at end of line";
	    } else if ( current == '\\' ) {
		state = InEscapeSequence;
	    } else {
		record16( current );
	    }
	    break;
	    // Escape Sequences inside of strings
	case InEscapeSequence:
	    if ( isOctalDigit( current ) ) {
		if ( current >= '0' && current <= '3' &&
		     isOctalDigit( next1 ) && isOctalDigit( next2 )) {
		    record16( convertOctal( current, next1, next2 ) );
		    shift( 2 );
		    state = InString;
		} else if ( isOctalDigit( current ) &&
			    isOctalDigit( next1 ) ) {
		    record16( convertOctal( '0', current, next1 ) );
		    shift( 1 );
		    state = InString;
		} else if ( isOctalDigit( current ) ) {
		    record16( convertOctal( '0', '0', current ) );
		    state = InString;
		} else {
		    setDone( Bad );
		    errmsg = "Illegal escape squence";
		}
	    } else if ( current == 'x' )
		state = InHexEscape;
	    else if ( current == 'u' )
		state = InUnicodeEscape;
	    else {
		record16( singleEscape( current ) );
		state = InString;
	    }
	    break;
	case InHexEscape:
	    if ( isHexDigit( current ) && isHexDigit( next1 ) ) {
		state = InString;
		record16( convertHex( current, next1 ) );
		shift( 1 );
	    } else if ( current == stringType ) {
		record16( 'x' );
		shift( 1 );
		setDone( String );
	    } else {
		record16( 'x' );
		record16( current );
		state = InString;
	    }
	    break;
	case InUnicodeEscape:
	    if ( isHexDigit( current ) && isHexDigit( next1 ) &&
		 isHexDigit( next2 ) && isHexDigit( next3 ) ) {
		record16( convertUnicode( current, next1, next2, next3 ) );
		shift( 3 );
		state = InString;
	    } else if ( current == stringType ) {
		record16( 'u' );
		shift( 1 );
		setDone( String );
	    } else {
		setDone( Bad );
		errmsg = "Illegal unicode escape sequence";
	    }
	    break;
	case InSingleLineComment:
	    if ( isLineTerminator() ) {
		shiftWindowsLineBreak();
		yylineno++;
		terminator = TRUE;
#ifndef QTSCRIPT_PURE_ECMA
		bol = TRUE;
#endif
		if ( restrKeyword ) {
		    token = ';';
		    setDone( Other );
		} else
		    state = Start;
	    } else if ( current == 0 ) {
		setDone( Eof );
	    }
	    break;
	case InMultiLineComment:
	    if ( current == 0 ) {
		setDone( Bad );
		errmsg = "Unclosed comment at end of file";
	    } else if ( isLineTerminator() ) {
		shiftWindowsLineBreak();
		yylineno++;
	    } else if ( current == '*' && next1 == '/' ) {
		state = Start;
		shift( 1 );
	    }
	    break;
	case InIdentifier:
	    if ( isIdentLetter( current ) || isDecimalDigit( current ) ) {
		record16( current );
		break;
	    }
	    setDone( Identifier );
	    break;
	case InNum0:
	    if ( current == 'x' || current == 'X' ) {
		record8( current );
		state = InHex;
	    } else if ( current == '.' ) {
		record8( current );
		state = InDecimal;
	    } else if ( current == 'e' || current == 'E' ) {
		record8( current );
		state = InExponentIndicator;
	    } else if ( isOctalDigit( current ) ) {
		record8( current );
		state = InOctal;
	    } else if ( isDecimalDigit( current ) ) {
		record8( current );
		state = InDecimal;
	    } else {
		setDone( Number );
	    }
	    break;
	case InHex:
	    if ( isHexDigit( current ) )
		record8( current );
	    else
		setDone( Hex );
	    break;
	case InOctal:
	    if ( isOctalDigit( current) ) {
		record8(current);
	    } else if ( isDecimalDigit( current ) ) {
		record8( current );
		state = InDecimal;
	    } else {
		setDone( Octal );
	    }
	    break;
	case InNum:
	    if ( isDecimalDigit( current ) ) {
		record8( current );
	    } else if ( current == '.' ) {
		record8( current );
		state = InDecimal;
	    } else if ( current == 'e' || current == 'E' ) {
		record8( current );
		state = InExponentIndicator;
	    } else {
		setDone( Number );
	    }
	    break;
	case InDecimal:
	    if ( isDecimalDigit( current ) ) {
		record8( current );
	    } else if ( current == 'e' || current == 'E' ) {
		record8( current );
		state = InExponentIndicator;
	    } else {
		setDone( Number );
	    }
	    break;
	case InExponentIndicator:
	    if ( current == '+' || current == '-' ) {
		record8( current );
	    } else if ( isDecimalDigit( current ) ) {
		record8( current );
		state = InExponent;
	    } else {
		setDone( Bad );
		errmsg = "Illegal syntax for exponential number";
	    }
	    break;
	case InExponent:
	    if ( isDecimalDigit( current ) ) {
		record8( current );
	    } else {
		setDone( Number );
	    }
	    break;
	default:
	    assert( !"Unhandled state in switch statement" );
	}

	// move on to the next character
	if ( !done )
	    shift( 1 );
	if ( state != Start && state != InSingleLineComment )
	    bol = FALSE;
    }

    // no identifiers allowed directly after numeric literal, e.g. "3in" is bad
    if ( (state == Number || state == Octal || state == Hex )
	 && isIdentLetter( current ) ) {
	state = Bad;
	errmsg = "Identifier cannot start with numeric literal";
    }

    // terminate string
    buffer8[pos8] = '\0';

    double dval = 0;
    if ( state == Number ) {
	dval = QLocale::c().toDouble(buffer8);
    } else if ( state == Hex ) { // scan hex numbers
	// TODO: support long uint
	uint i;
	sscanf( buffer8, "%x", &i );
	dval = i;
	state = Number;
    } else if ( state == Octal ) {   // scan octal number
	uint ui;
	sscanf( buffer8, "%o", &ui );
	dval = ui;
	state = Number;
    }

    restrKeyword = FALSE;
    delimited = FALSE;
#ifdef QSDEBUGGER
     yylloc.first_line = yylineno; // ???
     yylloc.last_line = yylineno;
#endif

    switch ( state ) {
    case Eof:
	return 0;
    case Other:
	if( token == '}' || token == ';' )
	    delimited = TRUE;
	return token;
    case Identifier:
	if ( (token = QSLookup::find( &mainTable, buffer16, pos16 )) < 0 ) {
	    /* TODO: close leak on parse error. same holds true for String */
	    qsyylval.ustr = new QString( buffer16, pos16 );
	    return IDENT;
	}
	if ( token == CONTINUE || token == BREAK ||
	     token == RETURN || token == THROW )
	    restrKeyword = TRUE;
	return token;
    case String:
	qsyylval.ustr = new QString( buffer16, pos16 ); return STRING;
    case Number:
	qsyylval.dval = dval;
	return NUMBER;
    case Bad:
	return -1;
    default:
	assert( !"unhandled numeration value in switch" );
	return -1;
    }
}

bool QSLexer::isWhiteSpace() const
{
    return ( current == ' ' || current == '\t' ||
	     current == 0x0b || current == 0x0c );
}

bool QSLexer::isLineTerminator() const
{
    return ( current == '\n' || current == '\r' );
}

bool QSLexer::isIdentLetter( ushort c )
{
    /* TODO: allow other legitimate unicode chars */
    return ( c >= 'a' && c <= 'z' ||
	     c >= 'A' && c <= 'Z' ||
	     c == '$' || c == '_' );
}

bool QSLexer::isDecimalDigit( ushort c )
{
    return ( c >= '0' && c <= '9' );
}

bool QSLexer::isHexDigit( ushort c ) const
{
    return ( c >= '0' && c <= '9' ||
	     c >= 'a' && c <= 'f' ||
	     c >= 'A' && c <= 'F' );
}

bool QSLexer::isOctalDigit( ushort c ) const
{
    return ( c >= '0' && c <= '7' );
}

int QSLexer::matchPunctuator( ushort c1, ushort c2,
			    ushort c3, ushort c4 )
{
    if ( c1 == '>' && c2 == '>' && c3 == '>' && c4 == '=' ) {
	shift( 4 );
	return URSHIFTEQUAL;
    } else if ( c1 == '=' && c2 == '=' && c3 == '=' ) {
	shift( 3 );
	return STREQ;
    } else if ( c1 == '!' && c2 == '=' && c3 == '=' ) {
	shift( 3 );
	return STRNEQ;
    } else if ( c1 == '>' && c2 == '>' && c3 == '>' ) {
	shift( 3 );
	return URSHIFT;
    } else if ( c1 == '<' && c2 == '<' && c3 == '=' ) {
	shift( 3 );
	return LSHIFTEQUAL;
    } else if ( c1 == '>' && c2 == '>' && c3 == '=' ) {
	shift( 3 );
	return RSHIFTEQUAL;
    } else if ( c1 == '<' && c2 == '=' ) {
	shift( 2 );
	return LE;
    } else if ( c1 == '>' && c2 == '=' ) {
	shift( 2 );
	return GE;
    } else if ( c1 == '!' && c2 == '=' ) {
	shift( 2 );
	return NE;
    } else if ( c1 == '+' && c2 == '+' ) {
	shift( 2 );
	return PLUSPLUS;
    } else if ( c1 == '-' && c2 == '-' ) {
	shift( 2 );
	return MINUSMINUS;
    } else if ( c1 == '=' && c2 == '=' ) {
	shift( 2 );
	return EQEQ;
    } else if ( c1 == '+' && c2 == '=' ) {
	shift( 2 );
	return PLUSEQUAL;
    } else if ( c1 == '-' && c2 == '=' ) {
	shift( 2 );
	return MINUSEQUAL;
    } else if ( c1 == '*' && c2 == '=' ) {
	shift( 2 );
	return MULTEQUAL;
    } else if ( c1 == '/' && c2 == '=' ) {
	shift( 2 );
	return DIVEQUAL;
    } else if ( c1 == '&' && c2 == '=' ) {
	shift( 2 );
	return ANDEQUAL;
    } else if ( c1 == '^' && c2 == '=' ) {
	shift( 2 );
	return XOREQUAL;
    } else if ( c1 == '%' && c2 == '=' ) {
	shift( 2 );
	return MODEQUAL;
    } else if ( c1 == '|' && c2 == '=' ) {
	shift( 2 );
	return OREQUAL;
    } else if ( c1 == '<' && c2 == '<' ) {
	shift( 2 );
	return LSHIFT;
    } else if ( c1 == '>' && c2 == '>' ) {
	shift( 2 );
	return RSHIFT;
    } else if ( c1 == '&' && c2 == '&' ) {
	shift( 2 );
	return AND;
    } else if ( c1 == '|' && c2 == '|' ) {
	shift( 2 );
	return OR;
    }

    switch( c1 ) {
    case '=':
    case '>':
    case '<':
    case ',':
    case '!':
    case '~':
    case '?':
    case ':':
    case '.':
    case '+':
    case '-':
    case '*':
    case '/':
    case '&':
    case '|':
    case '^':
    case '%':
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case ';':
	shift( 1 );
	return static_cast<int>( c1 );
    default:
	return -1;
    }
}

ushort QSLexer::singleEscape( ushort c ) const
{
    switch( c ) {
    case 'b':
	return 0x08;
    case 't':
	return 0x09;
    case 'n':
	return 0x0A;
    case 'v':
	return 0x0B;
    case 'f':
	return 0x0C;
    case 'r':
	return 0x0D;
    case '"':
	return 0x22;
    case '\'':
	return 0x27;
    case '\\':
	return 0x5C;
    default:
	return c;
    }
}

ushort QSLexer::convertOctal( ushort c1, ushort c2,
			    ushort c3 ) const
{
    return ((c1 - '0') * 64 + (c2 - '0') * 8 + c3 - '0');
}

unsigned char QSLexer::convertHex( ushort c )
{
    if ( c >= '0' && c <= '9' )
	return ( c - '0' );
    else if ( c >= 'a' && c <= 'f' )
	return ( c - 'a' + 10 );
    else
	return ( c - 'A' + 10 );
}

unsigned char QSLexer::convertHex( ushort c1, ushort c2 )
{
    return ( ( convertHex( c1 ) << 4 ) + convertHex( c2 ) );
}

QChar QSLexer::convertUnicode( ushort c1, ushort c2,
			     ushort c3, ushort c4 )
{
    return QChar( ( convertHex( c3 ) << 4 ) + convertHex( c4 ),
		  ( convertHex( c1 ) << 4 ) + convertHex( c2 ) );
}

void QSLexer::record8( ushort c )
{
    assert( c <= 0xff );

    // enlarge buffer if full
    if ( pos8 >= size8 - 1 ) {
	char *tmp = new char[2 * size8];
	memcpy( tmp, buffer8, size8 * sizeof(char) );
	delete [] buffer8;
	buffer8 = tmp;
	size8 *= 2;
    }

    buffer8[pos8++] = (char) c;
}

void QSLexer::record16( QChar c )
{
    // enlarge buffer if full
    if ( pos16 >= size16 - 1 ) {
	QChar *tmp = new QChar[2 * size16];
	memcpy(tmp, buffer16, size16 * sizeof(QChar));
	delete [] buffer16;
	buffer16 = tmp;
	size16 *= 2;
    }

    buffer16[pos16++] = c;
}

bool QSLexer::scanRegExp()
{
    pos16 = 0;
    bool lastWasEscape = FALSE;

    while ( 1 ) {
	if ( isLineTerminator() || current == 0 )
	    return FALSE;
	else if ( current != '/' || lastWasEscape == TRUE )
	    {
		record16( current );
		lastWasEscape = !lastWasEscape && ( current == '\\' );
	    }
	else {
	    pattern = QString( buffer16, pos16 );
	    pos16 = 0;
	    shift( 1 );
	    break;
	}
	shift( 1 );
    }

    while ( isIdentLetter( current ) ) {
	record16( current );
	shift( 1 );
    }
    flags = QString( buffer16, pos16 );

    return TRUE;
}
