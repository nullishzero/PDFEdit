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
#ifndef BLOCKDATA_H
#define BLOCKDATA_H

#include <QTextBlock>

const int ParenthesisMatcherPropertyId = QTextFormat::UserProperty;
const int ErrorMarkerPropertyId = QTextFormat::UserProperty + 1;

struct Parenthesis
{
    enum Type { Open, Closed };
    inline Parenthesis() : type(Open), pos(-1) {}
    inline Parenthesis(Type t, QChar c, int position)
        : type(t), chr(c), pos(position) {}
    Type type;
    QChar chr;
    int pos;
};

struct BlockData : public QTextBlockUserData
{
    static BlockData *data(const QTextBlock &block)
    { return static_cast<BlockData *>(QSyntaxHighlighter::userData(block)); }
    void setToBlock(const QTextBlock &block)
    { QSyntaxHighlighter::setUserData(block, this); }

    inline BlockData() : parenthesisMatchStart(-1), parenthesisMatchEnd(-1) {}

    QList<Parenthesis> parentheses;

    QTextCharFormat parenthesisMatchingFormat;
    int parenthesisMatchStart;
    int parenthesisMatchEnd;

    QTextCharFormat errorMarkerFormat;
};

#endif // BLOCKDATA_H
