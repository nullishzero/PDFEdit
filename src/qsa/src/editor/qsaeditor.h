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

#ifndef QUICKEDITOR_H
#define QUICKEDITOR_H

#include <editor.h>

class QSInterpreter;

class CIndent;
class EditorCompletion;
class EditorBrowser;

class QSAEditorToolTip;

class QSAEditor : public Editor
{
    Q_OBJECT

public:
    QSAEditor( const QString &fn, QWidget *parent, const char *name );
    ~QSAEditor();

    EditorCompletion *completionManager() { return completion; }
    EditorBrowser *browserManager() { return browser; }
    void configChanged();
    void setDebugging( bool b ) { debugging = b; }
    bool isDebugging() const { return debugging; }
    void makeFunctionVisible( QTextParagraph *p );
    void saveLineStates();
    void loadLineStates();
    void paste();

    void setInterpreter( QSInterpreter *ip );
    QSInterpreter *interpreter() const { return interp; }

    void clearError();
    bool supportsBreakPoints() const { return FALSE; } // ### For now

public slots:
    void expandFunction( QTextParagraph *p ) { expandFunction( p, TRUE ); }
    void collapseFunction( QTextParagraph *p ) { collapseFunction( p, TRUE ); }
    void collapse( bool all /*else only functions*/ );
    void expand( bool all /*else only functions*/ );

private:
    QTextParagraph *expandFunction( QTextParagraph *p, bool recalc );
    QTextParagraph *collapseFunction( QTextParagraph *p, bool recalc );
    void doRecalc();

private:
    QSInterpreter *interp;
    EditorCompletion *completion;
    EditorBrowser *browser;
    QSAEditorToolTip *tooltip;
    bool debugging;
    CIndent *indent;
};

#endif
