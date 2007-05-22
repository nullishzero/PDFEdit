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

#include "editor.h"
#include "parenmatcher.h"
#include <qfile.h>
//Added by qt3to4:
#include <Q3CString>
#include <QEvent>
#include <QKeyEvent>
#include "q3richtext_p.h"
#include "conf.h"
#include <qapplication.h>
#include <q3popupmenu.h>
#include <q3accel.h>
#include <qclipboard.h>

Editor::Editor( const QString &fn, QWidget *parent, const char *name )
    : Q3TextEdit( parent, name ), hasError( false )
{
    document()->setFormatter( new Q3TextFormatterBreakInWords );
    if ( !fn.isEmpty() )
	load( fn );
    setHScrollBarMode( Q3ScrollView::AlwaysOff );
    setVScrollBarMode( Q3ScrollView::AlwaysOn );
    document()->setUseFormatCollection( false );
    parenMatcher = new ParenMatcher;
    connect( this, SIGNAL( cursorPositionChanged( Q3TextCursor * ) ),
	     this, SLOT( cursorPosChanged( Q3TextCursor * ) ) );
    cfg = new Config;
    document()->addSelection( Error );
    document()->addSelection( Step );
    document()->setSelectionColor( Error, Qt::red );
    document()->setSelectionColor( Step, Qt::yellow );

//     document()->setInvertSelectionText( Error, false );
//     document()->setInvertSelectionText( Step, false );

    document()->addSelection( ParenMatcher::Match );
    document()->addSelection( ParenMatcher::Mismatch );
    document()->setSelectionColor( ParenMatcher::Match, QColor( 204, 232, 195 ) );
    document()->setSelectionColor( ParenMatcher::Mismatch, Qt::magenta );
//     document()->setInvertSelectionText( ParenMatcher::Match, false );
//     document()->setInvertSelectionText( ParenMatcher::Mismatch, false );

    accelComment = new Q3Accel( this );
    accelComment->connectItem( accelComment->insertItem( Qt::ALT + Qt::Key_C ),
			       this, SLOT( commentSelection() ) );
    accelUncomment = new Q3Accel( this );
    accelUncomment->connectItem( accelUncomment->insertItem( Qt::ALT + Qt::Key_U ),
				 this, SLOT( uncommentSelection() ) );
    editable = true;
}

Editor::~Editor()
{
    delete cfg;
    delete parenMatcher;
}

void Editor::cursorPosChanged( Q3TextCursor *c )
{
    if ( parenMatcher->match( c ) )
	repaintChanged();
    if ( hasError ) {
	emit clearErrorMarker();
	hasError = false;
    }
}

void Editor::load( const QString &fn )
{
    filename = fn;
    QFile f( filename );
    if ( !f.open( QIODevice::ReadOnly ) )
	return;
    Q3CString txt;
    txt.resize( f.size() );
    f.readBlock( txt.data(), f.size() );
    QString s( QString::fromLatin1( txt ) );
    setText( s );
}

void Editor::save( const QString &fn )
{
    if ( !filename.isEmpty() )
	filename = fn;
}

void Editor::configChanged()
{
    document()->invalidate();
    viewport()->repaint( false );
}

void Editor::setErrorSelection( int line )
{
    Q3TextParagraph *p = document()->paragAt( line );
    if ( !p )
	return;
    Q3TextCursor c( document() );
    c.setParagraph( p );
    c.setIndex( 0 );
    document()->removeSelection( Error );
    document()->setSelectionStart( Error, c );
    c.gotoLineEnd();
    document()->setSelectionEnd( Error, c );
    hasError = true;
    viewport()->repaint( false );
}

void Editor::setStepSelection( int line )
{
    Q3TextParagraph *p = document()->paragAt( line );
    if ( !p )
	return;
    Q3TextCursor c( document() );
    c.setParagraph( p );
    c.setIndex( 0 );
    document()->removeSelection( Step );
    document()->setSelectionStart( Step, c );
    c.gotoLineEnd();
    document()->setSelectionEnd( Step, c );
    viewport()->repaint( false );
}

void Editor::clearStepSelection()
{
    document()->removeSelection( Step );
    viewport()->repaint( false );
}

void Editor::doChangeInterval()
{
    emit intervalChanged();
    Q3TextEdit::doChangeInterval();
}

void Editor::commentSelection()
{
    int paragraphStart;
    int paragraphEnd;
    int indexStart;
    int indexEnd;
    getSelection(&paragraphStart, &indexStart, &paragraphEnd, &indexEnd);

    if (paragraphStart < 0) {
        getCursorPosition(&paragraphStart, &indexStart);
        paragraphEnd = paragraphStart;
    }

    if (paragraphStart >= 0 && paragraphEnd >= 0) {
        for (int i=paragraphStart; i<=paragraphEnd; ++i)
            insertAt(QLatin1String("//"), i, 0);

        repaintChanged();
        setModified(true);
    }


}

void Editor::uncommentSelection()
{
    int paragraphStart;
    int paragraphEnd;
    int indexStart;
    int indexEnd;
    getSelection(&paragraphStart, &indexStart, &paragraphEnd, &indexEnd);

    if (paragraphStart < 0) {
        getCursorPosition(&paragraphStart, &indexStart);
        paragraphEnd = paragraphStart;
    }

    if (paragraphStart >= 0 && paragraphEnd >= 0) {
        for (int i=paragraphStart; i<=paragraphEnd; ++i) {
            QString str = text(i);

            int whitespace = 0;
            while (str.startsWith("/") || (str.length() > 0 && str.at(0).isSpace())) {
                if (str.length() > 0 && str.at(0).isSpace()) ++whitespace;
                str.remove(0, 1);
            }

            if (whitespace > 0)
                str.prepend(QString(whitespace, QLatin1Char(' ')));

            insertParagraph(str, i);
            removeParagraph(i + 1);

        }

        repaintChanged();
        setModified(true);
    }
}

Q3PopupMenu *Editor::createPopupMenu( const QPoint &p )
{
    Q3PopupMenu *menu = Q3TextEdit::createPopupMenu( p );
    menu->insertSeparator();
    menu->insertItem( tr( "C&omment Code\tAlt+C" ), this, SLOT( commentSelection() ) );
    menu->insertItem( tr( "Unco&mment Code\tAlt+U" ), this, SLOT( uncommentSelection() ) );
    return menu;
}

bool Editor::eventFilter( QObject *o, QEvent *e )
{
    if ( ( e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut ) &&
	 ( o == this || o == viewport() ) ) {
	accelUncomment->setEnabled( e->type() == QEvent::FocusIn );
	accelComment->setEnabled( e->type() == QEvent::FocusIn );
    }
    return Q3TextEdit::eventFilter( o, e );
}

void Editor::doKeyboardAction( KeyboardAction action )
{
    if ( !editable )
	return;
    Q3TextEdit::doKeyboardAction( action );
}

void Editor::keyPressEvent( QKeyEvent *e )
{
    if ( editable ) {
	Q3TextEdit::keyPressEvent( e );
	return;
    }

    switch ( e->key() ) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Direction_L:
    case Qt::Key_Direction_R:
	Q3TextEdit::keyPressEvent( e );
	break;
    default:
	e->accept();
	break;
    }
}
