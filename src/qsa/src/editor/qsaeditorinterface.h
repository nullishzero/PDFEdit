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

#ifndef QUICKEDITORINTERFACE_H
#define QUICKEDITORINTERFACE_H

#include <qobject.h>
#include <qguardedptr.h>

class QTimer;
class ViewManager;
class QAction;
class QTextEdit;

class QSAEditorInterface : public QObject
{
    Q_OBJECT

public:
    QSAEditorInterface();
    virtual ~QSAEditorInterface();

    QWidget *editor( bool readonly, QWidget *parent );

    void setText( const QString &txt );
    QString text() const;
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    bool find( const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor );
    bool replace( const QString &find, const QString &replace, bool cs, bool wo,
		  bool forward, bool startAtCursor, bool replaceAll );
    void gotoLine( int line );
    void indent();
    void scrollTo( const QString &txt, const QString &first );
    void splitView();
    void setContext( QObject *this_ );
    void readSettings();

    void setError( int line );
    void clearError();
    void setStep( int line );
    void clearStep();
    void clearStackFrame();
    void setStackFrame( int line );
    void setModified( bool b );
    bool isModified() const;

    int numLines() const;

//     void breakPoints( QValueList<uint> & ) const;
//     void setBreakPoints( const QValueList<uint> & );

//     void setMode( Mode m );

//     void onBreakPointChange( QObject *receiver, const char *slot );

    ViewManager *view_manager() const { return viewManager; }

//     void toggleDebugger( bool enable );

protected:
    bool eventFilter( QObject*, QEvent* );

private slots:
    void modificationChanged( bool m );
    void update();
    void intervalChanged();
//     void toggleBreakpoint();
//     void editBreakpoints();
//     void isBreakpointPossible( bool &possible, const QString &code, int line );

private:
    QTextEdit *activeEditor( QSAEditorInterface **iface ) const;

private:
    QGuardedPtr<ViewManager> viewManager;
    QTimer *updateTimer;
//     QAction *actionToggleBreakPoint;
//     QAction *actionEditBreakpoints;
    static bool debuggerEnabled;

};

#endif
