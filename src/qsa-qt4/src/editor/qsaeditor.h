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

#ifndef QUICKEDITOR_H
#define QUICKEDITOR_H

#ifndef QSA_NO_EDITOR

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
    QSAEditor(const QString &fn, QWidget *parent, const char *name);
    ~QSAEditor();

    EditorCompletion *completionManager() { return completion; }
    EditorBrowser *browserManager() { return browser; }
    void configChanged();
    void setDebugging(bool b) { debugging = b; }
    bool isDebugging() const { return debugging; }
    void makeFunctionVisible(Q3TextParagraph *p);
    void paste();

    void setInterpreter(QSInterpreter *ip);
    QSInterpreter *interpreter() const { return interp; }

    void clearError();
    bool supportsBreakPoints() const { return false; } // ### For now

public slots:
    void expandFunction(Q3TextParagraph *p) { expandFunction(p, true); }
    void collapseFunction(Q3TextParagraph *p) { collapseFunction(p, true); }
    void collapse(bool all /*else only functions*/);
    void expand(bool all /*else only functions*/);

private:
    Q3TextParagraph *expandFunction(Q3TextParagraph *p, bool recalc);
    Q3TextParagraph *collapseFunction(Q3TextParagraph *p, bool recalc);
    void doRecalc();

private:
    QSInterpreter *interp;
    EditorCompletion *completion;
    EditorBrowser *browser;
    bool debugging;
    CIndent *indent;
};

#endif

#endif
