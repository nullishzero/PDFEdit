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

#ifndef QSPROJECT_H
#define QSPROJECT_H

#include "qsaglobal.h"
#include <qsscript.h>
#include <qseditor.h>

#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include <QtCore/QList>

class QSProjectPrivate;
class QSInterpreter;

class QSA_EXPORT QSProject : public QObject
{
    friend class QSInterpreter;
    friend class QSInterfaceFactory;

#ifndef QSA_NO_EDITOR   
    friend class QSEditor;
#endif

    Q_OBJECT

public:
    enum StorageMode {
	    Bundle,
	    TextFiles
    };

    QSProject(QObject *parent = 0, const char *name = 0);
    virtual ~QSProject();

#ifndef QSA_NO_EDITOR
    bool editorsModified() const;
#endif

    bool scriptsModified() const;

    QSInterpreter *interpreter() const;
    QStringList scriptNames() const;
    QList<QSScript *> scripts() const;
    QSScript *script(const QString &name) const;
    QSScript *script(QObject *context) const;

    QObject *object(const QString &name) const;
    QObjectList objects() const;

    QSScript *createScript(QObject *context, const QString &code = QString::null);
    QSScript *createScript(const QString &name, const QString &code = QString::null);

    void addSignalHandler(QObject *sender, const char *signal,
			   QObject *receiver, const char *qtscriptFunction);
    void addSignalHandler(QObject *sender, const char *signal,
			   const char *qtscriptFunction);
    void removeSignalHandler(QObject *sender, const char *signal,
			      QObject *receiver, const char *qtscriptFunction);
    void removeSignalHandler(QObject *sender, const char *signal,
			      const char *qtscriptFunction);

#ifndef QSA_NO_EDITOR
    QSEditor *activeEditor() const;
    QList<QSEditor *> editors() const;
    QSEditor *editor(QSScript *script) const;
    QSEditor *createEditor(QSScript *script, QWidget *parent=0, const char *name = 0);
#endif

    void setStorageMode(StorageMode mode);
    StorageMode storageMode() const;

public slots:
    bool load(const QString &fileName);
    bool save(const QString &fileName = QString::null);

    bool loadFromData(QByteArray *data);
    bool saveToData(QByteArray *data);

    void clearObjects();
    void addObject(QObject *object);
    void removeObject(const QObject *object);

#ifndef QSA_NO_EDITOR
    void commitEditorContents();
    void revertEditorContents();
#endif

signals:
#ifndef QSA_NO_EDITOR
    void editorTextChanged();
#endif
    void projectChanged();
    void projectEvaluated();

private slots:
    void objectDestroyed();
    void invalidateProject();
    void scriptChanged();
    void evaluate();

private:
    // disabled copy constructor and assignment operator
    QSProject(const QSProject &);
    QSProject& operator=(const QSProject &);

    void initObjects();
    void initEventHandlers();

    QSScript *createScriptInternal(const QString &name,
				   const QString &code,
				   QObject *context);
    bool saveInternal(QDataStream *strm);
    bool loadInternal(QDataStream *strm);

#ifndef QSA_NO_EDITOR
    void registerEditor(QSEditor *editor);
    void unregisterEditor(QSEditor *editor);
#endif

private:
    QSProjectPrivate *d;

};

#endif

