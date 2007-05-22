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

#ifndef QSLEXER_H
#define QSLEXER_H

#include <QString>

class QSLexer 
{
public:
    QSLexer();
    ~QSLexer();
    static QSLexer *lexer() { return lx; }

    void setCode(const QString &c, int id, int lineno=0);
    int lex();

    int lineNo() const { return yylineno + 1; }
    int sourceId() const { return sid; }

    bool prevTerminator() const { return terminator; }

    enum State { Start,
		 Identifier,
		 InIdentifier,
		 InSingleLineComment,
		 InMultiLineComment,
		 InNum,
		 InNum0,
		 InHex,
		 InOctal,
		 InDecimal,
		 InExponentIndicator,
		 InExponent,
		 Hex,
		 Octal,
		 Number,
		 String,
		 Eof,
		 InString,
		 InEscapeSequence,
		 InHexEscape,
		 InUnicodeEscape,
		 Other,
		 Bad };

    bool scanRegExp();
    QString pattern, flags;

    State lexerState() const { return state; }

    QString errorMessage() const { return errmsg; }
    void setErrorMessage(const QString &err) { errmsg = err; }
    void setErrorMessage(const char *err) { setErrorMessage(QString::fromLatin1(err)); }

private:
    int yylineno;
    bool done;
    char *buffer8;
    QChar *buffer16;
    uint size8, size16;
    uint pos8, pos16;
    bool terminator;
    bool restrKeyword;
    // encountered delimiter like "'" and "}" on last run
    bool delimited;
    int stackToken;

    State state;
    void setDone(State s);
    uint pos;
    void shift(uint p);
    int lookupKeyword(const char *);

    bool isWhiteSpace() const;
    bool isLineTerminator() const;
    bool isHexDigit(ushort c) const;
    bool isOctalDigit(ushort c) const;

    int matchPunctuator(ushort c1, ushort c2,
			 ushort c3, ushort c4);
    ushort singleEscape(ushort c) const;
    ushort convertOctal(ushort c1, ushort c2,
			 ushort c3) const;
public:
    static unsigned char convertHex(ushort c1);
    static unsigned char convertHex(ushort c1, ushort c2);
    static QChar convertUnicode(ushort c1, ushort c2,
				 ushort c3, ushort c4);
    static bool isIdentLetter(ushort c);
    static bool isDecimalDigit(ushort c);

private:

    static QSLexer *lx;
    static int sid;

    void record8(ushort c);
    void record16(QChar c);

    const QChar *code;
    uint length;
    int yycolumn;
    int bol;     // begin of line

    // current and following unicode characters
    ushort current, next1, next2, next3;

    struct keyword {
	const char *name;
	int token;
    };

    QString errmsg;
};

#endif
