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
#ifndef __PAGEVIEWMODE_H__
#define __PAGEVIEWMODE_H__

#include <qpainter.h>
#include <qevent.h>
#include <qwidget.h>
#include <vector>
#include <qpoint.h>
#include "rect2Darray.h"

/*class pdfobjects::CAnnotation;
class pdfobjects::CPage;
class pdfobjects::PdfOperator;*/
#include "kernel/cpage.h"
#include "kernel/cannotation.h"
#include "kernel/pdfoperators.h"
#include "kernel/static.h"

using namespace pdfobjects;

namespace gui {

//  ---------------------   drawing objects  ----------------- //
class DrawingObject;

class DrawingObjectFactory {
	public:
			static DrawingObject * create( const QString & nameOfObject );
};

/** Class is use as STRATEGY patern to drawing objects in mode.
 * This class define interface and base functionality.
 */
class DrawingObject {
	public:
			/** Drawing object useing \a painter from \a p1 to \a p2.
			 * @param painter initialize painter for drawing.
			 * @param p1 start point
			 * @param p2 end point
			 *
			 * This method is mostly useing if don't moving or resizing and is pressed left button.
			 */
			virtual void drawObject ( QPainter & painter, QPoint p1, QPoint p2 );
			/** Drawing region \a reg useing \a painter for draw.
			 * @param painter initialize painter for drawing.
			 * @param reg region for drawing.
			 *
			 * This method is mostly useing for drawing selected operators (nothing button is press or in text mode).
			 */
			virtual void drawObject ( QPainter & painter, QRegion reg );
			/** Drawing rectangle \a rect useing \a painter for draw.
			 * @param painter initialize painter for drawing.
			 * @param rect rectangle for drawing.
			 *
			 * This method is mostly useing for drawing selected bounding rectangle (moving and resizing selected region).
			 */
			virtual void drawObject ( QPainter & painter, QRect rect );

			/** Standard constructor.
			 * Initialize pen.
			 */
			DrawingObject();
			/** Standard destructor. */
			virtual ~DrawingObject ();
	protected:
			/** Pen for drawing line (color, width, solid or ...) */
			QPen	pen;
};

/** Class is STRATEGY pattern to draw line as new object.
 * (Resizing, moving and draw selected region is keeping from parent (see DrawingObject).)
 */
class DrawingLine: public DrawingObject {
	public:
			/** Standard constructor.
			 * Initialize pen.
			 */
			DrawingLine ();
			/** Standard destructor. */
			virtual ~DrawingLine ();

			/** Drawing object useing \a painter from \a p1 to \a p2.
			 * @param painter initialize painter for drawing.
			 * @param p1 start point
			 * @param p2 end point
			 *
			 * This method is mostly useing if don't moving or resizing and is pressed left button.
			 */
			virtual void drawObject ( QPainter & painter, QPoint p1, QPoint p2 );
};

/** Class is STRATEGY pattern to draw rectangle as new object.
 * (Resizing, moving and draw selected region is keeping from parent (see DrawingObject).)
 */
class DrawingRect: public DrawingObject {
	public:
			/** Standard constructor.
			 * Initialize pen.
			 */
			DrawingRect ();
			/** Standard destructor. */
			virtual ~DrawingRect ();

			/** Drawing object useing \a painter from \a p1 to \a p2.
			 * @param painter initialize painter for drawing.
			 * @param p1 start point
			 * @param p2 end point
			 *
			 * This method is mostly useing if don't moving or resizing and is pressed left button.
			 */
			virtual void drawObject ( QPainter & painter, QPoint p1, QPoint p2 );
};

/** Class is STRATEGY pattern to draw rectangle as new object.
 * (Resizing, moving and draw selected region is keeping from parent (see DrawingRect).)
 * Different between this class and DrawingRect (it's parent) is in draw selected region without
 * bounding-rectangle.
 */
class DrawingRect2: public DrawingRect {
	public:
			/** Standard constructor.
			 * Initialize pen.
			 */
			DrawingRect2 ();
			/** Standard destructor. */
			virtual ~DrawingRect2 ();

			/** Drawing region \a reg useing \a painter for draw.
			 * @param painter initialize painter for drawing.
			 * @param reg region for drawing.
			 *
			 * This method is mostly useing for drawing selected operators (nothing button is press or in text mode).
			 */
			virtual void drawObject ( QPainter & painter, QRegion reg );
};

/** Class is STRATEGY pattern to draw rectangle as new object.
 * (Resizing, moving and draw selected region is keeping from parent (see DrawingRect).)
 * Different between this class and DrawingRect (it's parent) is in draw selected region with
 * only bounding-rectangle (not fill). And draw rect from point to point is fill rectangle.
 */
class DrawingRect3: public DrawingRect {
	public:
			/** Standard constructor.
			 * Initialize pen.
			 */
			DrawingRect3 ();
			/** Standard destructor. */
			virtual ~DrawingRect3 ();

			/** Drawing object useing \a painter from \a p1 to \a p2.
			 * @param painter initialize painter for drawing.
			 * @param p1 start point
			 * @param p2 end point
			 *
			 * This method is mostly useing if don't moving or resizing and is pressed left button.
			 */
			virtual void drawObject ( QPainter & painter, QPoint p1, QPoint p2 );
			/** Drawing region \a reg useing \a painter for draw.
			 * @param painter initialize painter for drawing.
			 * @param reg region for drawing.
			 *
			 * This method is mostly useing for drawing selected operators (nothing button is press or in text mode).
			 */
			virtual void drawObject ( QPainter & painter, QRegion reg );
			/** Drawing rectangle \a rect useing \a painter for draw.
			 * @param painter initialize painter for drawing.
			 * @param rect rectangle for drawing.
			 *
			 * This method is mostly useing for drawing selected bounding rectangle (moving and resizing selected region).
			 */
			virtual void drawObject ( QPainter & painter, QRect rect );
};
//  ---------------------  selection mode  --------------------- //
class PageViewMode;

class PageViewModeFactory {
	public:
			static PageViewMode * create(	const QString & nameOfMode,
											const QString & drawingObject,
											const QString & scriptFncAtMouseRelease,
											const QString & scriptFncAtMoveSelectedObjects,
											const QString & scriptFncAtResizeSelectedObjects );
};

/** Class is STRATEGY pattern for mode construction.
 * Includes method for selecting objects on page and manipulating with him.
 */
class PageViewMode: public QObject {
	Q_OBJECT
	signals:
			/** Signal generated by new selection known operators.
			 * @param  objects Operators which are selected.
			 */
			void newSelectedOperators( const std::vector< boost::shared_ptr< PdfOperator > > & objects );
			/** Signal generated by new selection known annotations.
			 * @param  objects Annotations which are selected.
			 */
			void newSelectedAnnotations( const std::vector< boost::shared_ptr< CAnnotation > > & objects );

			/** Signal is generated if page need repaint after changes. */
			void needRepaint ( );
			/** Signal is generated when scrolling of the page should be done.
			 * @param x amount to scroll to left (can be negative to scroll to right)
			 * @param y amount to scroll to up (can be negative to scroll to bottom)
			 * Note: notion of left/right/up/down may be a bit counter-intuitive.
			 * If we press right arrow key, we want to scroll "to left"
			 */
			void scroll ( int x, int y );
			/** Signal is generated after right mouse button is released.
			 * @param PagePos Mouse position on page.
			 */
			void popupMenu ( const QPoint & PagePos /*, Cobject & */ );
			/** Signal is generated, when command \a cmd is to be executed in scripting.
			 * @param cmd Script command for executing.
			 */
			void executeCommand ( QString cmd );

			/** Signal is generated if need delete selected objects (e.g. was pressed Delete). */
			void deleteSelection ( );
	public slots:
			/** Method is calling if is need move selected region (operation 'move' is finished).
			 * @param relativeMove relativ move x and y position of selected region.
			 *
			 * This method call script.
			 *
			 * @see movedSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void movedSelectedObjects ( QPoint relativeMove );
			/** Method is calling if is need resize selected region (operation 'resize' is finished).
			 * @param dleft		delta position of bounding-rectangle's left edge of selected operators.
			 * @param dtop		delta position of bounding-rectangle's top edge of selected operators.
			 * @param dright	delta position of bounding-rectangle's right edge of selected operators.
			 * @param dbottom	delta position of bounding-rectangle's bottom edge of selected operators.
			 *
			 * This method call script.
			 *
			 * @see resizedSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void resizedSelectedObjects ( int dleft, int dtop, int dright, int dbottom );

			/** Method is calling if is need move selected region (operation 'move' is NOT finished).
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see mouseReleaseLeftButton
			 * @see mouseMoveWithPressedLeftButton
			 *
			 * @see movedSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void moveSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is need move selected region (operation 'move' is finished).
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see movedSelectedObjects( QPoint )
			 * @see moveSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void movedSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w );

			/** Method is calling if is need resize selected region (operation 'resize' is NOT finished).
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see mouseReleaseLeftButton
			 * @see mouseMoveWithPressedLeftButton
			 *
			 * @see resizedSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void resizeSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is need resize selected region (operation 'resize' is finished).
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see resizeSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 * @see resizedSelectedObjects ( int, int, int, int )
			 */
			virtual void resizedSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w );

			/* ------------------------------------------------------------------------------ *
			 * --- mouse press (and coresponding release) events not above selection area --- *
			 * ------------------------------------------------------------------------------ */

			/** Method is calling if is press left mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mouseReleaseLeftButton
			 * @see mousePressEvent
			 *
			 * Editing this function is the best way to create new mode.
			 */
			virtual void mousePressLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is release left mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see mousePressEvent
			 *
			 * Editing this function is the best way to create new mode.
			 */
			virtual void mouseReleaseLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is press right mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mouseReleaseRightButton
			 * @see mousePressEvent
			 *
			 * Editing this function is the best way to create new mode.
			 */
			virtual void mousePressRightButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is release right mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressRightButton
			 * @see mousePressEvent
			 *
			 * Editing this function is the best way to create new mode.
			 */
			virtual void mouseReleaseRightButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is press middle mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mouseReleaseMidButton
			 * @see mousePressEvent
			 *
			 * Editing this function is the best way to create new mode.
			 */
			virtual void mousePressMidButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is release middle mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressMidButton
			 * @see mousePressEvent
			 *
			 * Editing this function is the best way to create new mode.
			 */
			virtual void mouseReleaseMidButton ( QMouseEvent * e, QPainter * p, QWidget * w );

			/** Method is calling if is press left mouse button and is moving with mouse.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressEvent
			 *
			 * Editing this function is the best way to create new mode.
			 */
			virtual void mouseMoveWithPressedLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );

			/* ------------------------------------------------------------------------------ *
			 * ---        mouse events - equivalents of method called in QWidget          --- *
			 * ------------------------------------------------------------------------------ */

			/** Method is call if press mouse button (see Qt::QWidget::mousePressEvent)
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Is switch which call other method. For press left button is settings \a isPressedLeftButton, \a isMoving,
			 * \a isResizing, \a pomCur and undrawing old selectedOpRegion.
			 *
			 * @see mousePressLeftButton ( QMouseEvent *, QPainter *, QWidget * )
			 * @see mousePressRightButton ( QMouseEvent *, QPainter *, QWidget * )
			 * @see mousePressMidButton ( QMouseEvent *, QPainter *, QWidget * )
			 * @see moveSelectedObjects ( QMouseEvent *, QPainter *, QWidget * )
			 * @see resizeSelectedObjects ( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void mousePressEvent ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is call if release mouse button (see Qt::QWidget::mouseReleaseEvent)
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Is switch which call other method. For press left button is settings \a isPressedLeftButton and drawing
			 * selectedOpRegion.
			 *
			 * @see mouseReleaseLeftButton ( QMouseEvent *, QPainter *, QWidget * )
			 * @see mouseReleaseRightButton ( QMouseEvent *, QPainter *, QWidget * )
			 * @see mouseReleaseMidButton ( QMouseEvent *, QPainter *, QWidget * )
			 * @see movedSelectedObjects ( QMouseEvent *, QPainter *, QWidget * )
			 * @see resizedSelectedObjects ( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void mouseReleaseEvent ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is call if mouse double-click (see Qt::QWidget::mouseDoubleClickEvent)
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Call mousePressEvent.
			 *
			 * @see mousePressEvent ( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void mouseDoubleClickEvent ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is call if mouse is moving (see Qt::QWidget::mouseMoveEvent)
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Is switch which call other method. Is settings \a pomCur and mouse cursor for widget \a w.
			 *
			 * @see moveSelectedObjects ( QMouseEvent *, QPainter *, QWidget * )
			 * @see resizeSelectedObjects ( QMouseEvent *, QPainter *, QWidget * )
			 * @see mouseMoveWithPressedLeftButton ( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void mouseMoveEvent ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is call if some wheel event (see Qt::QWidget::wheelEvent)
			 * @param e		Pointer to wheel event (see Qt::QWheelEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Do nothing in this mode.
			 */
			virtual void wheelEvent ( QWheelEvent * e, QPainter * p, QWidget * w );

			/* ------------------------------------------------------------------------------ *
			 * ---         key events - equivalents of method called in QWidget           --- *
			 * ------------------------------------------------------------------------------ */

			/** Method is call if press key event (see Qt::QWidget::keyPressEvent)
			 * @param e		Pointer to key event (see Qt::QKeyEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Do nothing in this mode.
			 */
			virtual void keyPressEvent ( QKeyEvent * e, QPainter * p, QWidget * w );
			/** Method is call if release key event (see Qt::QWidget::keyReleaseEvent)
			 * @param e		Pointer to key event (see Qt::QKeyEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Esc and Delete key functionality is implemented.
			 */
			virtual void keyReleaseEvent ( QKeyEvent * e, QPainter * p, QWidget * w );

			/* ------------------------------------------------------------------------------ *
			 * ---        focus events - equivalents of method called in QWidget          --- *
			 * ------------------------------------------------------------------------------ */

			/** Method is call if focus-in event (see Qt::QWidget::focusInEvent)
			 * @param e		Pointer to focus event (see Qt::QFocusEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Do nothing in this mode.
			 */
			virtual void focusInEvent ( QFocusEvent * e, QPainter * p, QWidget * w );
			/** Method is call if focus-out event (see Qt::QWidget::focusOutEvent)
			 * @param e		Pointer to focus event (see Qt::QFocusEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Do nothing in this mode.
			 */
			virtual void focusOutEvent ( QFocusEvent * e, QPainter * p, QWidget * w );

			/** Repaint method for draw actual state of mode (selected region, moving and resizing in action, ...).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * It is only drawing method use, if viewing parent need clear repaint.
			 */
			virtual void repaint ( QPainter & p, QWidget * w  );

			/** Function return region of selected objects on the page.
			 * @return Region of selected objects.
			 */
			virtual QRegion getSelectedRegion ();
			/** Set selection region on the page.
			 * @param r Which region to set.
			 */
			virtual void setSelectedRegion ( QRegion r );

			/** Set operators for selection on the page.
			 * @param wOps Vector of operators.
			 */
			virtual void setWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps );
			/** Add operators for selection on the page.
			 * @param wOps Vector of operators.
			 */
			virtual void addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps );
			/** Clear all operators which acan be possible select on the page. */
			virtual void clearWorkOperators ();
			/** Clear all operators which are selected on the page. */
			virtual void clearSelectedOperators ();
			/** Set operators selected on the page.
			 * @param sOps Vector of operators.
			 */
			virtual void setSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps );
			/** Add operators selected on the page.
			 * @param sOps Vector of operators.
			 */
			virtual void addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps );
			/** Actualize selected operators on the page.
			 * This method is very very well call after change operators for posiible selection.
			 * If any selected operators are not longer inter operators which are possible select, are removed from
			 * selected operators. Otherwise stay selected.
			 */
			virtual void actualizeSelection ();

			/** Extra initialization mode.
			 * @param page			Page which are viewed.
			 * @param displayParams	Display parameters viewed page.
			 *
			 * This method is good called after change viewed page, or change display parameters (zoom, rotation, ...)
			 * Is good for initialize other objects then operators (E.g. for annotation).
			 */
			virtual void extraInitialize ( const boost::shared_ptr< CPage > & page, const DisplayParams & displayParams );
			/** This method emit all selected Objects.
			 * In mode work with operators emits signal newSelectedOperators with all selected operators.
			 * In mode work with annotations emit signal newSelectedAnnotations with all selected annotations.
			 */
			virtual void sendAllSelectedOperators ();
	public:
			/** Function return if some object is selected (not in selection mode PageView::SelectRect)
			 * @return Return TRUE, if some object is selected. Otherwise return FALSE.
			 */
			virtual bool isSomeoneSelected ();

			/** Method set width of resizing zone
			 * @param width width in pixels
			 *
			 * Default is set to 2.
			 */
			void setResizingZone ( unsigned int width );
			/** Function return actual set width of resizing zone
			 * @return Actual set resizing zone
			 */
			int getResizingZone ( );

			/** Standard constructor.
			 * @param drawingObject						Text definition of drawing method (see DrawingObjectFactory)
			 * @param _scriptFncAtMouseRelease			Script command for call after selected object(s)
			 * @param _scriptFncAtMoveSelectedObjects	Script command for call after move selected object(s)
			 * @param _scriptFncAtResizeSelectedObjects	Script command for call after resize selected object(s)
			 */
			PageViewMode( const QString & drawingObject,
							const QString & _scriptFncAtMouseRelease,
							const QString & _scriptFncAtMoveSelectedObjects,
							const QString & _scriptFncAtResizeSelectedObjects );

			/** Standard destructor. */
			virtual ~PageViewMode();
	protected:
			/** enum of resizing parts */
			enum resizingParts	{ none = 0, left = 1, right = 2, top = 4, bottom = 8, onUnselectedObject = 16 };
			/** Set mapping cursors for viewing on the page for actual selection mode */
			virtual void setMappingCursor();
			/** Method set currsor to correct for actual situation.
			 * @param point		Actual cursor position.
			 * @param painter	Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 					Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w			Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Cursor is set by \a mappingResizingModeToCursor.
			 * @see setMappingCursor
			 */
			virtual void setCorrectCursor( const QPoint & point, QPainter * painter, QWidget * w );

			/** Method for calculation resizing mode
			 * @param r rectsngle for resizing
			 * @param p cursor position
			 *
			 * @return combination of enum resizingParts
			 */
			virtual int	theNeerestResizingMode ( const QRegion & r, const QPoint & p );

			/** Method add operators bounding boxes to region.
			 * @param r		To which region add operators bounding boxes.
			 * @param ops	Container of operators which bboxes will be added to region \a r.
			 */
			void addOpsBBoxToRegion ( QRegion & r, const std::vector< boost::shared_ptr< PdfOperator > > & ops );
			/** Get bounding box of operator.
			 * @param op	Operator which bbox is required.
			 *
			 * @return	Rectangle = bounding box of operator \a op.
			 */
			QRect getBBox( const boost::shared_ptr<PdfOperator> & op ) const;
	protected:
			/** Vector of operators which will be possible selected. */
			std::vector< boost::shared_ptr< PdfOperator > >		workOperators;
			/** Vector of selected operators. */
			std::vector< boost::shared_ptr< PdfOperator > >		selectedOperators;

			/** Bounding region of possible selected objects. */
			QRegion		workOpRegion;
			/** Bounding region of selected objects */
			QRegion		selectedOpRegion;

			/** Helpes region of temporary selected area on page.
			 * Is using for moving, resizing or new selecting. */
			QRegion		mouseSelectedRegion;
			/** Point of press mouse button on page. */
			QPoint		pressPosition;
			/** Point of release mouse button on page. */
			QPoint		releasePosition;
			/** Relative position in selected area.
			 * Is using for member point in selected area in moving action. */
			QPoint		pointInRect;
			/** Coeficient for multiply with movable X position.
			 * Is using in resizing action. Defined if is resized width. */
			int			resizeCoefficientX;
			/** Coeficient for multiply with movable Y position.
			 * Is using in resizing action. Defined if is resized height. */
			int			resizeCoefficientY;

			/** Is true if was press left mouse button and not release. */
			bool		isPressedLeftButton;
			/** Is true if is actual moved selection area. */
			bool		isMoving;
			/** Is true if is actual resized selection area. */
			bool		isResizing;

			/** Smart pointer for drawing selection area on page.
			 * Drawing object is ceated in constructor.
			 */
			boost::shared_ptr< DrawingObject >			drawingObject;

			/** Width of resizing zone.
			 * If selected area is [ [a,b] , [c,d] ]  ([a,b] is left top edge and
			 *  [c,d] is bottom right edge of selected region) then resizing area is
			 *  [ [a,b] , [c,d] ]  xor  [ [a+resizingZone, b+resizingZone], [c-resizingZone,d-resizingZone] ]
			 *  (c-a) > 2*resizingZone  and  (d-b) > 2*resizingZone
			 */
			int resizingZone;
			/** mapping array resizing mode to cursor shape */
			int mappingResizingModeToCursor [17];

			/** Name of script function, which will call at mouse release.
			 * Must be in this format:
			 * 		"fncName ( %1, %2, %3, %4, %5, %6 )"
			 * where
			 * 		%1	will be replaced with x1 (X position at mouse press)
			 * 		%2	will be replaced with y1 (Y position at mouse press)
			 * 		%3	will be replaced with x2 (X position at mouse release)
			 * 		%4	will be replaced with y2 (Y position at mouse release)
			 * 		%3	will be replaced with global position of x2 (global X position at mouse release)
			 * 		%4	will be replaced with global position of y2 (global Y position at mouse release)
			 *
			 * @see QString::arg
			 *
			 * Function in script:
			 * 			fncName ( x1:Number, y1:Number, x2:Number, y2:Number )  { ... }
			 */
			QString scriptFncAtMouseRelease;
			/** Name of script function, which will call at move selected objects.
			 * Must be in this format:
			 * 		"fncName ( %1, %2 )"
			 * where
			 * 		%1	will be replaced with dx
			 * 		%2	will be replaced with dy
			 *
			 * @see QString::arg
			 *
			 * Function in script:
			 * 			fncName ( dx:Number, dy:Number )  { ... }
			 */
			QString scriptFncAtMoveSelectedObjects;
			/** Name of script function, which will call at resized selected objects.
			 * Must be in this format:
			 * 		"fncName ( %1, %2, %3, %4 )"
			 * where
			 * 		%1	will be replaced with dleft
			 * 		%2	will be replaced with dtop
			 * 		%3	will be replaced with dright
			 * 		%4	will be replaced with dbottom
			 *
			 * @see QString::arg
			 *
			 * Function in script:
			 * 			fncName ( dleft:Number, dtop:Number, dright:Number, dbottom:Number )  { ... }
			 *
			 */
			QString scriptFncAtResizeSelectedObjects;
};

/** Class is STRATEGY pattern for mode construction.
 * Includes method for selecting objects on page and manipulating with him.
 *
 * This implementation only draw new objects with draw methods (STRATEGY pattern) defined in
 * constructor.
 * Nothing operators or other objects are selected.
 */
class PageViewMode_NewObject: public PageViewMode {
	Q_OBJECT
	public slots:
			/** Method is calling if is press left mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mouseReleaseLeftButton
			 * @see PageViewMode::mousePressEvent
			 */
			virtual void mousePressLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is release left mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see PageViewMode::mousePressEvent
			 */
			virtual void mouseReleaseLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is press left mouse button and is moving with mouse.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see PageViewMode::mousePressEvent
			 */
			virtual void mouseMoveWithPressedLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );

			/** Repaint method for draw actual state of mode (selected region, moving and resizing in action, ...).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * It is only drawing method use, if viewing parent need clear repaint.
			 */
			virtual void repaint ( QPainter & p, QWidget * w  );
	public:
			/** Standard constructor.
			 * @param drawingObject						Text definition of drawing method (see DrawingObjectFactory)
			 * @param _scriptFncAtMouseRelease			Script command for call after selected object(s)
			 * @param _scriptFncAtMoveSelectedObjects	Script command for call after move selected object(s)
			 * @param _scriptFncAtResizeSelectedObjects	Script command for call after resize selected object(s)
			 */
			PageViewMode_NewObject ( const QString & drawingObject,
										const QString & _scriptFncAtMouseRelease,
										const QString & _scriptFncAtMoveSelectedObjects,
										const QString & _scriptFncAtResizeSelectedObjects );
};

/** Class is STRATEGY pattern for mode construction.
 * Includes method for selecting objects on page and manipulating with him.
 *
 * This implementation is for standard way to selecting text (line by line).
 */
class PageViewMode_TextSelection: public PageViewMode {
	Q_OBJECT
	public slots:
			/** Method is calling if is release left mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see PageViewMode::mousePressEvent
			 *
			 * Method find the nearest text operator below mouse cursor. If cursor is not over text operator, nothing do.
			 */
			virtual void mousePressLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is release left mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see PageViewMode::mousePressEvent
			 *
			 * Method end selection text.
			 */
			virtual void mouseReleaseLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is press left mouse button and is moving with mouse.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see updateSelection
			 * @see PageViewMode::mousePressEvent
			 *
			 * If is selecting action (mousePressLeftButton find and select one text operator), method find
			 * the nearest text operator to mouse cursor and select all text operator from first selected
			 * text operator to it respecting word and line order.
			 */
			virtual void mouseMoveWithPressedLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );

			/** Repaint method for draw actual state of mode (selected region, moving and resizing in action, ...).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * It is only drawing method use, if viewing parent need clear repaint.
			 */
			virtual void repaint ( QPainter & p, QWidget * w );

			/** Set selection region on the page.
			 * @param r Which region to set.
			 */
			virtual void setSelectedRegion ( QRegion r );

			/** Clear all operators which can be possible select on the page. */
			virtual void clearWorkOperators ();
			/** Clear all operators which are selected on the page. */
			virtual void clearSelectedOperators ();
			/** Add operators for selection on the page.
			 * @param wOps Vector of operators.
			 *
			 * Method filter only text operator.
			 */
			virtual void addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps );
			/** Add operators selected on the page.
			 * @param sOps Vector of operators.
			 *
			 * Method filter only text operator.
			 */
			virtual void addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps );
	public:
			/** Standard constructor.
			 * @param drawingObject						Text definition of drawing method (see DrawingObjectFactory)
			 * @param _scriptFncAtMouseRelease			Script command for call after selected object(s)
			 * @param _scriptFncAtMoveSelectedObjects	Script command for call after move selected object(s)
			 * @param _scriptFncAtResizeSelectedObjects	Script command for call after resize selected object(s)
			 */
			PageViewMode_TextSelection ( const QString & drawingObject,
											const QString & _scriptFncAtMouseRelease,
											const QString & _scriptFncAtMoveSelectedObjects,
											const QString & _scriptFncAtResizeSelectedObjects );

	protected:
			/** Set mapping cursors for viewing on the page for actual selection mode */
			virtual void setMappingCursor();

			/** Method reorder selected operators to order by from top to bottom and from left to right on the line. */
			virtual void reorderSelectedOp();

			/** Method update selected operators and add new or remove operators from vector of selected operators.
			 * @param first	First selected operator (pointer to sorted structure by line and word on line)
			 * @param last	Last selected operator (pointer to sorted structure by line and word on line)
			 * @param selOpsRegion	Bounding region of all selected operators.
			 * @param selOps		Vector of selected operators.
			 *
			 * Method find the nearest text operator to mouse cursor and select all text operator from first selected
			 * text operator to it respecting word and line order.
			 * New selected operators add to \a selOps. Removed selected operators remove from that.
			 * Actualize \a selOpsRegion by \a selOps.
			 */
			void updateSelection (	const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > *	first,
									const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> > *	last,
									QRegion *														selOpsRegion = NULL,
									std::vector< boost::shared_ptr< PdfOperator > > *				selOps = NULL );

			/** Method return the nearest text operator to point on the page.
			 * @param point	Point on the page.
			 *
			 * @return Return pointer to the nearest text operators to point \a point.
			 *			If point is not inside any text operator, return NULL.
			 */
			const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> >	* getNearestObject( const QPoint & point );
	protected:
			/** 2D array of bboxes objects for special selection */
			Rect2DArray< boost::shared_ptr<PdfOperator> >	arrayOfBBoxes;
			/** first selected objects */
			const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> >	* firstSelectedObject;
			/** last selected objects */
			const BBoxOfObjectOnPage< boost::shared_ptr<PdfOperator> >	* lastSelectedObject;
};

/** Class is STRATEGY pattern for mode construction.
 * Includes method for selecting objects on page and manipulating with him.
 *
 * This implementation is for selecting any operators on the page.
 * If press left button and move mouse, and then release left button, mode select all oprators which are all
 * inside the selected area.
 * If press and then release left button (don't move), then mode select the last (top) operator which contains
 * mouse button release position on the page. If press and then release left button over selected operator,
 * mode select previous operator (operator which is first below now selected operators).
 */
class PageViewMode_OperatorsSelection: public PageViewMode {
	Q_OBJECT
	public slots:
			/** Method is calling if is release left mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see PageViewMode::mousePressEvent
			 */
			virtual void mousePressLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is release left mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see PageViewMode::mousePressEvent
			 */
			virtual void mouseReleaseLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is need move selected region (operation 'move' is finished).
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see movedSelectedObjects( QPoint )
			 * @see moveSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void movedSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is need resize selected region (operation 'resize' is NOT finished).
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see mouseReleaseLeftButton
			 * @see mouseMoveWithPressedLeftButton
			 *
			 * @see resizedSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void resizeSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is need resize selected region (operation 'resize' is finished).
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see resizeSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 * @see resizedSelectedObjects ( int, int, int, int )
			 */
			virtual void resizedSelectedObjects ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Method is calling if is press left mouse button and is moving with mouse.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see updateSelection
			 * @see PageViewMode::mousePressEvent
			 */
			virtual void mouseMoveWithPressedLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );

			/** Set selection region on the page.
			 * @param r Which region to set.
			 */
			virtual void setSelectedRegion ( QRegion r );

			/** Add operators for selection on the page.
			 * @param wOps Vector of operators.
			 */
			virtual void addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps );
			/** Clear all operators which can be possible select on the page. */
			virtual void clearWorkOperators ();
	public:
			/** Standard constructor.
			 * @param drawingObject						Text definition of drawing method (see DrawingObjectFactory)
			 * @param _scriptFncAtMouseRelease			Script command for call after selected object(s)
			 * @param _scriptFncAtMoveSelectedObjects	Script command for call after move selected object(s)
			 * @param _scriptFncAtResizeSelectedObjects	Script command for call after resize selected object(s)
			 */
			PageViewMode_OperatorsSelection ( const QString & drawingObject,
												const QString & _scriptFncAtMouseRelease,
												const QString & _scriptFncAtMoveSelectedObjects,
												const QString & _scriptFncAtResizeSelectedObjects );

	protected:
			/** Finf all operators which are all in region \a r.
			 * @param in_v		Vector of operator in which are finding.
			 * @param founded	Vector to puch all founded operators.
			 * @param r			Region which is selected.
			 */
			void	findOperators (	const std::vector< boost::shared_ptr< PdfOperator > >	& in_v,
									std::vector< boost::shared_ptr< PdfOperator > >			& founded,
									const QRegion	& r );
			/** Finf prev operator which is under position \a p.
			 * @param it		Iterator in vector of all operators which can be selected.
			 * @param v			Vector which will be contains previous operator.
			 * @param fromEnd	Boolean will be true, if nothing previous operator found.
			 * @param p			Point under which will be search previous operator.
			 *
			 * @return Return true if exist some operator on position \a p. Otherwise return false (\a founded is empty).
			 */
			bool	findPrevOperator (	std::vector< boost::shared_ptr< PdfOperator > >::iterator	& it,
										std::vector< boost::shared_ptr< PdfOperator > >				& v,
										bool 			& fromEnd,
										const QPoint	& p );
			/** Iterator at last selected operator in \a workOperators */
			std::vector< boost::shared_ptr< PdfOperator > >::iterator		lastSelectedOperator;

			/** Helpes position for decide if user resize selected area or only click on that. */
			QPoint	resizingPress;
};

/** Class is STRATEGY pattern for mode construction.
 * Includes method for selecting objects on page and manipulating with him.
 *
 * This implementation is for selecting only annotation on the page.
 * If mouse cursor is over some annotation, then emits new selected annotation.
 * If click on annotation emit required execute script commad.
 */
class PageViewMode_Annotations: public PageViewMode {
	Q_OBJECT
	public slots:
			/** Method is calling if is need resize selected region (operation 'resize' is finished).
			 * @param dleft		delta position of bounding-rectangle's left edge of selected operators.
			 * @param dtop		delta position of bounding-rectangle's top edge of selected operators.
			 * @param dright	delta position of bounding-rectangle's right edge of selected operators.
			 * @param dbottom	delta position of bounding-rectangle's bottom edge of selected operators.
			 *
			 * This method call script.
			 *
			 * This implementation call script function 'scriptFncAtMouseRelease' if all parameters is 0.
			 *
			 * @see resizedSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 * @see PageViewMode::resizedSelectedObjects( int, int, int, int )
			 */
			virtual void resizedSelectedObjects ( int dleft, int dtop, int dright, int dbottom );
			/** Method is calling if is need move selected region (operation 'move' is finished).
			 * @param relativeMove relativ move x and y position of selected region.
			 *
			 * This method call script.
			 *
			 * This implementation call script function 'scriptFncAtMouseRelease' if \a relativeMove is [0,0].
			 *
			 * @see movedSelectedObjects( QMouseEvent *, QPainter *, QWidget * )
			 * @see PageViewMode::movedSelectedObjects( QPoint )
			 */
			virtual void movedSelectedObjects ( QPoint relativeMove );
			/** Method is call if mouse is moving (see Qt::QWidget::mouseMoveEvent)
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * Is switch which call other method. Is settings \a pomCur and mouse cursor for widget \a w.
			 *
			 * @see moveSelectedObjects ( QMouseEvent *, QPainter *, QWidget * )
			 * @see resizeSelectedObjects ( QMouseEvent *, QPainter *, QWidget * )
			 * @see mouseMoveWithPressedLeftButton ( QMouseEvent *, QPainter *, QWidget * )
			 */
			virtual void mouseMoveEvent ( QMouseEvent * e, QPainter * p, QWidget * w );

			/** Set operators for selection on the page.
			 * @param wOps Vector of operators.
			 *
			 * Method is reimplemented from parent and nothing operators are be set.
			 */
			virtual void setWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps );
			/** Add operators for selection on the page.
			 * @param wOps Vector of operators.
			 *
			 * Method is reimplemented from parent and nothing operators are be set.
			 */
			virtual void addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps );
			/** Clear all operators and annotations which can be possible select on the page. */
			virtual void clearWorkOperators ();
			/** Clear all operators and annotations which are selected on the page. */
			virtual void clearSelectedOperators ();
			/** Set operators selected on the page.
			 * @param sOps Vector of operators.
			 *
			 * Method is reimplemented from parent and nothing operators are be set.
			 */
			virtual void setSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps );
			/** Add operators selected on the page.
			 * @param sOps Vector of operators.
			 *
			 * Method is reimplemented from parent and nothing operators are be set.
			 */
			virtual void addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps );
			/** Actualize selected operators on the page.
			 *
			 * Method is reimplemented from parent and nothing operators are be set.
			 */
			virtual void actualizeSelection ();
			/** Extra initialization mode.
			 * @param page			Page which are viewed.
			 * @param displayParams	Display parameters viewed page.
			 *
			 * This method is good called after change viewed page, or change display parameters (zoom, rotation, ...)
			 * Initialize annotation for selecting on the page \a page.
			 */
			virtual void extraInitialize( const boost::shared_ptr< CPage > & page, const DisplayParams & displayParams );
			/** Repaint method for draw actual state of mode (selected region, moving and resizing in action, ...).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * It is only drawing method use, if viewing parent need clear repaint.
			 */
			virtual void repaint ( QPainter & p, QWidget * w  );
	public:
			/** Standard constructor.
			 * @param drawingObject						Text definition of drawing method (see DrawingObjectFactory)
			 * @param _scriptFncAtMouseRelease			Script command for call after selected object(s)
			 * @param _scriptFncAtMoveSelectedObjects	Script command for call after move selected object(s)
			 * @param _scriptFncAtResizeSelectedObjects	Script command for call after resize selected object(s)
			 */
			PageViewMode_Annotations ( const QString & drawingObject,
							const QString & _scriptFncAtMouseRelease,
							const QString & _scriptFncAtMoveSelectedObjects,
							const QString & _scriptFncAtResizeSelectedObjects );

	protected:
			typedef struct {
					boost::shared_ptr<CAnnotation>	annot;
					QRegion				activation_region;
				} annot_rect;

			/** Set mapping cursors for viewing on the page for actual selection mode */
			virtual void setMappingCursor();

			/** Method get rectangle of activation region for annotation.
			 * @param annot	Annotation which is required activayion rectangle.
			 *
			 * @return Return activation rectangle for \a annot.
			 */
			libs::Rectangle getRectOfAnnotation ( boost::shared_ptr<CAnnotation> & annot );
			/** Get struct for annotation and its activation rectangle for annotation under point on the page
			 * @param p	Position on the page.
			 *
			 * @return Return initialized struct for annotation and its activation region. If nothing annotation
			 * 			is under position \a p, struct is initialized to null shared pointer to annotation.
			 */
			annot_rect getAnnotationOnPosition ( const QPoint & p );
	protected:
			/** Vector of all annotations which are into the page. */
			std::vector< annot_rect >	annotations;
};

/** Class is STRATEGY pattern for mode construction.
 * Includes method for selecting objects on page and manipulating with him.
 *
 * This implementation is reimplemented class PageViewMode_TextSelection.
 * Different between parent and this implementation is after select new operator.
 * After select new text operator is called script and then all selected operators will be unselected.
 */
class PageViewMode_TextMarking: public PageViewMode_TextSelection {
	Q_OBJECT
	public slots:
			/** Method is calling if is release left mouse button.
			 * @param e		Pointer to mouse event (see Qt::QMouseEvent).
			 * @param p		Pointer to initialized painter for draw changes (see Qt::QPainter).
			 * 				Method emit at end 'needRepaint' if \a p is NULL.
			 * @param w		Pointer to widget (see Qt::QWidget). E.g. for change mouse cursor above operators.
			 *
			 * @see mousePressLeftButton
			 * @see mousePressEvent
			 *
			 * Call script and then unselect all selected operators.
			 */
			virtual void mouseReleaseLeftButton ( QMouseEvent * e, QPainter * p, QWidget * w );
			/** Add operators selected on the page.
			 * @param sOps Vector of operators.
			 *
			 * This method is reimplemented and don't add operators to selected operators.
			 */
			virtual void addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps );
	public:
			/** Standard constructor.
			 * @param drawingObject						Text definition of drawing method (see DrawingObjectFactory)
			 * @param _scriptFncAtMouseRelease			Script command for call after selected object(s)
			 * @param _scriptFncAtMoveSelectedObjects	Script command for call after move selected object(s)
			 * @param _scriptFncAtResizeSelectedObjects	Script command for call after resize selected object(s)
			 */
			PageViewMode_TextMarking ( const QString & drawingObject,
										const QString & _scriptFncAtMouseRelease,
										const QString & _scriptFncAtMoveSelectedObjects,
										const QString & _scriptFncAtResizeSelectedObjects );

	protected:
			/** Set mapping cursors for viewing on the page for actual selection mode */
			virtual void setMappingCursor();
};

/** Class is STRATEGY pattern for mode construction.
 * Includes method for selecting objects on page and manipulating with him.
 *
 * This implementation is for selecting graphical operators on the page (see PageViewMode_OperatorsSelection).
 * If press left button and move mouse, and then release left button, mode select all oprators which are all
 * inside the selected area.
 * If press and then release left button (don't move), then mode select the last (top) operator which contains
 * mouse button release position on the page. If press and then release left button over selected operator,
 * mode select previous operator (operator which is first below now selected operators).
 */
class PageViewMode_GraphicalOperatorsSelection: public PageViewMode_OperatorsSelection {
	Q_OBJECT
	public slots:
			/** Add operators for selection on the page.
			 * @param wOps Vector of operators.
			 *
			 * Method filter only graphical operator.
			 */
			virtual void addWorkOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & wOps );
			/** Add operators selected on the page.
			 * @param sOps Vector of operators.
			 *
			 * Method filter only graphical operator.
			 */
			virtual void addSelectedOperators ( const std::vector< boost::shared_ptr< PdfOperator > > & sOps );
	public:
			/** Standard constructor.
			 * @param drawingObject						Text definition of drawing method (see DrawingObjectFactory)
			 * @param _scriptFncAtMouseRelease			Script command for call after selected object(s)
			 * @param _scriptFncAtMoveSelectedObjects	Script command for call after move selected object(s)
			 * @param _scriptFncAtResizeSelectedObjects	Script command for call after resize selected object(s)
			 */
			PageViewMode_GraphicalOperatorsSelection ( const QString & drawingObject,
														const QString & _scriptFncAtMouseRelease,
														const QString & _scriptFncAtMoveSelectedObjects,
														const QString & _scriptFncAtResizeSelectedObjects );
};

} // namespace gui

#endif
