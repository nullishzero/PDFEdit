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

#ifndef QSCHECK_H
#define QSCHECK_H

#include "qsclass.h"
#include "qserrors.h"
#include <QStringList>
#include <QList>
#include <QMap>

class QSNode;
class QSScopeResolution;
class QSFuncDeclNode;

typedef QMap<QString, QSClass *> QSTypeMap;
typedef QList<QSScopeResolution> QSScopeResolutionList;

class QSScopeResolution
{
public:
    enum ScopeType { PackageScope,
		     ClassScope,
		     FunctionScope,
		     BlockScope,
		     GlobalScope,
		     EvalScope,
		     UndefinedScope };

    QSScopeResolution() : type(UndefinedScope), cl(0) { }
    QSScopeResolution(QSClass *c) : type(ClassScope), cl(c) { }
    QSScopeResolution(QSFunctionScopeClass *f) : type(FunctionScope), cl(f) { }
    QSScopeResolution(QSGlobalClass *g) : type(GlobalScope), cl(g) { }
    QSScopeResolution(QSBlockScopeClass *b) : type(BlockScope), cl(b) { }
    QSScopeResolution(QSEvalScopeClass *c) : type(EvalScope), cl(c) { }


    QString name() const { return cl->identifier(); }
    bool isPackageScope() const { return type==PackageScope; }
    bool isClassScope() const { return type==ClassScope; }
    bool isFunctionScope() const { return type==FunctionScope; }
    bool isGlobalScope() const { return type==GlobalScope; }
    bool isBlockScope() const { return type==BlockScope; }

    ScopeType type;
    QSClass *cl;

};

class QSCheckData {
public:
    QSCheckData(QSEnv *e, QSGlobalClass *g = 0);
    ~QSCheckData();

    QSEnv *env() const { return currenv; }

    void enterEval(QSEvalScopeClass *c) { scopeStack.push_front(c); }
    void leaveEval() { scopeStack.pop_front(); }

    void enterClass(QSClass *c);
    void leaveClass();
    bool insideClass(const QString &name) const;
    void registerType(QSClass *c);
    QSClass *typeInfo(const QString &name) const;

    void enterFunction(QSFunctionScopeClass *c);
    void leaveFunction();

    void pushLabel(const QString &label) { lablist.push_front(label); }
    void popLabel() { lablist.pop_front(); }
    void setLabel(const QString &label);
    QString currentLabel() const;
    bool seenLabel(const QString &label) const;
    void clearLabel();

    void enterBlock(QSBlockScopeClass *cl);
    void leaveBlock();

    void enterLoop(const QString &label = QString::null);
    void leaveLoop();

    void enterSwitch();
    void leaveSwitch();

    void enterPackage(const QString &name);
    void leavePackage();

    bool inGlobal() const;
    bool inClass() const;
    bool inFunction() const;
    bool inLoop() const;
    bool inSwitch() const;
    bool inPackage() const;
    bool canReturn() const;

    QSClass *innermostClass() const;

    QSClass *currentClass() const;
    QSFunctionScopeClass *currentFunction() const;
    QSClass *currentScope() const;

    void addError(const QSNode *node,
		   QSErrorCode code, const QString &msg);
    void addError(const QSNode *node, const QString &msg);
    void addWarning(const QSNode *node,
		     QSErrorCode code, const QString &msg);
    void addWarning(const QSNode *node, const QString &msg);

    bool hasError() const;
    QSErrorCode errorCode() const;
    QList<int> errorLines() const;
    QStringList errorMessages() const;

    int lastAttributes() const { return lattrs; }
    void setLastAttributes(int a) { lattrs = a; }

    QSClass *lastType() const { return ltype; }
    void setLastType(QSClass *t) { ltype = t; }

    // configuration options
    bool globalStatementsForbidden() const { return (bool)noGlobStatements; }
    void setGlobalStatementsForbidden(bool f) { noGlobStatements = f; }

    QString globalName(const QString &name) const;

    int varBlockCount() const { return vbCount; }
    void setVarBlockCount(int ct) { vbCount = ct; }

    void setDirectLookupEnabled(bool en) { directLookup = en; }
    bool directLookupEnabled() { return directLookup; }

private:
    QSEnv *currenv;
    //    QString currfunc;
    QString currpack; 	// current package definition name
    QString lastlab;		// last label seen
    QStringList lablist;	// list of label for iteration statements
    //    QStringList flist;
    QSTypeMap tmap;
    QSScopeResolutionList scopeStack;

    // last something
    int lattrs;
    QSClass *ltype;
    int vbCount;
    int switchLevel;

    QList<QSErrorCode> ecodes;
    QList<int> elines;
    QStringList emsgs;

    uint noGlobStatements : 1;
    uint directLookup : 1;
};

inline bool QSCheckData::inClass() const
{
    return scopeStack.size()>0 && scopeStack.first().isClassScope();
}

inline bool QSCheckData::inFunction() const
{
    return scopeStack.size()>0 && scopeStack.first().isFunctionScope();
}

inline bool QSCheckData::inLoop() const
{
    return !lablist.isEmpty();
}

inline bool QSCheckData::inSwitch() const
{
    return switchLevel>0;
}

inline bool QSCheckData::inPackage() const
{
    return !currpack.isEmpty();
}

inline bool QSCheckData::hasError() const
{
    return !ecodes.isEmpty();
}

inline QSErrorCode QSCheckData::errorCode() const
{
    return ecodes.first();
}

inline QList<int> QSCheckData::errorLines() const
{
    return elines;
}

inline QStringList QSCheckData::errorMessages() const
{
    return emsgs;
}

#endif

