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

#include "qsabrowser.h"
#include "qsacompletion.h"
#include "qsaeditor.h"
#include "qsasyntaxhighlighter.h"
#include "qsproject.h"
// #include "quickdebugger.h"
#include "quickinterpreter.h"
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3ValueStack>
#include <cindent.h>
#include <paragdata.h>
#include <parenmatcher.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qregexp.h>
#include <stdlib.h>

QSAEditor::QSAEditor( const QString &fn, QWidget *parent, const char *name )
    : Editor( fn, parent, name ),
      debugging( false )
{
    if ( !fn.isEmpty() )
	load( fn );
    document()->setPreProcessor( new QSASyntaxHighlighter );
    document()->setIndent( (indent = new CIndent) );
    setHScrollBarMode( Q3ScrollView::AlwaysOff );
    setVScrollBarMode( Q3ScrollView::AlwaysOn );
    completion = new QSACompletion( this );
    browser = new QSAEditorBrowser( this );
    configChanged();
}

QSAEditor::~QSAEditor()
{
    delete completion;
    delete browser;
}

void QSAEditor::setInterpreter( QSInterpreter *ip )
{
    interp = ip;
    ( (QSACompletion*) completion )->setInterpreter( ip );
}

void QSAEditor::configChanged()
{
    QMap<QString, ConfigStyle> styles = Config::readStyles();
    config()->styles = styles;
    ( (QSASyntaxHighlighter*)document()->preProcessor() )->updateStyles( config()->styles );

    completion->setEnabled( Config::completion() );
    parenMatcher->setEnabled( Config::parenMatching() );
    if ( Config::wordWrap() ) {
	if ( hScrollBarMode() != AlwaysOff ) {
	    document()->setFormatter( new Q3TextFormatterBreakInWords );
	    setHScrollBarMode( AlwaysOff );
	}
    } else {
	if ( hScrollBarMode() != AlwaysOn ) {
	    Q3TextFormatterBreakWords *f = new Q3TextFormatterBreakWords;
	    f->setWrapEnabled( false );
	    document()->setFormatter( f );
	    setHScrollBarMode( AlwaysOn );
	}
    }
    setFont( ( (QSASyntaxHighlighter*)document()->preProcessor() )->format( 0 )->font() );

    indent->setTabSize( Config::indentTabSize() );
    indent->setIndentSize( Config::indentIndentSize() );
    indent->setKeepTabs( Config::indentKeepTabs() );
    indent->setAutoIndent( Config::indentAutoIndent() );
    if ( !Config::indentAutoIndent() )
	document()->setIndent( 0 );
    else
	document()->setIndent( indent );

    document()->setTabStops( ( (QSASyntaxHighlighter*)document()->preProcessor() )->
			     format( Q3TextPreProcessor::Standard )->width( 'x' ) * Config::indentTabSize() );

    Editor::configChanged();
}

Q3TextParagraph *QSAEditor::expandFunction( Q3TextParagraph *p, bool recalc )
{
    if ( recalc )
	setCursorPosition( p->paragId(), 0 );
    ( (ParagData*)p->extraData() )->functionOpen = true;
    p = p->next();
    Q3ValueStack<int> stack;
    while ( p ) {
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart ) {
	    stack.push( true );
	    ( (ParagData*)p->extraData() )->functionOpen = true;
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
    }

    return p;
}

Q3TextParagraph *QSAEditor::collapseFunction( Q3TextParagraph *p, bool recalc )
{
    if ( recalc )
	setCursorPosition( p->paragId(), 0 );
    ( (ParagData*)p->extraData() )->functionOpen = false;
    p = p->next();
    Q3ValueStack<int> stack;
    while ( p ) {
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart )
	    stack.push( true );
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
    }

    return p;
}

void QSAEditor::doRecalc()
{
    document()->invalidate();
    Q3TextParagraph *p = document()->firstParagraph();
    while ( p ) {
	p->format();
	p = p->next();
    }
    ensureCursorVisible();
    repaintContents( false );
}

void QSAEditor::makeFunctionVisible( Q3TextParagraph *p )
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
    Q3TextParagraph *p = document()->firstParagraph();
    while ( p ) {
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart ) {
	    if ( all
                 || p->string()->toString().simplifyWhiteSpace().left(QString::fromLatin1("function").length()) == QString::fromLatin1("function")
                 || p->string()->toString().simplifyWhiteSpace().left(QString::fromLatin1("constructor").length()) == QString::fromLatin1("constructor") ) {
		p = collapseFunction( p, false );
		continue;
	    }
	}
	p = p->next();
    }
    doRecalc();
}

void QSAEditor::expand( bool all /*else only functions*/ )
{
    Q3TextParagraph *p = document()->firstParagraph();
    while ( p ) {
	if ( ( (ParagData*)p->extraData() )->lineState == ParagData::FunctionStart ) {
	    if ( all ||
		 p->string()->toString().simplifyWhiteSpace().left(QString::fromLatin1("function").length() ) == QString::fromLatin1("function") ||
		 p->string()->toString().simplifyWhiteSpace().left(QString::fromLatin1("constructor").length() ) == QString::fromLatin1("constructor") ) {
		p = expandFunction( p, false );
		continue;
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
    hasError = false;
}
