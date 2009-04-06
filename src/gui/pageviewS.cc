/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#include "pageviewS.h"

#include <stdlib.h>
#include <qpixmap.h>
#include <assert.h>

#include "util.h"
#include "utils/debug.h"
#include "kernel/pdfoperators.h"

#include "xpdf/OutputDev.h"
#include "QOutputDevPixmap.h"

#include "rect2Darray.h"

using namespace pdfobjects;

#include "pageviewmode.h"

namespace gui {

#define _splashMakeRGB8(to, r, g, b) \
		  (to[3]=0, to[2]=((r) & 0xff) , to[1]=((g) & 0xff) , to[0]=((b) & 0xff) )

PageViewS::PageViewS (QWidget *parent) : Q_ScrollView(parent) {
	// initialize variable
	pagePixmap = NULL;
	movedPageToCenter.setX( 0 );
	movedPageToCenter.setY( 0 );

	// initialize zoom
	basePpP		= QPaintDevice::x11AppDpiX() / 72.0;
	zoomFactor	= 1;

	displayParams = DisplayParams();

	// if something use on page, take focus
	setFocusPolicy( TheWheelFocus );
	viewport()->setFocusPolicy( TheWheelFocus );
	// call mouseMoveEvent everytime if mouse move (not only if is a button pressed)
	viewport()->setMouseTracking( true );
}

PageViewS::~PageViewS () {
	delete pagePixmap;
}

bool PageViewS::saveImage ( const QString & file, const char * format, int quality, bool onlySelectedArea) {
	if (! actualPage)
		return false;

	if (! onlySelectedArea) {
		// TODO   now is saving only slice of page
		if (pagePixmap)
			return pagePixmap->save( file, format, quality );
		else {
			guiPrintDbg ( debug::DBG_INFO, "Page is not loaded!" );
			return false;
		}
	}

	QRect r;
	if (! mode) {
		guiPrintDbg ( debug::DBG_INFO, "Mode is not set!" );
		return false;
	}
	if (mode->getSelectedRegion().isEmpty()) {
		guiPrintDbg ( debug::DBG_INFO, "Selected area is empty!" );
		return false;
	}
	r = mode->getSelectedRegion().boundingRect() & croppedPage;
	r.moveTopLeft( r.topLeft() - croppedPage.topLeft() );

	QPixmap * pom = new QPixmap( r.size() );

	copyBlt( pom, 0,0, pagePixmap, r.x(), r.y(), r.width(), r.height() );

	bool r_pom = pom->save( file, format, quality );
	delete pom;

	return r_pom;
}

//-------------------------------------------------------------------
void PageViewS::updateDisplayParameters ( OutputDev & output ) {
	// update upsideDown
	displayParams.upsideDown = output.upsideDown();

	// set null pageRect if actual page doesn't define
	if (actualPage == NULL) {
		displayParams.pageRect.xleft =
		displayParams.pageRect.yleft =
		displayParams.pageRect.xright =
		displayParams.pageRect.yright = 0;

		return;
	}

	// update mediabox
	try {
		libs::Rectangle mb = actualPage->getMediabox();
		displayParams.pageRect = mb;
	} catch (ElementNotFoundException) {
		// TODO find mediabox in parent
		displayParams.pageRect = DisplayParams().pageRect;
	}

	// update rotate
	try {
		displayParams.rotate = actualPage->getRotation();
	} catch (ElementNotFoundException) {
		// TODO find rotate in parent
		displayParams.rotate = 0;
	}
	// TODO  cropbox, ...
}
void PageViewS::setCorrectSize() {
	if (actualPage == NULL) {
		sizeOfPage.setWidth( 0 );
		sizeOfPage.setHeight( 0 );
		resizeContents( 0, 0 );

		return;
	}

	double x1,y1,x2,y2;

	displayParams.convertPdfPosToPixmapPos( displayParams.pageRect.xleft, displayParams.pageRect.yleft, x1, y1 );
	displayParams.convertPdfPosToPixmapPos( displayParams.pageRect.xright, displayParams.pageRect.yright, x2, y2 );

	sizeOfPage.setWidth( (int)std::max(x1,x2) );
	sizeOfPage.setHeight( (int)std::max(y1,y2) );
	resizeContents( sizeOfPage.width(), sizeOfPage.height() );

	// set correct position page on viewport
	centerPage( );
}
void PageViewS::showPage ( boost::shared_ptr<pdfobjects::CPage> page ) {
	actualPage = page;

	// reset saved crop of page
	delete pagePixmap;
	pagePixmap = NULL;
//	croppedPage.setRect(-1,-1,-1,-1);

	// initialize create pixmap for page
	SplashColor paperColor;
	QOutputDevPixmap output ( paperColor );

	// update display parameters
	updateDisplayParameters (output);

	// set correct size of viewport
	setCorrectSize();

	// center page on viewport if page is smaller then viewport
	centerPage( );

	if (actualPage) {
		int x,y, w,h;
		w = contentsX() - 100 - movedPageToCenter.x();
		h = contentsY() - 100 - movedPageToCenter.y();
		x = std::max( w, 0 );
		y = std::max( h, 0 );
		w = std::min( viewport()->width() + contentsX() - x - movedPageToCenter.x() + 200, sizeOfPage.width() );
		h = std::min( viewport()->height() + contentsY() - y - movedPageToCenter.y() + 200, sizeOfPage.height() );
		QRect hr (x,y,w,h);

		displayParams.rotate += 360;
		setPixmap( hr );
		displayParams.rotate -= 360;
	}
	// initialize work operators in mode - must be after change display parameters
	//		and reloaded BBox of operators (with displayPage)
	initializeWorkOperatorsInMode();

	// show new pixmap
	repaintContents( true );
}
void PageViewS::setPixmap (const QRect & r) {
	if (actualPage == NULL)
		return;

	// initialize create pixmap for page
	SplashColor paperColor;
	_splashMakeRGB8(paperColor, 0xff, 0xff, 0xff);
	QOutputDevPixmap output ( paperColor );

	// create pixmap for page
	// if width or height is 0 then change because call displayPage do segmentation fault in xpdf code
	actualPage->displayPage( output, displayParams, r.left(), r.top(), (r.width() != 0) ? r.width() : 1, (r.height() != 0) ? r.height() : 1 );

	// get created pixmap
	QImage img = output.getImage();
	delete pagePixmap;
	if (img.isNull()) {
		pagePixmap = NULL;
	} else {
		pagePixmap = new QPixmap( img );

		croppedPage = r;
	}

}
//--------------------------------------------------------------------

void PageViewS::setSelectionMode ( const boost::shared_ptr<PageViewMode> & m ) {
	mode = m;

	// initialize work operators in mode
	initializeWorkOperatorsInMode();

	connect( mode.get(), SIGNAL( needRepaint() ), this, SLOT( needRepaint() ) );
	connect( mode.get(), SIGNAL( scroll(int,int) ), this, SLOT( scrollBy(int,int) ) );
	repaintContents( false );
}

void PageViewS::centerPage (  ) {
	int posX, posY;

	// If no page is shown, don't center
	if (actualPage == NULL) {
		movedPageToCenter.setX( 0 );
		movedPageToCenter.setY( 0 );

		return;
	}

	// Calculation topLeft position of page on viewport
	if ((posX = (visibleWidth() - sizeOfPage.width()) / 2) < 0 )
		posX = 0;
	if ((posY = (visibleHeight() - sizeOfPage.height()) / 2) < 0 )
		posY = 0;

	// set page to center of viewport
	movedPageToCenter.setX( posX );
	movedPageToCenter.setY( posY );
}

void PageViewS::drawContents(QPainter* p, int cx, int cy, int cw, int ch) {
	if (! actualPage)
		return;

	int x,y, w,h;
	x = std::max( cx, movedPageToCenter.x() );
	y = std::max( cy, movedPageToCenter.y() );
	w = std::min( cx + cw - x+1, sizeOfPage.width() );
	h = std::min( cy + ch - y+1, sizeOfPage.height() );
	QRect dr ( x - movedPageToCenter.x(), y - movedPageToCenter.y(), w, h);
	
	if (! croppedPage.contains( dr )) {
		w = contentsX() - 100 - movedPageToCenter.x();
		h = contentsY() - 100 - movedPageToCenter.y();
		x = std::max( w, 0 );
		y = std::max( h, 0 );
		w = std::min( viewport()->width() + contentsX() - x - movedPageToCenter.x() + 200, sizeOfPage.width() );
		h = std::min( viewport()->height() + contentsY() - y - movedPageToCenter.y() + 200, sizeOfPage.height() );
		QRect hr (x,y,w,h);
		setPixmap( hr );
	}

	if (pagePixmap) {
		QRect hr ( dr );
		hr.moveTopLeft( hr.topLeft() - croppedPage.topLeft() );

		centerPage();
		p->translate( movedPageToCenter.x(), movedPageToCenter.y() );

		p->drawPixmap( dr.topLeft(), *pagePixmap, hr );

		if (mode) {
			RasterOp ro = p->rasterOp();
			p->setRasterOp( Qt::NotXorROP );

			mode->repaint( *p, viewport() );

			p->setRasterOp( ro );
		}

		p->translate( -movedPageToCenter.x(), -movedPageToCenter.y() );
	}


	dr.moveBy( movedPageToCenter.x(), movedPageToCenter.y() );
	if (cx < dr.left())
		p->fillRect( cx, cy, dr.left()-cx, ch, viewport()->eraseColor() );
	if (cx + cw > dr.right())
		p->fillRect( dr.right()+1, cy, cx + cw - dr.right(), ch, viewport()->eraseColor() );
	if (cy < dr.top())
		p->fillRect( cx, cy, cw, dr.top()-cy, viewport()->eraseColor() );
	if (cy + ch > dr.bottom())
		p->fillRect( cx, dr.bottom(), cw, cy + ch - dr.bottom(), viewport()->eraseColor() );
}

void PageViewS::viewportResizeEvent ( QResizeEvent * e ) {
	this->Q_ScrollView::viewportResizeEvent( e );
	repaintContents( true );
}

void PageViewS::needRepaint ( ) {
	repaintContents( false );
}

// ------------------------------------------------------------------ //
// -------------------- events to work in mode ---------------------- //
// ------------------------------------------------------------------ //

void PageViewS::contentsMousePressEvent ( QMouseEvent * e ) {
	if (mode) {
		QMouseEvent ee ( QEvent::MouseButtonPress, e->pos() - movedPageToCenter, e->globalPos(), e->button(), e->state() );
		mode->mousePressEvent( &ee, NULL, viewport() );
	}
	//repaintContents( false );
}

void PageViewS::contentsMouseReleaseEvent ( QMouseEvent * e ) {
	if (mode) {
		QMouseEvent ee ( QEvent::MouseButtonRelease, e->pos() - movedPageToCenter, e->globalPos(), e->button(), e->state() );
		mode->mouseReleaseEvent( &ee, NULL, viewport() );
	}
	//repaintContents( false );
}

void PageViewS::contentsMouseDoubleClickEvent ( QMouseEvent * e ) {
	if (mode) {
		QMouseEvent ee ( QEvent::MouseButtonDblClick, e->pos() - movedPageToCenter, e->globalPos(), e->button(), e->state() );
		mode->mouseDoubleClickEvent( e, NULL, viewport() );
	}
	//repaintContents( false );
}

void PageViewS::contentsMouseMoveEvent ( QMouseEvent * e ) {
	QMouseEvent ee ( QEvent::MouseMove, e->pos() - movedPageToCenter, e->globalPos(), e->button(), e->state() );

	if (mode) {
		mode->mouseMoveEvent( &ee, NULL, viewport() );
	}
	//repaintContents( false );

	// emit change moese cursor on page
	if ( mousePos != ee.pos() ) {
		mousePos = ee.pos();

		double x, y;
		displayParams.convertPixmapPosToPdfPos( mousePos.x(), mousePos.y(), x, y );
		emit changeMousePosition( x, y );
	}
}

void PageViewS::wheelEvent ( QWheelEvent * e ) {
	this->Q_ScrollView::wheelEvent( e );

	if (mode) {
		mode->wheelEvent( e, NULL, viewport() );
	}
	//repaintContents( false );
}

void PageViewS::keyPressEvent ( QKeyEvent * e ) {
	if (mode) {
		mode->keyPressEvent( e, NULL, viewport() );
	}
	//repaintContents( false );
}

void PageViewS::keyReleaseEvent ( QKeyEvent * e ) {
	if (mode) {
		mode->keyReleaseEvent( e, NULL, viewport() );
	}
	//repaintContents( false );
}

void PageViewS::focusInEvent ( QFocusEvent * e ) {
	this->Q_ScrollView::focusInEvent( e );

	if (mode) {
		mode->focusInEvent( e, NULL, viewport() );
	}
	//repaintContents( false );
}

void PageViewS::focusOutEvent ( QFocusEvent * e ) {
	this->Q_ScrollView::focusOutEvent( e );

	if (mode) {
		mode->focusOutEvent( e, NULL, viewport() );
	}
	//repaintContents( false );
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

void PageViewS::initializeWorkOperatorsInMode() {
	if (mode && actualPage) {
		mode->clearWorkOperators();

		// for annotation, ...
		mode->extraInitialize( actualPage, displayParams );
		guiPrintDbg( debug::DBG_DBG, "actualPage "<< (intptr_t) actualPage.get() );

		std::vector< boost::shared_ptr< PdfOperator > > ops;

		std::vector< boost::shared_ptr< CContentStream > > ccs;
		actualPage->getContentStreams( ccs );

		for ( std::vector<boost::shared_ptr<CContentStream> >::iterator ccsIt = ccs.begin(); ccsIt != ccs.end() ; ++ccsIt ) {
			(*ccsIt)->getPdfOperators( ops );

			if (! ops.empty()) {
				std::vector< boost::shared_ptr< PdfOperator > > all_ops;

				PdfOperator::Iterator iter (* ops.begin());
				for ( ; ! iter.isEnd() ; iter.next() )
				{
					all_ops.push_back( iter.getCurrent() );
				}

				mode->addWorkOperators( all_ops );
			}
		}

		// actualize selected operators in mode
		mode->actualizeSelection();
	} else
		if (mode && (! actualPage)) {
			mode->clearWorkOperators();
			mode->clearSelectedOperators();
		}
}

//  ------------------------------------------------------  //
//  --------------------   ZOOM  -------------------------  //
//  ------------------------------------------------------  //
float PageViewS::setZoomFactor ( float set_zoomFactor ) {
	if ((zoomFactor == set_zoomFactor) || (actualPage == NULL))
		return zoomFactor;

	if (set_zoomFactor == 0)
		set_zoomFactor = 0.01;

	if (displayParams.useMediaBox == gFalse) {
		float pom = set_zoomFactor / zoomFactor;
		zoomFactor = set_zoomFactor;
		// TODO
		return zoomFactor;
	}

	zoomFactor = set_zoomFactor;
	displayParams.hDpi = basePpP * zoomFactor * 72;
	displayParams.vDpi = basePpP * zoomFactor * 72;

	showPage( actualPage );

	return zoomFactor;
}

float PageViewS::getZoomFactor ( ) {
	return zoomFactor;
}

//  ---------------------------------------------------------------------  //
//  --------------------  converting positions  -------------------------  //
//  ---------------------------------------------------------------------  //

void PageViewS::convertPixmapPosToPdfPos( double fromX, double fromY, double & toX, double & toY ) {
	if (actualPage)
		displayParams.convertPixmapPosToPdfPos( fromX, fromY, toX, toY );
	else {
		toX = fromX;
		toY = fromY;
	}
}

void PageViewS::convertPdfPosToPixmapPos( double fromX, double fromY, double & toX, double & toY ) {
	if (actualPage)
		displayParams.convertPdfPosToPixmapPos( fromX, fromY, toX, toY );
	else {
		toX = fromX;
		toY = fromY;
	}
}

#undef _splashMakeRGB8

} // namespace gui
