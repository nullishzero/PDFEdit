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

#ifndef QUICKEDITORINTERFACE_H
#define QUICKEDITORINTERFACE_H

#ifndef QSA_NO_EDITOR

#include <QObject>
#include <QPointer>
#include <QEvent>
class QTimer;
class QAction;
class QTextEdit;
class QSAEditorInterfacePrivate;

class QSAEditorInterface : public QObject
{
    Q_OBJECT

public:
    QSAEditorInterface();
    virtual ~QSAEditorInterface();

    QWidget *editor(bool readonly, QWidget *parent);

    void setText(const QString &txt);
    QString text() const;
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
    void scrollTo(const QString &txt, const QString &first);
    void setContext(QObject *ctx);
    void readSettings();

    void setError(int line);
    void clearError();
    void setStep(int line);
    void clearStep();
    void clearStackFrame();
    void setStackFrame(int line);
    void setModified(bool b);
    bool isModified() const;

    int numLines() const;

private:
    QSAEditorInterfacePrivate *d;
};

#endif

#endif
