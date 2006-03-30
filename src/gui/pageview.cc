#include "pageview.h"
#include <stdlib.h>
#include <qpixmap.h>
#include "util.h"


PageView::PageView (QWidget *parent) : QLabel(parent) {
//	pageImage = NULL;
	mouseRectSelected = NULL;
	rectSelected = NULL;
	isPress = false;
	isMoving = false;
	quickSelection = false;
	selectionMode = FillRectRectSelection;
}

PageView::~PageView () {
//	delete pageImage;
	delete mouseRectSelected;
	delete rectSelected;
}

void PageView::setPixmap (const QPixmap & qp) {
	this->QLabel::setPixmap(qp);

//	delete pageImage;
//	pageImage = new QPixmap(qp);
	
	delete mouseRectSelected;
	mouseRectSelected = NULL;
	rectSelected = NULL;

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

		if (( rectSelected != NULL ) && (isMoving))
			drawRect( rectSelected, true );
	} else {
		changeSelection( IsSelected );

		if ( rectSelected != NULL )
			drawRect( rectSelected, true );
	}

	isMoving = false;
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

void PageView::mousePressEvent ( QMouseEvent * e ) {
	if (e->button() == Qt::LeftButton) {
		if (rectSelected != NULL) {
			if (rectSelected->contains( e->pos() )) {
				pointInRect.setX( e->x() - rectSelected->left() );	// it is positive (rectSelected is normalized)
				pointInRect.setY( e->y() - rectSelected->top() );
				* mouseRectSelected = * rectSelected;
				isMoving = true;
			} else {
				drawRect( rectSelected, true );
				mouseRectSelected->setTopLeft( e->pos() );
				mouseRectSelected->setBottomRight( e->pos()) ;
				isMoving = false;
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
		if ( isMoving )
			drawRect( mouseRectSelected );
	}
}

void PageView::mouseReleaseEvent ( QMouseEvent * e ) {
	if (! isPress ) {
		e->ignore();
		return;
	}
				printf("release button\n");
	switch (e->button()) {
		case Qt::LeftButton :
			if (mouseRectSelected != NULL) {
				drawRect( mouseRectSelected, true );
				changeSelection( IsSelected );
				if (rectSelected == NULL) {
					rectSelected = new QRect( mouseRectSelected->topLeft(), e->pos() );
				} else {
					if ( isMoving ) {
						mouseRectSelected->moveTopLeft( e->pos() - pointInRect );
						drawRect( rectSelected, mouseRectSelected );
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
			} else {
				printf("emit selecting\n");
				emit leftClicked( * rectSelected );
			}
			isPress = false;
			isMoving = false;
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
		else
			drawRect( mouseRectSelected, e->pos() );
	}
}

// drawing selected area

void PageView::drawRect ( QRect * newRect, bool unDraw ) {
	QPixmap * pm = this->pixmap();

	QPainter p( pm );
	p.setRasterOp( Qt::NotXorROP );
	if (! quickSelection) {
		p.fillRect( newRect->normalize(), Qt::yellow );
	} else {
		p.drawRect( newRect->normalize() );
	}
	
	repaint( newRect->normalize(), false );
}
void PageView::drawRect ( QRect * oldRect, QRect * newRect, enum SelectionSet ss) {
	QPixmap * pm = this->pixmap();

	QPainter p( pm );
	p.setRasterOp( Qt::NotXorROP );

	// draw old rectangle
	QRegion qr( oldRect->normalize() );
	if (! quickSelection) {
		p.fillRect( oldRect->normalize(), Qt::yellow );
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
	if (! quickSelection) {
		p.fillRect( normRect, Qt::yellow );
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
		qr = qr.eor( normRect );
	} else {
		p.drawRect( normRect );
		qr = qr.eor(	QRegion( normRect ) ^
				QRegion( normRect.left()+1, normRect.top()+1, normRect.width()-2, normRect.height()-2 ) );
	}

	repaint( qr, false );
}
