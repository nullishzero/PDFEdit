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

#include "qsasyntaxhighlighter.h"

#include <qapplication.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

#include "paragdata.h"
#include "q3richtext_p.h"

const char * const QSASyntaxHighlighter::keywords[] = {
    // ECMA keywords
    "break",
    "else",
    "new",
    "var",
    "case",
    "finally",
    "return",
    "void",
    "catch",
    "for",
    "switch",
    "while",
    "continue",
    "function",
    "this",
    "with",
    "default",
    "if",
    "throw",
    "delete",
    "in",
    "try",
    "do",
    "instanceof",
    "typeof",
    "class",
    "constructor",
    "extends",
    // future reserved words
    "abstract",
    "enum",
    "int",
    "short",
    "boolean",
    "export",
    "interface",
    "static",
    "byte",
    "long",
    "super",
    "char",
    "final",
    "native",
    "synchronized",
    "float",
    "package",
    "throws",
    "const",
    "goto",
    "private",
    "transient",
    "debugger",
    "implements",
    "protected",
    "volatile",
    "double",
    "import",
    "public",
    // end of array
    0
};

static QMap<int, QMap<QString, int > > *wordMap = 0;

QSASyntaxHighlighter::QSASyntaxHighlighter()
    : Q3TextPreProcessor(), lastFormat( 0 ), lastFormatId( -1 )
{
    int normalSize = QApplication::font().pointSize();
    QString normalFamily = QApplication::font().family();
    QString commentFamily = QString::fromLatin1("times");
    int normalWeight = QFont::Normal;
    addFormat( Standard,
	       new Q3TextFormat( QFont( normalFamily, normalSize, normalWeight ), Qt::black ) );
    addFormat( Comment,
	       new Q3TextFormat( QFont( commentFamily, normalSize, normalWeight, true ), Qt::red ) );
    addFormat( Number,
	       new Q3TextFormat( QFont( normalFamily, normalSize, normalWeight ), Qt::blue ) );
    addFormat( String,
	       new Q3TextFormat( QFont( normalFamily, normalSize, normalWeight ), Qt::darkGreen ) );
    addFormat( Type,
	       new Q3TextFormat( QFont( normalFamily, normalSize, normalWeight ), Qt::darkMagenta ) );
    addFormat( Keyword,
	       new Q3TextFormat( QFont( normalFamily, normalSize, QFont::Bold ), Qt::darkYellow ) );
    addFormat( Label,
	       new Q3TextFormat( QFont( normalFamily, normalSize, normalWeight ), Qt::darkRed ) );

    if ( wordMap )
	return;

    wordMap = new QMap<int, QMap<QString, int> >;
    int len;
    for ( int i = 0; keywords[ i ]; ++i ) {
	len = strlen( keywords[ i ] );
	if ( !wordMap->contains( len ) )
	    wordMap->insert( len, QMap<QString, int >() );
	QMap<QString, int> &map = wordMap->operator[]( len );
	map[ QString::fromLatin1(keywords[ i ]) ] = Keyword;
    }

    formats.setAutoDelete( true );
}

static int string2Id( const QString &s )
{
    if ( s == QString::fromLatin1("Standard") )
	return QSASyntaxHighlighter::Standard;
    if ( s == QString::fromLatin1("Comment") )
	return QSASyntaxHighlighter::Comment;
    if ( s == QString::fromLatin1("Number") )
	return QSASyntaxHighlighter::Number;
    if ( s == QString::fromLatin1("String") )
	return QSASyntaxHighlighter::String;
    if ( s == QString::fromLatin1("Type") )
	return QSASyntaxHighlighter::Type;
    if ( s == QString::fromLatin1("Label") )
	return QSASyntaxHighlighter::Label;
    if ( s == QString::fromLatin1("Keyword") )
	return QSASyntaxHighlighter::Keyword;
    return QSASyntaxHighlighter::Standard;
}

void QSASyntaxHighlighter::updateStyles( const QMap<QString, ConfigStyle> &styles )
{
    for ( QMap<QString, ConfigStyle>::ConstIterator it = styles.begin(); it != styles.end(); ++it ) {
	int id = string2Id( it.key() );
	Q3TextFormat *f = format( id );
	if ( !f )
	    continue;
	f->setFont( (*it).font );
	f->setColor( (*it).color );
    }
}

static bool checkFunctionEnd( Q3TextParagraph *p, int index )
{
    ParenList parenList = ( (ParagData*)p->extraData() )->parenList;

    int closed = 0;
    Q3TextCursor cursor( p->document() );
    cursor.setParagraph( p );
    cursor.setIndex( index );
    do {
	if ( cursor.paragraph()->at( cursor.index() )->c == '{' )
	    closed--;
	else if ( cursor.paragraph()->at( cursor.index() )->c == '}' )
	    closed++;
 	if ( cursor.index() == 0 && cursor.paragraph() == cursor.paragraph()->document()->firstParagraph() || !closed )
	    break;
	cursor.gotoLeft();

    } while ( closed );

    if ( !closed ) {
	QString text;
	cursor.gotoLeft();
	while ( true ) {
	    QChar c( cursor.paragraph()->at( cursor.index() )->c );
	    text.prepend( c );
	    if ( text.startsWith( QString::fromLatin1("function") )
                 || text.startsWith( QString::fromLatin1("constructor") ) ||
		 text.startsWith( QString::fromLatin1("class") ) /*|| text.startsWith( "if" ) || text.startsWith( "for" ) ||
					      text.startsWith( "while" ) || text.startsWith( "else" )*/ ) {
		( (ParagData*)p->extraData() )->lineState = ParagData::FunctionEnd;
		return true;
	    }
	    if ( c == '{' || c == '}' )
		return false;
	    if ( cursor.index() == 0 && cursor.paragraph() == cursor.paragraph()->document()->firstParagraph() )
		break;
	    cursor.gotoLeft();
	}
    }
    return false;
}

void QSASyntaxHighlighter::process( Q3TextDocument *doc, Q3TextParagraph *string, int, bool invalidate )
{

    Q3TextFormat *formatStandard = format( Standard );
    Q3TextFormat *formatComment = format( Comment );
    Q3TextFormat *formatNumber = format( Number );
    Q3TextFormat *formatString = format( String );
    Q3TextFormat *formatType = format( Type );
    Q3TextFormat *formatPreProcessor = format( PreProcessor );
    Q3TextFormat *formatLabel = format( Label );

    // states
    const int StateStandard = 0;
    const int StateCommentStart1 = 1;
    const int StateCCommentStart2 = 2;
    const int StateCppCommentStart2 = 3;
    const int StateCComment = 4;
    const int StateCppComment = 5;
    const int StateCCommentEnd1 = 6;
    const int StateCCommentEnd2 = 7;
    const int StateStringStart = 8;
    const int StateString = 9;
    const int StateStringEnd = 10;
    const int StateString2Start = 11;
    const int StateString2 = 12;
    const int StateString2End = 13;
    const int StateNumber = 14;
    const int StatePreProcessor = 15;

    // tokens
    const int InputAlpha = 0;
    const int InputNumber = 1;
    const int InputAsterix = 2;
    const int InputSlash = 3;
    const int InputParen = 4;
    const int InputSpace = 5;
    const int InputHash = 6;
    const int InputQuotation = 7;
    const int InputApostrophe = 8;
    const int InputSep = 9;

    static const uchar table[ 16 ][ 10 ] = {
	{ StateStandard,      StateNumber,     StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateStandard
	{ StateStandard,      StateNumber,   StateCCommentStart2, StateCppCommentStart2, StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateCommentStart1
	{ StateCComment,      StateCComment,   StateCCommentEnd1,   StateCComment,         StateCComment,   StateCComment,   StateCComment,     StateCComment,    StateCComment,     StateCComment }, // StateCCommentStart2
	{ StateCppComment,    StateCppComment, StateCppComment,     StateCppComment,       StateCppComment, StateCppComment, StateCppComment,   StateCppComment,  StateCppComment,   StateCppComment }, // CppCommentStart2
	{ StateCComment,      StateCComment,   StateCCommentEnd1,   StateCComment,         StateCComment,   StateCComment,   StateCComment,     StateCComment,    StateCComment,     StateCComment }, // StateCComment
	{ StateCppComment,    StateCppComment, StateCppComment,     StateCppComment,       StateCppComment, StateCppComment, StateCppComment,   StateCppComment,  StateCppComment,   StateCppComment }, // StateCppComment
	{ StateCComment,      StateCComment,   StateCCommentEnd1,   StateCCommentEnd2,     StateCComment,   StateCComment,   StateCComment,     StateCComment,    StateCComment,     StateCComment }, // StateCCommentEnd1
	{ StateStandard,      StateNumber,     StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateCCommentEnd2
	{ StateString,        StateString,     StateString,         StateString,           StateString,     StateString,     StateString,       StateStringEnd,   StateString,       StateString }, // StateStringStart
	{ StateString,        StateString,     StateString,         StateString,           StateString,     StateString,     StateString,       StateStringEnd,   StateString,       StateString }, // StateString
	{ StateStandard,      StateStandard,   StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateStringEnd
	{ StateString2,       StateString2,    StateString2,        StateString2,          StateString2,    StateString2,    StateString2,      StateString2,     StateString2End,   StateString2 }, // StateString2Start
	{ StateString2,       StateString2,    StateString2,        StateString2,          StateString2,    StateString2,    StateString2,      StateString2,     StateString2End,   StateString2 }, // StateString2
	{ StateStandard,      StateStandard,   StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateString2End
	{ StateNumber,        StateNumber,     StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateNumber
	{ StatePreProcessor,  StateStandard,   StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard } // StatePreProcessor
    };

    QString buffer;

    int state = StateStandard;
    if ( string->prev() ) {
	if ( string->prev()->endState() == -1 )
	    process( doc, string->prev(), 0, false );
	state = string->prev()->endState();
    }
    int input = -1;
    int i = 0;
    bool lastWasBackSlash = false;
    bool makeLastStandard = false;

    ParagData *paragData = (ParagData*)string->extraData();
    if ( paragData )
	paragData->parenList.clear();
    else
	paragData = new ParagData;
    string->setExtraData( paragData );

    static QString alphabeth = QString::fromLatin1("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    static QString mathChars = QString::fromLatin1("xXeE");
    static QString numbers = QString::fromLatin1("0123456789");
    bool questionMark = false;
    bool resetLineState = true;
    QChar lastChar;
    QString firstWord;
    while ( true ) {
	QChar c = string->at( i )->c;

	if ( lastWasBackSlash ) {
	    input = InputSep;
	} else {
	    switch ( c.latin1() ) {
	    case '*':
		input = InputAsterix;
		break;
	    case '/':
		input = InputSlash;
		break;
	    case '(': case '[': case '{':
		input = InputParen;
		if ( state == StateStandard ||
		     state == StateNumber ||
		     state == StatePreProcessor ||
		     state == StateCCommentEnd2 ||
		     state == StateCCommentEnd1 ||
		     state == StateString2End ||
		     state == StateStringEnd )
		    paragData->parenList << Paren( Paren::Open, c, i );
		break;
	    case ')': case ']': case '}':
		input = InputParen;
		if ( state == StateStandard ||
		     state == StateNumber ||
		     state == StatePreProcessor ||
		     state == StateCCommentEnd2 ||
		     state == StateCCommentEnd1 ||
		     state == StateString2End ||
		     state == StateStringEnd ) {
		    paragData->parenList << Paren( Paren::Closed, c, i );
		    if ( c == '}' ) {
			if ( checkFunctionEnd( string, i ) )
			    resetLineState = false;
		    }
		}
		break;
	    case '#':
		input = InputHash;
		break;
	    case '"':
		input = InputQuotation;
		break;
	    case '\'':
		input = InputApostrophe;
		break;
	    case ' ':
		input = InputSpace;
		if ( firstWord == QString::fromLatin1("function") || firstWord == QString::fromLatin1("constructor") || firstWord == QString::fromLatin1("class")/* ||
												     firstWord == "if" || firstWord == "for" || firstWord == "while" || firstWord == "else"*/ ) {
		    paragData->lineState = ParagData::FunctionStart;
		    resetLineState = false;
		}
		break;
	    case '1': case '2': case '3': case '4': case '5':
	    case '6': case '7': case '8': case '9': case '0':
		if ( alphabeth.find( lastChar ) != -1 &&
		     ( mathChars.find( lastChar ) == -1 || numbers.find( string->at( i - 1 )->c ) == -1 ) ) {
		    input = InputAlpha;
		} else {
		    if ( input == InputAlpha && numbers.find( lastChar ) != -1 )
			input = InputAlpha;
		    else
			input = InputNumber;
		}
		break;
	    case ':': {
		input = InputAlpha;
		QChar nextChar = ' ';
		if ( i < string->length() - 1 )
		    nextChar = string->at( i + 1 )->c;
		if ( state == StateStandard && !questionMark &&
		     lastChar != ':' && nextChar != ':' && lastChar.isLetter() ) {
		    for ( int j = 0; j < i; ++j ) {
			if ( string->at( j )->format() == formatStandard )
			    string->setFormat( j, 1, formatLabel, false );
		    }
		}
		break;
	    }
	    default: {
		if ( c != '\t' )
		    firstWord += c;
		QString s = firstWord.simplifyWhiteSpace();
		if ( s == QString::fromLatin1("private") || s == QString::fromLatin1("protected") || s == QString::fromLatin1("public") || s == QString::fromLatin1("static") )
		    firstWord = "";
		if ( !questionMark && c == '?' )
		    questionMark = true;
		if ( c.isLetter() || c == '_' )
		    input = InputAlpha;
		else
		    input = InputSep;
	    } break;
	    }
	}

	lastWasBackSlash = !lastWasBackSlash && c == '\\';

	if ( input == InputAlpha )
	    buffer += c;

    	state = table[ state ][ input ];

	switch ( state ) {
	case StateStandard: {
	    int len = buffer.length();
	    string->setFormat( i, 1, formatStandard, false );
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    if ( buffer.length() > 0 && input != InputAlpha ) {
		if ( buffer == QString::fromLatin1("true") || buffer == QString::fromLatin1("false") ||
		     buffer == QString::fromLatin1("NaN") || buffer == QString::fromLatin1("Infinity") ||
		     buffer == QString::fromLatin1("undefined") ) {
		    string->setFormat( i - buffer.length(), buffer.length(), formatType, false );
		} else {
		    QMap<int, QMap<QString, int > >::Iterator it = wordMap->find( len );
		    if ( it != wordMap->end() ) {
			QMap<QString, int >::Iterator it2 = ( *it ).find( buffer );
			if ( it2 != ( *it ).end() )
			    string->setFormat( i - buffer.length(), buffer.length(), format( ( *it2 ) ), false );
		    }
		}
		buffer = QString::null;
	    }
	} break;
	case StateCommentStart1:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = true;
	    buffer = QString::null;
	    break;
	case StateCCommentStart2:
	    string->setFormat( i - 1, 2, formatComment, false );
	    makeLastStandard = false;
	    buffer = QString::null;
	    break;
	case StateCppCommentStart2:
	    string->setFormat( i - 1, 2, formatComment, false );
	    makeLastStandard = false;
	    buffer = QString::null;
	    break;
	case StateCComment:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatComment, false );
	    buffer = QString::null;
	    break;
	case StateCppComment:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatComment, false );
	    buffer = QString::null;
	    break;
	case StateCCommentEnd1:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatComment, false );
	    buffer = QString::null;
	    break;
	case StateCCommentEnd2:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatComment, false );
	    buffer = QString::null;
	    break;
	case StateStringStart:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatStandard, false );
	    buffer = QString::null;
	    break;
	case StateString:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatString, false );
	    buffer = QString::null;
	    break;
	case StateStringEnd:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatStandard, false );
	    buffer = QString::null;
	    break;
	case StateString2Start:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatStandard, false );
	    buffer = QString::null;
	    break;
	case StateString2:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatString, false );
	    buffer = QString::null;
	    break;
	case StateString2End:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatStandard, false );
	    buffer = QString::null;
	    break;
	case StateNumber:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatNumber, false );
	    buffer = QString::null;
	    break;
	case StatePreProcessor:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, false );
	    makeLastStandard = false;
	    string->setFormat( i, 1, formatPreProcessor, false );
	    buffer = QString::null;
	    break;
	}

	lastChar = c;
	i++;
	if ( i >= string->length() )
	    break;
    }

    if ( resetLineState )
	paragData->lineState = ParagData::InFunction;
    string->setExtraData( paragData );

    int oldEndState = string->endState();
    if ( state == StateCComment ||
	 state == StateCCommentEnd1 ) {
	string->setEndState( StateCComment );
    } else if ( state == StateString ) {
	string->setEndState( StateString );
    } else if ( state == StateString2 ) {
	string->setEndState( StateString2 );
    } else {
	string->setEndState( StateStandard );
    }

    string->setFirstPreProcess( false );

    Q3TextParagraph *p = string->next();
    if ( (!!oldEndState || !!string->endState()) && oldEndState != string->endState() &&
	 invalidate && p && !p->firstPreProcess() && p->endState() != -1 ) {
	while ( p ) {
	    if ( p->endState() == -1 )
		return;
	    p->setEndState( -1 );
	    p = p->next();
	}
    }
}

Q3TextFormat *QSASyntaxHighlighter::format( int id )
{
    if ( lastFormatId == id  && lastFormat )
	return lastFormat;

    Q3TextFormat *f = formats[ id ];
    lastFormat = f ? f : formats[ 0 ];
    lastFormatId = id;
    return lastFormat;
}

void QSASyntaxHighlighter::addFormat( int id, Q3TextFormat *f )
{
    formats.insert( id, f );
}

void QSASyntaxHighlighter::removeFormat( int id )
{
    formats.remove( id );
}

