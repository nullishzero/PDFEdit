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

#include "viewmanager.h"
#include "editor.h"
#include "markerwidget.h"
#include <qlayout.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3Frame>
#include <Q3ValueList>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QEvent>
#include <QChildEvent>
#include "q3richtext_p.h"
#include "paragdata.h"
#include <qobject.h>
#include <qlabel.h>
#include <qtimer.h>

ViewManager::ViewManager( QWidget *parent, const char *name )
    : QWidget( parent, name ), curView( 0 )
{
    QHBoxLayout *l = new QHBoxLayout( this );
    markerWidget = new MarkerWidget( this, "editor_markerwidget" );
    connect( markerWidget, SIGNAL( markersChanged() ),
	     this, SIGNAL( markersChanged() ) );
    connect( markerWidget, SIGNAL( collapseFunction( Q3TextParagraph * ) ),
	     this, SIGNAL( collapseFunction( Q3TextParagraph * ) ) );
    connect( markerWidget, SIGNAL( expandFunction( Q3TextParagraph * ) ),
	     this, SIGNAL( expandFunction( Q3TextParagraph * ) ) );
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
    posLabel->setFrameStyle( Q3Frame::Sunken | Q3Frame::Panel );
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
    if ( e->type() == QEvent::ChildInserted && qobject_cast<Editor*>(e->child()))
        addView((QWidget*)e->child());
    QWidget::childEvent( e );
}

void ViewManager::setError( int line )
{
    Q3TextParagraph *p = ( (Editor*)curView )->document()->paragAt( line );
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
    Q3TextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->step = false;
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
 	paragData->step = true;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
    }
}

void ViewManager::clearStep()
{
    ( (Editor*)curView )->clearStepSelection();
    Q3TextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->step = false;
	p = p->next();
    }
    markerWidget->doRepaint();
}

void ViewManager::setStackFrame( int line )
{
    Q3TextParagraph *p = ( (Editor*)curView )->document()->paragAt( line );
    if ( p ) {
	( (Editor*)curView )->sync();
	( (Editor*)curView )->setCursorPosition( line, 0 );
	( (Editor*)curView )->ensureCursorVisible();
	( (Editor*)curView )->viewport()->setFocus();
	( (Editor*)curView )->makeFunctionVisible( p );
	ParagData *paragData = (ParagData*)p->extraData();
	if ( !paragData )
	    paragData = new ParagData;
 	paragData->stackFrame = true;
	p->setExtraData( paragData );
	markerWidget->doRepaint();
    }
}

void ViewManager::clearStackFrame()
{
    Q3TextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    while ( p ) {
	if ( p->extraData() ) {
	    ( (ParagData*)p->extraData() )->stackFrame = false;
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
    Q3TextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
    while ( p ) {
	if ( p->extraData() )
	    ( (ParagData*)p->extraData() )->marker = ParagData::NoMarker;
	p = p->next();
    }
    markerWidget->doRepaint();
}

void ViewManager::setBreakPoints( const Q3ValueList<uint> &l )
{
    Q3TextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
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

Q3ValueList<uint> ViewManager::breakPoints() const
{
    Q3ValueList<uint> l;
    int i = 0;
    Q3TextParagraph *p = ( (Editor*)curView )->document()->firstParagraph();
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
    ( (Q3TextEdit*)currentView() )->getCursorPosition( &row, &col );
    posLabel->setText( msg );
    messageTimer->start( 1000, true );
}

void ViewManager::clearStatusBar()
{
    int row;
    int col;
    ( (Q3TextEdit*)currentView() )->getCursorPosition( &row, &col );
    posLabel->setText( QString( " Line: %1 Col: %2" ).arg( row + 1 ).arg( col + 1 ) );
}
