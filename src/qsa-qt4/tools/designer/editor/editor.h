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

#ifndef EDITOR_H
#define EDITOR_H

#include "q3textedit.h"
#include <QKeyEvent>
#include <QEvent>
#include <Q3PopupMenu>

struct Config;
class ParenMatcher;
class EditorCompletion;
class EditorBrowser;
class Q3Accel;

class Editor : public Q3TextEdit
{
    Q_OBJECT

public:
    enum Selection {
	Error = 3,
	Step = 4
    };

    Editor(const QString &fn, QWidget *parent, const char *name);
    ~Editor();
    virtual void load(const QString &fn);
    virtual void save(const QString &fn);
    Q3TextDocument *document() const { return Q3TextEdit::document(); }
    void placeCursor(const QPoint &p, Q3TextCursor *c) { Q3TextEdit::placeCursor(p, c); }
    void setDocument(Q3TextDocument *doc) { Q3TextEdit::setDocument(doc); }
    Q3TextCursor *textCursor() const { return Q3TextEdit::textCursor(); }
    void repaintChanged() { Q3TextEdit::repaintChanged(); }

    virtual EditorCompletion *completionManager() { return 0; }
    virtual EditorBrowser *browserManager() { return 0; }
    virtual void configChanged();

    Config *config() { return cfg; }

    void setErrorSelection(int line);
    void setStepSelection(int line);
    void clearStepSelection();
    void clearSelections();

    virtual bool supportsErrors() const { return true; }
    virtual bool supportsBreakPoints() const { return true; }
    virtual void makeFunctionVisible(Q3TextParagraph *) {}

    void drawCursor(bool b) { Q3TextEdit::drawCursor(b); }

    Q3PopupMenu *createPopupMenu(const QPoint &p);
    bool eventFilter(QObject *o, QEvent *e);

    void setEditable(bool b) { editable = b; }

protected:
    void doKeyboardAction(KeyboardAction action);
    void keyPressEvent(QKeyEvent *e);

signals:
    void clearErrorMarker();
    void intervalChanged();

private slots:
    void cursorPosChanged(Q3TextCursor *c);
    void doChangeInterval();
    void commentSelection();
    void uncommentSelection();

protected:
    ParenMatcher *parenMatcher;
    QString filename;
    Config *cfg;
    bool hasError;
    Q3Accel *accelComment, *accelUncomment;
    bool editable;

};

#endif
