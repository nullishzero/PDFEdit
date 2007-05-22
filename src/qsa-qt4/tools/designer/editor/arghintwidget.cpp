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

#include "arghintwidget.h"
#include <q3button.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QHBoxLayout>

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

class ArrowButton : public Q3Button
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
    : Q3Button( parent, name )
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
	p->fillRect( 0, 0, width(), height(), Qt::darkGray );
    else
	p->fillRect( 0, 0, width(), height(), Qt::lightGray );
    if ( isEnabled() )
	p->drawPixmap( 0, 0, pix );
    else
	p->drawPixmap( 0, 0, pix_disabled );
}


ArgHintWidget::ArgHintWidget( QWidget *parent, const char*name )
    : Q3Frame( parent, name, Qt::WType_Popup ), curFunc( 0 ), numFuncs( 0 )
{
    setFrameStyle( Q3Frame::Box | Q3Frame::Plain );
    setLineWidth( 1 );
    setBackgroundColor( Qt::white );
    QHBoxLayout *hbox = new QHBoxLayout( this );
    hbox->setMargin( 1 );
    hbox->addWidget( ( prev = new ArrowButton( this, "editor_left_btn", ArrowButton::Left ) ) );
    hbox->addWidget( ( funcLabel = new QLabel( this, "editor_func_lbl" ) ) );
    hbox->addWidget( ( next = new ArrowButton( this, "editor_right_btn", ArrowButton::Right ) ) );
    funcLabel->setBackgroundColor( Qt::white );
    funcLabel->setAlignment( Qt::AlignCenter );
    connect( prev, SIGNAL( clicked() ), this, SLOT( gotoPrev() ) );
    connect( next, SIGNAL( clicked() ), this, SLOT( gotoNext() ) );
    updateState();
    setFocusPolicy( Qt::NoFocus );
    prev->setFocusPolicy( Qt::NoFocus );
    next->setFocusPolicy( Qt::NoFocus );
    funcLabel->setFocusPolicy( Qt::NoFocus );
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
