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

#include "qsacompletion.h"
#include "qsaeditor.h"
#include "qsaeditorinterface.h"
#include <private/qrichtext_p.h>

#include <qaction.h>
#include <qapplication.h>
#include <qiconset.h>
#include <qtimer.h>

#include <markerwidget.h>
#include <viewmanager.h>

// #include "qsabreakpointsettingsimpl.h"
// #include "qsdebugger.h"

static QPtrList<QSAEditorInterface> *editorInterfaces = 0;
bool QSAEditorInterface::debuggerEnabled = TRUE;

QSAEditorInterface::QSAEditorInterface()
    : viewManager( 0 )
{
    if ( !editorInterfaces )
	editorInterfaces = new QPtrList<QSAEditorInterface>;
    editorInterfaces->append( this );
    updateTimer = new QTimer( this );
    connect( updateTimer, SIGNAL( timeout() ), this, SLOT( update() ) );
//     actionToggleBreakPoint = 0;
//     actionEditBreakpoints = 0;
}

QSAEditorInterface::~QSAEditorInterface()
{
    editorInterfaces->removeRef( this );
    updateTimer->stop();
    delete viewManager;
//     delete actionToggleBreakPoint;
//     delete actionEditBreakpoints;
}

QWidget *QSAEditorInterface::editor( bool readonly, QWidget *parent )
{
    if ( !viewManager ) {
	viewManager = new ViewManager( parent, 0 );
	QSAEditor *e = new QSAEditor( QString::null, viewManager, "editor" );
	e->setEditable( !readonly );
	QObject::connect( viewManager, SIGNAL( collapseFunction( QTextParagraph * ) ),
			  e, SLOT( collapseFunction( QTextParagraph * ) ) );
	QObject::connect( viewManager, SIGNAL( expandFunction( QTextParagraph * ) ),
			  e, SLOT( expandFunction( QTextParagraph * ) ) );
	QObject::connect( viewManager, SIGNAL( collapse( bool ) ),
			  e, SLOT( collapse( bool ) ) );
	QObject::connect( viewManager, SIGNAL( expand( bool ) ),
			  e, SLOT( expand( bool ) ) );
// 	QObject::connect( viewManager, SIGNAL( editBreakPoints() ),
// 			  this, SLOT( editBreakpoints() ) );
// 	QObject::connect( viewManager, SIGNAL( isBreakpointPossible( bool&, const QString&, int ) ),
// 			  this, SLOT( isBreakpointPossible( bool &, const QString &, int ) ) );
	e->installEventFilter( this );
 	QApplication::sendPostedEvents(); // let the workspace do its reparenting work
	if ( viewManager->parentWidget() )
	    viewManager->parentWidget()->installEventFilter( this );

	connect( e, SIGNAL( intervalChanged() ), this, SLOT( intervalChanged() ) );
// 	QApplication::sendPostedEvents( viewManager, QEvent::ChildInserted );
    }
    return viewManager->currentView();
}

void QSAEditorInterface::setText( const QString &txt )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    QSAEditor *e = (QSAEditor*)viewManager->currentView();
    disconnect( e, SIGNAL( modificationChanged( bool ) ), this, SLOT( modificationChanged( bool ) ) );
    e->setText( txt );
    e->setModified( FALSE );
    e->sync();
    e->loadLineStates();
    connect( e, SIGNAL( modificationChanged( bool ) ), this, SLOT( modificationChanged( bool ) ) );
    viewManager->marker_widget()->update();
}

QString QSAEditorInterface::text() const
{
    if ( !viewManager || !viewManager->currentView() )
	return QString::null;
    return ( (QSAEditor*)viewManager->currentView() )->text();
}

bool QSAEditorInterface::isUndoAvailable() const
{
    if ( !viewManager || !viewManager->currentView() )
	return FALSE;
    return ( (QSAEditor*)viewManager->currentView() )->isUndoAvailable();
}

bool QSAEditorInterface::isRedoAvailable() const
{
    if ( !viewManager || !viewManager->currentView() )
	return FALSE;
    return ( (QSAEditor*)viewManager->currentView() )->isRedoAvailable();
}

void QSAEditorInterface::undo()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->undo();
}

void QSAEditorInterface::redo()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->redo();
}

void QSAEditorInterface::cut()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->cut();
}

void QSAEditorInterface::copy()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->copy();
}

void QSAEditorInterface::paste()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->paste();
}

void QSAEditorInterface::selectAll()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->selectAll();
}

bool QSAEditorInterface::find( const QString &expr, bool cs, bool wo, bool forward, bool startAtCursor )
{
    if ( !viewManager || !viewManager->currentView() )
	return FALSE;
    QSAEditor *e = (QSAEditor*)viewManager->currentView();
    if ( startAtCursor )
	return e->find( expr, cs, wo, forward );
    int dummy = 0;
    return e->find( expr, cs, wo, forward, &dummy, &dummy );

}

bool QSAEditorInterface::replace( const QString &find, const QString &replace, bool cs, bool wo,
				   bool forward, bool startAtCursor, bool replaceAll )
{
    if ( !viewManager || !viewManager->currentView() )
	return FALSE;
    QSAEditor *e = (QSAEditor*)viewManager->currentView();
    bool ok = FALSE;
    if ( startAtCursor ) {
	ok = e->find( find, cs, wo, forward );
    } else {
	int dummy = 0;
	ok =  e->find( find, cs, wo, forward, &dummy, &dummy );
    }

    if ( ok ) {
	e->removeSelectedText();
	e->insert( replace, FALSE, FALSE );
    }

    if ( !replaceAll || !ok ) {
	if ( ok )
	    e->setSelection( e->textCursor()->paragraph()->paragId(),
			     e->textCursor()->index() - replace.length(),
			     e->textCursor()->paragraph()->paragId(),
			     e->textCursor()->index() );
	return ok;
    }

    bool ok2 = TRUE;
    while ( ok2 ) {
	ok2 = e->find( find, cs, wo, forward );
	if ( ok2 ) {
	    e->removeSelectedText();
	    e->insert( replace, FALSE, FALSE );
	}
    }

    return TRUE;
}

void QSAEditorInterface::gotoLine( int line )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->setCursorPosition( line, 0 );
}

void QSAEditorInterface::indent()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->QTextEdit::indent();
}

void QSAEditorInterface::splitView()
{
    qFatal( "QSAEditorInterface::splitView()" );
#if 0
    if ( !viewManager || !viewManager->currentView() )
	return;
    QTextDocument *doc = ( (QSAEditor*)viewManager->currentView() )->document();
    QSAEditor *editor = new QSAEditor( QString::null, viewManager, "editor" );
    editor->setDocument( doc );
#endif
}

void QSAEditorInterface::scrollTo( const QString &txt, const QString &first )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    QString expr = first;
    ( (QSAEditor*)viewManager->currentView() )->sync();
    QTextDocument *doc = ( (QSAEditor*)viewManager->currentView() )->document();
    QTextParagraph *p = doc->firstParagraph();
    while ( p ) {
	if ( p->string()->toString().find( expr ) != -1 ) {
	    ( (QSAEditor*)viewManager->currentView() )->setCursorPosition( p->paragId() + 2, 0 );
	    if ( expr == txt )
		break;
	    expr = txt;
	}
	p = p->next();
    }
    ( (QSAEditor*)viewManager->currentView() )->setFocus();
}

void QSAEditorInterface::setContext( QObject *this_ )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->completionManager()->setContext( this_ );
}

void QSAEditorInterface::setError( int line )
{
    if ( !viewManager )
	return;
    viewManager->setError( line );
}

void QSAEditorInterface::clearError()
{
    QSAEditor *e = (QSAEditor*)viewManager->currentView();
    if ( !e )
	return;
    e->clearError();
}

void QSAEditorInterface::setStep( int line )
{
    if ( !viewManager )
	return;
    viewManager->setStep( line );
}

void QSAEditorInterface::clearStep()
{
    if ( !viewManager )
	return;
    viewManager->clearStep();
}

void QSAEditorInterface::clearStackFrame()
{
    if ( !viewManager )
	return;
    viewManager->clearStackFrame();
}

void QSAEditorInterface::setStackFrame( int line )
{
    if ( !viewManager )
	return;
    viewManager->setStackFrame( line );
}

void QSAEditorInterface::readSettings()
{
    if ( !viewManager )
	return;
    ( (QSAEditor*)viewManager->currentView() )->configChanged();
}

void QSAEditorInterface::modificationChanged( bool )
{
    qFatal( "QSAEditorInterface::modificationChanged()" );
//     if ( viewManager && dIface )
// 	dIface->setModified( m, viewManager->currentView() );
}

void QSAEditorInterface::setModified( bool m )
{
    if ( !viewManager )
	return;
    ( (QSAEditor*)viewManager->currentView() )->setModified( m );
}

bool QSAEditorInterface::isModified() const
{
    if ( !viewManager )
	return FALSE;
    return ( (QSAEditor*)viewManager->currentView() )->isModified();
}

// When the editor gets closed we get a focusOut event. On focusOut we
// re-parse the contents, etc., which requires the editor to be in a
// stable state. When we get the focusOut because of a close event, we
// are not in a stable state and we might crash. So we use that
// variable to ignore focusOut events after close events of the
// editor.
static bool ignoreNextFocusOut = FALSE;

bool QSAEditorInterface::eventFilter( QObject *o, QEvent *e )
{
    if ( viewManager && o == viewManager->currentView() ) {
	if ( e->type() == QEvent::KeyPress ) {
	    updateTimer->stop();
	} else if ( e->type() == QEvent::FocusOut && !ignoreNextFocusOut ) {
	    ignoreNextFocusOut = FALSE;
	    update();
	    for ( QSAEditorInterface *iface = editorInterfaces->first();
		  iface; iface = editorInterfaces->next() ) {
// 		if ( iface->actionToggleBreakPoint ) {
// 		    iface->actionToggleBreakPoint->setEnabled( FALSE );
// 		    break;
// 		}
	    }
	} else if ( e->type() == QEvent::FocusIn ) {
	    for ( QSAEditorInterface *iface = editorInterfaces->first();
		  iface; iface = editorInterfaces->next() ) {
// 		if ( iface->actionToggleBreakPoint ) {
// 		    iface->actionToggleBreakPoint->setEnabled( debuggerEnabled );
// 		    break;
// 		}
	    }
	}
    } else if ( viewManager ) {
	if ( e->type() == QEvent::Close )
	    ignoreNextFocusOut = TRUE;
    }

    return QObject::eventFilter( o, e );
}

int QSAEditorInterface::numLines() const
{
    if ( !viewManager || !viewManager->currentView() )
	return 0;
    return ( (QSAEditor*)viewManager->currentView() )->paragraphs();
}

// void QSAEditorInterface::breakPoints( QValueList<uint> &l ) const
// {
//     if ( !viewManager )
// 	return;
//     l = viewManager->breakPoints();
// }

// void QSAEditorInterface::setBreakPoints( const QValueList<uint> &l )
// {
//     if ( !viewManager )
// 	return;
//     viewManager->setBreakPoints( l );
// }

void QSAEditorInterface::intervalChanged()
{
    updateTimer->start( 2000, TRUE );
}

void QSAEditorInterface::update()
{
    if ( !viewManager || !viewManager->currentView() ||
	 !( (QTextEdit*)viewManager->currentView() )->isModified() )
	return;
}

// void QSAEditorInterface::onBreakPointChange( QObject *receiver, const char *slot )
// {
//     if ( !viewManager )
// 	return;
//     connect( viewManager, SIGNAL( markersChanged() ), receiver, slot );
// }

// void QSAEditorInterface::setMode( Mode m )
// {
//     if ( !viewManager || !viewManager->currentView() )
// 	return;
//     ( (QSAEditor*)viewManager->currentView() )->setDebugging( m == Debugging );
// }

// void QSAEditorInterface::toggleBreakpoint()
// {
//     QSAEditorInterface *iface;
//     QTextEdit *e = activeEditor( &iface );
//     if ( !e )
// 	return;
//     int para, index;
//     e->getCursorPosition( &para, &index );
//     QValueList<uint> bps;
//     iface->breakPoints( bps );
//     if ( bps.find( para ) != bps.end() ) {
// 	bps.remove( para );
//     } else {
// 	if ( Debugger::validBreakpoint( e->text(), para ) )
// 	     bps << para;
//     }
//     iface->setBreakPoints( bps );
//     if ( iface->view_manager() )
// 	iface->view_manager()->emitMarkersChanged();
//     iface->release();
// }

// void QSAEditorInterface::editBreakpoints()
// {
//     if ( !dIface ) {
// 	for ( QSAEditorInterface *iface = editorInterfaces->first(); iface; iface = editorInterfaces->next() ) {
// 	    if ( iface->hasDesignerInterface() ) {
// 		iface->editBreakpoints();
// 		return;
// 	    }
// 	}
// 	return;
//     }

//     QSAEditorInterface *iface;
//     QTextEdit *e = activeEditor( &iface );
//     if ( e ) {
// 	int para, index;
// 	e->getCursorPosition( &para, &index );
// 	QValueList<uint> bps;
// 	iface->breakPoints( bps );
// 	if ( bps.find( para ) != bps.end() ) {
// 	    // #### set some variable to initialize the dialog with that breakpoint
// 	}
// 	iface->release();
//     }
//     QMap<QString, QValueList<uint> > bps;
//     dIface->currentProject()->breakPoints( bps );
//     BreakPointSettings dlg( bps, dIface, 0, 0, TRUE );
//     dlg.exec();
// }

QTextEdit *QSAEditorInterface::activeEditor( QSAEditorInterface ** ) const
{
//     if ( !editorInterfaces )
// 	return 0;
//     for ( QSAEditorInterface *iface = editorInterfaces->first(); iface; iface = editorInterfaces->next() ) {
// 	if ( iface->view_manager() && iface->view_manager()->currentView() &&
// 	     ( iface->view_manager()->currentView()->hasFocus() ||
// 	       ( (QTextEdit*)iface->view_manager()->currentView() )->viewport()->hasFocus() ) ) {
// 	    *eiface = iface;
// 	    (*eiface)->addRef();
// 	    return (QTextEdit*)iface->view_manager()->currentView();
// 	}
//     }
    qFatal( "QSAEditorInterface::activeEditor()" );
    return 0;
}

// void QSAEditorInterface::isBreakpointPossible( bool &possible, const QString &code, int line )
// {
//     possible = Debugger::validBreakpoint( code, line );
// }

// void QSAEditorInterface::toggleDebugger( bool enable )
// {
//     debuggerEnabled = enable;
// //     actionEditBreakpoints->setEnabled( enable );
//     actionToggleBreakPoint->setEnabled( enable );
// }
