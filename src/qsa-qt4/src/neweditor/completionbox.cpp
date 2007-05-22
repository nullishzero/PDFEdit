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

#include "completionbox.h"
#include <QListWidget>
#include <QDebug>
#include <QApplication>
#include <QKeyEvent>

CompletionBox::CompletionBox(QSAEditor *_editor, const QVector<CompletionEntry> &_completions)
{
    editor = _editor;
    completions = _completions;

    populate();
}

CompletionBox::~CompletionBox()
{
}

void CompletionBox::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Down:
        case Qt::Key_Up:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Tab:
            QListWidget::keyPressEvent(e);
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            e->accept();
            finishCompletion();
            window()->close();
            break;
        default:
            if (e->key() == Qt::Key_Backspace) {
                searchString.chop(1);
                if (searchString.isEmpty())
                    window()->close();
            } else {
                searchString.append(e->text());
            }

            populate();

            if (count() == 0)
                window()->close();

            QApplication::sendEvent(editor, e);
            break;
    }
}

void CompletionBox::finishCompletion()
{
    QListWidgetItem *item = currentItem();
    if (!item)
        return;
    QString s = item->data(Qt::UserRole).toString().mid(searchString.length());
    editor->insertPlainText(s);
}

void CompletionBox::populate()
{
    clear();
    foreach (CompletionEntry entry, completions) {
        if (!searchString.isEmpty()
            && !entry.text.startsWith(searchString))
            continue;

        QString text(" ");
        if (!entry.type.isEmpty()) {
            text += entry.type;
            text += QLatin1Char('\t');
        } else {
            text += QLatin1String(" ");
        }
        text += entry.prefix;
        text += entry.text;
        text += entry.postfix;
        text += entry.postfix2;

        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, entry.text);
        addItem(item);
    }
}

