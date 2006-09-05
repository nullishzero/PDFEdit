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

#include "arghintwidget.h"
#include <qbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>

static const char * left_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #000000",
"                ",
"                ",
"          +     ",
"         ++     ",
"        +++     ",
"       ++++     ",
"      +++++     ",
"     ++++++     ",
"     ++++++     ",
"      +++++     ",
"       ++++     ",
"        +++     ",
"         ++     ",
"          +     ",
"                ",
"                "};

static const char * right_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #000000",
"                ",
"                ",
"     +          ",
"     ++         ",
"     +++        ",
"     ++++       ",
"     +++++      ",
"     ++++++     ",
"     ++++++     ",
"     +++++      ",
"     ++++       ",
"     +++        ",
"     ++         ",
"     +          ",
"                ",
"                "};

static const char * left_disabled_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c darkgray",
"                ",
"                ",
"          +     ",
"         ++     ",
"        +++     ",
"       ++++     ",
"      +++++     ",
"     ++++++     ",
"     ++++++     ",
"      +++++     ",
"       ++++     ",
"        +++     ",
"         ++     ",
"          +     ",
"                ",
"                "};

static const char * right_disabled_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c darkgray",
"                ",
"                ",
"     +          ",
"     ++         ",
"     +++        ",
"     ++++       ",
"     +++++      ",
"     ++++++     ",
"     ++++++     ",
"     +++++      ",
"     ++++       ",
"     +++        ",
"     ++         ",
"     +          ",
"                ",
"                "};

class ArrowButton : public QButton
{
    Q_OBJECT

public:
    enum Dir { Left, Right };

    ArrowButton( QWidget *parent, const char *name, Dir d );
    void drawButton( QPainter *p );

private:
    QPixmap pix, pix_disabled;

};

ArrowButton::ArrowButton( QWidget *parent, const char *name, Dir d )
    : QButton( parent, name )
{
    setFixedSize( 16, 16 );
    if ( d == Left ) {
	pix = QPixmap( left_xpm );
	pix_disabled = QPixmap( left_disabled_xpm );
    } else {
	pix = QPixmap( right_xpm );
	pix_disabled = QPixmap( right_disabled_xpm );
    }
}

void ArrowButton::drawButton( QPainter *p )
{
    if ( isDown() )
	p->fillRect( 0, 0, width(), height(), darkGray );
    else
	p->fillRect( 0, 0, width(), height(), lightGray );
    if ( isEnabled() )
	p->drawPixmap( 0, 0, pix );
    else
	p->drawPixmap( 0, 0, pix_disabled );
}


ArgHintWidget::ArgHintWidget( QWidget *parent, const char*name )
    : QFrame( parent, name, WType_Popup ), curFunc( 0 ), numFuncs( 0 )
{
    setFrameStyle( QFrame::Box | QFrame::Plain );
    setLineWidth( 1 );
    setBackgroundColor( white );
    QHBoxLayout *hbox = new QHBoxLayout( this );
    hbox->setMargin( 1 );
    hbox->addWidget( ( prev = new ArrowButton( this, "editor_left_btn", ArrowButton::Left ) ) );
    hbox->addWidget( ( funcLabel = new QLabel( this, "editor_func_lbl" ) ) );
    hbox->addWidget( ( next = new ArrowButton( this, "editor_right_btn", ArrowButton::Right ) ) );
    funcLabel->setBackgroundColor( white );
    funcLabel->setAlignment( AlignCenter );
    connect( prev, SIGNAL( clicked() ), this, SLOT( gotoPrev() ) );
    connect( next, SIGNAL( clicked() ), this, SLOT( gotoNext() ) );
    updateState();
    setFocusPolicy( NoFocus );
    prev->setFocusPolicy( NoFocus );
    next->setFocusPolicy( NoFocus );
    funcLabel->setFocusPolicy( NoFocus );
}

void ArgHintWidget::setFunctionText( int func, const QString &text )
{
    funcs.replace( func, text );
    if ( func == curFunc ) {
	funcLabel->clear();
	funcLabel->setText( text );
    }
}

void ArgHintWidget::setNumFunctions( int num )
{
    funcs.clear();
    numFuncs = num;
    curFunc = 0;
    updateState();
}

void ArgHintWidget::gotoPrev()
{
    if ( curFunc > 0 ) {
	curFunc--;
	funcLabel->setText( funcs[ curFunc ] );
	updateState();
    }
}

void ArgHintWidget::gotoNext()
{
    if ( curFunc < numFuncs - 1 ) {
	curFunc++;
	funcLabel->setText( funcs[ curFunc ] );
	updateState();
    }
}

void ArgHintWidget::updateState()
{
    prev->setEnabled( curFunc > 0 );
    next->setEnabled( curFunc < numFuncs - 1 );
}

void ArgHintWidget::relayout()
{
    funcLabel->setText( QString::fromLatin1("") );
    funcLabel->setText( funcs[ curFunc ] );
}

#include "arghintwidget.moc"
