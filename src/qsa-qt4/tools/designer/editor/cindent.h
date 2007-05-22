/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CINDENT_H
#define CINDENT_H

#include "q3richtext_p.h"

class CIndent : public Q3TextIndent
{
public:
    CIndent();
    virtual ~CIndent() {}
    void indent(Q3TextDocument *doc, Q3TextParagraph *parag, int *oldIndent, int *newIndent);

    void setTabSize(int ts);
    void setIndentSize(int is);
    void setAutoIndent(bool ai) { autoIndent = ai; reindent(); }
    void setKeepTabs(bool kt) { keepTabs = kt; }

private:
    void reindent();
    void indentLine(Q3TextParagraph *p, int &oldIndent, int &newIndent);
    void tabify(QString &s);

public:
    int tabSize, indentSize;
    bool autoIndent, keepTabs;
    Q3TextDocument *lastDoc;

};

#endif
