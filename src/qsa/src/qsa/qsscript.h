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

#ifndef QSSCRIPT_H
#define QSSCRIPT_H

#include <qobject.h>
#include <qstring.h>
#include <qsaglobal.h>

class QSProject;
class QSScriptPrivate;

class QSA_EXPORT QSScript : public QObject
{
    friend class QSProject;
    Q_OBJECT
public:
    virtual ~QSScript();

    QString name() const;
    QString code() const;
    QObject *context() const;

    void setCode( const QString &code );
    void addCode( const QString &code );
    bool addFunction( const QString &funcName, const QString &funcBody = QString::null );
    QSProject *project() const;

protected slots:
    virtual void objectDestroyed();

private:
    QSScript( QSProject *project,
	      const QString &name,
	      const QString &code,
	      QObject *context = 0 );
    QSScript( const QSScript &sc );
    QSScript& operator=( const QSScript &sc );

    void setContext( QObject *context );

signals:
    void codeChanged();

private:
    QSScriptPrivate *d;
};

#endif
