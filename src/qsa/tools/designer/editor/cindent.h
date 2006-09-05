/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef CINDENT_H
#define CINDENT_H

#include <private/qrichtext_p.h>

class CIndent : public QTextIndent
{
public:
    CIndent();
    virtual ~CIndent() {}
    void indent( QTextDocument *doc, QTextParagraph *parag, int *oldIndent, int *newIndent );

    void setTabSize( int ts );
    void setIndentSize( int is );
    void setAutoIndent( bool ai ) { autoIndent = ai; reindent(); }
    void setKeepTabs( bool kt ) { keepTabs = kt; }

private:
    void reindent();
    void indentLine( QTextParagraph *p, int &oldIndent, int &newIndent );
    void tabify( QString &s );

public:
    int tabSize, indentSize;
    bool autoIndent, keepTabs;
    QTextDocument *lastDoc;

};

#endif
