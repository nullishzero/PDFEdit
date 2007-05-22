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

#include "qsaeditorinterface.h"
#include "qsaeditor.h"
#include "qsyntaxhighlighter.h"
#include "blockdata.h"
#include "linenumberwidget.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QTextCursor>
#include <QTextLayout>

class QSAEditorInterfacePrivate
{
public:
    inline QSAEditorInterfacePrivate() : editor(0), lineNumberWidget(0) {}

    QTextCursor gotoLine(int line) const
    {
        int cnt = 0;
        for (QTextBlock blk = editor->document()->begin();
             blk.isValid(); blk = blk.next(), ++cnt) {
            if (cnt == line) {
                return QTextCursor(blk);
            }
        }
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfLine);
        return cursor;
    }

    QSAEditor *editor;
    LineNumberWidget *lineNumberWidget;
    QTextCharFormat errorMarkerFormat;
};

QSAEditorInterface::QSAEditorInterface()
{
    d = new QSAEditorInterfacePrivate;
    d->errorMarkerFormat.setBackground(Qt::red);
    d->errorMarkerFormat.setProperty(ErrorMarkerPropertyId, true);
}

QSAEditorInterface::~QSAEditorInterface()
{
    delete d;
}

QWidget *QSAEditorInterface::editor(bool readonly, QWidget *parent)
{
    if (!d->editor) {
        d->editor = new QSAEditor;
        d->lineNumberWidget = new LineNumberWidget(d->editor);

        QWidget *w = new QWidget(parent);
        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(d->lineNumberWidget);
        layout->addWidget(d->editor);

        w->setLayout(layout);

        d->editor->setReadOnly(readonly);
    }
    return d->editor;
}

void QSAEditorInterface::setText(const QString &txt)
{
    if (!d->editor) return;
    d->editor->setPlainText(txt);
}

QString QSAEditorInterface::text() const
{
    if (!d->editor) return QString();
    return d->editor->toPlainText();
}

bool QSAEditorInterface::isUndoAvailable() const
{
    if (!d->editor) return false;
    return d->editor->document()->isUndoAvailable();
}

bool QSAEditorInterface::isRedoAvailable() const
{
    if (!d->editor) return false;
    return d->editor->document()->isRedoAvailable();
}

void QSAEditorInterface::undo()
{
    if (!d->editor) return;
    d->editor->document()->undo();
}

void QSAEditorInterface::redo()
{
    if (!d->editor) return;
    d->editor->document()->redo();
}

void QSAEditorInterface::cut()
{
    if (!d->editor) return;
    d->editor->cut();
}

void QSAEditorInterface::copy()
{
    if (!d->editor) return;
    d->editor->copy();
}

void QSAEditorInterface::paste()
{
    if (!d->editor) return;
    d->editor->paste();
}

void QSAEditorInterface::selectAll()
{
    if (!d->editor) return;
    d->editor->selectAll();
}

bool QSAEditorInterface::find(const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor)
{
    if (!d->editor) return false;
    return false;
}

bool QSAEditorInterface::replace(const QString &find, const QString &replace, bool cs, bool wo,
        bool forward, bool startAtCursor, bool replaceAll)
{
    if (!d->editor) return false;
    return false;
}

void QSAEditorInterface::gotoLine(int line)
{
    if (!d->editor) return;
    d->editor->setTextCursor(d->gotoLine(line));
}

void QSAEditorInterface::indent()
{
    if (!d->editor) return;
}

void QSAEditorInterface::scrollTo(const QString &txt, const QString &first)
{
    if (!d->editor) return;
}

void QSAEditorInterface::setContext(QObject *ctx)
{
    if (!d->editor) return;
    d->editor->setContextObject(ctx);
}

void QSAEditorInterface::readSettings()
{
    if (!d->editor) return;
    d->editor->readSettings();
}

void QSAEditorInterface::setError(int line)
{
    if (!d->editor) return;
    clearError();
    QTextCursor cursor = d->gotoLine(line);
    QTextBlock blk = cursor.block();

    BlockData *bd = BlockData::data(blk);
    if (!bd) {
        bd = new BlockData;
        bd->setToBlock(blk);
    }

    bd->errorMarkerFormat = d->errorMarkerFormat;

    QSAEditor::clearMarkerFormat(blk, ErrorMarkerPropertyId);

    QTextLayout *layout = blk.layout();
    QList<QTextLayout::FormatRange> formats = layout->additionalFormats();

    QTextLayout::FormatRange r;
    r.start = 0;
    r.length = blk.length() - 1;
    r.format = d->errorMarkerFormat;
    formats.append(r);

    layout->setAdditionalFormats(formats);

    cursor.movePosition(QTextCursor::StartOfBlock);
    d->editor->setTextCursor(cursor);
}

void QSAEditorInterface::clearError()
{
    if (!d->editor) return;
    for (QTextBlock blk = d->editor->document()->begin();
         blk.isValid(); blk = blk.next()) {

        if (BlockData *bd = BlockData::data(blk))
            bd->errorMarkerFormat = QTextCharFormat();

        QSAEditor::clearMarkerFormat(blk, ErrorMarkerPropertyId);
    }
}

void QSAEditorInterface::setStep(int line)
{
    if (!d->editor) return;
}

void QSAEditorInterface::clearStep()
{
    if (!d->editor) return;
}

void QSAEditorInterface::clearStackFrame()
{
    if (!d->editor) return;
}

void QSAEditorInterface::setStackFrame(int line)
{
    if (!d->editor) return;
}

void QSAEditorInterface::setModified(bool b)
{
    if (!d->editor) return;
    d->editor->document()->setModified(b);
}

bool QSAEditorInterface::isModified() const
{
    if (!d->editor) return false;
    return d->editor->document()->isModified();
}

int QSAEditorInterface::numLines() const
{
    if (!d->editor) return 0;
    int cnt = 0;
    for (QTextBlock blk = d->editor->document()->begin();
         blk.isValid(); blk = blk.next())
        ++cnt;
    return cnt;
}

