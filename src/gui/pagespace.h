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
#ifndef __PAGESPACE_H__
#define __PAGESPACE_H__

#include "qtcompat.h"
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qevent.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qlineedit.h>
#include "pageviewS.h"
#include "qspage.h"
#include "qspdf.h"
#include "units.h"

using namespace pdfobjects;

class Q_ScrollView;

namespace gui {

class PageViewMode;

/** Class for showing (only) edit line. */
class TextLine: public QMainWindow {
	Q_OBJECT
	public:
		/** Standard constructor. */
		TextLine ();
		/** Standard destructor. */
		~TextLine ();
	public slots:
		/** Set text to edit line.
		 * @param text Text to set.
		 */
		void setText( const QString & text );
	signals:
		/** Signal emited if edit line lost focus.
		 * @param text Text which was typed to edit line
		 */
		void lostFocus ( const QString &text );
		/** Signal emited if press return in edit line.
		 * @param text Text which was typed to edit line
		 */
		void returnPressed ( const QString &text );
		/** Signal emited if press escape in edit line. */
		void escape ();
	protected:
		/** Method is call if release key event (see Qt::QWidget::keyReleaseEvent)
		 * @param e		Pointer to key event (see Qt::QKeyEvent).
		 *
		 * Esc and Delete key functionality is implemented.
		 */
		virtual void keyReleaseEvent ( QKeyEvent * e );
	private slots:
		/** On this slot is connected same named signal from edit line.
		 * Emit lostFocus(QString &)
		 */
		void lostFocus ();
		/** On this slot is connected same named signal from edit line.
		 * Emit returnPressed(QString &)
		 */
		void returnPressed ();
	private:
		/** Pointer to edit line widget. */
		QLineEdit * edit;
};

/** QWidget's class for viewing a page.
 */
class PageSpace : public QWidget {
	Q_OBJECT
	public:
		/** Constructor with parameters as parent (see QWidget in Qt documentation)
		 * @param parent Parent widget (see QWidget constroctor)
		 * @param name Name of widget (see QWidget constructor)
		 *
		 * Initialize all private variables, create scrollspace and bar with page number and position of cursor on page.
		 */
		PageSpace (QWidget *parent=0, const char *name=0);
		/** Destroctor of PageSpace */
		virtual ~PageSpace ( );
		/** Method for convert page position viewed on screen to pdf position (unapply rotation, viewing dpi, ...)
		 * @param pos Position on viewed page.
		 * @param pdfPos Position on page in pdf.
		 *
		 * Position is returned in \a pdfPos.
		 *
		 * @see convertPixmapPosToPdfPos_x
		 * @see convertPixmapPosToPdfPos_y
		 * @see convertPdfPosToPixmapPos
		 */
		void convertPixmapPosToPdfPos( const QPoint & pos, Point & pdfPos );
		/** Method for convert pdf position to page position viewed on screen (apply rotation, viewing dpi, ...)
		 * @param pdfPos Position on page in pdf.
		 * @param pos Position on viewed page.
		 *
		 * Position is returned in \a pos.
		 *
		 * @see convertPdfPosToPixmapPos_x
		 * @see convertPdfPosToPixmapPos_y
		 * @see convertPixmapPosToPdfPos
		 */
		void convertPdfPosToPixmapPos( const Point & pdfPos, QPoint & pos );
	public slots:
		/** Function return description of all units internal defined.
		 * @return Return description of units.
		 */
		QStringList getAllUnits ( ) const;
		/** Function return identifiers of all units internal defined.
		 * @return Return identifiers of units.
		 */
		QStringList getAllUnitIds ( ) const;
		/** Method set default unit for conversion.
		 * @param dunits Unit for set as default. Nothing chenage, if not find in internal units.
		 *
		 * @return Return true if default unit is change to \a dunits. Otherwise return false.
		 */
		bool setDefaultUnits ( const QString dunits = QString::null );
		/** Method return default conversion unit.
		 * @return Return defualt unit.
		 */
		QString getDefaultUnits ( ) const;
		/** Method return description for unit.
		 * @param _unit Which unit is required description.
		 *
		 * @return Description for \a _unit. If nothing description found for \a _unit, return \a _unit.
		 */
		QString getDescriptionForUnit( const QString _unit ) const;
		/** Function return unit from its aliases or description.
		 * @param _unit Alias or description of unit.
		 *
		 * @return Return unit for alias \a _unit. If alias \a _unit is not find in internal aliases or
		 *		descrition, return \a _unit.
		 */
		QString getUnitFromAlias( const QString _unit ) const;
		/** Convert value from one unit to other.
		 * @param num Value for conversion.
		 * @param fromUnits Unit of value \a num.
		 * @param toUnits To which unit is required conversion of value \a num.
		 *
		 * If \a fromUnits or \a toUnits is not set, then default unit use.
		 *
		 * @return Return value \a num conversed to unit \a toUnits.
		 */
		double convertUnits ( double num, const QString fromUnits = QString::null, const QString toUnits = QString::null ) const;
		/** Conver value to point ("pt").
		 * @param num Value for conversion.
		 * @param fromUnits Unit of value \a num.
		 *
		 * If \a fromUnits is not set, then default unit use.
		 *
		 * @return Return value \a num conversed to points.
		 */
		double convertFromUnitsToPoint ( double num, const QString & fromUnits ) const;
		/** Conver value from points ("pt") to some unite.
		 * @param num Value for conversion.
		 * @param toUnits To which unit is required conversion of value \a num.
		 *
		 * If \a toUnits is not set, then default unit use.
		 *
		 * @return Return value \a num conversed from points to units \a toUnits.
		 */
		double convertFromPointToUnits ( double num, const QString & toUnits ) const;

		/** Method set width of resizing zone
		 * @param width width in pixels
		 *
		 * Default is set to 2.
		 */
		void setResizingZone ( int width );
		/** Function return actual set width of resizing zone
		 * @return Actual set resizing zone
		 */
		int getResizingZone();

		/** Method for refreshing page on screen and actualize selected objects.
		 * @param pageToView Page for refresh. If \a pageToView is other then actual viewed, view this new page.
		 * 					(default is set to NULL = refresh actual viewed page)
		 * @param pdf Pdf in whitch is \a pageToView. If pdf is NULL then \a pageToView is from same pdf as
		 * 					actual view page, if exist actual viewed page. Otherwise don't view \a pageToView.
		 *
		 * @see refresh(int,QSPdf)
		 */
		void refresh ( QSPage * pageToView = NULL, QSPdf * pdf = NULL );	// if pageToView is NULL, refresh actual page
		/** @copydoc refresh(QSPage*,QSPdf*) */
		void refresh ( QSPage * pageToView, /*QSPdf * */ QObject * pdf );	// same as above
		/** Method for refreshing page on screen.
		 * @param pageToView Page position in \a pdf for refresh. If page is other then actual viewed, view this new page.
		 * 					Nothing do, if page position don't exist in \a pdf
		 * @param pdf Pdf in whitch is pageToView. If pdf is NULL then \a pageToView is from same pdf as
		 * 					actual viewed page, if exist actual viewed page. Otherwise don't view \a pageToView.
		 *
		 * @see refresh(QSPage*,QSPdf*)
		 */
		void refresh ( int pageToView, QSPdf * pdf = NULL );			// if pdf is NULL refresh page from current pdf
		/** @copydoc refresh(int,QSPdf*) */
		void refresh ( int pageToView, /*QSPdf * */ QObject * pdf );	// same as above

		/** Hide bar for view number of actual viewed page and mouse position on page.
		 * @see showPageNumberAndPosition
		 */
		void hidePageNumberAndPosition ( );
		/** Show bar for view number of actual viewed page and mouse position on page.
		 * @see hidePageNumberAndPosition
		 */
		void showPageNumberAndPosition ( );

		/** Set selection mode.
		 * @param mode Mode for selection.
		 * @param drawingObject What be drawing in \a mode
		 * @param scriptFncAtMouseRelease Name of function in script, which be call at mouse left button release
		 * @param scriptFncAtMoveSelectedObjects Name of function in script, which be call at mouse moved with selected area
		 * @param scriptFncAtResizeSelectedObjects Name of function in script, which be call at mouse resize selected area
		 *
		 * @return Return TRUE, if selection mode was changed. Otherwise return FALSE
		 *
		 * @see PageViewModeFactory
		 * @see DrawingObjectFactory
		 */
		void setSelectionMode( QString mode,
								QString drawingObject = QString::null,
								QString scriptFncAtMouseRelease = QString::null,
								QString scriptFncAtMoveSelectedObjects = QString::null,
								QString scriptFncAtResizeSelectedObjects = QString::null );

		/** Select area on viewed page.
		 * @param left X position of lefttop edge of new select rectangle
		 * @param top Y position of lefttop edge of new select rectangle
		 * @param right X position of rightbottom edge of new select rectangle
		 * @param bottom Y position of rightbottom edge of new select rectangle
		 *
		 * Position is in pixels on viewed page, not in pdf (see PageSpace::convertPdfPosToPixmapPos for convert position)
		 */
		void setSelectArea ( int left, int top, int right, int bottom );
		/** Select objects (PdfOperators) on page.
		 * @param ops Vector of PdfOperators for select.
		 *
		 * @see addSelectedObjectOnPage
		 * @see unselectObjectOnPage
		 * @see isSomeoneSelected
		 */
		void selectObjectOnPage ( const std::vector<boost::shared_ptr<PdfOperator> > & ops );
		/** Add objects (PdfOperators) to selection on page.
		 * @param ops Vector of PdfOperators for add.
		 *
		 * @see selectObjectOnPage
		 * @see unselectObjectOnPage
		 * @see isSomeoneSelected
		 */
		void addSelectedObjectOnPage ( const std::vector<boost::shared_ptr<PdfOperator> > & ops );
		/** Unselect objects selected on page
		 * @see selectObjectOnPage
		 * @see addSelectedObjectOnPage
		 * @see isSomeoneSelected
		 */
		void unselectObjectOnPage ( );
		/** Function return if some object is selected (not in selection mode PageView::SelectRect)
		 * @return Return TRUE, if some object is selected. Otherwise return FALSE.
		 */
		bool isSomeoneSelected ( );

		/** Function return actual zoom factor of viewed page.
		 * @return Return zoom factor (1.0 = 100%)
		 *
		 * @see setZoomFactor
		 * @see zoomTo
		 * @see zoomIn
		 * @see zoomOut
		 */
		float getZoomFactor ( );
		/** Set zoom factor of viewed page.
		 * @param set_zoomFactor Zoom factor (1.0 = 100%)
		 *
		 * @see getZoomFactor
		 * @see zoomTo
		 * @see zoomIn
		 * @see zoomOut
		 */
		void setZoomFactor ( float set_zoomFactor );
		/** Set zoom of viewed page.
		 * @param percentage Zoom in percentage
		 *
		 * @see getZoomFactor
		 * @see setZoomFactor
		 * @see zoomIn
		 * @see zoomOut
		 */
		void zoomTo ( int percentage );
		/** Zoom in of viewed page.
		 * @param step Step of zoom in (default step is  0.1 = 10%)
		 *
		 * @see getZoomFactor
		 * @see setZoomFactor
		 * @see zoomTo
		 * @see zoomOut
		 */
		void zoomIn ( float step = 0.1 );
		/** Zoom out of viewed page.
		 * @param step Step of zoom out (default step is  0.1 = 10%)
		 *
		 * @see getZoomFactor
		 * @see setZoomFactor
		 * @see zoomTo
		 * @see zoomIn
		 */
		void zoomOut ( float step = 0.1 );

		/** Change viewed page to first page of documents
		 * If nothing page is viewed, nothing view
		 *
		 * @see prevPage
		 * @see nextPage
		 * @see lastPage
		 */
		void firstPage ( );
		/** Change viewed page to previous page in documents
		 * If nothing page is viewed, nothing view
		 *
		 * @see firstPage
		 * @see nextPage
		 * @see lastPage
		 */
		void prevPage ( );
		/** Change viewed page to next page in documents
		 * If nothing page is viewed, nothing view
		 *
		 * @see firstPage
		 * @see prevPage
		 * @see lastPage
		 */
		void nextPage ( );
		/** Change viewed page to last page of documents
		 * If nothing page is viewed, nothing view
		 *
		 * @see firstPage
		 * @see prevPage
		 * @see nextPage
		 */
		void lastPage ( );

		/** Function save viewed page to file.
		 * @param filename		Name of destination file.
		 * @param format		Format of file (see QImageIO::outputFormats)
		 * @param quality		Quality factor. Must be in range [0, 100] or -1 (default) (see QPixmap::save for more)
		 * @param onlySelectedArea	Save all page, if is FALSE (default). Otherwise save only bounding rectangle of selected area.
		 *
		 * @return	Return TRUE, if image is saved. Otherwise return FALSE.
		 *
		 * @see saveImageWithDialog
		 */
		bool saveImage ( const QString & filename, const char * format, int quality = -1, bool onlySelectedArea = false);
		/** Function save viewed page to file and for choose destination file view dialog with all available destination file format.
		 * @param onlySelectedArea	Save all page, if is FALSE (default). Otherwise save only bounding rectangle of selected area.
		 *
		 * @return	Return TRUE, if image is saved. Otherwise return FALSE.
		 *
		 * @see saveImage
		 */
		bool saveImageWithDialog ( bool onlySelectedArea = false );

		/** Function return X position in pdf page from position of viewed page on screen (unapply rotation, viewing dpi, ...)
		 * @param fromX	X position on viewed page.
		 * @param fromY	Y position on viewed page.
		 *
		 * @return Return X position in pdf page.
		 *
		 * @see convertPixmapPosToPdfPos_y
		 * @see convertPixmapPosToPdfPos
		 * @see convertPdfPosToPixmapPos
		 */
		double convertPixmapPosToPdfPos_x ( double fromX, double fromY );
		/** Function return Y position in pdf page from position of viewed page on screen (unapply rotation, viewing dpi, ...)
		 * @param fromX	X position on viewed page.
		 * @param fromY	Y position on viewed page.
		 *
		 * @return Return Y position in pdf page.
		 *
		 * @see convertPixmapPosToPdfPos_x
		 * @see convertPixmapPosToPdfPos
		 * @see convertPdfPosToPixmapPos
		 */
		double convertPixmapPosToPdfPos_y ( double fromX, double fromY );
		/** Function return X position on viewed page from position in pdf page (apply rotation, viewing dpi, ...)
		 * @param fromX	X position in pdf page.
		 * @param fromY	Y position in pdf page.
		 *
		 * @return Return X position in pdf page.
		 *
		 * @see convertPdfPosToPixmapPos_y
		 * @see convertPdfPosToPixmapPos
		 * @see convertPixmapPosToPdfPos
		 */
		double convertPdfPosToPixmapPos_x ( double fromX, double fromY );
		/** Function return Y position on viewed page from position in pdf page (apply rotation, viewing dpi, ...)
		 * @param fromX	X position in pdf page.
		 * @param fromY	Y position in pdf page.
		 *
		 * @return Return Y position in pdf page.
		 *
		 * @see convertPdfPosToPixmapPos_x
		 * @see convertPdfPosToPixmapPos
		 * @see convertPixmapPosToPdfPos
		 */
		double convertPdfPosToPixmapPos_y ( double fromX, double fromY );

		/** Function find text in actual viewed page, and select all operators whitch contains this text.
		 * @param text			Text for find.
		 * @param startAtTop	Start search text at top of page, if is TRUE (default). Otherwise start search at ...
		 * @param xStart		
		 * @param yStart		
		 * @param xEnd			
		 * @param yEnd			
		 *
		 * @return Return count of founded \a text.
		 */
		int findText ( QString &text, bool startAtTop = true, double xStart = 0, double yStart = 0, double xEnd = -1, double yEnd = -1);

		/** Get pointer to insance of widget to show only edit line.
		 * @param x			Global X position for showing edit line.
		 * @param y			Global Y position for showing edit line.
		 * @param fontsize	To which font size will be set text in edit line.
		 * @param fontName	To which font will be set text in edit line.
		 *
		 * @return Return instance of widget to show only edit line.
		 */
		QMainWindow * getTextLine( int x, int y, int fontsize = 12, const QString & fontName = QString::null );
	signals:
		/** Signal emitted when viewed page is changed.
		 * @param page			Page which is actual viewed.
		 * @param numberOfPage	Position of \a page in documents.
		 */
		void changedPageTo ( const QSPage & page, int numberOfPage );
		/** Signal emitted when zoom of viewed page is changed.
		 * @param zoom	Current zoom factor (1.0 = 100%)
		 */
		void changedZoomFactorTo ( float zoom );
		/** Signal emitted when new operators are selected.
		 * @param ops Vector of selected operators.
		 */
		void changeSelection ( std::vector<boost::shared_ptr<PdfOperator> > ops );
		/** Signal emitted when new annotations are selected.
		 * @param annots Vector of selected annotations.
		 */
		void changeSelection ( std::vector< boost::shared_ptr< CAnnotation > > annots);
		/** Signal emitted when mouse position over the viewed page is changed.
		 * @param x	Horizontal position on page.
		 * @param y	Vertical position on page.
		 */
		void changeMousePosition ( double x, double y );
		/** Signal emitted when popup menu for page is requested
		 * @param pagePos	Position on the page when popup menu was invoked.
		 */
		void popupMenu ( const QPoint & pagePos /*, Cobject & */ );
		/** Signal emitted when command in script needs to be executed
		 * @param cmd	Command for executing.
		 */
		void executeCommand ( QString cmd );
		/** Signal emitted, when selected objects on page should be deleted. */
		void deleteSelection ( );
	private slots:
		/** On this slot is connected same named signal from class viewed the page (see pageImage).
		 * @param objects Vector of selected operators.
		 *
		 * This method emit signal "changeSelection".
		 */
		void newSelection ( const std::vector< boost::shared_ptr< PdfOperator > > & objects);
		/** On this slot is connected same named signal from class viewed the page (see pageImage).
		 * @param objects Vector of selected annotations.
		 *
		 * This method emit signal "changeSelection".
		 */
		void newSelection ( const std::vector< boost::shared_ptr< CAnnotation > > & objects );
		/** On this slot is connected same named signal from class viewed the page (see pageImage).
		 * @param p		Position on the page over which is calling popup menu.
		 *
		 * This method emit signal "popupMenu".
		 */
		void requestPopupMenu ( const QPoint & p );
		/** On this slot is connected same named signal from class viewed the page (see pageImage).
		 * @param x	Horizontal position on the page.
		 * @param y	Vertical position on the page.
		 *
		 * This method emit signal "changeMousePosition".
		 */
		void showMousePosition ( double x, double y );
	private:
		/** Text contains number of actual page and how many pages has documents. */
		QLabel		* pageNumber;
		/** Text contains mouse position on the page. */
		QLabel		* mousePositionOnPage;
		/** Pointer to class which is viewing pages. */
		PageViewS	* pageImage;
		/** Helpes layout. It contains page view widget and information line */
		QVBoxLayout	* vBox;
		/** Helpes layout. It contains information's label in information line */
		QHBoxLayout	* hBox;
		/** Pointer to class which is default for srcroll viewing pages (now is same as \a pageImage). */
		Q_ScrollView	* scrollPageSpace;

		/** Pointer to actual viewed Pdf. */
		QSPdf						* actualPdf;
		/** Actual viewed page from \a actualPdf. */
		boost::shared_ptr<CPage>	actualPage;
		/** Page position of actual viewed page. */
		int							actualPagePos;

		/** Concrete implementation of selection mode. */
		boost::shared_ptr< PageViewMode >	selectionMode;

		/** Class which implement conversion between diferent units. */
		Units		actualUnits;

		/** Pointer to widget for showing only edit line to type text. */
		TextLine	* textLine;
};

} // namespace gui

#endif
