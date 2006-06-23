#include <stdlib.h>
#include <math.h>
#include <qcursor.h>

#include "pageviewmode.h"
#include "pdfoperatorsiter.h"

namespace gui {

//  ---------------------   drawing objects  ----------------- //
DrawingObject * DrawingObjectFactory::create( const QString & nameOfObject )
		{
			if (nameOfObject == "line")  return new DrawingLine();
			if (nameOfObject == "rect")  return new DrawingRect();

			guiPrintDbg( debug::DBG_DBG, "Undefined drawing!!!" );
			return NULL;
		}
void DrawingObject::drawObject ( QPainter & /* painter */, QPoint /* p1 */, QPoint /* p2 */ )
		{};
void DrawingObject::drawObject ( QPainter & painter, QRegion reg )
		{
			QMemArray<QRect> h_mar (reg.rects());
			QMemArray<QRect>::Iterator it = h_mar.begin();
			for ( ; it != h_mar.end() ; ++it )
				painter.fillRect( it->normalize(), Qt::yellow );

			painter.drawRect( reg.boundingRect().normalize() );
		};
void DrawingObject::drawObject ( QPainter & painter, QRect rect )
		{
			drawObject ( painter, QRegion( rect.normalize() ) );
		};
DrawingLine::DrawingLine ()
		{
			pen.setWidth( 1 );
			pen.setColor( Qt::black );
			pen.setStyle( Qt::SolidLine );
		};
DrawingLine::~DrawingLine ()
		{};
void DrawingLine::drawObject ( QPainter & painter, QPoint p1, QPoint p2 )
		{
			QPen old_pen ( painter.pen() );

			painter.setPen( pen );
			painter.drawLine( p1, p2 );

			painter.setPen( old_pen );
		};
DrawingRect::DrawingRect ()
		{
			pen.setWidth( 1 );
			pen.setColor( Qt::black );
			pen.setStyle( Qt::SolidLine );
		};
DrawingRect::~DrawingRect ()
		{};
void DrawingRect::drawObject ( QPainter & painter, QPoint p1, QPoint p2 )
		{
			QPen old_pen ( painter.pen() );

			painter.setPen( pen );
			painter.drawRect( QRect(p1, p2).normalize() );

			painter.setPen( old_pen );
		};

//  ---------------------  selection mode  --------------------- //
PageViewMode * PageViewModeFactory::create(	const QString & nameOfMode,
							const QString & drawingObject,
							const QString & _scriptFncAtMouseRelease )
		{
			if (nameOfMode == "new_object")			return new PageViewMode_NewObject		( drawingObject, _scriptFncAtMouseRelease );
			if (nameOfMode == "text_selection")		return new PageViewMode_TextSelection	( drawingObject, _scriptFncAtMouseRelease );
			if (nameOfMode == "")					return new PageViewMode					( drawingObject, _scriptFncAtMouseRelease );

			guiPrintDbg( debug::DBG_DBG, "Undefined mode!!!" );
			return NULL;
		}

void PageViewMode::movedSelectedObjects ( QPoint relativeMove )
		{
			emit executeCommand ( scriptFncAtMoveSelectedObjects	.arg( relativeMove.x() )
																	.arg( relativeMove.y() ) );
		};
void PageViewMode::resizedSelectedObjects ( int dleft, int dtop, int dright, int dbottom )
		{
			emit executeCommand ( scriptFncAtResizeSelectedObjects	.arg( dleft )
																	.arg( dtop )
																	.arg( dright )
																	.arg( dbottom ) );
		};

void PageViewMode::mousePressLeftButton ( QMouseEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mouseReleaseLeftButton ( QMouseEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{
			emit executeCommand ( scriptFncAtMouseRelease	.arg( pressPosition.x() )
															.arg( pressPosition.y() )
															.arg( releasePosition.x() )
															.arg( releasePosition.y() ) );
		};
void PageViewMode::mouseMoveWithPressedLeftButton ( QMouseEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mousePressMidButton ( QMouseEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mouseReleaseMidButton ( QMouseEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mousePressRightButton ( QMouseEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mouseReleaseRightButton ( QMouseEvent * e, QPainter & /* p */, QWidget * /* w */ )
		{
			emit popupMenu ( e->pos() );
		};

void PageViewMode::moveSelectedObjects ( QMouseEvent * e, QPainter & p, QWidget * /* w */ )
		{
			if (! isMoving)
			{			// initialize moving process
				pointInRect.setX( e->x() - selectedOpRegion.boundingRect().normalize().left() );
				pointInRect.setY( e->y() - selectedOpRegion.boundingRect().normalize().top() );
				// pointInRect is positive point (x and y are positive)
				mouseSelectedRegion = selectedOpRegion;

				isMoving = true;

				drawingObject->drawObject( p, mouseSelectedRegion );		// draw new selection

				return;
			}

			QPoint h_p (mouseSelectedRegion.boundingRect().normalize().topLeft() - e->pos() + pointInRect);

			drawingObject->drawObject( p, mouseSelectedRegion );		// undraw old selection

			mouseSelectedRegion.translate( -h_p.x(), -h_p.y() );	// move selection
			drawingObject->drawObject( p, mouseSelectedRegion );		// draw new selection
		};
void PageViewMode::movedSelectedObjects ( QMouseEvent * e, QPainter & p, QWidget * w )
		{
			moveSelectedObjects( e, p, w );

			drawingObject->drawObject( p, mouseSelectedRegion );		// undraw old selection

			isMoving = false;

			movedSelectedObjects(  mouseSelectedRegion.boundingRect().normalize().topLeft()
									- selectedOpRegion.boundingRect().normalize().topLeft() );
		};

void PageViewMode::resizeSelectedObjects ( QMouseEvent * e, QPainter & p, QWidget * /* w */ )
		{
			if (! isResizing)
			{			// initialize resizing process
				int resizingMode = theNeerestResizingMode ( selectedOpRegion, e->pos() );

				QRect rectSelected (selectedOpRegion.boundingRect().normalize());

				if (resizingMode & left) {     // resizing to left side
					pressPosition.setX( rectSelected.right() );
					releasePosition.setX( rectSelected.left() );
				} else {            // resizing to right side or only vertical
					pressPosition.setX( rectSelected.left() );
					releasePosition.setX( rectSelected.right() );
				}

				if ((resizingMode & (left | right)) == none)
					resizeCoefficientX = 0;     // resizing only vertikal
				else
					resizeCoefficientX = 1;

				if (resizingMode & top) {      // resizing to top side
					pressPosition.setY( rectSelected.bottom() );
					releasePosition.setY( rectSelected.top() );
				} else {            // resizing to bottom side or only horizontal
					pressPosition.setY( rectSelected.top() );
					releasePosition.setY( rectSelected.bottom() );
				}

				if ((resizingMode & (top | bottom)) == none)
					resizeCoefficientY = 0;     // risizing only horizontal
				else
					resizeCoefficientY = 1;

				pointInRect = releasePosition;
				isResizing = true;

				drawingObject->drawObject( p, QRect( pressPosition, releasePosition ) );
				return;
			}

			drawingObject->drawObject( p, QRect( pressPosition, releasePosition ) );		// undraw

			releasePosition.setX( pointInRect.x() * (! resizeCoefficientX) + e->pos().x() * resizeCoefficientX );
			releasePosition.setY( pointInRect.y() * (! resizeCoefficientY) + e->pos().y() * resizeCoefficientY );

			drawingObject->drawObject( p, QRect( pressPosition, releasePosition ) );		// draw new
		};
void PageViewMode::resizedSelectedObjects ( QMouseEvent * e, QPainter & p, QWidget * w )
		{
			resizeSelectedObjects( e, p, w );
			
			drawingObject->drawObject( p, QRect( pressPosition, releasePosition ) );		// undraw

			isResizing = false;

			int dleft, dtop, dright, dbottom;
			QRect rectSelected (selectedOpRegion.boundingRect().normalize());
			// pointInRect is primary top-bottom edge
			if (pointInRect == rectSelected.topLeft()) {
				dleft	= rectSelected.left()	- releasePosition.x();
				dtop	= rectSelected.top()	- releasePosition.y();
				dright	= rectSelected.right()	- pressPosition.x();		// = 0
				dbottom	= rectSelected.bottom()	- pressPosition.y();		// = 0

			} else if (pointInRect == rectSelected.topRight()) {
				dleft	= rectSelected.left()	- pressPosition.x();		// = 0
				dtop	= rectSelected.top()	- releasePosition.y();
				dright	= rectSelected.right()	- releasePosition.x();
				dbottom	= rectSelected.bottom()	- pressPosition.y();		// = 0

			} else if (pointInRect == rectSelected.bottomLeft()) {
				dleft	= rectSelected.left()	- releasePosition.x();
				dtop	= rectSelected.top()	- pressPosition.y();		// = 0
				dright	= rectSelected.right()	- pressPosition.x();		// = 0
				dbottom	= rectSelected.bottom()	- releasePosition.y();

			} else if (pointInRect == rectSelected.bottomRight()) {
				dleft	= rectSelected.left()	- pressPosition.x();		// = 0
				dtop	= rectSelected.top()	- pressPosition.y();		// = 0
				dright	= rectSelected.right()	- releasePosition.x();
				dbottom	= rectSelected.bottom()	- releasePosition.y();
			}
			resizedSelectedObjects ( dleft, dtop, dright, dbottom );
		};
void PageViewMode::mousePressEvent ( QMouseEvent * e, QPainter & p, QWidget * w )
		{
			switch (e->button()) {
				case Qt::LeftButton:
						if (! isPressedLeftButton) {
							isPressedLeftButton = true;
							isMoving = false;
							isResizing = false;

							drawingObject->drawObject ( p, selectedOpRegion );		// undraw

							int pomCur = none;
							if (selectedOpRegion.boundingRect().normalize().contains( e->pos() ) )
								pomCur = theNeerestResizingMode ( selectedOpRegion, e->pos() );

							switch (pomCur) {
								case none:
										mousePressLeftButton ( e, p, w );
										break;
								case left | right | top | bottom:
										moveSelectedObjects ( e, p, w );
										break;
								default:
										resizeSelectedObjects ( e, p, w );
							}
						}
						break;
				case Qt::RightButton:
						mousePressRightButton ( e, p, w );
						break;
				case Qt::MidButton:
						mousePressMidButton ( e, p, w );
						break;

				default :
						break;
			}
		};
void PageViewMode::mouseReleaseEvent ( QMouseEvent * e, QPainter & p, QWidget * w )
		{
			switch (e->button()) {
				case Qt::LeftButton:
						if (isPressedLeftButton) {
							if (isMoving) {
								movedSelectedObjects( e, p, w );
							} else if (isResizing) {
								resizedSelectedObjects( e, p, w );
							} else
								mouseReleaseLeftButton ( e, p, w );

							drawingObject->drawObject ( p, selectedOpRegion );		// draw
						}
						isPressedLeftButton = false;
						break;
				case Qt::RightButton:
						mouseReleaseRightButton ( e, p, w );
						break;
				case Qt::MidButton:
						mouseReleaseMidButton ( e, p, w );
						break;

				default :
						break;
			}
		};
void PageViewMode::mouseDoubleClickEvent ( QMouseEvent * e, QPainter & p, QWidget * w )
		{
			mousePressEvent( e, p, w );
		};
void PageViewMode::mouseMoveEvent ( QMouseEvent * e, QPainter & p, QWidget * w )
		{
			if (isPressedLeftButton) {
				if (isMoving) {
					moveSelectedObjects ( e, p, w );
				} else if (isResizing) {
					resizeSelectedObjects ( e, p, w );
				} else
					mouseMoveWithPressedLeftButton ( e, p, w );

				return;
			}

			int pomCur = none;
			if (selectedOpRegion.boundingRect().normalize().contains( e->pos() ) )
				pomCur = theNeerestResizingMode( selectedOpRegion, e->pos() );
			if ((pomCur == none) && (workOpRegion.contains( e->pos() )))
				pomCur = onUnselectedObject;

			w->setCursor( QCursor( mappingResizingModeToCursor[ pomCur ] ) );
		};
void PageViewMode::wheelEvent ( QWheelEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};

void PageViewMode::keyPressEvent ( QKeyEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};
void PageViewMode::keyReleaseEvent ( QKeyEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};

void PageViewMode::focusInEvent ( QFocusEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};
void PageViewMode::focusOutEvent ( QFocusEvent * /* e */, QPainter & /* p */, QWidget * /* w */ )
		{};

void PageViewMode::repaint ( QPainter & p,  QWidget * /* w */ )
		{
			drawingObject->drawObject( p, selectedOpRegion );
		};

void PageViewMode::setWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps )
		{
			clearWorkOperators();

			addWorkOperators ( wOps );
		};
void PageViewMode::addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps )
		{
			workOperators.insert( workOperators.end(), wOps.begin(), wOps.end() );

			addOpsBBoxToRegion ( workOpRegion, wOps );
		};
void PageViewMode::clearWorkOperators ()
		{
			workOperators.clear();

			workOpRegion = QRegion();
		};
void PageViewMode::clearSelectedOperators ()
		{
			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			selectedOperators.clear();
			selectedOpRegion = QRegion();

			emit needRepaint();
		};
void PageViewMode::setSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps )
		{
			clearSelectedOperators();

			addSelectedOperators( sOps );
		};
void PageViewMode::addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps )
		{
			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			selectedOperators.insert( selectedOperators.end(), sOps.begin(), sOps.end() );

			addOpsBBoxToRegion ( selectedOpRegion, sOps );

			emit needRepaint();
		};

void PageViewMode::actualizeSelection ()
		{
			std::vector< boost::shared_ptr< PdfOperator > >::iterator	it_selected;
			std::vector< boost::shared_ptr< PdfOperator > >::iterator	it_work	= workOperators.begin();

			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			for ( ; it_work != workOperators.end() ; ++it_work ) {
				it_selected = selectedOperators.begin();
				for ( ; it_selected != selectedOperators.end() ; ++it_selected ) {
					if ( (*it_work) == (*it_selected) ) {
						//(*it_selected) = (*it_work);
						break ;
					}
				}
				if ( (it_selected != selectedOperators.end()) && ((*it_work) != (*it_selected)) )
					selectedOperators.erase( it_selected );
			}

			selectedOpRegion = QRegion();
			addOpsBBoxToRegion ( selectedOpRegion, selectedOperators );
			emit needRepaint();
		};

QRect PageViewMode::getBBox( const boost::shared_ptr<PdfOperator> & op ) const
		{
			Rectangle bbox = op->getBBox();
			QRect box ( (int) floor(std::min(bbox.xleft,bbox.xright)), (int) floor(std::min(bbox.yleft, bbox.yright)),
						std::abs((int)ceil(bbox.xright - bbox.xleft))+1, std::abs((int)ceil(bbox.yleft - bbox.yright))+1);
			return box;
		};
void PageViewMode::addOpsBBoxToRegion ( QRegion & r, const std::vector< boost::shared_ptr< PdfOperator > > & ops )
		{
			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			std::vector< boost::shared_ptr< PdfOperator > >::const_iterator   it = ops.begin();
			for ( ; it != ops.end() ; ++it ) {
				r |= QRegion( getBBox(*it) );
			}
		};

QRegion PageViewMode::getSelectedRegion ()
		{ return selectedOpRegion; };

void PageViewMode::setSelectedRegion ( QRegion r )
		{
			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			selectedOpRegion = r;
		};

bool PageViewMode::isSomeoneSelected ()
		{
			return (! selectedOpRegion.isNull()) || (! selectedOperators.empty());
		};

void PageViewMode::setResizingZone ( unsigned int width)
		{
			resizingZone = width;
		};

PageViewMode::~PageViewMode()
		{};

PageViewMode::PageViewMode( const QString & _drawingObject, const QString & _scriptFncAtMouseRelease ) :
			QObject()
		{
			resizingZone = 2;
			scriptFncAtMouseRelease = _scriptFncAtMouseRelease;
			scriptFncAtMoveSelectedObjects = "moveSelectedObject( %1, %2 )";
			scriptFncAtResizeSelectedObjects = "resizedSelectedObjects( %1, %2, %3, %4 )";

			isPressedLeftButton = false;

			// create drawingObject
			DrawingObject * pom = DrawingObjectFactory::create( _drawingObject );
			if (pom != NULL)
				drawingObject.reset( pom );
			else
				drawingObject.reset( new DrawingObject() );

			// initialize mapping cursors
			setMappingCursor();
		};

void PageViewMode::setMappingCursor()
		{
			for (int i = 0; i<17 ; i++)
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

			// set correct cursor
			/* cursorIsSetTo = -1;
			mouseMoveEvent( new QMouseEvent( QEvent::MouseMove, mousePos, Qt::NoButton, Qt::NoButton ) );*/
		}

int	PageViewMode::theNeerestResizingMode ( const QRegion & r, const QPoint & p )
		{
			if (r.isNull()) {
				return none;
			}

			int	resizingMode = none;
			QRect h_br ( r.boundingRect().normalize() );

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
					cz = std::min( cz, resizingZone );

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
					cz = std::min( cz, resizingZone );

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
					if (! r.contains( p ))
						resizingMode = none;
			}

			return resizingMode;
		};

// ----------------------------------  PageViewMode_NewObject  --------------------------- //
PageViewMode_NewObject::PageViewMode_NewObject ( const QString & drawingObject, const QString & _scriptFncAtMouseRelease ) :
			PageViewMode ( drawingObject, _scriptFncAtMouseRelease )
		{};

void	PageViewMode_NewObject::mousePressLeftButton ( QMouseEvent * e, QPainter & p, QWidget * /* w */ )
		{
			pressPosition = releasePosition = e->pos();
			drawingObject->drawObject( p, e->pos(), e->pos() );
		}
void	PageViewMode_NewObject::mouseReleaseLeftButton ( QMouseEvent * e, QPainter & p, QWidget * w )
		{
			drawingObject->drawObject( p, pressPosition, releasePosition );		// undraw

			releasePosition = e->pos();

			this->PageViewMode::mouseReleaseLeftButton (e, p, w);
		}
void	PageViewMode_NewObject::mouseMoveWithPressedLeftButton ( QMouseEvent * e, QPainter & p, QWidget * /* w */ )
		{
			drawingObject->drawObject( p, pressPosition, releasePosition );		// undraw
			releasePosition = e->pos();
			drawingObject->drawObject( p, pressPosition, releasePosition );		// draw new
		}

void	PageViewMode_NewObject::repaint ( QPainter & p, QWidget * w  )
		{
			if (isPressedLeftButton)
				drawingObject->drawObject( p, pressPosition, releasePosition );
			else
				this->PageViewMode::repaint( p, w );
		}
// ----------------------------------  PageViewMode_TextSelection  --------------------------- //
PageViewMode_TextSelection::PageViewMode_TextSelection ( const QString & drawingObject, const QString & _scriptFncAtMouseRelease ) :
			PageViewMode ( drawingObject, _scriptFncAtMouseRelease )
		{
			// initialize mapping cursors
			setMappingCursor();
		};

void PageViewMode_TextSelection::setMappingCursor()
		{
			this->PageViewMode::setMappingCursor();

			mappingResizingModeToCursor [ onUnselectedObject ] = Qt::IbeamCursor;
		};

void PageViewMode_TextSelection::setSelectedRegion ( QRegion r )
		//TODO
		{};

void PageViewMode_TextSelection::clearWorkOperators ()
		{
			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			this->PageViewMode::clearWorkOperators();
		};
void PageViewMode_TextSelection::addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps )
		{
			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			guiPrintDbg( debug::DBG_DBG, "->" );
			std::vector< boost::shared_ptr< PdfOperator > > hOps;
			TextOperatorIterator textIter ( wOps[0] );
			for ( ; ! textIter.isEnd() ; textIter.next() )
			{
				hOps.push_back( textIter.getCurrent() );
				QRect r = getBBox(textIter.getCurrent());
				boost::shared_ptr<PdfOperator> o = textIter.getCurrent();
				arrayOfBBoxes.myAppend( new BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> >( r, o ) );

				std::string s;
				textIter.getCurrent()->getStringRepresentation( s );
				guiPrintDbg( debug::DBG_DBG, s );
			}

			arrayOfBBoxes.initAllBBoxPtr();

			this->PageViewMode::addWorkOperators ( hOps );
		};
void PageViewMode_TextSelection::addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps )
		{
			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			std::vector< boost::shared_ptr< PdfOperator > > hOps;
			TextOperatorIterator textIter ( sOps[0] );
			for ( ; ! textIter.isEnd() ; textIter.next() )
			{
				hOps.push_back( textIter.getCurrent() );
			}

			this->PageViewMode::addWorkOperators ( hOps );
		};

const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * PageViewMode_TextSelection::getNearestObject( const QPoint & point )
		{
			if (arrayOfBBoxes.isEmpty())
				return NULL;

			RectArray< boost::shared_ptr<PdfOperator> > * prevLine = NULL;
			RectArray< boost::shared_ptr<PdfOperator> > * line = arrayOfBBoxes.first();
			for ( ; line ; prevLine = line, line = arrayOfBBoxes.next() ) {
				if (line->getMinY() > point.y())
				break;
			}

			if (prevLine == NULL)
				prevLine = line;

			BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * nearest = prevLine->first();
			BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * prevNearest = nearest;
			for ( ; nearest ; prevNearest = nearest, nearest = prevLine->next() ) {
				if (nearest->left() > point.x())
					break;
				if (nearest->right() > point.x()) {
					prevNearest = nearest;
					break;
				}
			}

			return prevNearest;
		};

void PageViewMode_TextSelection::mousePressLeftButton ( QMouseEvent * e, QPainter & p, QWidget * /* w */ )
		{
			nearestObjectToClick = getNearestObject( e->pos() );
			firstSelectedObject = nearestObjectToClick;
			for ( ; firstSelectedObject ; firstSelectedObject = firstSelectedObject->getRightBBox() ) {
				if (firstSelectedObject->left() > e->pos().x()) {
					firstSelectedObject = NULL;
					break;
				}
				if (firstSelectedObject->contains( e->pos() ) )
					break;
			}
			lastSelectedObject = firstSelectedObject;
			if (firstSelectedObject) {
				mouseSelectedRegion = QRegion( * firstSelectedObject );
				drawingObject->drawObject( p, mouseSelectedRegion );	// draw new selected object
			} else {
				isPressedLeftButton = false;
				drawingObject->drawObject( p, selectedOpRegion );		// draw old selected object
			}
		};
void PageViewMode_TextSelection::updateSelection (	const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > *	first,
													const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > *	last,
													QRegion *														selOpsRegion,
													std::vector< boost::shared_ptr< PdfOperator > > *				selOps )
		{
			if (selOps)
				selOps->clear();
			if (selOpsRegion)
				* selOpsRegion = QRegion();

			if (*first > *last) {
				// selecting back direction
				const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * h = first;
				while (h != last) {
					assert( h );

					if (selOps)
						selOps->push_back( h->getObject() );
					if (selOpsRegion)
						* selOpsRegion |= QRegion( getBBox( h->getObject() ) );

					if (h->getLeftBBox())
						h = h->getLeftBBox();
					else
						h = h->getPrevLineLastBBox();
				}
			} else {
				// selecting normal direction
				const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * h = first;
				while (h != last) {
					assert( h );

					if (selOps)
						selOps->push_back( h->getObject() );
					if (selOpsRegion)
						* selOpsRegion |= QRegion( getBBox( h->getObject() ) );

					if (h->getRightBBox())
						h = h->getRightBBox();
					else
						h = h->getNextLineFirstBBox();
				}
			}

			if (selOps)
				selOps->push_back( last->getObject() );
			if (selOpsRegion)
				* selOpsRegion |= QRegion( getBBox( last->getObject() ) );
		};
void PageViewMode_TextSelection::mouseReleaseLeftButton ( QMouseEvent * e, QPainter & p, QWidget * /* w */ )
		{
			assert( firstSelectedObject );
			assert( lastSelectedObject );

			drawingObject->drawObject( p, mouseSelectedRegion );		// undraw

			// actualize selected operators
			updateSelection( firstSelectedObject, lastSelectedObject,  & selectedOpRegion, & selectedOperators );

			emit newSelectedOperators( selectedOperators );
		};
void PageViewMode_TextSelection::mouseMoveWithPressedLeftButton ( QMouseEvent * e, QPainter & p, QWidget * /* w */ )
		{
			drawingObject->drawObject( p, mouseSelectedRegion );		// undraw

			const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * h = lastSelectedObject;
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
				assert( lastSelectedObject );
			} else {
				while (h) {
					if (e->pos().y() > h->top()) {
						const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * h2 = h->getDownBBox();
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
				if (h == NULL) {
					lastSelectedObject = lastSelectedObject->getLastBBox();
					assert( lastSelectedObject );
				}
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
						const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * h2 = h->getRightBBox();
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
			updateSelection( firstSelectedObject, lastSelectedObject,  & mouseSelectedRegion );

			drawingObject->drawObject( p, mouseSelectedRegion );		// draw
		};

void PageViewMode_TextSelection::repaint ( QPainter & p, QWidget * w  )
		{
			if (isPressedLeftButton)
				drawingObject->drawObject( p, mouseSelectedRegion );
			else
				this->PageViewMode::repaint( p, w );
		};
} // namespace gui
