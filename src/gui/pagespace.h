#ifndef __PAGESPACE_H__
#define __PAGESPACE_H__

#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qevent.h>
#include <qscrollview.h>
#include <qlayout.h>
#include "pageviewS.h"
#include "qspage.h"
#include "qspdf.h"

using namespace pdfobjects;

namespace gui {

class PageViewMode;

class Units : protected QObject {
	public:
		Units ( const QString _defaultUnit = QString::null );
		virtual ~Units ();

		bool setDefaultUnits ( const QString dunits = QString::null );
		QString getDefaultUnits ( ) const;
		double convertUnits ( double num, const QString fromUnits = QString::null, const QString toUnits = QString::null ) const;
		double convertFromUnitsToPoint ( double num, const QString & fromUnits ) const;
		double convertFromPointToUnits ( double num, const QString & toUnits ) const;

		void getAllUnits( QStringList & names );
	protected:
		QMap<QString, double>	units;
		QMap<QString, QString>	aliases;
		QString					defaultUnit;
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
		QStringList getAllUnits ( );
		bool setDefaultUnits ( const QString dunits = QString::null );
		QString getDefaultUnits ( ) const;
		double convertUnits ( double num, const QString fromUnits = QString::null, const QString toUnits = QString::null ) const;
		double convertFromUnitsToPoint ( double num, const QString & fromUnits ) const;
		double convertFromPointToUnits ( double num, const QString & toUnits ) const;

		/** Method set width of resizing zone
		 * @param width width in pixels
		 *
		 * Default is set to 2.
		 */
		void setResizingZone ( int width );
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
		/** @copydoc refresh(QSPage,QSPdf) */
		void refresh ( QSPage * pageToView, /*QSPdf * */ QObject * pdf );	// same as above
		/** Method for refreshing page on screen.
		 * @param pageToView Page position in \a pdf for refresh. If page is other then actual viewed, view this new page.
		 * 					Nothing do, if page position don't exist in \a pdf 
		 * @param pdf Pdf in whitch is pageToView. If pdf is NULL then \a pageToView is from same pdf as
		 * 					actual viewed page, if exist actual viewed page. Otherwise don't view \a pageToView.
		 *
		 * @see refresh(QSPage,QSPdf)
		 */
		void refresh ( int pageToView, QSPdf * pdf = NULL );			// if pdf is NULL refresh page from current pdf
		/** @copydoc refresh(int,QSPdf) */
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
		 * @onlySelectedArea	Save all page, if is FALSE (default). Otherwise save only bounding rectangle of selected area.
		 *
		 * @return	Return TRUE, if image is saved. Otherwise return FALSE.
		 *
		 * @see saveImageWithDialog
		 */
		bool saveImage ( const QString & filename, const char * format, int quality = -1, bool onlySelectedArea = false);
		/** Function save viewed page to file and for choose destination file view dialog with all available destination file format.
		 * @onlySelectedArea	Save all page, if is FALSE (default). Otherwise save only bounding rectangle of selected area.
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

	signals:
		/** Signal is emited, if is changed viewed page.
		 * @param page			Page whitch is actual viewed.
		 * @param numberOfPage	Position of \a page in documents.
		 */
		void changedPageTo ( const QSPage & page, int numberOfPage );
		/** Signal is emited, if changed zoom of viewed page.
		 * @param zoom	Zoom factor, whitch is current set (1.0 = 100%)
		 */
		void changedZoomFactorTo ( float zoom );
		/** Signal is emited, if 
		 */
		void changeSelection ( std::vector<boost::shared_ptr<PdfOperator> > );
		void changeSelection ( std::vector< boost::shared_ptr< CAnnotation > > );

		void changeMousePosition ( double x, double y );
		void popupMenu ( const QPoint & PagePos /*, Cobject & */ );
		void executeCommand ( QString cmd );
		void deleteSelection ( );
	private slots:
		// slots for connecting pageImage's signals
		void newSelection ( const std::vector< boost::shared_ptr< PdfOperator > > & );
		void newSelection ( const std::vector< boost::shared_ptr< CAnnotation > > & objects );
		void requestPopupMenu ( const QPoint & );
		void showMousePosition ( double x, double y );
	private:
		void newSelection ();
		void actualizeSelection ();
	private:
		QLabel		* pageNumber;
		QLabel		* mousePositionOnPage;
		PageViewS	* pageImage;
		QVBoxLayout	* vBox;	// mozna nebude potreba
		QHBoxLayout	* hBox;	// mozna nebude potreba
		QScrollView	* scrollPageSpace;

		QSPdf						* actualPdf;
		boost::shared_ptr<CPage>	actualPage;
		int							actualPagePos;

		boost::shared_ptr< PageViewMode >	selectionMode;

		Units		actualUnits;
};

} // namespace gui

#endif
