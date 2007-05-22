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

#include "qsacompletion.h"
#include "qsaeditor.h"
#include "qsaeditorinterface.h"
//Added by qt3to4:
#include <QEvent>
#include <Q3PtrList>
#include "q3richtext_p.h"

#include <qaction.h>
#include <qapplication.h>
#include <qicon.h>
#include <qtimer.h>

#include <markerwidget.h>
#include <viewmanager.h>

// #include "qsabreakpointsettingsimpl.h"
// #include "qsdebugger.h"

static Q3PtrList<QSAEditorInterface> *editorInterfaces = 0;
bool QSAEditorInterface::debuggerEnabled = true;

QSAEditorInterface::QSAEditorInterface()
    : viewManager( 0 )
{
    if ( !editorInterfaces )
	editorInterfaces = new Q3PtrList<QSAEditorInterface>;
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
	QObject::connect( viewManager, SIGNAL( collapseFunction( Q3TextParagraph * ) ),
			  e, SLOT( collapseFunction( Q3TextParagraph * ) ) );
	QObject::connect( viewManager, SIGNAL( expandFunction( Q3TextParagraph * ) ),
			  e, SLOT( expandFunction( Q3TextParagraph * ) ) );
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

//         QApplication::processEvents();
//
        QApplication::sendPostedEvents( viewManager, QEvent::ChildInserted );

//         QChildEvent event(QEvent::ChildAdded, e);
//         QApplication::sendEvent(viewManager, &event);
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
    e->setModified( false );
    e->sync();
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
	return false;
    return ( (QSAEditor*)viewManager->currentView() )->isUndoAvailable();
}

bool QSAEditorInterface::isRedoAvailable() const
{
    if ( !viewManager || !viewManager->currentView() )
	return false;
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
	return false;
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
	return false;
    QSAEditor *e = (QSAEditor*)viewManager->currentView();
    bool ok = false;
    if ( startAtCursor ) {
	ok = e->find( find, cs, wo, forward );
    } else {
	int dummy = 0;
	ok =  e->find( find, cs, wo, forward, &dummy, &dummy );
    }

    if ( ok ) {
	e->removeSelectedText();
	e->insert( replace, false, false );
    }

    if ( !replaceAll || !ok ) {
	if ( ok )
	    e->setSelection( e->textCursor()->paragraph()->paragId(),
			     e->textCursor()->index() - replace.length(),
			     e->textCursor()->paragraph()->paragId(),
			     e->textCursor()->index() );
	return ok;
    }

    bool ok2 = true;
    while ( ok2 ) {
	ok2 = e->find( find, cs, wo, forward );
	if ( ok2 ) {
	    e->removeSelectedText();
	    e->insert( replace, false, false );
	}
    }

    return true;
}

void QSAEditorInterface::gotoLine( int line )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    QSAEditor *editor = qobject_cast<QSAEditor *>(viewManager->currentView());
    editor->setCursorPosition(qMin(line, editor->lines() - 1), 0);
}

void QSAEditorInterface::indent()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (QSAEditor*)viewManager->currentView() )->Q3TextEdit::indent();
}

void QSAEditorInterface::scrollTo( const QString &txt, const QString &first )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    QString expr = first;
    ( (QSAEditor*)viewManager->currentView() )->sync();
    Q3TextDocument *doc = ( (QSAEditor*)viewManager->currentView() )->document();
    Q3TextParagraph *p = doc->firstParagraph();
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
	return false;
    return ( (QSAEditor*)viewManager->currentView() )->isModified();
}

// When the editor gets closed we get a focusOut event. On focusOut we
// re-parse the contents, etc., which requires the editor to be in a
// stable state. When we get the focusOut because of a close event, we
// are not in a stable state and we might crash. So we use that
// variable to ignore focusOut events after close events of the
// editor.
static bool ignoreNextFocusOut = false;

bool QSAEditorInterface::eventFilter( QObject *o, QEvent *e )
{
    if ( viewManager && o == viewManager->currentView() ) {
	if ( e->type() == QEvent::KeyPress ) {
	    updateTimer->stop();
	} else if ( e->type() == QEvent::FocusOut && !ignoreNextFocusOut ) {
	    ignoreNextFocusOut = false;
	    update();
	    for ( QSAEditorInterface *iface = editorInterfaces->first();
		  iface; iface = editorInterfaces->next() ) {
// 		if ( iface->actionToggleBreakPoint ) {
// 		    iface->actionToggleBreakPoint->setEnabled( false );
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
	    ignoreNextFocusOut = true;
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
    updateTimer->start( 2000, true );
}

void QSAEditorInterface::update()
{
    if ( !viewManager || !viewManager->currentView() ||
	 !( (Q3TextEdit*)viewManager->currentView() )->isModified() )
	return;
}
