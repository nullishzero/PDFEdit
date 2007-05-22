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

#include "qsyntaxhighlighter.h"

#include <qtextdocument.h>
#include <qtextlayout.h>
#include <qpointer.h>
#include <qtextobject.h>
#include <qtextcursor.h>
#include <qdebug.h>
#include <qtextedit.h>

struct QSyntaxHighlighterPrivate
{
    inline QSyntaxHighlighterPrivate(QSyntaxHighlighter *_q) : q(_q) {}

    QPointer<QTextDocument> doc;

    void reformatBlocks(int from, int charsRemoved, int charsAdded);
    void reformatBlock(QTextBlock block);

    void applyFormatChanges();
    QVector<QTextCharFormat> formatChanges;
    QTextBlock currentBlock;

    QSyntaxHighlighter *q;
};

void QSyntaxHighlighterPrivate::applyFormatChanges()
{
    QList<QTextLayout::FormatRange> ranges;

    QTextCharFormat emptyFormat;

    QTextLayout::FormatRange r;
    r.start = r.length = -1;

    int i = 0;
    while (i < formatChanges.count()) {

        while (i < formatChanges.count() && formatChanges.at(i) == emptyFormat)
            ++i;

        if (i >= formatChanges.count())
            break;

        r.start = i;
        r.format = formatChanges.at(i);

        while (i < formatChanges.count() && formatChanges.at(i) == r.format)
            ++i;

        if (i >= formatChanges.count())
            break;

        r.length = i - r.start;
        ranges << r;
        r.start = r.length = -1;
    }

    if (r.start != -1) {
        r.length = formatChanges.count() - r.start;
        ranges << r;
    }

    currentBlock.layout()->setAdditionalFormats(ranges);
}

void QSyntaxHighlighterPrivate::reformatBlocks(int from, int charsRemoved, int charsAdded)
{
    QTextBlock block = doc->findBlock(from);
    if (!block.isValid())
        return;

    QTextBlock endBlock;
    if (charsAdded > charsRemoved || charsAdded == charsRemoved)
        endBlock = doc->findBlock(from + charsAdded);
    else
        endBlock = block;

    bool forceHighlightOfNextBlock = false;

    while (block.isValid() 
           && (!(endBlock < block)
               || forceHighlightOfNextBlock
              )
          ) {
        const int stateBeforeHighlight = QSyntaxHighlighter::blockState(block);

        reformatBlock(block);

        forceHighlightOfNextBlock = (QSyntaxHighlighter::blockState(block) != stateBeforeHighlight);

        block = block.next();
    }

    formatChanges.clear();
}

void QSyntaxHighlighterPrivate::reformatBlock(QTextBlock block)
{
    Q_ASSERT_X(!currentBlock.isValid(), "QSyntaxHighlighter::reformatBlock()", "reFormatBlock() called recursively");

    currentBlock = block;
    QTextBlock previous = block.previous();

    formatChanges.fill(QTextCharFormat(), block.length() - 1);
    q->highlightBlock(block.text());
    applyFormatChanges();

    doc->markContentsDirty(block.position(), block.length());

    currentBlock = QTextBlock();
}

/*!
    \class QSyntaxHighlighter qsyntaxhighlighter.h
    \brief The QSyntaxHighlighter class is a base class for
    implementing QTextEdit syntax highlighters.

    \ingroup text

    A syntax highligher automatically highlights parts of the text in
    a QTextEdit, or more generally in a QTextDocument. Syntax highlighters
    are often used when the user is entering text in a specific format
    (for example, source code) and help the user to read the text and
    identify syntax errors.

    To provide your own syntax highlighting, you must subclass
    QSyntaxHighlighter and reimplement highlightBlock().

    When you create an instance of your QSyntaxHighlighter subclass,
    pass it the QTextEdit or QTextDocument that you want the syntax
    highlighting to be applied to. After this your highlightBlock()
    function will be called automatically whenever necessary. Use your
    highlightBlock() function to apply formatting (e.g. setting the
    font and color) to the text that is passed to it.
*/

/*!
    Constructs the QSyntaxHighlighter
*/
QSyntaxHighlighter::QSyntaxHighlighter(QObject *parent)
    : QObject(parent)
{
    d = new QSyntaxHighlighterPrivate(this);
}

/*!
    Constructs the QSyntaxHighlighter and installs it on \a parent.
    The specified QTextDocument also becomes the owner of the
    QSyntaxHighlighter.
*/
QSyntaxHighlighter::QSyntaxHighlighter(QTextDocument *parent)
    : QObject(parent)
{
    d = new QSyntaxHighlighterPrivate(this);
    setDocument(parent);
}

/*!
    Constructs the QSyntaxHighlighter and installs it on \a parent 's
    QTextDocument.
    The specified QTextEdit also becomes the owner of the
    QSyntaxHighlighter.
*/
QSyntaxHighlighter::QSyntaxHighlighter(QTextEdit *parent)
    : QObject(parent)
{
    d = new QSyntaxHighlighterPrivate(this);
    setDocument(parent->document());
}

/*!
    Destructor. Uninstalls this syntax highlighter from the text document.
*/
QSyntaxHighlighter::~QSyntaxHighlighter()
{
    setDocument(0);
    delete d;
}

/*!
    Registers the syntaxhighlighter at the given QTextDocument \a doc.
    A QSyntaxHighlighter can only be used with one document at a time.
*/
void QSyntaxHighlighter::setDocument(QTextDocument *doc)
{
    if (d->doc) {
        disconnect(d->doc, SIGNAL(contentsChange(int, int, int)),
                   this, SLOT(reformatBlocks(int, int, int)));

        QTextCursor cursor(d->doc);
        cursor.beginEditBlock();
        for (QTextBlock blk = d->doc->begin(); blk.isValid(); blk = blk.next())
            blk.layout()->clearAdditionalFormats();
        cursor.endEditBlock();
    }
    d->doc = doc;
    if (d->doc) {
        connect(d->doc, SIGNAL(contentsChange(int, int, int)),
                this, SLOT(reformatBlocks(int, int, int)));
    }
}

/*!
    Returns the QTextDocument on which this syntax highlighter is
    installed.
*/
QTextDocument *QSyntaxHighlighter::document() const
{
    return d->doc;
}

/*!
    \fn void QSyntaxHighlighter::highlightBlock(const QString &text)

    This function is called when necessary by the rich text engine,
    i.e. on text blocks which have changed.

    In your reimplementation you should parse the block's \a text
    and call setFormat() as often as necessary to apply any font and
    color changes that you require.

    Some syntaxes can have constructs that span text blocks. For
    example, a C++ syntax highlighter should be able to cope with
    \c{/}\c{*...*}\c{/} comments that span text blocks. To deal
    with these cases it is necessary to know the end state of the
    previous text block (e.g. "in comment").

    Inside your highlightBlock() implementation you can query the
    end state of the previous text block using previousBlockState().
    If no value was previously set then the returned value is -1.

    After parsing the block you can save the last state using
    setCurrentBlockState().

    For example, if you're writing a simple C++ syntax highlighter,
    you might designate 1 to signify "in comment". For a text block
    that ended in the middle of a comment you'd set 1 using
    setCurrentBlockState, and for other paragraphs you'd set 0.
    In your parsing code if the return value of previousBlockState()
    is 1, you would highlight the text as a C++ comment until you
    reached the closing \c{*}\c{/}.
*/

/*!
    This function is applied to the syntax highlighter's current
    text block (the text of which is passed to the highlightBlock()
    function).

    The specified \a format is applied to the text from
    position \a start for \a count characters. (If \a count is 0,
    nothing is done.). The formatting properties set in \a format
    are merged at display time with the formatting information
    stored directly in the document. For example as previously set
    with QTextCursor's functions. Note that the document itself
    remains unmodified by the format set through this function.
*/
void QSyntaxHighlighter::setFormat(int start, int count, const QTextCharFormat &format)
{
    if (start < 0 || start >= d->formatChanges.count())
        return;

    const int end = qMin(start + count, d->formatChanges.count());
    for (int i = start; i < end; ++i)
        d->formatChanges[i] = format;
}

/*!
    \overload
*/
void QSyntaxHighlighter::setFormat(int start, int count, const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(color);
    setFormat(start, count, format);
}

/*!
    \overload
*/
void QSyntaxHighlighter::setFormat(int start, int count, const QFont &font)
{
    QTextCharFormat format;
    format.setFont(font);
    setFormat(start, count, format);
}

/*!
    Returns the format at \a pos inside the syntax highlighter's
    current text block.
*/
QTextCharFormat QSyntaxHighlighter::format(int pos) const
{
    if (pos < 0 || pos >= d->formatChanges.count())
        return QTextCharFormat();
    return d->formatChanges.at(pos);
}

/*!
    Returns the end state of the text block previous to the
    syntax highlighter's current block. If no value was
    previously set the returned value is -1.

    \sa highlightBlock(), setCurrentBlockState()
*/
int QSyntaxHighlighter::previousBlockState() const
{
    if (!d->currentBlock.isValid())
        return -1;

    const QTextBlock previous = d->currentBlock.previous();
    if (!previous.isValid())
        return -1;

    return previous.userState();
}

/*!
    Returns the state of the current text block.
*/
int QSyntaxHighlighter::currentBlockState() const
{
    if (!d->currentBlock.isValid())
        return -1;

    return d->currentBlock.userState();
}

/*!
    Sets the state of the current text block to \a newState.
*/
void QSyntaxHighlighter::setCurrentBlockState(int newState)
{
    if (!d->currentBlock.isValid())
        return;

    d->currentBlock.setUserState(newState);
}

/*!
    Sets the QTextBlockUserData object on the current text block.

    \sa QTextBlock::setUserData()
*/
void QSyntaxHighlighter::setCurrentBlockUserData(QTextBlockUserData *data)
{
    if (!d->currentBlock.isValid())
        return;

    d->currentBlock.setUserData(data);
}

/*!
    Returns the QTextBlockUserData object previously set on the current text block.

    \sa QTextBlock::userData(), QTextBlock::setUserData()
*/
QTextBlockUserData *QSyntaxHighlighter::currentBlockUserData() const
{
    if (!d->currentBlock.isValid())
        return 0;

    return d->currentBlock.userData();
}

#include "moc_qsyntaxhighlighter.cpp"

