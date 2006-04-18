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

namespace gui {

using namespace pdfobjects;

class PageSpace : public QWidget {
	Q_OBJECT
	public:
		PageSpace (QWidget *parent=0, const char *name=0);
		virtual ~PageSpace ( );
		void convertPixmapPosToPdfPos( const QPoint & pos, QPoint & pdfPos );
	public slots:
void refresh1(/* CPage * = NULL*/);  //TODO smazat, jen pro testovani
void refresh2(/* CPage * = NULL*/);  //TODO smazat, jen pro testovani
		void refresh ( /* QSPdf * pdf,*/ QSPage * pageToView = NULL );	// if pageToView is NULL, refresh actual page
		void hideButtonsAndPageNumber ( );
		void showButtonsAndPageNumber ( );
//		/*TODO*/void selectObjectOnPage ( /* CObject &*/ );
//		/*TODO*/void unselectObjectOnPage ( );
//		/*TODO*/void zoomTo ( unsigned int percentage );
	signals:
		void changePageTo ( const QSPage &, int numberOfPage );

		/*TODO*/ void popupMenu ( const QPoint & globalPos /*, Cobject & */ );
	protected:
		virtual void resizeEvent ( QResizeEvent * );
		virtual void keyPressEvent ( QKeyEvent * e );
	private slots:
		// slots for connecting pageImage's signals
		void newSelection ( const QRect & );
		void requirementPopupMenu ( const QPoint &, const QRect & );
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
		QSPage		* actualPage;
		QPixmap		* actualPagePixmap;
QPixmap * r1, * r2; /*TODO smazat, jenom pro testovani */
};

} // namespace gui

#endif
