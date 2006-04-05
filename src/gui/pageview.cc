#include "pageview.h"
#include <stdlib.h>
#include <qpixmap.h>
#include "util.h"


PageView::PageView (QWidget *parent) : QLabel(parent) {
	pageImage = NULL;
	mouseRectSelected = NULL;
	rectSelected = NULL;
	oldRectSelected = NULL;
	isPress = false;
	isMoving = false;
	quickSelection = false;
	selectionMode = FillRectRectSelection;
	resizeCoefficientX = 1;
	resizeCoefficientY = 1;

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

	setMouseTracking( true );	// the widget receives mouse move event even if no buttons is pressed

	resizingCursorZone = 2;
}

PageView::~PageView () {
	delete pageImage;
	delete mouseRectSelected;
	delete rectSelected;
	delete oldRectSelected;
}

void PageView::setResizingZone ( unsigned int width ) {
	resizingCursorZone = width;
}

void PageView::setPixmap (const QPixmap & qp) {
	this->QLabel::setPixmap(qp);

	delete pageImage;
	pageImage = new QPixmap(qp);
	
	delete mouseRectSelected;
	delete rectSelected;
	delete oldRectSelected;
	mouseRectSelected = NULL;
	rectSelected = NULL;
	oldRectSelected = NULL;

	isPress = false;
	isMoving = false;
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
		unDrawSelection();
		if (rectSelected != NULL)
			drawRect( rectSelected );
		return true;
	}
	return false;
}

void PageView::unDrawSelection ( void ) {
	if ( isPress ) {
		isPress = false;
		drawRect( mouseRectSelected, true );

		changeSelection( IsSelected );

		if (( rectSelected != NULL ) && (isMoving || isResizing))
			drawRect( rectSelected, true );
	} else {
		changeSelection( IsSelected );

		if ( rectSelected != NULL )
			drawRect( rectSelected, true );
	}

	isMoving = false;
	isResizing = false;
}

void PageView::setSelectedRect ( QRect & newRect ) {
	unDrawSelection();

	if (rectSelected == NULL) {
		rectSelected = new QRect( newRect );
		if (mouseRectSelected == NULL)
			mouseRectSelected = new QRect( );
	} else {
		drawRect( rectSelected, true );
		* rectSelected = newRect;
	}
	drawRect( rectSelected );
}

void PageView::unSelect ( ) {
	unDrawSelection();

	if (rectSelected != NULL) {
		delete rectSelected;
		rectSelected = NULL;
	}
}

int PageView::theNeerestResizingMode ( QRect * r, const QPoint & p ) {
	int resizingMode = none;
	switch (r->width()) {
		case 0:
			resizingMode |= right;
			break;
		case 1:
			if (p.x() - r->left() > 0)
				resizingMode |= right;
			else
				resizingMode |= left;
			break;
		default:
			int cz = (r->width() -2) / 3;
			cz = min( cz, resizingCursorZone );
			if (p.x() - r->left() -cz <= 0)
				resizingMode |= left;
			else if (p.x() - r->right() +cz >= 0)
				resizingMode |= right;
			else
				resizingMode |= left | right;
	}
	switch (r->height()) {
		case 0:
			resizingMode |= bottom;
			break;
		case 1:
			if (p.y() - r->top() > 0)
				resizingMode |= bottom;
			else
				resizingMode |= top;
			break;
		default:
			int cz = (r->height() -2) / 3;
			cz = min( cz, resizingCursorZone );
			if (p.y() - r->top() -cz <= 0)
				resizingMode |= top;
			else if (p.y() - r->bottom() +cz >= 0)
				resizingMode |= bottom;
			else
				resizingMode |= top | bottom;
	}

	switch (resizingMode) {
		case left | top | bottom:
		case right | top | bottom:
			resizingMode ^= top | bottom;
			break;
		case left | right | top:
		case left | right | bottom:
			resizingMode ^= left | right;
			break;
//		default:
	}

	return resizingMode;
}

void PageView::mousePressEvent ( QMouseEvent * e ) {
	if (e->button() == Qt::LeftButton) {
		if (rectSelected != NULL) {
			if (rectSelected->contains( e->pos() )) {
				if (cursorIsSetTo == (left | right | top | bottom) ) {	// is e->pos() inside selected area?
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
			} else {	// create new selected area
				drawRect( rectSelected, true );		// undraw old selected area
				mouseRectSelected->setTopLeft( e->pos() );
				mouseRectSelected->setBottomRight( e->pos()) ;
				isMoving = false;
				resizeCoefficientX = 1;
				resizeCoefficientY = 1;
			}
		} else {
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
	}
}

void PageView::mouseReleaseEvent ( QMouseEvent * e ) {
	switch (e->button()) {
		case Qt::LeftButton :
			if (! isPress ) {
				e->ignore();
				return;
			}
			if (oldRectSelected == NULL) {
				if (rectSelected != NULL) {
					oldRectSelected = new QRect();
					* oldRectSelected = * rectSelected;
				}
			} else {
				* oldRectSelected = * rectSelected;
			}
			if (mouseRectSelected != NULL) {
				drawRect( mouseRectSelected, true );
				changeSelection( IsSelected );
				if (rectSelected == NULL) {
					rectSelected = new QRect( mouseRectSelected->topLeft(), e->pos() );
				} else {
					if ( isMoving ) {
						mouseRectSelected->moveTopLeft( e->pos() - pointInRect );
						drawRect( rectSelected, mouseRectSelected );
					} else if ( isResizing ) {
						mouseRectSelected->setRight( pointInRect.x() * (! resizeCoefficientX) +
										e->pos().x() * resizeCoefficientX );
						mouseRectSelected->setBottom( pointInRect.y() * (! resizeCoefficientY) +
										e->pos().y() * resizeCoefficientY );
						drawRect( rectSelected, true );
					} else
						mouseRectSelected->setBottomRight( e->pos()) ;

					* rectSelected = mouseRectSelected->normalize();
				}
				if ( ! isMoving ) {
					* rectSelected = rectSelected->normalize();
					* rectSelected &= QRect( QPoint( 0,0 ) ,
								QSize( pixmap()->width(), pixmap()->height() ) );
					drawRect( rectSelected );
				}

			}
			if ( isMoving ) {
				printf("emit moving\n");
				emit selectionMovedTo( rectSelected->topLeft() );
			} else if ( isResizing ) {
				printf("emit resizing\n");
				emit selectionResized( * oldRectSelected, * rectSelected );
			} else {
				printf("emit selecting\n");
				emit leftClicked( * rectSelected );
			}
			isPress = false;
			isMoving = false;
			isResizing = false;
			break;
		case Qt::RightButton :
			if ( rectSelected->contains( e->pos() ) ) {
				printf("emit popup\n");
				emit rightClicked( e->globalPos(), * rectSelected );
			} else {
				printf("emit popup\n");
				emit rightClicked( e->globalPos(), QRect( e->pos(), e->pos() ) );
			}
			break;
		default:
			// sent mouseEvent to the parent Widget
			e->ignore();
	}
}

//void PageView::mouseDoubleClickEvent ( QMouseEvent * e ) {}

void PageView::mouseMoveEvent ( QMouseEvent * e ) {
	if ( isPress && (mouseRectSelected != NULL) ) {
		if ( isMoving )
			drawRect( mouseRectSelected, e->pos() - pointInRect );
		else {
			QPoint p( pointInRect.x() * (! resizeCoefficientX) + e->pos().x() * resizeCoefficientX,
				pointInRect.y() * (! resizeCoefficientY) + e->pos().y() * resizeCoefficientY );
			drawRect( mouseRectSelected, p );
		}
		return ;
	}

	if ( (rectSelected != NULL) && (rectSelected->contains( e->pos() )) ) {
		int pomCur =  theNeerestResizingMode( rectSelected, e->pos() );
		if (pomCur != cursorIsSetTo)
			setCursor( QCursor( mappingResizingModeToCursor[ cursorIsSetTo = pomCur ] ));
		return ;
	}

	if (cursorIsSetTo != none )
		setCursor( mappingResizingModeToCursor[ cursorIsSetTo = none ] );
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

	// draw old rectangle
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
	if (! quickSelection) {
		p.fillRect( normRect, Qt::yellow );
		p.drawRect( normRect );
	} else {
		p.drawRect( normRect );
		qr = qr.eor( QRegion(normRect.left()+1, normRect.top()+1, normRect.width()-2, normRect.height()-2) );
	}

	if ( isMoving )
		oldRect->moveTopLeft( toPoint );
	else
		oldRect->setBottomRight( toPoint );

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
