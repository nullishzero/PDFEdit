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

#ifndef QSACOMPLETION_H
#define QSACOMPLETION_H

#ifndef QSA_NO_EDITOR

#include "quickobjects.h"
#include <Q3ValueList>
#include <QPair>

#include <completion.h>

class QSInterpreter;
class QuickInterpreter;
class QSEnv;

struct QSCompletionObject
{
    // this should be an anonymous union, but doesn't work as you
    // can't have objects with a constructor in an union
    QSObject qsobj;
    const QMetaObject *meta;
    QVector<QObject *> qobj;

    enum Type { TQSObject, TQMetaObject, TQObject, TNull } type;

    QSCompletionObject(const QSObject &o)
	: qsobj(o), type(TQSObject) {}
    QSCompletionObject(const QMetaObject *m)
	: meta(m), type(TQMetaObject) {}
    QSCompletionObject(const QVector<QObject *> &o)
	: qobj(o), type(TQObject) {}
    QSCompletionObject()
	: type(TNull) {}

    QSCompletionObject &operator=(const QSObject &o) {
	qsobj = o;
	type = TQSObject;
	return *this;
    }
    QSCompletionObject &operator=(const QMetaObject *m) {
	meta = m;
	type = TQMetaObject;
	return *this;
    }
    QSCompletionObject &operator=(const QVector<QObject *> &o) {
	qobj = o;
	type = TQObject;
	return *this;
    }

    void resolve();
    bool isNull() const;
};

class QSACompletion : public EditorCompletion
{
    Q_OBJECT

public:
    QSACompletion(Editor *e);

    bool doObjectCompletion(const QString &object);
    Q3ValueList<QStringList> functionParameters(const QString &func, QChar &, QString &prefix, QString &postfix);
    void setContext(QObject *this_);

    void setInterpreter(QSInterpreter *ip) { qsInterp = ip; }

private:
    QString functionCode() const;

    Q3ValueList<QPair<QString, QString> > parseAssignments(const QString &code) const;
    QString resolveValue(const QString &value, const Q3ValueList<QPair<QString, QString> > &assignments) const;
    QString resolveFullyQualifiedValue(const QString &value, const Q3ValueList<QPair<QString, QString> > &assignments) const;

    QSCompletionObject queryObject(const QString &object);
    QSCompletionObject queryCompletionObject(QSCompletionObject &ctx, const QString &property) const;
    QSObject queryQSObject(QSObject &ctx, const QString &property) const;
    QSObject queryQSObject(const QMetaObject *meta, const QString &property, bool includeSuperClass) const;
    QSObject queryQSObject(const QVector<QObject *> &objects, const QString &property) const;
    const QMetaObject *queryQMetaObject(const QMetaObject *meta, const QString &property, bool includeSuperClass) const;
    const QMetaObject *queryQMetaObject(const QVector<QObject *> &objects, const QString &property) const;
    QVector<QObject *> queryQObject(const QVector<QObject *> &objects, const QString &property) const;

    void completeQSObject(QSObject &obj, Q3ValueList<CompletionEntry> &res, bool assumedStatic);
    enum QSMetaObjectCompletionFlags { IncludeSuperClass = 1, IsUnnamed = 2 };
    void completeQMetaObject(const QMetaObject *meta,
			      const QString &object,
			      Q3ValueList<CompletionEntry> &res,
			      int flags,
			      QSObject &obj);
    void completeQObject(const QVector<QObject *> &objects,
			  const QString &object,
			  Q3ValueList<CompletionEntry> &res);

    QString cppClassForScript(const QString &className) const;
    const QMetaObject *locateMetaObject(const QString &className) const;

private:
    QSEnv *env() const;
    QuickInterpreter *interpreter() const;

    QVector<QObject *>* interfaceObjects(const QSObject &o) const;
    QSObject vTypeToQSType(const QString &type) const;
//     QSObject uTypeToQSType(QUType *t, const char *extra) const;

    QObject *thisObject;
    QuickInterpreter *interp;
    QSInterpreter *qsInterp;
};

#endif

#endif
