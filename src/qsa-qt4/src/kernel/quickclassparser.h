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

#ifndef QUICKCLASSPARSER_H
#define QUICKCLASSPARSER_H

#include <QString>
#include <QList>
#include <QStringList>

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
	bool operator==(const Function &f) const {
	    return (name == f.name &&
		     body == f.body &&
		     returnType == f.returnType &&
		     comments == f.comments);
	}
    };

    struct Connection
    {
	QString sender;
	QString signal;
	QString slot;
	bool operator==(const Connection &c) const {
	    return (sender == c.sender &&
		     signal == c.signal &&
		     slot == c.slot);
	}
    };

};

struct QuickClass
{
    enum Type { Global, Class } type;
    QList<LanguageInterface::Connection> connections;
    QList<LanguageInterface::Function> functions;
    QStringList variables;
    QString inherits;
    QString access;
    QString name;

    bool operator==(const QuickClass &other) const;
    QuickClass &operator=(const QuickClass &other) {
	connections = other.connections;
	functions = other.functions;
	variables = other.variables;
	name = other.name;
	inherits = other.inherits;
	access = other.access;
	return *this;
    }
};

class QuickClassParser
{
public:
    QuickClassParser();

    void parse(const QString &code);
    QList<QuickClass> classes() const { return clsses; }
    void setGlobalName(const QString &n) { gname = n; }

protected:
    void parseConnections();
    void parseFunction(int functionLength);
    void parseVariableDecl(const QString &expr);
    void parseClassStart();
    void parseCppComment();
    void parseCComment();

private:
    QList<QuickClass> clsses;
    QuickClass *currClass;
    QuickClass globalClass;
    int pos;
    QString formCode;
    QString lastAccess;
    QString gname;

    static QString legalChars;

};

#endif
