#ifndef __PAGESPACE_H__
#define __PAGESPACE_H__

#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qevent.h>
#include <qscrollview.h>
#include <qlayout.h>
#include "pageview.h"
#include "qspage.h"
#include "qspdf.h"

namespace gui {

using namespace pdfobjects;

class PageSpace : public QWidget {
	Q_OBJECT
	public:
		PageSpace (QWidget *parent=0, const char *name=0);
		virtual ~PageSpace ( );
		void convertPixmapPosToPdfPos( const QPoint & pos, Point & pdfPos );
	public slots:
		void refresh ( QSPage * pageToView = NULL, QSPdf * pdf = NULL );	// if pageToView is NULL, refresh actual page
		void refresh ( QSPage * pageToView, /*QSPdf * */ QObject * pdf );	// same as above
		void refresh ( int pageToView, QSPdf * pdf = NULL );			// if pdf is NULL refresh page from current pdf
		void refresh ( int pageToView, /*QSPdf * */ QObject * pdf );	// same as above

		void hideButtonsAndPageNumber ( );
		void showButtonsAndPageNumber ( );

//		/*TODO*/void selectObjectOnPage ( /* CObject &*/ );
//		/*TODO*/void unselectObjectOnPage ( );

		float getZoomFactor ( );
		void setZoomFactor ( float set_zoomFactor );
		void zoomTo ( int percentage );
		void zoomIn ( float step = 0.1 );
		void zoomOut ( float step = 0.1 );

		void firstPage ( );
		void prevPage ( );
		void nextPage ( );
		void lastPage ( );

		bool saveImage ( const QString & filename, const char * format, int quality = -1, bool onlySelectedArea = false);
		bool saveImageWithDialog ( bool onlySelectedArea = false );

		bool isSomeoneSelected ( );
	signals:
		void changedPageTo ( const QSPage &, int numberOfPage );
		void changedZoomFactorTo ( float zoom );

		/*TODO*/ void popupMenu ( const QPoint & PagePos /*, Cobject & */ );
	protected:
		virtual void resizeEvent ( QResizeEvent * );
		virtual void keyPressEvent ( QKeyEvent * e );
	private slots:
		// slots for connecting pageImage's signals
		void newSelection ( const QRect & );
		void requirementPopupMenu ( const QPoint &, const QRect * );
		void moveSelection ( const QPoint & );
		void resizeSelection ( const QRect &, const QRect & );
		void showMousePosition ( const QPoint & );
	private:
		void newPageView();
		void newPageView( QPixmap &qp );
		void centerPageView( );
	private:
		QLabel		* pageNumber;
		QLabel		* mousePositionOnPage;
		PageView	* pageImage;
		QVBoxLayout	* vBox;	// mozna nebude potreba
		QHBoxLayout	* hBox;	// mozna nebude potreba
		QScrollView	* scrollPageSpace;
		QPushButton	* bFirstPage,
				* bPrevPage,
				* bNextPage,
				* bLastPage;

		/*TODO CObject */ void		* actualSelectedObjects;
		QSPdf		* actualPdf;
		QSPage		* actualPage;
		QPixmap		* actualPagePixmap;

		/** Pixels per point when zoom is 100 % */
		float		basePpP;
		/** Zoom factor requirement by user */
		float		zoomFactor;

		/** Display parameters ( hDpi, vDpi, rotate, ... ) */
		DisplayParams	displayParams;
};

} // namespace gui

#endif
