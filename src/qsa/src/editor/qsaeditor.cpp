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

#include "qsabrowser.h"
#include "qsacompletion.h"
#include "qsaeditor.h"
#include "qsasyntaxhighlighter.h"
#include "qsproject.h"
// #include "quickdebugger.h"
#include "quickinterpreter.h"
#include <cindent.h>
#include <paragdata.h>
#include <parenmatcher.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <stdlib.h>

class QSAEditorToolTip : public QToolTip
{
public:
    QSAEditorToolTip( QWidget *parent, QSAEditor *e );

    void maybeTip( const QPoint &pos );

private:
    QSAEditor *editor;

};

QSAEditorToolTip::QSAEditorToolTip( QWidget *parent, QSAEditor *e )
    : QToolTip( parent ), editor( e )
{
}

void QSAEditorToolTip::maybeTip( const QPoint &pos )
{
    if ( !editor )
	return;

    QPoint ps = editor->viewportToContents( pos );
    QTextParagraph *p = editor->document()->firstParagraph();
    while ( p ) {
	if ( ps.y() >= p->rect().y() && ps.y() <= p->rect().y() + p->rect().height() )
	    break;
	p = p->next();
    }
    if ( !p )
	return;
    QTextCursor c( editor->document() );
    c.place( ps, p );
    QTextCursor c2 = c;
    c2.gotoWordLeft();
    c.gotoWordRight();
    if ( c.paragraph() != c2.paragraph() )
	return;
    QString s( c.paragraph()->string()->toString().mid( c2.index(), c.index() - c2.index() + 1 ) );
    s = s.simplifyWhiteSpace();
    static QString legalChars = QString::fromLatin1("abcdefghijklmnopqrstuvwxyzABSCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
    while ( s.length() > 0 ) {
	if ( legalChars.find( s[0] ) == -1 )
	    s.remove( 0, 1 );
	else
	    break;
    }
    while ( s.length() > 0 ) {
	if ( legalChars.find( s[ (int)s.length() - 1 ] ) == -1 )
	    s.remove( s.length() - 1, 1 );
	else
	    break;
    }

    if ( s[ (int)s.length() - 1 ] == ';' )
	s.remove( s.length() - 1, 1 );

#if 0
    if ( editor->isDebugging() ) {
	QString type, val;
	QSProject::ideQSAInterpreter()->debuggerEngine()->watch( s, type, val );
	if ( !val.isEmpty() && !type.isEmpty() ) {
	    QRect r( c2.x() - editor->contentsX(), p->rect().y() + c2.y() - editor->contentsY(),
		     editor->fontMetrics().width( s ), editor->fontMetrics().height() );
	    tip( r, val + " : " + type );
	}
    } else {
    }
#endif
}

QSAEditor::QSAEditor( const QString &fn, QWidget *parent, const char *name )
    : Editor( fn, parent, name ),
      debugging( FALSE )
{
    if ( !fn.isEmpty() )
	load( fn );
    document()->setPreProcessor( new QSASyntaxHighlighter );
    document()->setIndent( (indent = new CIndent) );
    setHScrollBarMode( QScrollView::AlwaysOff );
    setVScrollBarMode( QScrollView::AlwaysOn );
    completion = new QSACompletion( this );
    browser = new QSAEditorBrowser( this );
    configChanged();
    tooltip = new QSAEditorToolTip( this, this );
}

QSAEditor::~QSAEditor()
{
    delete completion;
    delete browser;
    delete tooltip;
}

void QSAEditor::setInterpreter( QSInterpreter *ip )
{
    interp = ip;
    ( (QSACompletion*) completion )->setInterpreter( ip );
}

void QSAEditor::configChanged()
{
    QString path = QString::fromLatin1("/Trolltech/QSAScriptEditor/");
    QMap<QString, ConfigStyle> styles = Config::readStyles( QString::fromLatin1("/Trolltech/QSAScriptEditor/") );
    config()->styles = styles;
    ( (QSASyntaxHighlighter*)document()->preProcessor() )->updateStyles( config()->styles );

    completion->setEnabled( Config::completion( path ) );
    parenMatcher->setEnabled( Config::parenMatching( path ) );
    if ( Config::wordWrap( path ) ) {
	if ( hScrollBarMode() != AlwaysOff ) {
	    document()->setFormatter( new QTextFormatterBreakInWords );
	    setHScrollBarMode( AlwaysOff );
	}
    } else {
	if ( hScrollBarMode() != AlwaysOn ) {
	    QTextFormatterBreakWords *f = new QTextFormatterBreakWords;
	    f->setWrapEnabled( FALSE );
	    document()->setFormatter( f );
	    setHScrollBarMode( AlwaysOn );
	}
    }
    setFont( ( (QSASyntaxHighlighter*)document()->preProcessor() )->format( 0 )->font() );

    indent->setTabSize( Config::indentTabSize( path ) );
    indent->setIndentSize( Config::indentIndentSize( path ) );
    indent->setKeepTabs( Config::indentKeepTabs( path ) );
    indent->setAutoIndent( Config::indentAutoIndent( path ) );
    if ( !Config::indentAutoIndent( path ) )
	document()->setIndent( 0 );
    else
	document()->setIndent( indent );

    document()->setTabStops( ( (QSASyntaxHighlighter*)document()->preProcessor() )->
			     format( QTextPreProcessor::Standard )->width( 'x' ) * Config::indentTabSize( path ) );

    Editor::configChanged();
}

QTextParagraph *QSAEditor::expandFunction( QTextParagraph *p, bool recalc )
{
    if ( recalc )
	setCursorPosition( p->paragId(), 0 );
    ( (ParagData*)p->extraData() )->functionOpen = TRUE;
    p = p->next();
    QValueStack<int> stack;
    while ( p ) {
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart ) {
	    stack.push( TRUE );
	    ( (ParagData*)p->extraData() )->functionOpen = TRUE;
	}
	p->show();
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionEnd ) {
	    if ( stack.isEmpty() )
		break;
	    else
		stack.pop();
	}
	p = p->next();
    }

    if ( recalc ) {
	doRecalc();
	saveLineStates();
    }

    return p;
}

QTextParagraph *QSAEditor::collapseFunction( QTextParagraph *p, bool recalc )
{
    if ( recalc )
	setCursorPosition( p->paragId(), 0 );
    ( (ParagData*)p->extraData() )->functionOpen = FALSE;
    p = p->next();
    QValueStack<int> stack;
    while ( p ) {
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart )
	    stack.push( TRUE );
	p->hide();
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionEnd ) {
	    if ( stack.isEmpty() )
		break;
	    else
		stack.pop();
	}
	p = p->next();
    }

    if ( recalc ) {
	doRecalc();
	saveLineStates();
    }

    return p;
}

void QSAEditor::doRecalc()
{
    document()->invalidate();
    QTextParagraph *p = document()->firstParagraph();
    while ( p ) {
	p->format();
	p = p->next();
    }
    ensureCursorVisible();
    repaintContents( FALSE );
}

void QSAEditor::makeFunctionVisible( QTextParagraph *p )
{
    if ( p->isVisible() )
	return;
    while ( p && ( (ParagData*)p->extraData() )->lineState != ParagData::FunctionStart )
	p = p->prev();
    if ( !p )
	return;
    expandFunction( p );
}

void QSAEditor::collapse( bool all /*else only functions*/ )
{
    QTextParagraph *p = document()->firstParagraph();
    while ( p ) {
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart ) {
	    if ( all
                 || p->string()->toString().simplifyWhiteSpace().left(QString::fromLatin1("function").length()) == QString::fromLatin1("function")
                 || p->string()->toString().simplifyWhiteSpace().left(QString::fromLatin1("constructor").length()) == QString::fromLatin1("constructor") ) {
		p = collapseFunction( p, FALSE );
		continue;
	    }
	}
	p = p->next();
    }
    doRecalc();
    saveLineStates();
}

void QSAEditor::expand( bool all /*else only functions*/ )
{
    QTextParagraph *p = document()->firstParagraph();
    while ( p ) {
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart ) {
	    if ( all ||
		 p->string()->toString().simplifyWhiteSpace().left(QString::fromLatin1("function").length() ) == QString::fromLatin1("function") ||
		 p->string()->toString().simplifyWhiteSpace().left(QString::fromLatin1("constructor").length() ) == QString::fromLatin1("constructor") ) {
		p = expandFunction( p, FALSE );
		continue;
	    }
	}
	p = p->next();
    }
    doRecalc();
    saveLineStates();
}

void QSAEditor::saveLineStates()
{
    QValueList<Q_UINT32> states;
    QTextParagraph *p = document()->firstParagraph();
    while ( p ) {
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart )
	    states << (Q_UINT32)( (ParagData*)p->extraData() )->functionOpen;
	p = p->next();
    }

    // hacky
    QString fn;
    QObject *o = this;
    while ( o ) {
	if ( o->inherits( "SourceEditor" ) ) {
	    fn = ( (QWidget*)o )->caption();
	    fn = fn.mid( 5 );
	    fn = fn.simplifyWhiteSpace();
	    break;
	}
	o = o->parent();
    }

    if ( fn.isEmpty() )
	return;
    fn.replace( QRegExp( QString::fromLatin1("/") ), QString::fromLatin1(";") );

    QString home = QString::fromLatin1(getenv( "HOME" ));
    QFile f( home + QString::fromLatin1("/.designer/cache_") + fn ); // #### should also prefix with project name
    if ( !f.open( IO_WriteOnly ) )
	return;
    QDataStream ts( &f );
    ts << states;
    f.close();
}

void QSAEditor::loadLineStates()
{
    QValueList<Q_UINT32> states;

    // hacky
    QString fn;
    QObject *o = this;
    while ( o ) {
	if ( o->inherits( "SourceEditor" ) ) {
	    fn = ( (QWidget*)o )->caption();
	    fn = fn.mid( 5 );
	    fn = fn.simplifyWhiteSpace();
	    break;
	}
	o = o->parent();
    }

    if ( fn.isEmpty() )
	return;
    fn.replace( QRegExp( QString::fromLatin1("/") ), QString::fromLatin1(";") );

    QString home = QString::fromLatin1(getenv( "HOME" ));
    QFile f( home + QString::fromLatin1("/.designer/cache_") + fn ); // #### should also prefix with project name
    if ( !f.open( IO_ReadOnly ) )
	return;
    QDataStream ts( &f );
    ts >> states;
    f.close();

    QTextParagraph *p = document()->firstParagraph();
    QValueList<Q_UINT32>::Iterator it = states.begin();
    while ( p ) {
	if ( p->extraData() &&
	     ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart ) {
	    if ( it != states.end() ) {
		if ( !(bool)*it )
		    collapseFunction( p, FALSE );
		else
		    expandFunction( p, FALSE );
		++it;
	    } else {
		expandFunction( p, FALSE );
	    }
	}
	p = p->next();
    }
    doRecalc();
}

void QSAEditor::paste()
{
    Editor::paste();
    emit intervalChanged();
}

void QSAEditor::clearError()
{
    emit clearErrorMarker();
    hasError = FALSE;
}
