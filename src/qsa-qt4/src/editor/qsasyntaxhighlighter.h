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

#ifndef QSASYNTAXHIGHLIGHTER_H
#define QSASYNTAXHIGHLIGHTER_H

#ifndef QSA_NO_EDITOR

#include "conf.h"
#include "q3richtext_p.h"
#include <Q3IntDict>

class QSASyntaxHighlighter : public Q3TextPreProcessor
{
public:
    enum CppIds {
	Comment = 1,
	Number,
	String,
	Type,
	Keyword,
	PreProcessor,
	Label
    };

    QSASyntaxHighlighter();
    virtual ~QSASyntaxHighlighter() {}
    void process(Q3TextDocument *doc, Q3TextParagraph *string, int start, bool invalidate = true);
    void updateStyles(const QMap<QString, ConfigStyle> &styles);

    static const char *const keywords[];
    Q3TextFormat *format(int id);

private:
    void addFormat(int id, Q3TextFormat *f);
    void removeFormat(int id);

    void createFormats();

    Q3TextFormat *lastFormat;
    int lastFormatId;
    Q3IntDict<Q3TextFormat> formats;

};

#endif

#endif
