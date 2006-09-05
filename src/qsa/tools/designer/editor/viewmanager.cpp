 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#include "viewmanager.h"
#include "editor.h"
#include "markerwidget.h"
#include <qlayout.h>
#include <private/qrichtext_p.h>
#include "paragdata.h"
#include <qobjectlist.h>
#include <qlabel.h>
#include <qtimer.h>

ViewManager::ViewManager( QWidget *parent, const char *name )
    : QWidget( parent, name ), curView( 0 )
{
    QHBoxLayout *l = new QHBoxLayout( this );
    markerWidget = new MarkerWidget( this, "editor_markerwidget" );
    connect( markerWidget, SIGNAL( markersChanged() ),
	     this, SIGNAL( markersChanged() ) );
    connect( markerWidget, SIGNAL( collapseFunction( QTextParagraph * ) ),
	     this, SIGNAL( collapseFunction( QTextParagraph * ) ) );
    connect( markerWidget, SIGNAL( expandFunction( QTextParagraph * ) ),
	     this, SIGNAL( expandFunction( QTextParagraph * ) ) );
    connect( markerWidget, SIGNAL( collapse( bool ) ),
	     this, SIGNAL( collapse( bool ) ) );
    connect( markerWidget, SIGNAL( expand( bool ) ),
	     this, SIGNAL( expand( bool ) ) );
    connect( markerWidget, SIGNAL( editBreakPoints() ),
	     this, SIGNAL( editBreakPoints() ) );
    connect( markerWidget, SIGNAL( isBreakpointPossible( bool&, const QString &, int ) ),
	     this, SIGNAL( isBreakpointPossible( bool&, const QString &, int ) ) );
    connect( markerWidget, SIGNAL( showMessage( const QString & ) ),
	     this, SLOT( showMessage( const QString & ) ) );
    messageTimer = new QTimer( this );
    connect( messageTimer, SIGNAL( timeout() ), this, SLOT( clearStatusBar() ) );
    markerWidget->setFixedWidth( fontMetrics().width( QString::fromLatin1("0000") ) + 20 );
    l->addWidget( markerWidget );
    layout = new QVBoxLayout( l );
}

void ViewManager::addView( QWidget *view )
{
    layout->addWidget( view );
    curView = view;
    connect( ( (Editor*)curView )->verticalScrollBar(), SIGNAL( valueChanged( int ) ),
	     markerWidget, SLOT( doRepaint() ) );
    connect( (Editor*)curView, SIGNAL( textChanged() ),
	     markerWidget, SLOT( doRepaint() ) );
    connect( (Editor*)curView, SIGNAL( clearErrorMarker() ),
	     this, SLOT( clearErrorMarker() ) );
    posLabel = new QLabel( this, "editor_poslabel" );
    posLabel->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    posLabel->setText( QString::fromLatin1(" Line: 1 Col: 1") );
    posLabel->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    posLabel->setLineWidth( 1 );
    posLabel->setFixedHeight( posLabel->fontMetrics().height() );
    layout->addWidget( posLabel );
    connect( curView, SIGNAL( cursorPositionChanged( int, int ) ),
	     this, SLOT( cursorPositionChanged( int, int ) ) );
}

QWidget *ViewManager::currentView() const
{
    return curView;
}

void ViewManager::childEvent( QChildEvent *e )
{
    if ( e->type() == QEvent::ChildInserted &&
#if (QT_VERSION) < 0x030200
	 e->child()->inherits("Editor")
#else
	 ::qt_cast<Editor*>(e->child())
#endif
	 )
	addView( (QWidget*)e->child() );
    QWidget::childEvent( e );
}

void ViewManager::setError( int line )
{
    QTextParagraph *p = ( (Editor*)curView )->document()->paragAt( line );
    if ( p ) {
	( (Editor*)curView )->setErrorSelection( line );
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->viewport()->setFocus();
	( (Editor*)curView )->makeFunctionVisible( p );
	ParagData *paragData = (ParagData*)p->extraData();
	if ( !paragData )
	    paragData = new ParagData;
	paragData->marker = ParagData::Error;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
    }
}

void ViewManager::setStep( int line )
{
    QTextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->step = FALSE;
	p = p->next();
    }
    p = ( (Editor*)curView )->document()->paragAt( line );
    if ( p ) {
	( (Editor*)curView )->setStepSelection( line );
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->viewport()->setFocus();
	( (Editor*)curView )->makeFunctionVisible( p );
	ParagData *paragData = (ParagData*)p->extraData();
	if ( !paragData )
	    paragData = new ParagData;
 	paragData->step = TRUE;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
    }
}

void ViewManager::clearStep()
{
    ( (Editor*)curView )->clearStepSelection();
    QTextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->step = FALSE;
	p = p->next();
    }
    markerWidget->doRepaint();
}

void ViewManager::setStackFrame( int line )
{
    QTextParagraph *p = ( (Editor*)curView )->document()->paragAt( line );
    if ( p ) {
	( (Editor*)curView )->sync();
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->ensureCursorVisible();
	( (Editor*)curView )->viewport()->setFocus();
	( (Editor*)curView )->makeFunctionVisible( p );
	ParagData *paragData = (ParagData*)p->extraData();
	if ( !paragData )
	    paragData = new ParagData;
 	paragData->stackFrame = TRUE;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
    }
}

void ViewManager::clearStackFrame()
{
    QTextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    while ( p ) {
	if ( p->extraData() ) {
	    ( (ParagData*)p->extraData() )->stackFrame = FALSE;
	    if ( ( (ParagData*)p->extraData() )->marker == ParagData::Error )
		( (ParagData*)p->extraData() )->marker = ParagData::NoMarker;
	}
	p = p->next();
    }
    markerWidget->doRepaint();
}

void ViewManager::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    markerWidget->doRepaint();
}

void ViewManager::clearErrorMarker()
{
    QTextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->marker = ParagData::NoMarker;
	p = p->next();
    }
    markerWidget->doRepaint();
}

void ViewManager::setBreakPoints( const QValueList<uint> &l )
{
    QTextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    int i = 0;
    while ( p ) {
	if ( l.find( i ) != l.end() ) {
	    if ( !p->extraData() ) {
		ParagData *data = new ParagData;
		p->setExtraData( data );
	    }
	    ParagData *data = (ParagData*)p->extraData();
	    data->marker = ParagData::Breakpoint;
	} else if ( p->extraData() ) {
	    ParagData *data = (ParagData*)p->extraData();
	    data->marker = ParagData::NoMarker;
	}
	p = p->next();
	++i;
    }
    markerWidget->doRepaint();
}

QValueList<uint> ViewManager::breakPoints() const
{
    QValueList<uint> l;
    int i = 0;
    QTextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    while ( p ) {
	if ( p->extraData() &&
	     ( (ParagData*)p->extraData() )->marker == ParagData::Breakpoint )
	    l << i;
	p = p->next();
	++i;
    }
    return l;
}

void ViewManager::showMarkerWidget( bool b )
{
    if ( b )
	markerWidget->show();
    else
	markerWidget->hide();
}

void ViewManager::emitMarkersChanged()
{
    emit markersChanged();
}

void ViewManager::cursorPositionChanged( int row, int col )
{
    posLabel->setText( QString( " Line: %1 Col: %2" ).arg( row + 1 ).arg( col + 1 ) );
}

void ViewManager::showMessage( const QString &msg )
{
    int row;
    int col;
    ( (QTextEdit*)currentView() )->getCursorPosition( &row, &col );
    posLabel->setText( msg );
    messageTimer->start( 1000, TRUE );
}

void ViewManager::clearStatusBar()
{
    int row;
    int col;
    ( (QTextEdit*)currentView() )->getCursorPosition( &row, &col );
    posLabel->setText( QString( " Line: %1 Col: %2" ).arg( row + 1 ).arg( col + 1 ) );
}
