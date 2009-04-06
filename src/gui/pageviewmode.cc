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
// vim:tabstop=4:shiftwidth=4:noexpandtab
#include <stdlib.h>
#include <math.h>
#include <qcursor.h>

#include "pageviewmode.h"
#include "kernel/pdfoperatorsiter.h"

#include "kernel/cpage.h"
#include "kernel/cannotation.h"
#include "kernel/pdfoperators.h"
#include "kernel/carray.h"

using namespace pdfobjects;
using namespace utils;

namespace gui {

//  ---------------------   drawing objects  ----------------- //
DrawingObject * DrawingObjectFactory::create( const QString & nameOfObject )
		{
			if (nameOfObject == "line")  return new DrawingLine();
			if (nameOfObject == "rect")  return new DrawingRect();
			if (nameOfObject == "rect2")  return new DrawingRect2();
			if (nameOfObject == "rect3")  return new DrawingRect3();

			guiPrintDbg( debug::DBG_DBG, "Undefined drawing!!!" );
			return NULL;
		}
DrawingObject::DrawingObject ()
		{
			pen.setWidth( 1 );
			pen.setColor( Qt::black );
			pen.setStyle( Qt::SolidLine );
		};
DrawingObject::~DrawingObject ()
		{};
void DrawingObject::drawObject ( QPainter & /* painter */, QPoint /* p1 */, QPoint /* p2 */ )
		{};
void DrawingObject::drawObject ( QPainter & painter, QRegion reg )
		{
			QPen old_pen ( painter.pen() );
			painter.setPen( pen );

			QMemArray<QRect> h_mar (reg.rects());
			QMemArray<QRect>::Iterator it = h_mar.begin();
			for ( ; it != h_mar.end() ; ++it )
				painter.fillRect( it->normalize(), Qt::yellow );

			painter.drawRect( reg.boundingRect().normalize() );

			painter.setPen( old_pen );
		};
void DrawingObject::drawObject ( QPainter & painter, QRect rect )
		{
			drawObject ( painter, QRegion( rect.normalize() ) );
		};
DrawingLine::DrawingLine ()
		{};
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
		{};
DrawingRect::~DrawingRect ()
		{};
void DrawingRect::drawObject ( QPainter & painter, QPoint p1, QPoint p2 )
		{
			QPen old_pen ( painter.pen() );

			painter.setPen( pen );
			painter.drawRect( QRect(p1, p2).normalize() );

			painter.setPen( old_pen );
		};

DrawingRect2::DrawingRect2 ()
		{};
DrawingRect2::~DrawingRect2 ()
		{};
void DrawingRect2::drawObject ( QPainter & painter, QRegion reg )
		{
			QPen old_pen ( painter.pen() );
			painter.setPen( pen );

			QMemArray<QRect> h_mar (reg.rects());
			QMemArray<QRect>::Iterator it = h_mar.begin();
			for ( ; it != h_mar.end() ; ++it )
				painter.fillRect( it->normalize(), Qt::yellow );

			painter.setPen( old_pen );
		};

DrawingRect3::DrawingRect3 ()
		{};
DrawingRect3::~DrawingRect3 ()
		{};
void DrawingRect3::drawObject ( QPainter & painter, QPoint p1, QPoint p2 )
		{
			QPen old_pen ( painter.pen() );

			painter.setPen( pen );
			painter.fillRect( QRect(p1, p2).normalize(), Qt::yellow );

			painter.setPen( old_pen );
		};
void DrawingRect3::drawObject ( QPainter & painter, QRegion reg )
		{
			QPen old_pen ( painter.pen() );
			painter.setPen( pen );

			QMemArray<QRect> h_mar (reg.rects());
			QMemArray<QRect>::Iterator it = h_mar.begin();
			for ( ; it != h_mar.end() ; ++it )
				painter.drawRect( it->normalize() );

			painter.setPen( old_pen );
		};
void DrawingRect3::drawObject ( QPainter & painter, QRect rect )
		{
			drawObject( painter, rect.topLeft(), rect.bottomRight() );
		};

//  ---------------------  selection mode  --------------------- //
PageViewMode * PageViewModeFactory::create(	const QString & nameOfMode,
											const QString & drawingObject,
											const QString & scriptFncAtMouseRelease,
											const QString & scriptFncAtMoveSelectedObjects,
											const QString & scriptFncAtResizeSelectedObjects )
		{
			if (nameOfMode == "new_object")				return new PageViewMode_NewObject			( drawingObject,
																										scriptFncAtMouseRelease,
																										scriptFncAtMoveSelectedObjects,
																										scriptFncAtResizeSelectedObjects);
			if (nameOfMode == "text_selection")			return new PageViewMode_TextSelection		( drawingObject,
																										scriptFncAtMouseRelease,
																										scriptFncAtMoveSelectedObjects,
																										scriptFncAtResizeSelectedObjects);
			if (nameOfMode == "operators_selection")	return new PageViewMode_OperatorsSelection	( drawingObject,
																										scriptFncAtMouseRelease,
																										scriptFncAtMoveSelectedObjects,
																										scriptFncAtResizeSelectedObjects);
			if (nameOfMode == "graphical_operators")	return new PageViewMode_GraphicalOperatorsSelection	( drawingObject,
																										scriptFncAtMouseRelease,
																										scriptFncAtMoveSelectedObjects,
																										scriptFncAtResizeSelectedObjects);
			if (nameOfMode == "annotations")			return new PageViewMode_Annotations			( drawingObject,
																										scriptFncAtMouseRelease,
																										scriptFncAtMoveSelectedObjects,
																										scriptFncAtResizeSelectedObjects);
			if (nameOfMode == "text_marking")			return new PageViewMode_TextMarking			( drawingObject,
																										scriptFncAtMouseRelease,
																										scriptFncAtMoveSelectedObjects,
																										scriptFncAtResizeSelectedObjects);
			if (nameOfMode == "")						return new PageViewMode						( drawingObject,
																										scriptFncAtMouseRelease,
																										scriptFncAtMoveSelectedObjects,
																										scriptFncAtResizeSelectedObjects);

			guiPrintDbg( debug::DBG_DBG, "Undefined mode!!!" );
			return NULL;
		}

void PageViewMode::movedSelectedObjects ( QPoint relativeMove )
		{
			if (relativeMove != QPoint(0,0)) {
				//Return if script not defined
				if (scriptFncAtMoveSelectedObjects.isNull()) return;
				emit executeCommand ( scriptFncAtMoveSelectedObjects	.arg( relativeMove.x() )
											.arg( relativeMove.y() ) );
			}
		};
void PageViewMode::resizedSelectedObjects ( int dleft, int dtop, int dright, int dbottom )
		{
			//Return if script not defined
			if (scriptFncAtResizeSelectedObjects.isNull()) return;
			emit executeCommand ( scriptFncAtResizeSelectedObjects	.arg( dleft )
										.arg( dtop )
										.arg( dright )
										.arg( dbottom ) );
		};

void PageViewMode::mousePressLeftButton ( QMouseEvent * /* e */, QPainter * /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mouseReleaseLeftButton ( QMouseEvent * e, QPainter * /* p */, QWidget * /* w */ )
		{
			//Return if script not defined
			if (scriptFncAtMouseRelease.isNull()) return;
			emit executeCommand ( scriptFncAtMouseRelease	.arg( pressPosition.x() )
															.arg( pressPosition.y() )
															.arg( releasePosition.x() )
															.arg( releasePosition.y() )
															.arg( e->globalPos().x() )
															.arg( e->globalPos().y() ) );
		};
void PageViewMode::mouseMoveWithPressedLeftButton ( QMouseEvent * /* e */, QPainter * /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mousePressMidButton ( QMouseEvent * /* e */, QPainter * /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mouseReleaseMidButton ( QMouseEvent * /* e */, QPainter * /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mousePressRightButton ( QMouseEvent * /* e */, QPainter * /* p */, QWidget * /* w */ )
		{};
void PageViewMode::mouseReleaseRightButton ( QMouseEvent * e, QPainter * /* p */, QWidget * /* w */ )
		{
			emit popupMenu ( e->pos() );
		};

void PageViewMode::moveSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * /* w */ )
		{
			if (! isMoving)
			{			// initialize moving process
				pointInRect.setX( e->x() - selectedOpRegion.boundingRect().normalize().left() );
				pointInRect.setY( e->y() - selectedOpRegion.boundingRect().normalize().top() );
				// pointInRect is positive point (x and y are positive)
				mouseSelectedRegion = selectedOpRegion;

				isMoving = true;

				if (p) {
					drawingObject->drawObject( *p, mouseSelectedRegion );		// draw new selection
				} else {
					emit needRepaint();
				}

				return;
			}

			QPoint h_p (mouseSelectedRegion.boundingRect().normalize().topLeft() - e->pos() + pointInRect);

			if (p)
				drawingObject->drawObject( *p, mouseSelectedRegion );		// undraw old selection

			mouseSelectedRegion.translate( -h_p.x(), -h_p.y() );	// move selection

			if (p)
				drawingObject->drawObject( *p, mouseSelectedRegion );		// draw new selection
			else {
				emit needRepaint();
			}
		};
void PageViewMode::movedSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			moveSelectedObjects( e, p, w );
			if (p)
				drawingObject->drawObject( *p, mouseSelectedRegion );		// undraw old selection

			isMoving = false;

			movedSelectedObjects(  mouseSelectedRegion.boundingRect().normalize().topLeft()
									- selectedOpRegion.boundingRect().normalize().topLeft() );

			if (p == NULL)
				emit needRepaint();
		};

void PageViewMode::resizeSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * /* w */ )
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

				if (p)
					drawingObject->drawObject( *p, QRect( pressPosition, releasePosition ) );
				else
					emit needRepaint();

				return;
			}

			if (p)
				drawingObject->drawObject( *p, QRect( pressPosition, releasePosition ) );		// undraw

			releasePosition.setX( pointInRect.x() * (! resizeCoefficientX) + e->pos().x() * resizeCoefficientX );
			releasePosition.setY( pointInRect.y() * (! resizeCoefficientY) + e->pos().y() * resizeCoefficientY );

			if (p)
				drawingObject->drawObject( *p, QRect( pressPosition, releasePosition ) );		// draw new
			else
				needRepaint();
		};
void PageViewMode::resizedSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			resizeSelectedObjects( e, p, w );
			if (p)
				drawingObject->drawObject( *p, QRect( pressPosition, releasePosition ) );		// undraw

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
			} else {
				//Should not get here, but compiler is screaming
				assert(0);
				return;
			}

			resizedSelectedObjects ( dleft, dtop, dright, dbottom );

			if (p == NULL)
				needRepaint();
		};
void PageViewMode::mousePressEvent ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			switch (e->button()) {
				case Qt::LeftButton:
						if (! isPressedLeftButton) {
							isPressedLeftButton = true;
							isMoving = false;
							isResizing = false;

							if (p)
								drawingObject->drawObject ( *p, selectedOpRegion );		// undraw

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
void PageViewMode::mouseReleaseEvent ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			switch (e->button()) {
				case Qt::LeftButton:
						if (isPressedLeftButton) {
							if (isMoving) {
								movedSelectedObjects( e, p, w );
							} else if (isResizing) {
								resizedSelectedObjects( e, p, w );
							} else {
								mouseReleaseLeftButton ( e, p, w );
							}

							if (p)
								drawingObject->drawObject ( *p, selectedOpRegion );		// draw
						}
						isPressedLeftButton = false;

						if (p == NULL)
							emit needRepaint();

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
void PageViewMode::mouseDoubleClickEvent ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			mousePressEvent( e, p, w );
		};
void PageViewMode::setCorrectCursor (  const QPoint & p, QPainter * /* p */, QWidget * w )
		{
			int pomCur = none;
			if (selectedOpRegion.boundingRect().normalize().contains( p ) )
				pomCur = theNeerestResizingMode( selectedOpRegion, p );
			if ((pomCur == none) && (workOpRegion.contains( p )))
				pomCur = onUnselectedObject;

			w->setCursor( QCursor( mappingResizingModeToCursor[ pomCur ] ) );
		};
void PageViewMode::mouseMoveEvent ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			if (isPressedLeftButton) {
				if (isMoving) {
					moveSelectedObjects ( e, p, w );
				} else if (isResizing) {
					resizeSelectedObjects ( e, p, w );
				} else {
					mouseMoveWithPressedLeftButton ( e, p, w );
				}

				return;
			}

			setCorrectCursor( e->pos(), p, w );
		};
void PageViewMode::wheelEvent ( QWheelEvent * /* e */, QPainter * /* p */, QWidget * /* w */ )
		{};

void PageViewMode::keyPressEvent ( QKeyEvent * e , QPainter * /* p */, QWidget * w  ) {
 switch (e->key()) {
  case Qt::Key_Up:
   emit scroll(0,-16);
   break;
  case Qt::Key_Down:
   emit scroll(0,16);
   break;
  case Qt::Key_Left:
   emit scroll(-16,0);
   break;
  case Qt::Key_Right:
   emit scroll(16,0);
   break;
  case Qt::Key_Home:
   emit scroll(-32767,0);
   break;
  case Qt::Key_End:
   emit scroll(32767,0);
   break;
  case Qt::Key_PageUp:
   emit scroll(0,-w->height());
   break;
  case Qt::Key_PageDown:
   emit scroll(0,w->height());
   break;
  default:
   return;
 }
 emit needRepaint();
}

void PageViewMode::keyReleaseEvent ( QKeyEvent * e, QPainter * /* p */, QWidget * /* w */ )
		{
			switch (e->key()) {
				case Qt::Key_Escape:
						if (isPressedLeftButton)
							isPressedLeftButton = false;
						else {
							clearSelectedOperators();
							emit newSelectedOperators( selectedOperators );
						}

						break;
				case Qt::Key_Delete:
						if (isPressedLeftButton)
							isPressedLeftButton = false;

						emit deleteSelection();
			}

			emit needRepaint();
		};

void PageViewMode::focusInEvent ( QFocusEvent * /* e */, QPainter * /* p */, QWidget * /* w */ )
		{};
void PageViewMode::focusOutEvent ( QFocusEvent * /* e */, QPainter * /* p */, QWidget * /* w */ )
		{};

void PageViewMode::repaint ( QPainter & p,  QWidget * /* w */ )
		{
			if (isPressedLeftButton) {
				if (isMoving) {
					drawingObject->drawObject( p, mouseSelectedRegion );
					drawingObject->drawObject( p, selectedOpRegion );
				} else if (isResizing) {
					drawingObject->drawObject( p, QRect( pressPosition, releasePosition ) );
				} else {
					drawingObject->drawObject( p, pressPosition, releasePosition );		// undraw
				}

				return;
			}

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
			std::vector< boost::shared_ptr< PdfOperator > >				temp_selectedOperators;
			std::vector< boost::shared_ptr< PdfOperator > >::iterator	it_selected = selectedOperators.begin();
			std::vector< boost::shared_ptr< PdfOperator > >::iterator	it_work;

			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			for ( ; it_selected != selectedOperators.end() ; ++it_selected ) {
				it_work = workOperators.begin();
				for ( ; it_work != workOperators.end() ; ++it_work ) {
					if ( (*it_work) == (*it_selected) ) {
						temp_selectedOperators.push_back( *it_work );
						break ;
					}
				}
			}
			selectedOperators.swap( temp_selectedOperators );

			selectedOpRegion = QRegion();
			addOpsBBoxToRegion ( selectedOpRegion, selectedOperators );
			emit needRepaint();
			emit newSelectedOperators( selectedOperators );
		};

QRect PageViewMode::getBBox( const boost::shared_ptr<PdfOperator> & op ) const
		{
			libs::Rectangle bbox = op->getBBox();
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
void PageViewMode::extraInitialize( const boost::shared_ptr< CPage > & /* page */, const DisplayParams & /*displayParams*/ )
		{};
void PageViewMode::sendAllSelectedOperators ()
		{
			emit newSelectedOperators( selectedOperators );
		};

bool PageViewMode::isSomeoneSelected ()
		{
			return (! selectedOpRegion.isNull()) || (! selectedOperators.empty());
		};

void PageViewMode::setResizingZone ( unsigned int width)
		{
			resizingZone = width;
		};
int PageViewMode::getResizingZone ( )
		{
			return resizingZone;
		};

PageViewMode::~PageViewMode()
		{};

PageViewMode::PageViewMode( const QString & _drawingObject,
							const QString & _scriptFncAtMouseRelease,
							const QString & _scriptFncAtMoveSelectedObjects,
							const QString & _scriptFncAtResizeSelectedObjects ) :
			QObject()
		{
			resizingZone = 2;
			scriptFncAtMouseRelease = _scriptFncAtMouseRelease;

			if (_scriptFncAtMoveSelectedObjects.isNull())
				scriptFncAtMoveSelectedObjects = "moveSelectedObject( %1, %2 )";
			else
				scriptFncAtMoveSelectedObjects = _scriptFncAtMoveSelectedObjects;

			if (_scriptFncAtResizeSelectedObjects.isNull())
				scriptFncAtResizeSelectedObjects = "resizedSelectedObjects( %1, %2, %3, %4 )";
			else
				scriptFncAtResizeSelectedObjects = _scriptFncAtResizeSelectedObjects;

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
PageViewMode_NewObject::PageViewMode_NewObject ( const QString & drawingObject,
												const QString & _scriptFncAtMouseRelease,
												const QString & _scriptFncAtMoveSelectedObjects,
												const QString & _scriptFncAtResizeSelectedObjects ) :
			PageViewMode ( drawingObject,
							_scriptFncAtMouseRelease,
							_scriptFncAtMoveSelectedObjects,
							_scriptFncAtResizeSelectedObjects )
		{};

void	PageViewMode_NewObject::mousePressLeftButton ( QMouseEvent * e, QPainter * p, QWidget * /* w */ )
		{
			pressPosition = releasePosition = e->pos();
			if (p)
				drawingObject->drawObject( *p, e->pos(), e->pos() );
			else
				emit needRepaint();
		}
void	PageViewMode_NewObject::mouseReleaseLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			if (p)
				drawingObject->drawObject( *p, pressPosition, releasePosition );		// undraw

			releasePosition = e->pos();

			this->PageViewMode::mouseReleaseLeftButton (e, p, w);

			if (p == NULL)
				emit needRepaint();
		}
void	PageViewMode_NewObject::mouseMoveWithPressedLeftButton ( QMouseEvent * e, QPainter * p, QWidget * /* w */ )
		{
			if (p)
				drawingObject->drawObject( *p, pressPosition, releasePosition );		// undraw

			releasePosition = e->pos();

			if (p)
				drawingObject->drawObject( *p, pressPosition, releasePosition );		// draw new
			else
				emit needRepaint();
		}

void	PageViewMode_NewObject::repaint ( QPainter & p, QWidget * w  )
		{
			if ((isPressedLeftButton) && (isMoving))
				drawingObject->drawObject( p, pressPosition, releasePosition );
			else
				this->PageViewMode::repaint( p, w );
		}
// ----------------------------------  PageViewMode_TextSelection  --------------------------- //
PageViewMode_TextSelection::PageViewMode_TextSelection ( const QString & drawingObject,
														const QString & _scriptFncAtMouseRelease,
														const QString & _scriptFncAtMoveSelectedObjects,
														const QString & _scriptFncAtResizeSelectedObjects ) :
			PageViewMode ( drawingObject,
							_scriptFncAtMouseRelease,
							_scriptFncAtMoveSelectedObjects,
							_scriptFncAtResizeSelectedObjects )
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

			arrayOfBBoxes.clear();

			this->PageViewMode::clearWorkOperators();
		};
void PageViewMode_TextSelection::clearSelectedOperators ()
		{
			firstSelectedObject = NULL;
			lastSelectedObject = NULL;

			this->PageViewMode::clearSelectedOperators();
		};
void PageViewMode_TextSelection::addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps )
		{
			if (wOps.empty())
				return;

			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			std::vector< boost::shared_ptr< PdfOperator > >					hOps;
			std::vector< boost::shared_ptr< PdfOperator > >::const_iterator	wOps_iter = wOps.begin();
			for ( ; wOps_iter != wOps.end() ; ++wOps_iter ) {
				// select only text operators
				TextOperatorIterator textIter ( * wOps_iter );

				if ((! textIter.isEnd()) && (textIter.getCurrent() == (* wOps_iter))) {
					hOps.push_back( * wOps_iter );

					QRect r = getBBox( * wOps_iter );
					boost::shared_ptr<PdfOperator> o = * wOps_iter;
					arrayOfBBoxes.myAppend( new BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> >( r, o ) );
				}
			}

			arrayOfBBoxes.initAllBBoxPtr();

			this->PageViewMode::addWorkOperators ( hOps );
		};
void PageViewMode_TextSelection::addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps )
		{
			if (sOps.empty())
				return;

			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			std::vector< boost::shared_ptr< PdfOperator > >					hOps;
			std::vector< boost::shared_ptr< PdfOperator > >::const_iterator	sOps_iter = sOps.begin();
			for ( ; sOps_iter != sOps.end() ; ++sOps_iter ) {
				// select only text operators
				TextOperatorIterator textIter ( * sOps_iter );
				if ((! textIter.isEnd()) && (textIter.getCurrent() == (* sOps_iter)))
					hOps.push_back( * sOps_iter );
			}

			this->PageViewMode::addSelectedOperators ( hOps );
		};

const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * PageViewMode_TextSelection::getNearestObject( const QPoint & point )
		{
			if (arrayOfBBoxes.isEmpty())
				return NULL;

			RectArray< boost::shared_ptr<PdfOperator> > * line = arrayOfBBoxes.first();
			for ( ; line ; line = arrayOfBBoxes.next() ) {
				if (line->getMaxY() < point.y())
					continue ;
				if (line->getMinY() > point.y())
					break ;
				
				BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > * nearest = line->first();
				for ( ; nearest ; nearest = line->next() ) {
					if (nearest->right() < point.x())
						continue;
					if (nearest->left() > point.x())
						break;
					if (nearest->contains( point ))
						return nearest;
				}
			}

			return NULL;
		};

void PageViewMode_TextSelection::mousePressLeftButton ( QMouseEvent * e, QPainter * p, QWidget * /* w */ )
		{
			if (workOpRegion.contains( e->pos() ))
				firstSelectedObject = getNearestObject( e->pos() );
			else
				firstSelectedObject = NULL;

			lastSelectedObject = firstSelectedObject;
			if (firstSelectedObject) {
				mouseSelectedRegion = QRegion( * firstSelectedObject );
				if (p)
					drawingObject->drawObject( *p, mouseSelectedRegion );	// draw new selected object
			} else {
				isPressedLeftButton = false;
				if (p)
					drawingObject->drawObject( *p, selectedOpRegion );		// draw old selected object
			}

			if (p == NULL)
				emit needRepaint();
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

void PageViewMode_TextSelection::reorderSelectedOp()
		{
			if (firstSelectedObject && lastSelectedObject && (*firstSelectedObject > *lastSelectedObject) ) {
				std::vector< boost::shared_ptr< PdfOperator > >		h_sop;
				h_sop.swap( selectedOperators );
				selectedOperators.insert( selectedOperators.begin(), h_sop.rbegin(), h_sop.rend() );
			}
		};
void PageViewMode_TextSelection::mouseReleaseLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			assert( firstSelectedObject );
			assert( lastSelectedObject );

			if (p)
				drawingObject->drawObject( *p, mouseSelectedRegion );		// undraw

			// actualize selected operators
			updateSelection( firstSelectedObject, lastSelectedObject,  & selectedOpRegion, & selectedOperators );

			reorderSelectedOp ();
			emit newSelectedOperators( selectedOperators );

			if (p == NULL)
				emit needRepaint();
		};
void PageViewMode_TextSelection::mouseMoveWithPressedLeftButton ( QMouseEvent * e, QPainter * p, QWidget * /* w */ )
		{
			if (p)
				drawingObject->drawObject( *p, mouseSelectedRegion );		// undraw

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

			if (p)
				drawingObject->drawObject( *p, mouseSelectedRegion );		// draw
			else
				emit needRepaint();
		};

void PageViewMode_TextSelection::repaint ( QPainter & p, QWidget * w  )
		{
			if ((isPressedLeftButton) && (! isMoving) && (! isResizing))
				drawingObject->drawObject( p, mouseSelectedRegion );
			else
				this->PageViewMode::repaint( p, w );
		};

// ----------------------------------  PageViewMode_OperatorsSelection  --------------------------- //
PageViewMode_OperatorsSelection::PageViewMode_OperatorsSelection ( const QString & drawingObject,
																	const QString & _scriptFncAtMouseRelease,
																	const QString & _scriptFncAtMoveSelectedObjects,
																	const QString & _scriptFncAtResizeSelectedObjects ) :
			PageViewMode ( drawingObject,
							_scriptFncAtMouseRelease,
							_scriptFncAtMoveSelectedObjects,
							_scriptFncAtResizeSelectedObjects )
		{
			lastSelectedOperator = workOperators.end();
		};

void	PageViewMode_OperatorsSelection::findOperators (	const std::vector< boost::shared_ptr< PdfOperator > >	& in_v,
															std::vector< boost::shared_ptr< PdfOperator > >			& founded,
															const QRegion	& r )
		{
				std::vector< boost::shared_ptr< PdfOperator > >::const_iterator		h_it = in_v.begin();

				for ( ; h_it != in_v.end() ; ++h_it ) {
					QRegion bbox ( getBBox( * h_it ) );
					if ( ((r & bbox) ^ bbox).isEmpty() )
						founded.push_back( * h_it );
				}
		}

bool	PageViewMode_OperatorsSelection::findPrevOperator ( std::vector< boost::shared_ptr< PdfOperator > >::iterator	& it,
															std::vector< boost::shared_ptr< PdfOperator > >				& v,
															bool 			& fromEnd,
															const QPoint	& p )
		{
				std::vector< boost::shared_ptr< PdfOperator > >::iterator	h_it = it;

				fromEnd = false;

				if (v.empty()) {
					fromEnd = true;
					return false;
				}

				if (it != v.begin()) {
					do {
						--it;
						if (getBBox( * it ).contains( p ))
							return true;
					} while ( it != v.begin() );
				}

				fromEnd = true;

				it = v.end();
				do {
					--it;
					if (getBBox( * it ).contains( p ))
						return true;
				} while ( (it != h_it) && (it != v.begin()) );

				return false;
		}

void	PageViewMode_OperatorsSelection::mousePressLeftButton ( QMouseEvent * e, QPainter * p, QWidget * /* w */ )
		{
			pressPosition = releasePosition = e->pos();
			if (p)
				drawingObject->drawObject( *p, e->pos(), e->pos() );
			else
				emit needRepaint();
		}
void	PageViewMode_OperatorsSelection::mouseReleaseLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			if (p)
				drawingObject->drawObject( *p, pressPosition, releasePosition );		// undraw

			releasePosition = e->pos();

			lastSelectedOperator = workOperators.end();

			std::vector< boost::shared_ptr< PdfOperator > >		h_v;
			if ( pressPosition != releasePosition ) {
				QRegion h_r ( QRect( pressPosition, releasePosition ).normalize() );
				findOperators ( workOperators, h_v, h_r );
			} else {
				bool fromEnd;
				if ( findPrevOperator ( lastSelectedOperator, workOperators, fromEnd, releasePosition ) ) {
					h_v.push_back( * lastSelectedOperator );
				}
			}
			setSelectedOperators ( h_v );

			emit newSelectedOperators( selectedOperators );

			// execute skript command
			this->PageViewMode::mouseReleaseLeftButton (e, p, w);

			if (p == NULL)
				emit needRepaint();
		}
void	PageViewMode_OperatorsSelection::movedSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			// if is only clicked
			if ( mouseSelectedRegion.boundingRect().normalize().topLeft() ==
				 selectedOpRegion.boundingRect().normalize().topLeft() )
			{
				std::vector< boost::shared_ptr< PdfOperator > >		h_v;
				bool fromEnd;
				if ( findPrevOperator ( lastSelectedOperator, workOperators, fromEnd, e->pos() ) ) {
					h_v.push_back( * lastSelectedOperator );
				}
				setSelectedOperators ( h_v );

				emit newSelectedOperators( selectedOperators );
			} else {
				this->PageViewMode::movedSelectedObjects ( e, p, w );
			}
		}
void	PageViewMode_OperatorsSelection::resizeSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			// if isResizing == false then user press left mouse button now
			if (! isResizing)
				resizingPress = e->pos();

			this->PageViewMode::resizeSelectedObjects ( e, p, w );
		}
void	PageViewMode_OperatorsSelection::resizedSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			// if is only clicked
			if ( resizingPress == e->pos()  )
			{
				std::vector< boost::shared_ptr< PdfOperator > >		h_v;
				bool fromEnd;
				if ( findPrevOperator ( lastSelectedOperator, workOperators, fromEnd, e->pos() ) ) {
					h_v.push_back( * lastSelectedOperator );
				}
				setSelectedOperators ( h_v );

				emit newSelectedOperators( selectedOperators );
			} else {
				this->PageViewMode::resizedSelectedObjects ( e, p, w );
			}
		}
void	PageViewMode_OperatorsSelection::mouseMoveWithPressedLeftButton ( QMouseEvent * e, QPainter * p, QWidget * /* w */ )
		{
			if (p)
				drawingObject->drawObject( *p, pressPosition, releasePosition );		// undraw

			releasePosition = e->pos();

			if (p)
				drawingObject->drawObject( *p, pressPosition, releasePosition );		// draw new
			else
				emit needRepaint();
		}

void	PageViewMode_OperatorsSelection::setSelectedRegion ( QRegion r )
		{
			std::vector< boost::shared_ptr< PdfOperator > >		h_v;
			findOperators ( workOperators, h_v, r );
			setSelectedOperators ( h_v );

			emit newSelectedOperators( selectedOperators );
		}

void	PageViewMode_OperatorsSelection::addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps )
		{
			this->PageViewMode::addWorkOperators ( wOps );

			lastSelectedOperator = workOperators.end();
		}
void	PageViewMode_OperatorsSelection::clearWorkOperators ()
		{
			this->PageViewMode::clearWorkOperators ();

			lastSelectedOperator = workOperators.end();
		}

// ----------------------------------  PageViewMode_Annotations  --------------------------- //
PageViewMode_Annotations::PageViewMode_Annotations ( const QString & drawingObject,
														const QString & _scriptFncAtMouseRelease,
														const QString & _scriptFncAtMoveSelectedObjects,
														const QString & _scriptFncAtResizeSelectedObjects ) :
			PageViewMode ( drawingObject,
							_scriptFncAtMouseRelease,
							_scriptFncAtMoveSelectedObjects,
							_scriptFncAtResizeSelectedObjects )
		{
			// initialize mapping cursors
			setMappingCursor();
		};

void PageViewMode_Annotations::setMappingCursor()
		{
			this->PageViewMode::setMappingCursor();

			mappingResizingModeToCursor [ left | right | top | bottom ] = Qt::PointingHandCursor;
			mappingResizingModeToCursor [ onUnselectedObject ] = Qt::PointingHandCursor;
		};

void PageViewMode_Annotations::setWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & /* wOps */ )
		{};
void PageViewMode_Annotations::addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & /* wOps */ )
		{};
void PageViewMode_Annotations::clearWorkOperators ()
		{};
void PageViewMode_Annotations::clearSelectedOperators ()
		{};
void PageViewMode_Annotations::setSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & /* sOps */ )
		{};
void PageViewMode_Annotations::addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & /* sOps */ )
		{};
void PageViewMode_Annotations::actualizeSelection ()
		{};

libs::Rectangle PageViewMode_Annotations::getRectOfAnnotation ( boost::shared_ptr<CAnnotation> & annot )
		{
			libs::Rectangle rc;

			boost::shared_ptr< CDict >	dictionary = annot->getDictionary();
			try {
				boost::shared_ptr<IProperty>	prop = dictionary->getProperty("Rect");
				boost::shared_ptr<CArray>		rect= prop->getSmartCObjectPtr<CArray> (prop);
				//if (prop->getType == ) = prop->getCObjectFromRef<CArray> (prop);

				rc.xleft	= getDoubleFromArray (rect, 0);
				rc.yleft	= getDoubleFromArray (rect, 1);
				rc.xright	= getDoubleFromArray (rect, 2);
				rc.yright	= getDoubleFromArray (rect, 3);
			} catch (ElementNotFoundException) {
				// Rect is requirement.
				guiPrintDbg( debug::DBG_WARN, "For Annotation is property 'Rect' requirement but is missing !!!" );
				rc.xleft	= -1;
				rc.yleft	= -1;
				rc.xright	= -1;
				rc.yright	= -1;
			}

			return rc;
		};
void PageViewMode_Annotations::extraInitialize( const boost::shared_ptr< CPage > & page, const DisplayParams & displayParams )
		{
			if (page == NULL)
				return;

			selectedOpRegion	= QRegion();
			workOpRegion		= QRegion();
			annotations.clear();

			// get all annotations
			std::vector< boost::shared_ptr< CAnnotation > >	annots;
			page->getAllAnnotations ( annots );

			const boost::shared_ptr< CDict >							dictionary;
			std::vector< boost::shared_ptr< CAnnotation > >::iterator	it_annots = annots.begin();
			for ( ; it_annots != annots.end() ; ++it_annots )
			{
				libs::Rectangle rc = getRectOfAnnotation( * it_annots );
				displayParams.convertPdfPosToPixmapPos( rc.xleft, rc.yleft, rc.xleft, rc.yleft );
				displayParams.convertPdfPosToPixmapPos( rc.xright, rc.yright, rc.xright, rc.yright );
				QRect rr;
				rr.setCoords((int) rc.xleft, (int) rc.yleft, (int) rc.xright, (int) rc.yright );
				QRegion r ( rr.normalize() );

				workOpRegion += r;
				
				annot_rect ar;
				ar.annot = * it_annots;
				ar.activation_region = r;	// TODO use correct activation region
				annotations.push_back( ar );
			}
		};
void PageViewMode_Annotations::repaint ( QPainter & p, QWidget * w )
		{
			drawingObject->drawObject( p, workOpRegion );

			if (isPressedLeftButton && (isMoving || isResizing))
			{
				if (isMoving)
					drawingObject->drawObject( p, mouseSelectedRegion.boundingRect() );
				else
					this->PageViewMode::repaint ( p, w );
			} else
				if (! selectedOpRegion.isEmpty()) {
					QRect r = selectedOpRegion.boundingRect();
					drawingObject->drawObject( p, r.topLeft(), r.bottomRight() );
				}
		};
PageViewMode_Annotations::annot_rect PageViewMode_Annotations::getAnnotationOnPosition ( const QPoint & p )
		{
			std::vector< annot_rect >::iterator		it_annot = annotations.begin();
			for ( ; it_annot != annotations.end() ; ++it_annot )
				if ( (*it_annot).activation_region.contains( p ) )
					return (* it_annot);

			annot_rect h_ar;
			h_ar.annot.reset ();
			h_ar.activation_region = QRegion();
			return h_ar;
		};
void PageViewMode_Annotations::mouseMoveEvent ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			annot_rect	h_ar = getAnnotationOnPosition( e->pos() );

			std::vector< boost::shared_ptr< CAnnotation > >	h_v;
			if (h_ar.annot)
				h_v.push_back( h_ar.annot );

			if ((! h_ar.activation_region.isEmpty()) && (selectedOpRegion != h_ar.activation_region))
			{
				selectedOpRegion = h_ar.activation_region;
				emit newSelectedAnnotations( h_v );
				emit needRepaint();
			}

			if (isPressedLeftButton && ! (isMoving || isResizing))
				isPressedLeftButton = false;
					
			this->PageViewMode::mouseMoveEvent( e, p, w );
		};
void PageViewMode_Annotations::movedSelectedObjects ( QPoint relativeMove )
		{
			if (relativeMove == QPoint(0,0)) {
				emit executeCommand ( scriptFncAtMouseRelease	.arg( relativeMove.x() )
																.arg( relativeMove.y() )
																.arg( relativeMove.x() )
																.arg( relativeMove.y() )
																.arg( relativeMove.x() )
																.arg( relativeMove.y() ) );
			} else
				this->PageViewMode::movedSelectedObjects( relativeMove );
		};
void PageViewMode_Annotations::resizedSelectedObjects ( int dleft, int dtop, int dright, int dbottom )
		{
			if (dleft == 0 && dtop == 0 && dright == 0 && dbottom == 0) {
				emit executeCommand ( scriptFncAtResizeSelectedObjects	.arg( releasePosition.x() )
																		.arg( releasePosition.y() )
																		.arg( releasePosition.x() )
																		.arg( releasePosition.y() ) );
			} else
				this->PageViewMode::resizedSelectedObjects ( dleft, dtop, dright, dbottom );
		};


// ----------------------------------  PageViewMode_TextMarking  --------------------------- //
PageViewMode_TextMarking::PageViewMode_TextMarking ( const QString & drawingObject,
														const QString & _scriptFncAtMouseRelease,
														const QString & _scriptFncAtMoveSelectedObjects,
														const QString & _scriptFncAtResizeSelectedObjects )
		: PageViewMode_TextSelection( drawingObject,
										_scriptFncAtMouseRelease,
										_scriptFncAtMoveSelectedObjects,
										_scriptFncAtResizeSelectedObjects )
		{
			// initialize mapping cursors
			setMappingCursor();
		};

void PageViewMode_TextMarking::setMappingCursor()
		{
			this->PageViewMode::setMappingCursor();

			mappingResizingModeToCursor [ left | right | top | bottom ] = Qt::PointingHandCursor;
			mappingResizingModeToCursor [ onUnselectedObject ] = Qt::PointingHandCursor;
		};
void PageViewMode_TextMarking::addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & /* sOps */ )
		{};
void PageViewMode_TextMarking::mouseReleaseLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w )
		{
			this->PageViewMode_TextSelection::mouseReleaseLeftButton ( e, p, w );

			clearSelectedOperators ();

			this->PageViewMode::mouseReleaseLeftButton ( e, p, w );
		}

// ----------------------------------  PageViewMode_GraphicalOperatorsSelection  --------------------------- //
PageViewMode_GraphicalOperatorsSelection::PageViewMode_GraphicalOperatorsSelection
				( const QString & drawingObject,
					const QString & _scriptFncAtMouseRelease,
					const QString & _scriptFncAtMoveSelectedObjects,
					const QString & _scriptFncAtResizeSelectedObjects ) :
			PageViewMode_OperatorsSelection ( drawingObject,
												_scriptFncAtMouseRelease,
												_scriptFncAtMoveSelectedObjects,
												_scriptFncAtResizeSelectedObjects )
		{};
void PageViewMode_GraphicalOperatorsSelection::addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps )
		{
			if (sOps.empty())
				return;

			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			std::vector< boost::shared_ptr< PdfOperator > >					hOps;
			std::vector< boost::shared_ptr< PdfOperator > >::const_iterator	sOps_iter = sOps.begin();
			for ( ; sOps_iter != sOps.end() ; ++sOps_iter ) {
				// select only graphical operators
				GraphicalOperatorIterator iter ( * sOps_iter );
				if ((! iter.isEnd()) && (iter.getCurrent() == (* sOps_iter)))
					hOps.push_back( * sOps_iter );
			}

			this->PageViewMode_OperatorsSelection::addSelectedOperators ( hOps );
		};
void PageViewMode_GraphicalOperatorsSelection::addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps )
		{
			if (wOps.empty())
				return;

			if (isPressedLeftButton && (isResizing || isMoving))
				isPressedLeftButton = false;

			std::vector< boost::shared_ptr< PdfOperator > >					hOps;
			std::vector< boost::shared_ptr< PdfOperator > >::const_iterator	wOps_iter = wOps.begin();
			for ( ; wOps_iter != wOps.end() ; ++wOps_iter ) {
				// select only graphical operators
				GraphicalOperatorIterator iter ( * wOps_iter );

				if ((! iter.isEnd()) && (iter.getCurrent() == (* wOps_iter)))
					hOps.push_back( * wOps_iter );
			}

			this->PageViewMode_OperatorsSelection::addWorkOperators ( hOps );
		};
} // namespace gui
