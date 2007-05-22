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

#include "markerwidget.h"
#include "viewmanager.h"
#include "q3richtext_p.h"
#include "editor.h"
#include <qpainter.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QContextMenuEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include "paragdata.h"

static const char * error_xpm[] = {
"15 15 35 1",
"       c None",
".      c #FF0000",
"+      c #F50F0F",
"@      c #BF5F5F",
"#      c #FF1010",
"$      c #FF7878",
"%      c #FF0A0A",
"&      c #FF0606",
"*      c #FF1414",
"=      c #FFFFFF",
"-      c #FFA3A3",
";      c #FF0707",
">      c #FF0202",
",      c #FF9898",
"'      c #FF8888",
")      c #D04747",
"!      c #FFA7A7",
"~      c #FF9D9D",
"{      c #FFB1B1",
"]      c #FF0C0C",
"^      c #F90A0A",
"/      c #FFB5B5",
"(      c #FF0909",
"_      c #A08F8F",
":      c #FFACAC",
"<      c #FF0303",
"[      c #9F8F8F",
"}      c #FB0606",
"|      c #9F9090",
"1      c #CE4949",
"2      c #999999",
"3      c #FF1919",
"4      c #F70C0C",
"5      c #A38A8A",
"6      c #B37272",
"     ....      ",
"   ........    ",
"  .........+@  ",
" ..#$%..&$*.+  ",
" ..$=-;>,='..) ",
"...%-=!~={]..^ ",
"....;!==/(...._",
"....>~==:<....[",
"...&,=/:=!;..}|",
" ..$={(<!='..12",
" ..*']..;'3.45 ",
"  +........462 ",
"  @+......462  ",
"    )^..}152   ",
"      _[|2     "};

static const char * breakpoint_xpm[] = {
"15 15 3 1",
"       c None",
".      c #8B0000",
"+      c #FFFFFF",
"    .......    ",
"   .........   ",
"  ...........  ",
" ............. ",
"..+.+++.+..++..",
".+.+.+.+.+.+.+.",
".+...+.+.+.+.+.",
"..+..+.+.+.++..",
"...+.+.+.+.+...",
".+.+.+.+.+.+...",
"..+..+..+..+...",
" ............. ",
"  ...........  ",
"   .........   ",
"    .......    " };

static const char * step_xpm[] = {
"16 16 128 2",
"  	c None",
". 	c #B4B6BF",
"+ 	c #7893D8",
"@ 	c #8D95BF",
"# 	c #B8BFC1",
"$ 	c #B6D1E6",
"% 	c #7193E6",
"& 	c #8893C2",
"* 	c #B3BDC4",
"= 	c #AAD2EC",
"- 	c #9AD0FF",
"; 	c #6690EF",
"> 	c #8894C8",
", 	c #AFBAC4",
"' 	c #95BFEC",
") 	c #99CBFF",
"! 	c #8EC3FF",
"~ 	c #6D95F0",
"{ 	c #8792CA",
"] 	c #9DA7C3",
"^ 	c #8BA2E3",
"/ 	c #809AE0",
"( 	c #8398D1",
"_ 	c #93A0CC",
": 	c #ACB3CB",
"< 	c #B4B9C4",
"[ 	c #B6BAC4",
"} 	c #93A4CC",
"| 	c #82B0F5",
"1 	c #8BBCFF",
"2 	c #8EC0FF",
"3 	c #8FC1FF",
"4 	c #6594F4",
"5 	c #7381CC",
"6 	c #81A7E9",
"7 	c #D0F5FF",
"8 	c #C1EBFF",
"9 	c #AEDAFF",
"0 	c #A2D1FC",
"a 	c #A3C8F3",
"b 	c #AACAE6",
"c 	c #B4CFE9",
"d 	c #ADCCF9",
"e 	c #84B2FF",
"f 	c #82B4FF",
"g 	c #86B7FF",
"h 	c #88B7FF",
"i 	c #83B4FF",
"j 	c #5F8AF3",
"k 	c #7585C8",
"l 	c #77A4F3",
"m 	c #ABDFFF",
"n 	c #9CCAFF",
"o 	c #96C7FF",
"p 	c #97C8FF",
"q 	c #95C5FF",
"r 	c #9DCCFF",
"s 	c #A0CDFF",
"t 	c #90C0FF",
"u 	c #82AFFF",
"v 	c #7FAFFF",
"w 	c #7DAEFF",
"x 	c #79AAFF",
"y 	c #6C9EFF",
"z 	c #4366EB",
"A 	c #6894F2",
"B 	c #93C6FF",
"C 	c #82B3FF",
"D 	c #7AABFF",
"E 	c #73A5FF",
"F 	c #71A3FF",
"G 	c #6C9DFF",
"H 	c #699BFF",
"I 	c #76A8FF",
"J 	c #7EB0FF",
"K 	c #7BADFF",
"L 	c #74A5FF",
"M 	c #608BFF",
"N 	c #3462FF",
"O 	c #2444E5",
"P 	c #577AE0",
"Q 	c #5D90FF",
"R 	c #4C7AFF",
"S 	c #3B66FF",
"T 	c #335CF9",
"U 	c #365AF1",
"V 	c #3858E5",
"W 	c #3959E0",
"X 	c #416CF9",
"Y 	c #75A5FF",
"Z 	c #78A9FF",
"` 	c #74A4FF",
" .	c #6191FF",
"..	c #3059FF",
"+.	c #1B37F1",
"@.	c #6A75C7",
"#.	c #828BC1",
"$.	c #4358D8",
"%.	c #374BDA",
"&.	c #4759CA",
"*.	c #636CC4",
"=.	c #8489C0",
"-.	c #9DA1C1",
";.	c #A3A6BF",
">.	c #7486CB",
",.	c #6E98F5",
"'.	c #719EFF",
").	c #608DFF",
"!.	c #315EFF",
"~.	c #1432F4",
"{.	c #5C63C8",
"].	c #B1B4B9",
"^.	c #B3BABB",
"/.	c #ABB4C3",
"(.	c #7299E9",
"_.	c #5486FF",
":.	c #224EFF",
"<.	c #1733F2",
"[.	c #7079C5",
"}.	c #5C7DE9",
"|.	c #2450FF",
"1.	c #1B39EC",
"2.	c #7077C5",
"3.	c #3A54E1",
"4.	c #1E36EA",
"5.	c #858CBF",
"6.	c #525FCB",
"7.	c #727CBC",
"                                ",
"                . + @           ",
"                # $ % &         ",
"                * = - ; >       ",
"                , ' ) ! ~ {     ",
"] ^ / ( _ : < [ } | 1 2 3 4 5   ",
"6 7 8 9 0 a b c d e f g h i j k ",
"l m n o p q r s t u v u w x y z ",
"A B C D E F G H I J K D L M N O ",
"P Q R S T U V W X Y Z `  ...+.@.",
"#.$.%.&.*.=.-.;.>.,.'.).!.~.{.  ",
"  ].^.          /.(._.:.<.[.    ",
"                  }.|.1.2.      ",
"                  3.4.5.        ",
"                  6.7.          ",
"                                "};

static const char *stack_frame_xpm[]={
"16 16 2 1",
". c None",
"# c #00c000",
"................",
".###............",
".#####..........",
".#######........",
".#########......",
".###########....",
".#############..",
".##############.",
".##############.",
".#############..",
".###########....",
".#########......",
".#######........",
".#####..........",
".###............",
"................"};

static QPixmap *errorPixmap = 0;
static QPixmap *breakpointPixmap = 0;
static QPixmap *stepPixmap = 0;
static QPixmap *stackFrame = 0;

MarkerWidget::MarkerWidget( ViewManager *parent, const char*name )
    : QWidget(parent, name), viewManager( parent )
{
    if ( !errorPixmap ) {
	errorPixmap = new QPixmap( error_xpm );
	breakpointPixmap = new QPixmap( breakpoint_xpm );
	stepPixmap = new QPixmap( step_xpm );
	stackFrame = new QPixmap( stack_frame_xpm );
    }
}

void MarkerWidget::paintEvent( QPaintEvent * )
{
    Q3TextParagraph *p = ( (Editor*)viewManager->currentView() )->document()->firstParagraph();
    QPainter painter(this);

    int yOffset = ( (Editor*)viewManager->currentView() )->contentsY();
    while ( p ) {
	if ( !p->isVisible() ) {
	    p = p->next();
	    continue;
	}
	if ( p->rect().y() + p->rect().height() - yOffset < 0 ) {
	    p = p->next();
	    continue;
	}
	if ( p->rect().y() - yOffset > height() )
	    break;
	if ( !((p->paragId() + 1) % 10) ) {
	    painter.save();
	    painter.setPen( colorGroup().dark() );
	    painter.drawText( 0, p->rect().y() - yOffset, width() - 20, p->rect().height(),
			      Qt::AlignRight | Qt::AlignTop, QString::number( p->paragId() + 1 ) );
	    painter.restore();
	}
	ParagData *paragData = (ParagData*)p->extraData();
	if ( paragData ) {
	    switch ( paragData->marker ) {
	    case ParagData::Error:
		painter.drawPixmap( 3, p->rect().y() +
				    ( p->rect().height() - errorPixmap->height() ) / 2 -
				    yOffset, *errorPixmap );
		break;
	    case ParagData::Breakpoint:
		painter.drawPixmap( 3, p->rect().y() +
				    ( p->rect().height() - breakpointPixmap->height() ) / 2 -
				    yOffset, *breakpointPixmap );
		break;
	    default:
		break;
	    }
	    switch ( paragData->lineState ) {
	    case ParagData::FunctionStart:
		painter.setPen( colorGroup().foreground() );
		painter.setBrush( colorGroup().base() );
		painter.drawLine( width() - 11, p->rect().y() - yOffset,
				  width() - 11, p->rect().y() + p->rect().height() - yOffset );
		painter.drawRect( width() - 15, p->rect().y() + ( p->rect().height() - 9 ) / 2 - yOffset, 9, 9 );
		painter.drawLine( width() - 13, p->rect().y() + ( p->rect().height() - 9 ) / 2 - yOffset + 4,
				  width() - 9, p->rect().y() +
				  ( p->rect().height() - 9 ) / 2 - yOffset + 4 );
		if ( !paragData->functionOpen )
		    painter.drawLine( width() - 11,
				      p->rect().y() + ( p->rect().height() - 9 ) / 2 - yOffset + 2,
				      width() - 11,
				      p->rect().y() +
				      ( p->rect().height() - 9 ) / 2 - yOffset + 6 );
		break;
	    case ParagData::InFunction:
		painter.setPen( colorGroup().foreground() );
		painter.drawLine( width() - 11, p->rect().y() - yOffset,
				  width() - 11, p->rect().y() + p->rect().height() - yOffset );
		break;
	    case ParagData::FunctionEnd:
		painter.setPen( colorGroup().foreground() );
		painter.drawLine( width() - 11, p->rect().y() - yOffset,
				  width() - 11, p->rect().y() + p->rect().height() - yOffset );
		painter.drawLine( width() - 11, p->rect().y() + p->rect().height() - yOffset,
				  width() - 7, p->rect().y() + p->rect().height() - yOffset );
		break;
	    default:
		break;
	    }
	    if ( paragData->step )
		painter.drawPixmap( 3, p->rect().y() +
				    ( p->rect().height() - stepPixmap->height() ) / 2 -
				    yOffset, *stepPixmap );
	    if ( paragData->stackFrame )
		painter.drawPixmap( 3, p->rect().y() +
				    ( p->rect().height() - stackFrame->height() ) / 2 -
				    yOffset, *stackFrame );
	}
	p = p->next();
    }
}

void MarkerWidget::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != Qt::LeftButton )
	return;
    bool supports = ( (Editor*)viewManager->currentView() )->supportsBreakPoints();
    Q3TextParagraph *p = ( (Editor*)viewManager->currentView() )->document()->firstParagraph();
    int yOffset = ( (Editor*)viewManager->currentView() )->contentsY();
    while ( p ) {
	if ( e->y() >= p->rect().y() - yOffset && e->y() <= p->rect().y() + p->rect().height() - yOffset ) {
	    Q3TextParagraphData *d = p->extraData();
	    if ( !d )
		return;
	    ParagData *data = (ParagData*)d;
	    if ( supports && ( e->x() < width() - 15 ) ) {
		if ( data->marker == ParagData::Breakpoint ) {
		    data->marker = ParagData::NoMarker;
		} else {
		    bool ok = true;
		    isBreakpointPossible( ok, ( (Editor*)viewManager->currentView() )->text(), p->paragId() );
		    if ( ok )
			data->marker = ParagData::Breakpoint;
		    else
			emit showMessage( tr( "<font color=red>Can't set breakpoint here!</font>" ) );
		}
	    } else {
		if ( data->lineState == ParagData::FunctionStart ) {
		    if ( data->functionOpen )
			emit collapseFunction( p );
		    else
			emit expandFunction( p );
		}
	    }
	    break;
	}
	p = p->next();
    }
    doRepaint();
    emit markersChanged();
}

void MarkerWidget::contextMenuEvent( QContextMenuEvent *e )
{
    Q3PopupMenu m( 0, "editor_breakpointsmenu" );

    int toggleBreakPoint = 0;
//    int editBreakpoints = 0;

    Q3TextParagraph *p = ( (Editor*)viewManager->currentView() )->document()->firstParagraph();
    int yOffset = ( (Editor*)viewManager->currentView() )->contentsY();
    bool supports = ( (Editor*)viewManager->currentView() )->supportsBreakPoints();
    while ( p && supports ) {
	if ( e->y() >= p->rect().y() - yOffset && e->y() <= p->rect().y() + p->rect().height() - yOffset ) {
	    if ( ( (ParagData*)p->extraData() )->marker == ParagData::Breakpoint )
		toggleBreakPoint = m.insertItem( tr( "Clear Breakpoint\tF9" ) );
	    else
		toggleBreakPoint = m.insertItem( tr( "Set Breakpoint\tF9" ) );
// 	    editBreakpoints = m.insertItem( tr( "Edit Breakpoints..." ) );
	    m.insertSeparator();
	    break;
	}
	p = p->next();
    }

    const int collapseAll = m.insertItem( tr( "Collapse All" ) );
    const int expandAll = m.insertItem( tr( "Expand All" ) );
    const int collapseFunctions = m.insertItem( tr( "Collapse all Functions" ) );
    const int expandFunctions = m.insertItem( tr( "Expand all Functions" ) );

    int res = m.exec( e->globalPos() );
    if ( res == -1)
	return;

    if ( res == collapseAll ) {
	emit collapse( true );
    } else if ( res == collapseFunctions ) {
	emit collapse( false );
    } else if ( res == expandAll ) {
	emit expand( true );
    } else if ( res == expandFunctions ) {
	emit expand( false );
    } else if ( res == toggleBreakPoint ) {
	if ( ( (ParagData*)p->extraData() )->marker == ParagData::Breakpoint ) {
	    ( (ParagData*)p->extraData() )->marker = ParagData::NoMarker;
	} else {
	    bool ok;
	    isBreakpointPossible( ok, ( (Editor*)viewManager->currentView() )->text(), p->paragId() );
	    if ( ok )
		( (ParagData*)p->extraData() )->marker = ParagData::Breakpoint;
	    else
		emit showMessage( tr( "<font color=red>Can't set breakpoint here!</font>" ) );
	}
//    } else if ( res == editBreakpoints ) {
//	emit editBreakPoints();
    }
    doRepaint();
    emit markersChanged();
}
