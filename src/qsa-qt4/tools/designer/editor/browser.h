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

#ifndef BROWSER_H
#define BROWSER_H

#include <QObject>
#include <QEvent>

class Editor;
class Q3TextCursor;
class Q3TextParagraph;
class Q3TextFormat;

class EditorBrowser : public QObject
{
    Q_OBJECT

public:
    EditorBrowser(Editor *e);
    ~EditorBrowser();

    bool eventFilter(QObject *o, QEvent *e);
    virtual void setCurrentEdior(Editor *e);
    virtual void addEditor(Editor *e);
    virtual bool findCursor(const Q3TextCursor &c, Q3TextCursor &from, Q3TextCursor &to);
    virtual void showHelp(const QString &) {}

protected:
    Editor *curEditor;
    Q3TextParagraph *oldHighlightedParag;
    QString lastWord;
    Q3TextFormat *highlightedFormat;

};

#endif
