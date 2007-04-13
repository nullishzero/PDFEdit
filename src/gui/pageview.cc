/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#include "pageview.h"
#include <stdlib.h>
#include <qpixmap.h>
#include "rect2Darray.h"
#include "util.h"
#include "debug.h"
#include <assert.h>
#include "pdfoperators.h"

using namespace pdfobjects;

#include "pageviewmode.h"

namespace gui {

PageView::PageView (QWidget *parent) : QWidget(parent) {
	// initialize privet variable
//	pageImage = NULL;

	// if something use on page, take focus
	setFocusPolicy( ClickFocus );
	// call mouseMoveEvent everytime if mouse move (not only if is a button pressed)
	setMouseTracking( true );
}

PageView::~PageView () {
//	delete pageImage;
}

bool PageView::saveImage ( const QString & file, const char * format, int quality, bool onlySelectedArea) {
	if (! onlySelectedArea) {
		return pixmap()->save( file, format, quality );
	}

	QRect r;
	if (! mode) {
		guiPrintDbg ( debug::DBG_INFO, tr("Mode is not set!") );
		return false;
	}
	if (mode->getSelectedRegion().isEmpty()) {
		guiPrintDbg ( debug::DBG_INFO, tr("Selected area is empty!") );
		return false;
	}
	r = mode->getSelectedRegion().boundingRect() & pixmap()->rect();

	QPixmap * pom = new QPixmap( r.size() );

	copyBlt( pom, 0,0, pixmap(), r.x(), r.y(), r.width(), r.height() );

	bool r_pom = pom->save( file, format, quality );
	delete pom;

	return r_pom;
}

void PageView::setPixmap (const QPixmap & qp) {
	this->QLabel::setPixmap(qp);

//	delete pageImage;
//	pageImage = new QPixmap(qp);

	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->repaint( painter, this );

		painter.end();
	}
	repaint( false );
}

void PageView::setSelectionMode ( const boost::shared_ptr<PageViewMode> & m ) {
	mode = m;
}
void PageView::needRepaint ( ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->repaint( painter, this );

		painter.end();
	}
	repaint( false );
}

void PageView::mousePressEvent ( QMouseEvent * e ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->mousePressEvent( e, painter, this );

		painter.end();
	}
	repaint( false );
}

void PageView::mouseReleaseEvent ( QMouseEvent * e ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->mouseReleaseEvent( e, painter, this );

		painter.end();
	}
	repaint( false );
}

void PageView::mouseDoubleClickEvent ( QMouseEvent * e ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->mouseDoubleClickEvent( e, painter, this );

		painter.end();
	}
	repaint( false );
}

void PageView::mouseMoveEvent ( QMouseEvent * e ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->mouseMoveEvent( e, painter, this );

		painter.end();
	}
	repaint( false );

	// emit change moese cursor on page
	if ( mousePos != e->pos() ) {
		mousePos = e->pos();
		emit changeMousePosition( mousePos );
	}
}

void PageView::wheelEvent ( QWheelEvent * e ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->wheelEvent( e, painter, this );

		painter.end();
	}
	repaint( false );

	e->ignore();
}

void PageView::keyPressEvent ( QKeyEvent * e ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->keyPressEvent( e, painter, this );

		painter.end();
	}
	repaint( false );
}

void PageView::keyReleaseEvent ( QKeyEvent * e ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->keyReleaseEvent( e, painter, this );

		painter.end();
	}
	repaint( false );
}

void PageView::focusInEvent ( QFocusEvent * e ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->focusInEvent( e, painter, this );

		painter.end();
	}
	repaint( false );
}

void PageView::focusOutEvent ( QFocusEvent * e ) {
	if (mode) {
		painter.begin( this->pixmap() );
		painter.setRasterOp( Qt::NotXorROP );

		mode->focusOutEvent( e, painter, this );

		painter.end();
	}
	repaint( false );
}

} // namespace gui
