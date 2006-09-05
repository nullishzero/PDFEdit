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

#ifndef QSACOMPLETION_H
#define QSACOMPLETION_H

#include "quickobjects.h"

#include <private/qcom_p.h>
#include <private/qucom_p.h>
#include <private/qucomextra_p.h>

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
    QPtrVector<QObject> qobj;

    enum Type { TQSObject, TQMetaObject, TQObject, TNull } type;

    QSCompletionObject( const QSObject &o )
	: qsobj( o ), type( TQSObject ) {}
    QSCompletionObject( const QMetaObject *m )
	: meta( m ), type( TQMetaObject ) {}
    QSCompletionObject( const QPtrVector<QObject> &o )
	: qobj( o ), type( TQObject ) {}
    QSCompletionObject()
	: type( TNull ) {}

    QSCompletionObject &operator=( const QSObject &o ) {
	qsobj = o;
	type = TQSObject;
	return *this;
    }
    QSCompletionObject &operator=( const QMetaObject *m ) {
	meta = m;
	type = TQMetaObject;
	return *this;
    }
    QSCompletionObject &operator=( const QPtrVector<QObject> &o ) {
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
    QSACompletion( Editor *e );

    bool doObjectCompletion( const QString &object );
    QValueList<QStringList> functionParameters( const QString &func, QChar &, QString &prefix, QString &postfix );
    void setContext( QObject *this_ );

    void setInterpreter( QSInterpreter *ip ) { qsInterp = ip; }

private:
    QString functionCode() const;

    QValueList<QPair<QString, QString> > parseAssignments( const QString &code ) const;
    QString resolveValue( const QString &value, const QValueList<QPair<QString, QString> > &assignments ) const;
    QString resolveFullyQualifiedValue( const QString &value, const QValueList<QPair<QString, QString> > &assignments ) const;

    QSCompletionObject queryObject( const QString &object );
    QSCompletionObject queryCompletionObject( QSCompletionObject &ctx, const QString &property ) const;
    QSObject queryQSObject( QSObject &ctx, const QString &property ) const;
    QSObject queryQSObject( const QMetaObject *meta, const QString &property, bool includeSuperClass ) const;
    QSObject queryQSObject( const QPtrVector<QObject> &objects, const QString &property ) const;
    const QMetaObject *queryQMetaObject( const QMetaObject *meta, const QString &property, bool includeSuperClass ) const;
    const QMetaObject *queryQMetaObject( const QPtrVector<QObject> &objects, const QString &property ) const;
    QPtrVector<QObject> queryQObject( const QPtrVector<QObject> &objects, const QString &property ) const;

    void completeQSObject( QSObject &obj, QValueList<CompletionEntry> &res, bool assumedStatic );
    enum QSMetaObjectCompletionFlags { IncludeSuperClass = 1, IsUnnamed = 2 };
    void completeQMetaObject( const QMetaObject *meta,
			      const QString &object,
			      QValueList<CompletionEntry> &res,
			      int flags,
			      QSObject &obj );
    void completeQObject( const QPtrVector<QObject> &objects,
			  const QString &object,
			  QValueList<CompletionEntry> &res );

    QString cppClassForScript( const QString &className ) const;
    const QMetaObject *locateMetaObject( const QString &className ) const;

private:
    QSEnv *env() const;
    QuickInterpreter *interpreter() const;

    QPtrVector<QObject>* interfaceObjects( const QSObject &o ) const;
    QSObject vTypeToQSType( const QString &type ) const;
    QSObject uTypeToQSType( QUType *t, const char *extra ) const;

    QObject *thisObject;
    QuickInterpreter *interp;
    QSInterpreter *qsInterp;
};

#endif
