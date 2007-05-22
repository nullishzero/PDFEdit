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

#ifndef QSYNTAXHIGHLIGHTER_H
#define QSYNTAXHIGHLIGHTER_H

#include <QtCore/qobject.h>
#include <QtGui/qtextobject.h>

class QTextDocument;
class QSyntaxHighlighterPrivate;
class QTextCharFormat;
class QFont;
class QColor;
class QTextBlockUserData;
class QTextEdit;
class QSyntaxHighlighterPrivate;

class QSyntaxHighlighter : public QObject
{
    Q_OBJECT
    friend class QSyntaxHighlighterPrivate;
public:
    QSyntaxHighlighter(QObject *parent);
    QSyntaxHighlighter(QTextDocument *parent);
    QSyntaxHighlighter(QTextEdit *parent);
    virtual ~QSyntaxHighlighter();

    void setDocument(QTextDocument *doc);
    QTextDocument *document() const;

    // ####### hooks until 4.1
    static QTextBlockUserData *userData(const QTextBlock block) { return block.userData(); }
    static void setUserData(QTextBlock block, QTextBlockUserData *data) { block.setUserData(data); }

protected:
    virtual void highlightBlock(const QString &text) = 0;

    void setFormat(int start, int count, const QTextCharFormat &format);
    void setFormat(int start, int count, const QColor &color);
    void setFormat(int start, int count, const QFont &font);
    QTextCharFormat format(int pos) const;

    int previousBlockState() const;
    int currentBlockState() const;
    void setCurrentBlockState(int newState);

    void setCurrentBlockUserData(QTextBlockUserData *data);
    QTextBlockUserData *currentBlockUserData() const;

private:
    static int blockState(const QTextBlock &block) { return block.userState(); }
    Q_DISABLE_COPY(QSyntaxHighlighter)
    Q_PRIVATE_SLOT(d, void reformatBlocks(int from, int charsRemoved, int charsAdded));
    QSyntaxHighlighterPrivate *d;
};

#endif // QSYNTAXHIGHLIGHTER_H
