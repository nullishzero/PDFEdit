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

#ifndef QSEDITOR_H
#define QSEDITOR_H

#include <qsaglobal.h>
#include <qwidget.h>

struct EditorInterface;
class QTextEdit;
class QSInterpreter;
class QSProject;
class QSScript;
class QSEditorPrivate;

class QSA_EXPORT QSEditor : public QWidget
{
    Q_OBJECT
public:
    QSEditor( QWidget *parent, const char *name = 0 );
    ~QSEditor();

    bool setScript( QSScript *source );
    QSScript *script() const;

    bool setInterpreter( QSInterpreter *interpreter, QObject *context = 0 );
    QSInterpreter *interpreter() const;

    void setText( const QString &text );
    QString text() const;

    bool isModified() const;
    void commit();
    void revert();

public:
    bool isActive() const { return active; }

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    bool find( const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor );
    bool replace( const QString &find, const QString &replace, bool cs, bool wo,
		  bool forward, bool startAtCursor, bool replaceAll );
    void gotoLine( int line );
    void indent();
    void readSettings();

    void setErrorMark( int line );
    void removeErrorMark();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QTextEdit *textEdit() const;
    QSProject *project() const;

signals:
    void textChanged();

private slots:
    void scriptChanged();
    void scriptDestroyed();
    void modifyEditor();

private:
    bool activate();
    void release();

protected:
    void resizeEvent( QResizeEvent *e );
    bool eventFilter( QObject *o, QEvent *e );

private:
    void init();

private:
    QSEditorPrivate *d;
    uint active:1;
};

#endif
