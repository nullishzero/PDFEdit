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

#ifndef QSINTERPRETER_H
#define QSINTERPRETER_H

#include "qsaglobal.h"
#include "qsproject.h"

#include <qobject.h>
#include <qvariant.h>
#include <qobject.h>
#include <QtCore/QList>
#include <QtCore/QSet>


class QSProject;
class QSInterpreterPrivate;
class QuickInterpreter;
class QSObjectFactory;
class QSWrapperFactory;

class QSA_EXPORT QSStackFrame
{
public:
    QSStackFrame(const QString &fun = QString::null,
		  const QString &file = QString::null,
		  int line = 0,
		  QObject *con = 0) :
	fname(file), func(fun), ln(line), cont(con)
    {
    }

    QString scriptName() const { return fname; }
    QString function() const { return func; }
    int line() const { return ln; }
    QObject *context() const { return cont; }
private:
    QString fname, func;
    int ln;
    QObject *cont;
};

bool operator==(const QSStackFrame &a, const QSStackFrame &b);

typedef QList<QSStackFrame> QSStackTrace;

class QSA_EXPORT QSInterpreter : public QObject
{
    friend class QSProject;
    friend class QSWrapperFactory; // for access to interpreter()
    friend class QSObjectFactory;
    friend class QSInterfaceFactory;
    friend class QSToolkitFactory;
    friend QuickInterpreter *get_quick_interpreter(QSInterpreter *ip);

    Q_OBJECT
    Q_ENUMS(ErrorMode)
    Q_PROPERTY(ErrorMode errorMode READ errorMode WRITE setErrorMode)
    Q_PROPERTY(int timeoutInterval READ timeoutInterval WRITE setTimeoutInterval)

public:
    enum ErrorMode { Notify, Nothing };
    enum ClassFlags { AllClasses, GlobalClasses };
    enum FunctionFlags { FunctionNames = 0,
			 FunctionSignatures = 1,
			 IncludeMemberFunctions = 2 };

    QSInterpreter(QObject *parent = 0, const char *name = 0);
    virtual ~QSInterpreter();

    static QSInterpreter *defaultInterpreter();

    QVariant evaluate(const QString &code, QObject *context = 0,
                      const QString &scriptName = QString::null);
    QVariant call(const QString &function, const QVariantList &arguments = QVariantList(),
                  QObject *context = 0);
    QObject *currentContext() const;

    bool checkSyntax(const QString &code);

    void setErrorMode(ErrorMode m);
    ErrorMode errorMode() const;

    QSProject* project() const;

    QStringList functions(FunctionFlags flag = FunctionNames) const;
    QStringList functions(const QString &context,
			   uint mask = FunctionNames) const;
    QStringList functions(QObject *context,
			   FunctionFlags flag = FunctionNames) const;

    QStringList classes(ClassFlags = AllClasses) const;
    QStringList classes(const QString &context) const;
    QStringList classes(QObject *context) const;

    QStringList variables() const;
    QStringList variables(const QString &context) const;
    QStringList variables(QObject *context) const;

    bool hasFunction(const QString &function) const;
    bool hasClass(const QString &className) const;
    bool hasVariable(const QString &variable) const;

    void addTransientVariable(const QString &variable, const QVariant &arg, QObject *context=0);
    QVariant variable(const QString &variable, QObject *context=0) const;

    QObjectList presentObjects() const;
    void addTransientObject(QObject *object);

    bool isRunning() const;

    void throwError(const QString &message);

    void addObjectFactory(QSObjectFactory *factory);
    void addWrapperFactory(QSWrapperFactory *factory);

    QString stackTraceString() const;
    QSStackTrace stackTrace() const;
    QString errorMessage() const;
    bool hadError() const;

    void addTransientSignalHandler(QObject *sender,
				    const char *signal,
				    const char *qtscriptFunction);

    void removeTransientSignalHandler(QObject *sender,
				       const char *signal,
				       const char *qtscriptFunction);

    void setTimeoutInterval(int msecs);
    int timeoutInterval() const;

    static void registerMetaObject(const QMetaObject *metaObject);

public slots:
    void clear();
    void stopExecution();

signals:
    void error(const QString &message, const QString &scriptName,
		int lineNumber);
    void error(const QString &message, QObject *context,
		const QString &scriptName, int lineNumber);

    void timeout(int runnningTime);

private:
    // disabled copy constructor and assignment operator
    QSInterpreter(const QSInterpreter &);
    QSInterpreter& operator=(const QSInterpreter &);
    QSInterpreter(QSProject *project, const char *name = 0);

    void init();

    // used by factory classes
    QuickInterpreter *interpreter() const;

    void removeObjectFactory(QSObjectFactory *factory);
    void removeWrapperFactory(QSWrapperFactory *factory);

private slots:
    void runtimeError(const QString &message, const QString &filename, int lineNumber);
    void runtimeError();
    void parseError();

private:
    QSInterpreterPrivate *d;

    uint running:1;
};

inline bool QSInterpreter::isRunning() const
{
    return running;
}

#endif
