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

#include "browser.h"
#include "editor.h"
#include <private/qrichtext_p.h>


EditorBrowser::EditorBrowser( Editor *e )
    : curEditor( e ), oldHighlightedParag( 0 )
{
    curEditor = e;

    // ### disabled for now
//     curEditor->viewport()->installEventFilter( this );
//     curEditor->installEventFilter( this );

    QFont fn( curEditor->font() );
    fn.setUnderline( TRUE );
    highlightedFormat = new QTextFormat( fn, blue );
}

EditorBrowser::~EditorBrowser()
{
    delete highlightedFormat;
}

bool EditorBrowser::eventFilter( QObject *o, QEvent *e )
{
    if (
#if (QT_VERSION) < 0x030200
	o->parent()->inherits("Editor") || o->inherits("Editor")
#else
	::qt_cast<Editor*>(o->parent()) || ::qt_cast<Editor*>(o)
#endif
	) {
	QMouseEvent *me;
	QKeyEvent *ke;
	switch ( e->type() ) {
	case QEvent::MouseMove:
	    me = (QMouseEvent*)e;
	    if ( ( me->state() & ControlButton ) == ControlButton ) {
		curEditor->viewport()->setCursor( pointingHandCursor );
		QTextCursor c( curEditor->document() );
		curEditor->placeCursor( curEditor->viewportToContents( me->pos() ), &c );
		QTextCursor from, to;
		if ( oldHighlightedParag ) {
		    oldHighlightedParag->setEndState( -1 );
		    oldHighlightedParag->format();
		    oldHighlightedParag = 0;
		}
		if ( findCursor( c, from, to ) && from.paragraph() == to.paragraph() ) {
		    // avoid collision with other selections
		    for ( int i = 0; i < curEditor->document()->numSelections(); ++i )
			curEditor->document()->removeSelection( i );
		    from.paragraph()->setFormat( from.index(), to.index() - from.index() + 1, highlightedFormat, FALSE );
		    lastWord = from.paragraph()->string()->toString().mid( from.index(), to.index() - from.index() + 1 );
		    oldHighlightedParag = from.paragraph();
		} else {
		    lastWord = "";
		}
		curEditor->repaintChanged();
		return TRUE;
	    }
	    break;
	case QEvent::MouseButtonPress: {
	    bool killEvent = !lastWord.isEmpty();
	    if ( !lastWord.isEmpty() )
		showHelp( lastWord );
	    lastWord = "";
	    curEditor->viewport()->setCursor( ibeamCursor );
	    if ( oldHighlightedParag ) {
		oldHighlightedParag->setEndState( -1 );
		oldHighlightedParag->format();
		curEditor->repaintChanged();
		oldHighlightedParag = 0;
	    }
	    if ( killEvent )
		return TRUE;
	} break;
	case QEvent::KeyRelease:
	    lastWord = "";
	    ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Control ) {
		curEditor->viewport()->setCursor( ibeamCursor );
		if ( oldHighlightedParag ) {
		    oldHighlightedParag->setEndState( -1 );
		    oldHighlightedParag->format();
		    curEditor->repaintChanged();
		    oldHighlightedParag = 0;
		}
	    }
	default:
	    break;
	}
    }
    return FALSE;
}

void EditorBrowser::setCurrentEdior( Editor *e )
{
    curEditor = e;
    curEditor->installEventFilter( this );
}

void EditorBrowser::addEditor( Editor *e )
{
    e->installEventFilter( this );
}

bool EditorBrowser::findCursor( const QTextCursor &c, QTextCursor &from, QTextCursor &to )
{
    from = c;
    while ( from.paragraph()->at( from.index() )->c != ' ' && from.paragraph()->at( from.index() )->c != '\t'  && from.index() > 0 )
	from.gotoLeft();
    if ( from.paragraph()->at( from.index() )->c == ' ' || from.paragraph()->at( from.index() )->c == '\t' )
	from.gotoRight();
    to = c;
    while ( to.paragraph()->at( to.index() )->c != ' ' && to.paragraph()->at( to.index() )->c != '\t' &&
	    to.index() < to.paragraph()->length() - 1 )
	to.gotoRight();
    if ( to.paragraph()->at( to.index() )->c == ' ' || to.paragraph()->at( to.index() )->c == '\t' )
	to.gotoLeft();
    return TRUE;
}
