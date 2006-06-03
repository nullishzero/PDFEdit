#include "pageview.h"
#include <stdlib.h>
#include <qpixmap.h>
#include "rect2Darray.h"
#include "util.h"
#include "debug.h"


namespace gui {

PageView::PageView (QWidget *parent) : QLabel(parent) {
	// initialize privet variable
//	pageImage = NULL;
	mouseRectSelected = NULL;

	selectedRegion = NULL;
	oldSelectedRegion = NULL;
	mouseSelectedRegion = NULL;

	isPress = false;
	isMoving = false;
	isResizing = false;
	quickSelection = false;
	selectionMode = SelectAllObjects;
	selectionAllMode = FillRectRectSelection;
	oldSelectionAllMode = FillRectRectSelection;
	resizeCoefficientX = 1;
	resizeCoefficientY = 1;

	selectedObjects.clear();
	selectedObjects.setAutoDelete( true );
	regionOfAllObjects = QRegion();
	arrayOfBBoxes = NULL;
	firstSelectedObject = NULL;
	lastSelectedObject = NULL;
	nearestObjectToClick = NULL;

	isRepaintingBlocked = false;
	regionForRepaint = QRegion();

	// initialize maping mode to cursor
	setMappingCursor();

	cursorIsSetTo = none;

	// the widget receives mouse move event even if no buttons is pressed
	setMouseTracking( true );

	resizingCursorZone = 2;

	// if something use on page, take focus
	setFocusPolicy( ClickFocus );
}

PageView::~PageView () {
//	delete pageImage;
	delete mouseRectSelected;

	delete selectedRegion;
	delete oldSelectedRegion;
	delete mouseSelectedRegion;

	delete arrayOfBBoxes;
	selectedObjects.clear();
}

void PageView::setMappingCursor() {

	for (int i = 0; i<17 ; i++)
		mappingResizingModeToCursor [i] = Qt::ArrowCursor;

	switch (selectionMode) {
		case SelectText:
				mappingResizingModeToCursor [ onUnselectedObject ] = Qt::IbeamCursor;
		case SelectAllObjects: {
			mappingResizingModeToCursor [ left  ] = Qt::SizeHorCursor;
			mappingResizingModeToCursor [ top   ] = Qt::SizeVerCursor;
			mappingResizingModeToCursor [ left | top  ] = Qt::SizeFDiagCursor;
			mappingResizingModeToCursor [ right | top ] = Qt::SizeBDiagCursor;
			mappingResizingModeToCursor [ left | right | top | bottom ] = Qt::SizeAllCursor;
			break;
		}
		case SelectRect: {
			for (int i = 0; i<17 ; i++)
				mappingResizingModeToCursor [i] = Qt::CrossCursor;
			mappingResizingModeToCursor [ left  ] = Qt::SizeHorCursor;
			mappingResizingModeToCursor [ top   ] = Qt::SizeVerCursor;
			mappingResizingModeToCursor [ left | top  ] = Qt::SizeFDiagCursor;
			mappingResizingModeToCursor [ right | top ] = Qt::SizeBDiagCursor;
			mappingResizingModeToCursor [ left | right | top | bottom ] = Qt::SizeAllCursor;
			break;
		}
		default:
			break;
	}

	mappingResizingModeToCursor [ right  ] = mappingResizingModeToCursor [ left ];
	mappingResizingModeToCursor [ bottom ] = mappingResizingModeToCursor [ top ];
	mappingResizingModeToCursor [ right | bottom ] = mappingResizingModeToCursor [ left | top ];
	mappingResizingModeToCursor [ left | bottom  ] = mappingResizingModeToCursor [ right | top ];

	// set correct cursor
	cursorIsSetTo = -1;
	mouseMoveEvent( new QMouseEvent( QEvent::MouseMove, mousePos, Qt::NoButton, Qt::NoButton ) );
}

bool PageView::saveImage ( const QString & file, const char * format, int quality, bool onlySelectedArea) {
	if (! onlySelectedArea) {
		return pixmap()->save( file, format, quality );
	}

	QRect r;
	switch (selectionMode) {
		case SelectAllObjects:
		case SelectText:
		case SelectRect:
			if (selectedRegion == NULL) {
				guiPrintDbg ( debug::DBG_INFO, tr("Selected area is not set!") );
				return false;
			}
			r = selectedRegion->boundingRect() & pixmap()->rect();
			break;
		default:
			guiPrintDbg ( debug::DBG_INFO, tr("Selection mode is set to nonsense!") );
			return false;
	}

	QPixmap * pom = new QPixmap( r.size() );

	copyBlt( pom, 0,0, pixmap(), r.x(), r.y(), r.width(), r.height() );

	bool r_pom = pom->save( file, format, quality );
	delete pom;

	return r_pom;
}

void PageView::setResizingZone ( unsigned int width ) {
	resizingCursorZone = width;
}

void PageView::setPixmap (const QPixmap & qp) {
	this->QLabel::setPixmap(qp);

//	delete pageImage;
//	pageImage = new QPixmap(qp);
	
	// clear all rectangles
	delete mouseRectSelected;
	mouseRectSelected = NULL;

	// clear all regions
	delete selectedRegion;
	delete oldSelectedRegion;
	delete mouseSelectedRegion;
	selectedRegion = NULL;
	oldSelectedRegion = NULL;
	mouseSelectedRegion = NULL;

	delete arrayOfBBoxes;
	arrayOfBBoxes = NULL;

	// clear all states
	isPress = false;
	isMoving = false;
	isResizing = false;
	quickSelection = false;
}

bool PageView::setSelectionMode ( enum SelectionMode m ) {
	if ((m == SelectText) && (arrayOfBBoxes == NULL))
		return false;

	if (m == SelectAllObjects)
		selectionAllMode = oldSelectionAllMode;
	else {
		if (selectionMode == SelectAllObjects) {
			oldSelectionAllMode = selectionAllMode;
		}
		if (m == SelectText) {
			arrayOfBBoxes->initAllBBoxPtr();
			setSelectionAllMode( FillRectSelection );
		} else
			setSelectionAllMode( RectSelection );
	}

	selectionMode = m;
	setMappingCursor();
	return true;
}
void PageView::setSelectionAllMode ( enum SelectionAllMode m ) {
	selectionAllMode = m;
}

void PageView::changeSelection ( enum SelectionSet s ) {
	if (s == KeepSelection)
		return;

	switch (selectionAllMode) {
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
					quickSelection = (selectionAllMode == FillRectRectSelection);
					break;
				case IsSelected:	// if is selected area
					quickSelection = (selectionAllMode != FillRectRectSelection);
					break;
				default:
					break;
			}
	}
}

bool PageView::escapeSelection () {
	if ( isPress ) {
		blockRepaint( true );
		// undraw selection
		unDrawSelection();
		if (selectedRegion != NULL)
			drawRect( selectedRegion );
		blockRepaint( false );
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

//		drawRect( mouseRectSelected, true );
		QRegion h_reg;
		if (mouseSelectedRegion != NULL) {
			h_reg = *mouseSelectedRegion;
			delete mouseSelectedRegion;
			mouseSelectedRegion = NULL;
		}
		if (selectionMode != SelectText) {
			if (mouseRectSelected != NULL) {
				h_reg = h_reg.unite( QRegion(mouseRectSelected->normalize()) );
//				drawRect( & h_reg, true );
				delete mouseRectSelected;
				mouseRectSelected = NULL;
			}
			drawRect( & h_reg, true );

			if ( isResizing )
				drawRect( selectedRegion );	// see end of mousePressEvent - see better
		} else {
			if (isResizing)
				h_reg = * selectedRegion;

			if (mouseRectSelected != NULL)
				drawRect( & h_reg, mouseRectSelected, NULL );
			else
				drawRect( & h_reg, true );
		}

		changeSelection( IsSelected );
		if (isMoving || (isResizing && (selectionMode != SelectText)))
			drawRect( selectedRegion, true );
	} else {
		changeSelection( IsSelected );
		if ( selectedRegion != NULL )
			drawRect( selectedRegion, true );
	}

	// clear states
	isMoving = false;
	isResizing = false;
}

void PageView::addSelectedRegion ( QRect & newRect, const void * ptr_object ) {
	QRegion h_reg ( newRect.normalize() );
	addSelectedRegion( h_reg, ptr_object );
}
void PageView::addSelectedRegion ( QRegion & newReg, const void * ptr_object ) {
	blockRepaint( true );
	// undraw selection
	unDrawSelection();

	// set new selection
	if (selectedRegion == NULL) {
		selectedRegion = new QRegion( newReg );
		if (mouseRectSelected != NULL) {
			delete mouseRectSelected;
			mouseRectSelected = NULL;
		}
	} else {
		// undraw 
	//	drawRect( selectedRegion, true );
		(* selectedRegion) += newReg;
	}
	drawRect( selectedRegion );

	blockRepaint( false );

	// set correct cursor
	mouseMoveEvent( new QMouseEvent( QEvent::MouseMove, mousePos, Qt::NoButton, Qt::NoButton ) );

	// append object to selectedObjects
	selectedObjects.append( new BBoxOfObjectOnPage( newReg.boundingRect(), ptr_object ) );
}

void PageView::setSelectedRegion ( QRect & newRect, const void * ptr_object ) {
	QRegion h_reg ( newRect.normalize() );
	setSelectedRegion( h_reg, ptr_object );
}

void PageView::setSelectedRegion ( QRegion & newReg, const void * ptr_object ) {
	// undraw selection
	unDrawSelection();

	// set new selection
	if (selectedRegion == NULL) {
		selectedRegion = new QRegion( newReg );
		if (mouseRectSelected != NULL) {
			delete mouseRectSelected;
			mouseRectSelected = NULL;
		}
	} else {
		// undraw 
	//	drawRect( selectedRegion, true );
		* selectedRegion = newReg;
	}
	drawRect( selectedRegion );

	// set correct cursor
	mouseMoveEvent( new QMouseEvent( QEvent::MouseMove, mousePos, Qt::NoButton, Qt::NoButton ) );

	// set selectedObjects
	selectedObjects.clear();
	selectedObjects.append( new BBoxOfObjectOnPage( newReg.boundingRect(), ptr_object ) );
}

void PageView::unSelect ( ) {
	blockRepaint( false );
	// undraw selection
	unDrawSelection();

	// clear old selected region
	if (oldSelectedRegion != NULL) {
		delete oldSelectedRegion;
		oldSelectedRegion = NULL;
	}
	// clear selected region
	if (selectedRegion != NULL) {
		delete selectedRegion;
		selectedRegion = NULL;
	}
	// clear temporary selected region
	if (mouseSelectedRegion != NULL) {
		delete mouseSelectedRegion;
		mouseSelectedRegion = NULL;
	}
	// clear temporary selected rectangles
	if (mouseRectSelected != NULL) {
		delete mouseRectSelected;
		mouseRectSelected = NULL;
	}
	// 
	selectedObjects.clear();

	// set correct cursor
	setCursor( QCursor( mappingResizingModeToCursor[ cursorIsSetTo = none ] ));
}

int PageView::theNeerestResizingMode ( QRegion * r, const QPoint & p ) {
	int resizingMode = none;
	QRect h_br ( r->boundingRect().normalize() );

	switch (h_br.width()) {
		case 0:		// if width of rectangle is 0 then  every set resizing mode to right resizing
			resizingMode |= right;
			break;
		case 1:		// if width of rectangle is 1 then  set resizing mode to the nearest site of rectangle
			if (p.x() - h_br.left() > 0)
				resizingMode |= right;
			else
				resizingMode |= left;
			break;
		default:	// if width of rectangle is greate then 1 then
			// calculate zone for resizing
			int cz = (h_br.width() -2) / 3;
			cz = MIN( cz, resizingCursorZone );

			// set resizing mode to the nearest site of (rectangle without zone for resizing)
			if (p.x() - h_br.left() -cz <= 0)
				resizingMode |= left;
			else if (p.x() - h_br.right() +cz >= 0)
				resizingMode |= right;
			else	// if point is inside in (rectangle without zone for resizing) then set both site resizing mode
				resizingMode |= left | right;
	}
	switch (h_br.height()) {
		case 0:		// if height of rectangle is 0 then  every set resizing mode to bottom resizing
			resizingMode |= bottom;
			break;
		case 1:		// if height of rectangle is 1 then  set resizing mode to the nearest site of rectangle
			if (p.y() - h_br.top() > 0)
				resizingMode |= bottom;
			else
				resizingMode |= top;
			break;
		default:	// if height of rectangle is greate then 1 then
			// calculate zone for resizing
			int cz = (h_br.height() -2) / 3;
			cz = MIN( cz, resizingCursorZone );

			// set resizing mode to the nearest site of (rectangle without zone for resizing)
			if (p.y() - h_br.top() -cz <= 0)
				resizingMode |= top;
			else if (p.y() - h_br.bottom() +cz >= 0)
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
		case left | right | top | bottom:
			if (! r->contains( p ))
				resizingMode = none;
	}

	return resizingMode;
}

BBoxOfObjectOnPage * PageView::getNearestObject( QPoint & point ) {
	if ((arrayOfBBoxes == NULL) || (arrayOfBBoxes->isEmpty()))
		return NULL;

	RectArray * prevLine = NULL;
	RectArray * line = arrayOfBBoxes->first();
	for ( ; line ; prevLine = line, line = arrayOfBBoxes->next() ) {
		if (line->getMinY() > point.y())
			break;
	}

	if (prevLine == NULL)
		prevLine = line;

	BBoxOfObjectOnPage * nearest = prevLine->first();
	BBoxOfObjectOnPage * prevNearest = nearest;
	for ( ; nearest ; prevNearest = nearest, nearest = prevLine->next() ) {
		if (nearest->left() > point.x())
			break;
		if (nearest->right() > point.x()) {
			prevNearest = nearest;
			break;
		}
	}

	return prevNearest;
}

void PageView::mousePressEvent ( QMouseEvent * e ) {
	if ( isPress )
		return ;

	if (e->button() == Qt::LeftButton) {
		mousePosInLeftClick = e->pos();
		blockRepaint( true );

		delete mouseSelectedRegion;
		mouseSelectedRegion = NULL;

		if (selectedRegion != NULL) {
			// some rectangle was selected
			// what do it ? (resizing, moving or creating new selection)
			if ((cursorIsSetTo != none) && (cursorIsSetTo != onUnselectedObject)) {
				if (cursorIsSetTo == (left | right | top | bottom) ) {	// is e->pos() inside selected area?
					// moving mode
					pointInRect.setX( e->x() - selectedRegion->boundingRect().normalize().left() );
					pointInRect.setY( e->y() - selectedRegion->boundingRect().normalize().top() );
									// pointInRect is positive point (x and y are positive)
					delete mouseRectSelected;
					mouseRectSelected = NULL;
					mouseSelectedRegion = new QRegion( * selectedRegion );
					isMoving = true;
				} else {	// e->pos() is on the edge of selected area
					// resizing selected area
					QRect rectSelected (selectedRegion->boundingRect().normalize());
					if (mouseRectSelected == NULL)
						mouseRectSelected = new QRect();

					if (cursorIsSetTo & left) {		// resizing to left side
						mouseRectSelected->setLeft( rectSelected.right() );
						mouseRectSelected->setRight( rectSelected.left() );
					} else {			// resizing to right side or only vertical
						mouseRectSelected->setLeft( rectSelected.left() );
						mouseRectSelected->setRight( rectSelected.right() );
					}

					if ((cursorIsSetTo & (left | right)) == none)
						resizeCoefficientX = 0;		// resizing only vertikal
					else
						resizeCoefficientX = 1;

					if (cursorIsSetTo & top) {		// resizing to top side
						mouseRectSelected->setTop( rectSelected.bottom() );
						mouseRectSelected->setBottom( rectSelected.top() );
					} else {			// resizing to bottom side or only horizontal
						mouseRectSelected->setTop( rectSelected.top() );
						mouseRectSelected->setBottom( rectSelected.bottom() );
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
				drawRect( selectedRegion, true );		// undraw old selected area
				switch (selectionMode) {
					case SelectRect:
					case SelectAllObjects: {
						if (mouseRectSelected == NULL)
							mouseRectSelected = new QRect();
						mouseRectSelected->setTopLeft( e->pos() );
						mouseRectSelected->setBottomRight( e->pos()) ;
						break;
					}
					case SelectText: {
						delete mouseRectSelected;
						mouseRectSelected = NULL;

						nearestObjectToClick = getNearestObject( mousePosInLeftClick );
						firstSelectedObject = nearestObjectToClick;
						for ( ; firstSelectedObject ; firstSelectedObject = firstSelectedObject->getRightBBox() ) {
							if (firstSelectedObject->left() > mousePosInLeftClick.x()) {
								firstSelectedObject = NULL;
								break;
							}
							if (firstSelectedObject->contains( mousePosInLeftClick ) )
								break;
						}
						lastSelectedObject = firstSelectedObject;
						if (firstSelectedObject) {
							mouseSelectedRegion = new QRegion( *firstSelectedObject );
						}
						break;
					}
					default:
						break;
				}
				isMoving = false;
				resizeCoefficientX = 1;
				resizeCoefficientY = 1;
			}
		} else {
			// nothing region is selected
			// create new selected area
			if (selectionMode != SelectText) {
				if (mouseRectSelected == NULL)
					mouseRectSelected = new QRect();
				mouseRectSelected->setTopLeft( e->pos() );
				mouseRectSelected->setBottomRight( e->pos()) ;
				isMoving = false;
			} else {
				nearestObjectToClick = getNearestObject( mousePosInLeftClick );
				firstSelectedObject = nearestObjectToClick;
				for ( ; firstSelectedObject ; firstSelectedObject = firstSelectedObject->getRightBBox() ) {
					if (firstSelectedObject->left() > mousePosInLeftClick.x()) {
						firstSelectedObject = NULL;
						break;
					}
					if (firstSelectedObject->contains( mousePosInLeftClick ) )
						break;
				}
				lastSelectedObject = firstSelectedObject;
				if (firstSelectedObject) {
					if (mouseSelectedRegion)
						* mouseSelectedRegion = QRegion( * firstSelectedObject );
					else
						mouseSelectedRegion = new QRegion( *firstSelectedObject );
				} else {
					delete mouseSelectedRegion;
					mouseSelectedRegion = NULL;
				}
			}
		}
		isPress = true;
		changeSelection( IsSelecting );
		if ( isMoving || (selectionMode == SelectText))
			drawRect( mouseSelectedRegion );		// mouseSelectedRegion is not NULL

//		if ( isResizing )		// resizing - undraw edge of selected area - see better
//			drawRect( , true );	// selectedRegion is not NULL
		blockRepaint( false );
	}
}

void PageView::mouseReleaseEvent ( QMouseEvent * e ) {
	QRegion h_reg;
	switch (e->button()) {
		case Qt::LeftButton :
			if (! isPress ) {
				// nothing mousepress and some release
				return;
			}
			// set old region for step back
			if (oldSelectedRegion == NULL) {
				if (selectedRegion != NULL) {
					oldSelectedRegion = new QRegion(*selectedRegion);
				}
			} else {
				* oldSelectedRegion = * selectedRegion;
			}

			// undraw selected area
			if (mouseSelectedRegion != NULL) {
				h_reg = *mouseSelectedRegion;
			}
			if (selectionMode != SelectText) {
				if (mouseRectSelected != NULL)
					h_reg = h_reg.unite( QRegion(mouseRectSelected->normalize()) );
				drawRect( & h_reg, true );

				if ( isResizing )
					drawRect( selectedRegion );	// see end of mousePressEvent - see better
			} else {
				if (isResizing)
					h_reg = * selectedRegion;

				if (mouseRectSelected != NULL)
					drawRect( & h_reg, mouseRectSelected, NULL );
				else
					drawRect( & h_reg, true );
			}

			changeSelection( IsSelected );
			// save new selection
			if (mouseSelectedRegion != NULL)
				h_reg = *mouseSelectedRegion;
			else
				h_reg = QRegion();

			if (selectedRegion == NULL) {
				selectedRegion = new QRegion();
				if (mouseRectSelected)
					* selectedRegion |= QRegion( QRect(mouseRectSelected->topLeft(), e->pos()).normalize() );
				* selectedRegion |= h_reg;
			} else {
				if ( isMoving ) {
					// apply last change and redraw selection
					QPoint h_p (mouseSelectedRegion->boundingRect().normalize().topLeft() - (e->pos() - pointInRect));
					mouseSelectedRegion->translate( h_p.x(), h_p.y() );
					drawRect( & h_reg, mouseSelectedRegion );
					drawRect( selectedRegion, true );
					* selectedRegion = * mouseSelectedRegion;
				} else if ( isResizing ) {
					// apply last change
					mouseRectSelected->setRight( pointInRect.x() * (! resizeCoefficientX) +
									e->pos().x() * resizeCoefficientX );
					mouseRectSelected->setBottom( pointInRect.y() * (! resizeCoefficientY) +
									e->pos().y() * resizeCoefficientY );
					if (selectionMode != SelectText) {
						// undraw old (now) selection 
						drawRect( selectedRegion, true );
					}
				} else {
					// set new selection
					* selectedRegion = QRegion();
					if (mouseRectSelected) {
						mouseRectSelected->setBottomRight( e->pos());
						* selectedRegion |= QRegion(mouseRectSelected->normalize());
					}
					* selectedRegion |= h_reg;
				}
			}
			if (selectionMode == SelectRect) {
				selectedObjects.clear();
				if (mouseRectSelected)
					selectedObjects.append( new BBoxOfObjectOnPage( * mouseRectSelected, NULL ) );
				else
					selectedObjects.append( new BBoxOfObjectOnPage( selectedRegion->boundingRect().normalize(), NULL ) );
			}

			// draw new selection
			drawRect( selectedRegion );

			// emit correct signal
			if ( isMoving ) {
				if (*selectedRegion != *oldSelectedRegion) {	// no emit if no change
					emit selectionMoved( selectedRegion->boundingRect().normalize().topLeft() -
											oldSelectedRegion->boundingRect().normalize().topLeft(),
										selectedObjects );
				}
			} else if ( isResizing ) {
				if (*selectedRegion != *oldSelectedRegion) {	// no emit if no change
					emit selectionResized( selectedRegion->boundingRect().normalize(), mouseRectSelected->normalize(), selectedObjects );
				}
			} else {
				if (selectionMode != SelectText) {
					if (selectionMode == SelectRect)
						emit leftClicked( * selectedObjects.at(0) );
					else
						emit leftClicked( selectedRegion->boundingRect().normalize() );
				} else if ((firstSelectedObject != NULL) && (lastSelectedObject != NULL)) {
					//  ------------   upddate list of selected objects   ------------
					selectedObjects.clear();

					if (*firstSelectedObject > *lastSelectedObject) {
						// selecting back direction
						BBoxOfObjectOnPage * h = firstSelectedObject;
						while (h != lastSelectedObject) {
							selectedObjects.append( new BBoxOfObjectOnPage( *h ) );
							if (h->getLeftBBox())
								h = h->getLeftBBox();
							else
								h = h->getPrevLineLastBBox();
						}
					} else {
						// selecting normal direction
						BBoxOfObjectOnPage * h = firstSelectedObject;
						while (h != lastSelectedObject) {
							selectedObjects.append( new BBoxOfObjectOnPage( *h ) );
							if (h->getRightBBox())
								h = h->getRightBBox();
							else
								h = h->getNextLineFirstBBox();
						}
					}
					selectedObjects.append( new BBoxOfObjectOnPage( *lastSelectedObject ) );

					emit newSelectedObjects( selectedObjects );
				}
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
			if ( (selectedRegion != NULL) && selectedRegion->contains( e->pos() ) ) {
				guiPrintDbg( debug::DBG_DBG, "selectedRegion contains e->pos() " <<e->pos().x() <<","<<e->pos().y());
				emit rightClicked( e->pos(), selectedRegion );
			} else {
				emit rightClicked( e->pos(), NULL );
			}
			break;
		default:
			guiPrintDbg( debug::DBG_DBG, "ignore");
	}
}

//void PageView::mouseDoubleClickEvent ( QMouseEvent * e ) {}

void PageView::mouseMoveEvent ( QMouseEvent * e ) {
	if ( isPress ) {
		// redraw selected rectangle after change
		if ( isMoving ) {
			QPoint h_p (mouseSelectedRegion->boundingRect().normalize().topLeft() - e->pos() + pointInRect);
			QRegion h_reg (*mouseSelectedRegion);
			mouseSelectedRegion->translate( -h_p.x(), -h_p.y() );
			drawRect( & h_reg, mouseSelectedRegion );
		} else {
			if ((selectionMode != SelectText) || isResizing ) {
				QPoint p( pointInRect.x() * (! resizeCoefficientX) + e->pos().x() * resizeCoefficientX,
					pointInRect.y() * (! resizeCoefficientY) + e->pos().y() * resizeCoefficientY );
				drawRect( mouseSelectedRegion, mouseRectSelected, & p );
			} else {
				blockRepaint( true );
				drawRect( mouseSelectedRegion, true );
				if (lastSelectedObject == NULL) {
					delete mouseSelectedRegion;
					mouseSelectedRegion = NULL;
				} else {
					BBoxOfObjectOnPage * h = lastSelectedObject;
					//  -----------   y   -----------
					while (h) {
						if (e->pos().y() < h->top()) {
							lastSelectedObject = h;
							if (h->getUpBBox())
								h = h->getUpBBox();
							else
								h = h->getPrevLineLastBBox();
						} else
							break;
					}
					if (h == NULL) {
						lastSelectedObject = lastSelectedObject->getFirstBBox();
					} else {
						while (h) {
							if (e->pos().y() > h->top()) {
								BBoxOfObjectOnPage * h2 = h->getDownBBox();
								if (h2 == NULL)
									h2 = h->getNextLineFirstBBox();
								if ((h2 != NULL) && (e->pos().y() > h2->top())) {
									lastSelectedObject = h = h2;
								} else {
									lastSelectedObject = h;
									break;
								}
							} else
								break;
						}
						if (h == NULL)
							lastSelectedObject = lastSelectedObject->getLastBBox();
					}
					//  ------------   x   ------------
					if (h) {
						h = lastSelectedObject;
						while (h) {
							if (e->pos().x() < h->left()) {
								lastSelectedObject = h;
								h = h->getLeftBBox();
							} else
								break;
						}
						while (h) {
							if (e->pos().x() > h->left()) {
								BBoxOfObjectOnPage * h2 = h->getRightBBox();
								if ((h2 != NULL) && (e->pos().x() > h2->left()))
									lastSelectedObject = h = h2;
								else {
									lastSelectedObject = h;
									break;
								}
							} else
								break;
						}
					}
					/* if ((lastSelectedObject->right() < e->pos().x()) || (lastSelectedObject->bottom() < e->pos().y())) {
						if (lastSelectedObject->getRightBBox())
							lastSelectedObject = lastSelectedObject->getRightBBox();
						else if (lastSelectedObject->getNextLineFirstBBox())
							lastSelectedObject = lastSelectedObject->getNextLineFirstBBox();
					}*/
						
					//  ------------   upddate region   ------------
					if (mouseSelectedRegion)
						* mouseSelectedRegion = QRegion();
					else
						mouseSelectedRegion = new QRegion( * firstSelectedObject );

					if (*firstSelectedObject > *lastSelectedObject) {
						// selecting back direction
						BBoxOfObjectOnPage * h = firstSelectedObject;
						while (h != lastSelectedObject) {
							* mouseSelectedRegion |= QRegion( *h );
							if (h->getLeftBBox()) {
								h = h->getLeftBBox();
							} else {
								h = h->getPrevLineLastBBox();
							}
						}
					} else {
						// selecting normal direction
						BBoxOfObjectOnPage * h = firstSelectedObject;
						while (h != lastSelectedObject) {
							* mouseSelectedRegion |= QRegion( *h );
							if (h->getRightBBox()) {
								h = h->getRightBBox();
							} else {
								h = h->getNextLineFirstBBox();
							}
						}
					}
					* mouseSelectedRegion |= QRegion( *lastSelectedObject );

					drawRect( mouseSelectedRegion );
				}
				blockRepaint( false );
			}
		}
	} else {	// if is not set changing mode then:

		// change cursor to correct
		int pomCur = none;
		if ( (selectedRegion != NULL) && (selectedRegion->boundingRect().contains( e->pos() )) ) {
			pomCur =  theNeerestResizingMode( selectedRegion, e->pos() );
			if ((pomCur == none) && (regionOfAllObjects.contains( e->pos() )))
				pomCur = onUnselectedObject;
		} else {

			if (regionOfAllObjects.contains( e->pos() ))
				pomCur = onUnselectedObject;
		}
		if (pomCur != cursorIsSetTo)
			setCursor( QCursor( mappingResizingModeToCursor[ cursorIsSetTo = pomCur ] ));
	}

	// emit change moese cursor on page
	if ( mousePos != e->pos() ) {
		mousePos = e->pos();
		emit changeMousePosition( mousePos );
	}
}

void PageView::clearObjectsBBox () {
//	if (selectionMode != SelectText) {
		delete arrayOfBBoxes;
		arrayOfBBoxes = NULL;
		regionOfAllObjects = QRegion();
//	}
}

void PageView::addObjectsBBox ( const QRect & bbox, const void * ptr_object ) {
	if (arrayOfBBoxes == NULL) {
		arrayOfBBoxes = new Rect2DArray();
		arrayOfBBoxes->setAutoDeleteAll(true);
	}
	arrayOfBBoxes->myAppend( new BBoxOfObjectOnPage( bbox, ptr_object ) );
	regionOfAllObjects |= QRegion( bbox );
	if (selectionMode == SelectText)
		arrayOfBBoxes->initAllBBoxPtr();
}

// *************************			drawing selected area			********************

void PageView::blockRepaint ( bool block ) {
	isRepaintingBlocked = block;
	if ((! block) && (! regionForRepaint.isEmpty())) {
		repaint( regionForRepaint, false );
		regionForRepaint = QRegion();
	} 
}

void PageView::drawRect ( QRegion * newReg, bool /* unDraw */ ) {
	if (newReg == NULL)
		return ;

	QPixmap * pm = this->pixmap();

	QPainter p( pm );
	p.setRasterOp( Qt::NotXorROP );
	p.setPen( QColor(85,170,170) );
	if (! quickSelection) {
		QMemArray<QRect> h_mar (newReg->rects());
		QMemArray<QRect>::Iterator it = h_mar.begin();
		for ( ; it != h_mar.end() ; ++it )
			p.fillRect( it->normalize(), Qt::yellow );
		p.drawRect( newReg->boundingRect().normalize() );
	} else {
		p.drawRect( newReg->boundingRect().normalize() );
	}
	
	if (isRepaintingBlocked)
		regionForRepaint += newReg->unite( QRegion(newReg->boundingRect().normalize()) );
	else
		repaint( newReg->unite( QRegion(newReg->boundingRect().normalize()) ), false );
}
void PageView::drawRect ( QRegion * oldReg, QRegion * newReg, enum SelectionSet ss) {
	QPixmap * pm = this->pixmap();

	QPainter p( pm );
	p.setRasterOp( Qt::NotXorROP );
	p.setPen( QColor(85,170,170) );

	// undraw old rerion
	QRegion qr( oldReg->boundingRect().normalize() );
	if (! quickSelection) {
		QMemArray<QRect> h_mar (oldReg->rects());
		QMemArray<QRect>::Iterator it = h_mar.begin();
		for ( ; it != h_mar.end() ; ++it )
			p.fillRect( it->normalize(), Qt::yellow );
		p.drawRect( oldReg->boundingRect().normalize() );
	} else {
		QRect normRect;
		normRect = oldReg->boundingRect().normalize();
		p.drawRect( normRect );
		qr = qr.eor( QRegion( normRect.left()+1, normRect.top()+1, normRect.width()-2, normRect.height()-2 ) );
	}

	// change drawing mode for new region
	changeSelection( ss );

	// draw new rectangle
	if (! quickSelection) {
		QMemArray<QRect> h_mar (newReg->rects());
		QMemArray<QRect>::Iterator it = h_mar.begin();
		for ( ; it != h_mar.end() ; ++it )
			p.fillRect( it->normalize(), Qt::yellow );
		p.drawRect( newReg->boundingRect().normalize() );
		qr = qr.unite( newReg->boundingRect().normalize() );
	} else {
		QRect normRect;
		normRect = newReg->boundingRect().normalize();
		p.drawRect( normRect );
		qr = qr.unite(	QRegion( normRect ) ^
				QRegion( normRect.left()+1, normRect.top()+1, normRect.width()-2, normRect.height()-2 ) );
	}

	if (isRepaintingBlocked)
		regionForRepaint += qr;
	else
		repaint( qr, false );
}
void PageView::drawRect ( const QRegion * reg, QRect * oldRect, const QPoint * toPoint ) {
	// !!! oldRect will be changed
	QRect normRect;
	normRect = oldRect->normalize();
	QRegion h_reg;
	if (reg == NULL)
		h_reg = QRegion( );
	else
		h_reg = * reg;

	QRegion qr( h_reg.unite( QRegion(normRect) ).boundingRect().normalize() );

	QPixmap * pm = this->pixmap();

	QPainter p( pm );
	p.setRasterOp( Qt::NotXorROP );
	p.setPen( QColor(85,170,170) );
	// undraw old rectangle
	if (! quickSelection) {
		QMemArray<QRect> h_mar (h_reg.rects());
		QMemArray<QRect>::Iterator it = h_mar.begin();
		for ( ; it != h_mar.end() ; ++it )
			p.fillRect( it->normalize(), Qt::yellow );
		p.drawRect( /*h_reg.unite( QRegion(*/normRect/*) ).boundingRect()*/.normalize() );
	} else {
		QRect h_normRect = /*h_reg.unite( QRegion(*/normRect/*) ).boundingRect()*/.normalize();
		p.drawRect( h_normRect );
		qr = qr.unite( QRegion(h_normRect.left()+1, h_normRect.top()+1, h_normRect.width()-2, h_normRect.height()-2) );
	}

	// if toPoint == NULL, don't new region to select
	if (toPoint) {
		// change selected rectangle
		if ( isMoving )
			oldRect->moveTopLeft( *toPoint );
		else
			oldRect->setBottomRight( *toPoint );

		// draw new rectangle
		normRect = oldRect->normalize();

		if (! quickSelection) {
			QMemArray<QRect> h_mar (h_reg.rects());
			QMemArray<QRect>::Iterator it = h_mar.begin();
			for ( ; it != h_mar.end() ; ++it )
				p.fillRect( it->normalize(), Qt::yellow );
			p.drawRect( /*h_reg.unite( QRegion(*/normRect/*)).boundingRect()*/.normalize());
			qr = qr.unite( h_reg.unite( normRect ).boundingRect().normalize() );
		} else {
			p.drawRect( normRect );
			qr = qr.unite(	QRegion( normRect ) ^
					QRegion( normRect.left()+1, normRect.top()+1, normRect.width()-2, normRect.height()-2 ) );
		}
	}

	if (isRepaintingBlocked)
		regionForRepaint += qr;
	else
		repaint( qr, false );
}

} // namespace gui


