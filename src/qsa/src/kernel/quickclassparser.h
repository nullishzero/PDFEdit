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

#ifndef QUICKCLASSPARSER_H
#define QUICKCLASSPARSER_H

#include "dlldefs.h"
#include <qstring.h>
#include <qvaluelist.h>
#include <qstringlist.h>

struct LanguageInterface
{
    struct Function
    {
	QString name;
	QString body;
	QString returnType;
	QString comments;
	int start;
	int end;
	QString access;
	bool operator==( const Function &f ) const {
	    return ( name == f.name &&
		     body == f.body &&
		     returnType == f.returnType &&
		     comments == f.comments );
	}
    };

    struct Connection
    {
	QString sender;
	QString signal;
	QString slot;
	bool operator==( const Connection &c ) const {
	    return ( sender == c.sender &&
		     signal == c.signal &&
		     slot == c.slot );
	}
    };

};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class QUICKCORE_EXPORT QValueList<LanguageInterface::Connection>;
template class QUICKCORE_EXPORT QValueList<LanguageInterface::Function>;
// MOC_SKIP_END
#endif

struct QUICKCORE_EXPORT QuickClass
{
    enum Type { Global, Class } type;
    QValueList<LanguageInterface::Connection> connections;
    QValueList<LanguageInterface::Function> functions;
    QStringList variables;
    QString inherits;
    QString access;
    QString name;

    bool operator==( const QuickClass &other ) const;
    QuickClass &operator=( const QuickClass &other ) {
	connections = other.connections;
	functions = other.functions;
	variables = other.variables;
	name = other.name;
	inherits = other.inherits;
	access = other.access;
	return *this;
    }
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class QUICKCORE_EXPORT QValueList<QuickClass>;
// MOC_SKIP_END
#endif

class QUICKCORE_EXPORT QuickClassParser
{
public:
    QuickClassParser();

    void parse( const QString &code );
    QValueList<QuickClass> classes() const { return clsses; }
    void setGlobalName( const QString &n ) { gname = n; }

protected:
    void parseConnections();
    void parseFunction( int functionLength );
    void parseVariableDecl( const QString &expr );
    void parseClassStart();
    void parseCppComment();
    void parseCComment();

private:
    QValueList<QuickClass> clsses;
    QuickClass *currClass;
    QuickClass globalClass;
    int pos;
    QString formCode;
    QString lastAccess;
    QString gname;

    static QString legalChars;

};

#endif
