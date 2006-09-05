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

#ifndef QSASYNTAXHIGHLIGHTER_H
#define QSASYNTAXHIGHLIGHTER_H

#include <conf.h>
#include <private/qrichtext_p.h>

class QSASyntaxHighlighter : public QTextPreProcessor
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
    void process( QTextDocument *doc, QTextParagraph *string, int start, bool invalidate = TRUE );
    void updateStyles( const QMap<QString, ConfigStyle> &styles );

    static const char *const keywords[];
    QTextFormat *format( int id );

private:
    void addFormat( int id, QTextFormat *f );
    void removeFormat( int id );

    void createFormats();

    QTextFormat *lastFormat;
    int lastFormatId;
    QIntDict<QTextFormat> formats;

};

#endif
