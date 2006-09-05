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

#ifndef QSPROJECT_H
#define QSPROJECT_H

#include "qsaglobal.h"

#include <qobject.h>
#include <qobjectlist.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include <qsscript.h>

class QSEditor;
class QSProjectPrivate;
class QSInterpreter;

class QSA_EXPORT QSProject : public QObject
{
    friend class QSInterpreter;
    friend class QSInterfaceFactory;
    friend class QSEditor;

    Q_OBJECT

public:
    enum StorageMode {
	Bundle,
	TextFiles
    };

    QSProject( QObject *parent = 0, const char *name = 0 );
    virtual ~QSProject();

    bool editorsModified() const;
    bool scriptsModified() const;

    QSInterpreter *interpreter() const;
    QStringList scriptNames() const;
    QPtrList<QSScript> scripts() const;
    QSScript *script( const QString &name ) const;
    QSScript *script( QObject *context ) const;

    QObject *object( const QString &name ) const;
    QObjectList objects() const;

    QSScript *createScript( QObject *context, const QString &code = QString::null );
    QSScript *createScript( const QString &name, const QString &code = QString::null );

    void addSignalHandler( QObject *sender, const char *signal,
			   QObject *receiver, const char *qtscriptFunction );
    void addSignalHandler( QObject *sender, const char *signal,
			   const char *qtscriptFunction );
    void removeSignalHandler( QObject *sender, const char *signal,
			      QObject *receiver, const char *qtscriptFunction );
    void removeSignalHandler( QObject *sender, const char *signal,
			      const char *qtscriptFunction );

    QSEditor *activeEditor() const;
    QPtrList<QSEditor> editors() const;
    QSEditor *editor( QSScript *script ) const;
    QSEditor *createEditor( QSScript *script, QWidget *parent=0, const char *name = 0 );

    void setStorageMode(StorageMode mode);
    StorageMode storageMode() const;

public slots:
    bool load( const QString &fileName );
    bool save( const QString &fileName = QString::null );

    bool loadFromData( QByteArray data );
    bool saveToData( QByteArray data );

    void clearObjects();
    void addObject( QObject *object );
    void removeObject( const QObject *object );

    void commitEditorContents();
    void revertEditorContents();

signals:
    void editorTextChanged();
    void projectChanged();
    void projectEvaluated();

private slots:
    void objectDestroyed();
    void invalidateProject();
    void scriptChanged();
    void evaluate();

private:
    // disabled copy constructor and assignment operator
    QSProject( const QSProject & );
    QSProject& operator=( const QSProject & );

    void initObjects();
    void initEventHandlers();

    QSScript *createScriptInternal( const QString &name,
				   const QString &code,
				   QObject *context );
    bool saveInternal( QDataStream *strm );
    bool loadInternal( QDataStream *strm );
    void registerEditor( QSEditor *editor );
    void unregisterEditor( QSEditor *editor );

private:
    QSProjectPrivate *d;

};

#endif

