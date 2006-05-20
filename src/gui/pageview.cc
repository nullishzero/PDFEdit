#include "pageview.h"
#include <stdlib.h>
#include <qpixmap.h>
#include "util.h"

namespace gui {

PageView::PageView (QWidget *parent) : QLabel(parent) {
	// initialize privet variable
	pageImage = NULL;
	mouseRectSelected = NULL;
	rectSelected = NULL;
	oldRectSelected = NULL;
	isPress = false;
	isMoving = false;
	isResizing = false;
	quickSelection = false;
	selectionMode = FillRectRectSelection;
	resizeCoefficientX = 1;
	resizeCoefficientY = 1;

	// initialize maping mode to cursor
	for (int i = 0; i<16 ; i++)
		mappingResizingModeToCursor [i] = Qt::ArrowCursor;
	mappingResizingModeToCursor [ left  ] = Qt::SizeHorCursor;
	mappingResizingModeToCursor [ top   ] = Qt::SizeVerCursor;
	mappingResizingModeToCursor [ left | top  ] = Qt::SizeFDiagCursor;
	mappingResizingModeToCursor [ right | top ] = Qt::SizeBDiagCursor;
	mappingResizingModeToCursor [ left | right | top | bottom ] = Qt::SizeAllCursor;

	mappingResizingModeToCursor [ right  ] = mappingResizingModeToCursor [ left ];
	mappingResizingModeToCursor [ bottom ] = mappingResizingModeToCursor [ top ];
	mappingResizingModeToCursor [ right | bottom ] = mappingResizingModeToCursor [ left | top ];
	mappingResizingModeToCursor [ left | bottom  ] = mappingResizingModeToCursor [ right | top ];

	cursorIsSetTo = none;

	// the widget receives mouse move event even if no buttons is pressed
	setMouseTracking( true );

	resizingCursorZone = 2;

	// if something use on page, take focus
	setFocusPolicy( ClickFocus );
}

PageView::~PageView () {
	delete pageImage;
	delete mouseRectSelected;
	delete rectSelected;
	delete oldRectSelected;
}

bool PageView::saveImage ( const QString & file, const char * format, int quality, bool onlySelectedArea) {
	if (! onlySelectedArea) {
		return pageImage->save( file, format, quality );
	} else if (rectSelected) {
		QRect r = (*rectSelected) & pageImage->rect();
		QPixmap * pom = new QPixmap( r.size() );
		
		copyBlt( pom, 0,0, pageImage, r.x(), r.y(), r.width(), r.height() );

		bool r_pom = pom->save( file, format, quality );
		delete pom;
		return r_pom;
	}

	return false;
}

void PageView::setResizingZone ( unsigned int width ) {
	resizingCursorZone = width;
}

void PageView::setPixmap (const QPixmap & qp) {
	this->QLabel::setPixmap(qp);

	delete pageImage;
	pageImage = new QPixmap(qp);
	
	// clear all rectangles
	delete mouseRectSelected;
	delete rectSelected;
	delete oldRectSelected;
	mouseRectSelected = NULL;
	rectSelected = NULL;
	oldRectSelected = NULL;

	// clear all states
	isPress = false;
	isMoving = false;
	isResizing = false;
	quickSelection = false;
}

void PageView::setSelectionMode ( enum SelectionMode m ) {
	selectionMode = m;
}

void PageView::changeSelection ( enum SelectionSet s ) {
	if (s == KeepSelection)
		return;

	switch (selectionMode) {
		case RectSelection:
			quickSelection = true;
			return ;
		case FillRectSelection:
			quickSelection = false;
			return ;
		case RectFillRectSelection:
		case FillRectRectSelection:
			switch (s) {
				case IsSelecting:	// if is changing selection area
					quickSelection = (selectionMode == FillRectRectSelection);
					break;
				case IsSelected:	// if is selected area
					quickSelection = (selectionMode != FillRectRectSelection);
					break;
				default:
					break;
			}
	}
}

bool PageView::escapeSelection () {
	if ( isPress ) {
		// undraw selection
		unDrawSelection();
		if (rectSelected != NULL)
			drawRect( rectSelected );
		// show correct corsor
		mouseMoveEvent( new QMouseEvent( QEvent::MouseMove, mousePos, Qt::NoButton, Qt::NoButton ) );
		return true;
	}
	// if nothing is changing return false
	return false;
}

void PageView::unDrawSelection ( void ) {
	// undraw selection
	if ( isPress ) {
		isPress = false;

		drawRect( mouseRectSelected, true );

		if ( isResizing )
			drawRect( rectSelected );	// see end of mousePressEvent - see better

		changeSelection( IsSelected );
		if (isMoving || isResizing)
			drawRect( rectSelected, true );
	} else {
		changeSelection( IsSelected );
		if ( rectSelected != NULL )
			drawRect( rectSelected, true );
	}

	// clear states
	isMoving = false;
	isResizing = false;
}

void PageView::setSelectedRect ( QRect & newRect ) {
	// undraw selection
	unDrawSelection();

	// set new selection
	if (rectSelected == NULL) {
		rectSelected = new QRect( newRect );
		if (mouseRectSelected == NULL)
			mouseRectSelected = new QRect( newRect );
	} else {
		// undraw 
		drawRect( rectSelected, true );
		* rectSelected = newRect;
	}
	drawRect( rectSelected );

	// set correct cursor
	mouseMoveEvent( new QMouseEvent( QEvent::MouseMove, mousePos, Qt::NoButton, Qt::NoButton ) );
}

void PageView::unSelect ( ) {
	// undraw selection
	unDrawSelection();

	// clear old selected rectangle
	if (oldRectSelected != NULL) {
		delete oldRectSelected;
		oldRectSelected = NULL;
	}
	// clear selected rectangle
	if (rectSelected != NULL) {
		delete rectSelected;
		rectSelected = NULL;
	}
	// set correct cursor
	setCursor( QCursor( mappingResizingModeToCursor[ cursorIsSetTo = none ] ));
}

int PageView::theNeerestResizingMode ( QRect * r, const QPoint & p ) {
	int resizingMode = none;
	switch (r->width()) {
		case 0:		// if width of rectangle is 0 then  every set resizing mode to right resizing
			resizingMode |= right;
			break;
		case 1:		// if width of rectangle is 1 then  set resizing mode to the nearest site of rectangle
			if (p.x() - r->left() > 0)
				resizingMode |= right;
			else
				resizingMode |= left;
			break;
		default:	// if width of rectangle is greate then 1 then
			// calculate zone for resizing
			int cz = (r->width() -2) / 3;
			cz = MIN( cz, resizingCursorZone );

			// set resizing mode to the nearest site of (rectangle without zone for resizing)
			if (p.x() - r->left() -cz <= 0)
				resizingMode |= left;
			else if (p.x() - r->right() +cz >= 0)
				resizingMode |= right;
			else	// if point is inside in (rectangle without zone for resizing) then set both site resizing mode
				resizingMode |= left | right;
	}
	switch (r->height()) {
		case 0:		// if height of rectangle is 0 then  every set resizing mode to bottom resizing
			resizingMode |= bottom;
			break;
		case 1:		// if height of rectangle is 1 then  set resizing mode to the nearest site of rectangle
			if (p.y() - r->top() > 0)
				resizingMode |= bottom;
			else
				resizingMode |= top;
			break;
		default:	// if height of rectangle is greate then 1 then
			// calculate zone for resizing
			int cz = (r->height() -2) / 3;
			cz = MIN( cz, resizingCursorZone );

			// set resizing mode to the nearest site of (rectangle without zone for resizing)
			if (p.y() - r->top() -cz <= 0)
				resizingMode |= top;
			else if (p.y() - r->bottom() +cz >= 0)
				resizingMode |= bottom;
			else	// if point is inside in (rectangle without zone for resizing) then set both site resizing mode
				resizingMode |= top | bottom;
	}

	// if isn't set clear resizing mode then set clear it (resizing mode to all side is moving mode)
	switch (resizingMode) {
		case left | top | bottom:
		case right | top | bottom:
			resizingMode ^= top | bottom;
			break;
		case left | right | top:
		case left | right | bottom:
			resizingMode ^= left | right;
			break;
	}

	return resizingMode;
}

void PageView::mousePressEvent ( QMouseEvent * e ) {
	if ( isPress )
		return ;

	if (e->button() == Qt::LeftButton) {
		if (rectSelected != NULL) {
			// some rectangle was selected
			// what do it ? (resizing, moving or creating new selection)
			if (rectSelected->contains( e->pos() )) {
				if (cursorIsSetTo == (left | right | top | bottom) ) {	// is e->pos() inside selected area?
					// moving mode
					pointInRect.setX( e->x() - rectSelected->left() );
					pointInRect.setY( e->y() - rectSelected->top() );
									// it is positive (rectSelected is normalized)
					* mouseRectSelected = * rectSelected;
					isMoving = true;
				} else {	// e->pos() is on the edge of selected area
					// resizing selected area
					if (cursorIsSetTo & left) {		// resizing to left side
						mouseRectSelected->setLeft( rectSelected->right() );
						mouseRectSelected->setRight( rectSelected->left() );
					} else {			// resizing to right side or only vertical
						mouseRectSelected->setLeft( rectSelected->left() );
						mouseRectSelected->setRight( rectSelected->right() );
					}

					if ((cursorIsSetTo & (left | right)) == none)
						resizeCoefficientX = 0;		// resizing only vertikal
					else
						resizeCoefficientX = 1;

					if (cursorIsSetTo & top) {		// resizing to top side
						mouseRectSelected->setTop( rectSelected->bottom() );
						mouseRectSelected->setBottom( rectSelected->top() );
					} else {			// resizing to bottom side or only horizontal
						mouseRectSelected->setTop( rectSelected->top() );
						mouseRectSelected->setBottom( rectSelected->bottom() );
					}

					if ((cursorIsSetTo & (top | bottom)) == none)
						resizeCoefficientY = 0;		// risizing only horizontal
					else
						resizeCoefficientY = 1;

					pointInRect = mouseRectSelected->bottomRight();
					isResizing = true;
				}
			} else {
				// create new selected area
				drawRect( rectSelected, true );		// undraw old selected area
				mouseRectSelected->setTopLeft( e->pos() );
				mouseRectSelected->setBottomRight( e->pos()) ;
				isMoving = false;
				resizeCoefficientX = 1;
				resizeCoefficientY = 1;
			}
		} else {
			// nothing rectangle is selected
			// create new selected area
			if (mouseRectSelected == NULL)
				mouseRectSelected = new QRect( );
			mouseRectSelected->setTopLeft( e->pos() );
			mouseRectSelected->setBottomRight( e->pos()) ;
			isMoving = false;
		}
		isPress = true;
		changeSelection( IsSelecting );
		if ( isMoving || isResizing )
			drawRect( mouseRectSelected );

		if ( isResizing )		// resizing - undraw edge of selected area - see better
			drawRect( rectSelected, true );
	}
}

void PageView::mouseReleaseEvent ( QMouseEvent * e ) {
	switch (e->button()) {
		case Qt::LeftButton :
			if (! isPress ) {
				// if nothing mousepress and some release
				return;
			}
			// set old rectangle for step back
			if (oldRectSelected == NULL) {
				if (rectSelected != NULL) {
					oldRectSelected = new QRect();
					* oldRectSelected = * rectSelected;
				}
			} else {
				* oldRectSelected = * rectSelected;
			}
			// if  isPress == true  then  mouseRectSelected is not NULLL
//			if (mouseRectSelected != NULL) {
				// undraw selected area
				drawRect( mouseRectSelected, true );
				if ( isResizing )
					drawRect( rectSelected );	// see end of mousePressEvent - see better

				changeSelection( IsSelected );
				// save new selection
				if (rectSelected == NULL) {
					rectSelected = new QRect( mouseRectSelected->topLeft(), e->pos() );
				} else {
					if ( isMoving ) {
						// apply last change and redraw selection
						mouseRectSelected->moveTopLeft( e->pos() - pointInRect );
						drawRect( rectSelected, mouseRectSelected );
					} else if ( isResizing ) {
						// apply last change
						mouseRectSelected->setRight( pointInRect.x() * (! resizeCoefficientX) +
										e->pos().x() * resizeCoefficientX );
						mouseRectSelected->setBottom( pointInRect.y() * (! resizeCoefficientY) +
										e->pos().y() * resizeCoefficientY );
						// undraw old (now) selection 
						drawRect( rectSelected, true );
					} else
						// apply last change
						mouseRectSelected->setBottomRight( e->pos()) ;

					// set new selection (must be normalized)
					* rectSelected = mouseRectSelected->normalize();
				}
				if ( ! isMoving ) {
					// set new selection (must be normalized)
					* rectSelected = rectSelected->normalize();
					// cut new or resized selection to page
					* rectSelected &= QRect( QPoint( 0,0 ) ,
								QSize( pixmap()->width(), pixmap()->height() ) );
					// draw new selection
					drawRect( rectSelected );
				}

//			}
			// emit correct signal
			if ( isMoving ) {
				if (*rectSelected != * oldRectSelected) {	// no emit if no change
					emit selectionMovedTo( rectSelected->topLeft() );
				}
			} else if ( isResizing ) {
				if (* rectSelected != * oldRectSelected) {	// no emit if no change
					emit selectionResized( * oldRectSelected, * rectSelected );
				}
			} else {
				emit leftClicked( * rectSelected );
			}
			// clear states
			isPress = false;
			isMoving = false;
			isResizing = false;
			break;
		case Qt::RightButton :
			if ( isPress )
				break;		// left button is press and not released

			// emit signal with correct parameters
			if ( (rectSelected != NULL) && rectSelected->contains( e->pos() ) ) {
				emit rightClicked( e->pos(), rectSelected );
			} else {
				emit rightClicked( e->pos(), NULL );
			}
			break;
		default:
			printf("ignore ");
	}
}

//void PageView::mouseDoubleClickEvent ( QMouseEvent * e ) {}

void PageView::mouseMoveEvent ( QMouseEvent * e ) {
	if ( isPress ) {
		// redraw selected rectangle after change
		if ( isMoving )
			drawRect( mouseRectSelected, e->pos() - pointInRect );
		else {
			QPoint p( pointInRect.x() * (! resizeCoefficientX) + e->pos().x() * resizeCoefficientX,
				pointInRect.y() * (! resizeCoefficientY) + e->pos().y() * resizeCoefficientY );
			drawRect( mouseRectSelected, p );
		}
	} else {	// if is not set changing mode then:

		// change cursor to correct
		if ( (rectSelected != NULL) && (rectSelected->contains( e->pos() )) ) {
			int pomCur =  theNeerestResizingMode( rectSelected, e->pos() );
			if (pomCur != cursorIsSetTo)
				setCursor( QCursor( mappingResizingModeToCursor[ cursorIsSetTo = pomCur ] ));
		} else {

			if (cursorIsSetTo != none )
				setCursor( mappingResizingModeToCursor[ cursorIsSetTo = none ] );
		}
	}

	// emit change moese cursor on page
	if ( mousePos != e->pos() ) {
		mousePos = e->pos();
		emit changeMousePosition( mousePos );
	}
}

// drawing selected area

void PageView::drawRect ( QRect * newRect, bool unDraw ) {
	QPixmap * pm = this->pixmap();

	QPainter p( pm );
	p.setRasterOp( Qt::NotXorROP );
	p.setPen( QColor(85,170,170) );
	if (! quickSelection) {
		p.fillRect( newRect->normalize(), Qt::yellow );
		p.drawRect( newRect->normalize() );
	} else {
		p.drawRect( newRect->normalize() );
	}
	
	repaint( newRect->normalize(), false );
}
void PageView::drawRect ( QRect * oldRect, QRect * newRect, enum SelectionSet ss) {
	QPixmap * pm = this->pixmap();

	QPainter p( pm );
	p.setRasterOp( Qt::NotXorROP );
	p.setPen( QColor(85,170,170) );

	// undraw old rectangle
	QRegion qr( oldRect->normalize() );
	if (! quickSelection) {
		p.fillRect( oldRect->normalize(), Qt::yellow );
		p.drawRect( oldRect->normalize() );
	} else {
		QRect normRect;
		normRect = oldRect->normalize();
		p.drawRect( normRect );
		qr = qr.eor( QRegion( normRect.left()+1, normRect.top()+1, normRect.width()-2, normRect.height()-2 ) );
	}

	// change drawing mode for new rectangle
	changeSelection( ss );

	// draw new rectangle
	if (! quickSelection) {
		p.fillRect( newRect->normalize(), Qt::yellow );
		p.drawRect( newRect->normalize() );
		qr = qr.unite( newRect->normalize() );
	} else {
		QRect normRect;
		normRect = newRect->normalize();
		p.drawRect( normRect );
		qr = qr.unite(	QRegion( normRect ) ^
				QRegion( normRect.left()+1, normRect.top()+1, normRect.width()-2, normRect.height()-2 ) );
	}

	repaint( qr, false );
}
void PageView::drawRect ( QRect * oldRect, const QPoint & toPoint ) {
	// !!! oldRect will be changed
	QRect normRect;
	normRect = oldRect->normalize();

	QRegion qr( normRect );

	QPixmap * pm = this->pixmap();

	QPainter p( pm );
	p.setRasterOp( Qt::NotXorROP );
	p.setPen( QColor(85,170,170) );
	// undraw old rectangle
	if (! quickSelection) {
		p.fillRect( normRect, Qt::yellow );
		p.drawRect( normRect );
	} else {
		p.drawRect( normRect );
		qr = qr.eor( QRegion(normRect.left()+1, normRect.top()+1, normRect.width()-2, normRect.height()-2) );
	}

	// change selected rectangle
	if ( isMoving )
		oldRect->moveTopLeft( toPoint );
	else
		oldRect->setBottomRight( toPoint );

	// draw new rectangle
	normRect = oldRect->normalize();
	if (! quickSelection) {
		p.fillRect( normRect, Qt::yellow );
		p.drawRect( normRect );
		qr = qr.eor( normRect );
	} else {
		p.drawRect( normRect );
		qr = qr.eor(	QRegion( normRect ) ^
				QRegion( normRect.left()+1, normRect.top()+1, normRect.width()-2, normRect.height()-2 ) );
	}

	repaint( qr, false );
}

} // namespace gui
