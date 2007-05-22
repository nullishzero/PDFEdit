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

#ifndef QSEDITOR_H
#define QSEDITOR_H

#include <qsaglobal.h>

#ifndef QSA_NO_EDITOR

#include <QtGui/QWidget>
#include <QtGui/QResizeEvent>
#include <QtCore/QEvent>

struct EditorInterface;
class Q3TextEdit;
class QTextEdit;
class QSInterpreter;
class QSProject;
class QSScript;
class QSEditorPrivate;

class QSA_EXPORT QSEditor : public QWidget
{
    Q_OBJECT
public:
    QSEditor(QWidget *parent, const char *name = 0);
    ~QSEditor();

    bool setScript(QSScript *source);
    QSScript *script() const;

    bool setInterpreter(QSInterpreter *interpreter, QObject *context = 0);
    QSInterpreter *interpreter() const;

    void setText(const QString &text);
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
    bool find(const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor);
    bool replace(const QString &find, const QString &replace, bool cs, bool wo,
		  bool forward, bool startAtCursor, bool replaceAll);
    void gotoLine(int line);
    void indent();
    void readSettings();

    void setErrorMark(int line);
    void removeErrorMark();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

#if defined(QSA_NEW_EDITOR)
    QTextEdit *textEdit() const;
#else
    Q3TextEdit *textEdit() const;
#endif
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
    void resizeEvent(QResizeEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private:
    void init();

private:
    QSEditorPrivate *d;
    uint active:1;
};

#endif

#endif
