#include <stdlib.h>
#include <math.h>

#include <qlabel.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qdockarea.h>
#include <qfiledialog.h>

#include "pagespace.h"
#include "settings.h"
#include "cpdf.h"
#include "cpage.h"
#include "static.h"
#include "rect2Darray.h"

#include "pageviewmode.h"

typedef struct { int labelWidth, labelHeight; } initStruct;
// TODO asi prepracovat
void Init( initStruct * is, const QString & s ) {
	QLabel pageNumber( s ,0);
	pageNumber.show();
	is->labelWidth = pageNumber.width();
	is->labelHeight = pageNumber.height();
}


// using namespace std;

namespace gui {

QString PAGESPC = "gui/PageSpace/";
QString format = "x:%'.2f y:%'.2f";


PageSpace::PageSpace(QWidget *parent /*=0*/, const char *name /*=0*/) : QWidget(parent,name) {
	initStruct is;

	vBox = new QVBoxLayout(this);
	
	scrollPageSpace = pageImage = new PageViewS(this);
	connect( pageImage, SIGNAL( changeMousePosition(double, double) ), this, SLOT( showMousePosition(double, double) ) );
	connect( pageImage, SIGNAL( changeMousePosition(double, double) ), this, SIGNAL( changeMousePosition(double, double) ) );
	vBox->addWidget(scrollPageSpace);
//QObject *pomObj = this;
//while (pomObj->parent() !=NULL) pomObj = pomObj->parent();
QDockArea * da = new QDockArea( Qt::Horizontal, QDockArea::Normal, this /*dynamic_cast<QWidget *>(pomObj)*/ );
vBox->addWidget( da );
	
	hBox = new QHBoxLayout(vBox);

	actualPdf = NULL;
	actualPage = NULL;
	actualPagePos = -1;

	hBox->addStretch();
	pageNumber = new QLabel( QString(tr("%1 of %2")).arg(0).arg(0), this, "cisStr" );
	pageNumber->setAlignment( AlignCenter | pageNumber->alignment() );
	hBox->addWidget( pageNumber );
	hBox->addStretch();
        
	QString pom;
	Init( &is , format + "0000" );
	mousePositionOnPage = new QLabel( pom.sprintf( format, 0.0,0.0 ), this );
	mousePositionOnPage->setMinimumWidth( is.labelWidth );
	mousePositionOnPage->setAlignment( AlignRight | mousePositionOnPage->alignment() );
	hBox->addWidget( mousePositionOnPage, 0, AlignRight);

	hBox->insertSpacing( 0, is.labelWidth );	// for center pageNumber

	hBox->setResizeMode(QLayout::Minimum);

	selectionMode.reset();
	setSelectionMode( "new_object","script","line" );
	setSelectionMode( "text_selection","script","line" );
}

PageSpace::~PageSpace() {
	delete actualPdf;
	delete actualPage;
}

void PageSpace::hidePageNumberAndPosition  ( ) {
	pageNumber->hide();
	mousePositionOnPage->hide();
}
void PageSpace::showPageNumberAndPosition  ( ) {
	pageNumber->show();
	mousePositionOnPage->show();
}

void PageSpace::refresh ( int pageToView, /*QSPdf * */ QObject * pdf ) {	// same as above
	QSPdf * p = dynamic_cast<QSPdf *>(pdf);
	if (p)
		refresh( pageToView, p );
}
void PageSpace::refresh ( QSPage * pageToView, /*QSPdf * */ QObject * pdf ) {
	QSPdf * p = dynamic_cast<QSPdf *>(pdf);
	if (p)
		refresh( pageToView, p );
}

void PageSpace::refresh ( int pageToView, QSPdf * pdf ) {			// if pdf is NULL refresh page from current pdf
	int pageCount;

	if (pdf == NULL) {
		if (actualPdf == NULL)
			return ;
		pdf = actualPdf;
	}

	if (pageToView < 1)
			pageToView = 1;
		
	pageCount = pdf->get()->getPageCount();
	if (pageToView > pageCount)
			pageToView = pageCount;
		
	QSPage p( pdf->get()->getPage( pageToView ) , NULL );
	refresh( &p, pdf );
}

void PageSpace::refresh ( QSPage * pageToView, QSPdf * pdf ) {		// if pageToView is NULL, refresh actual page
									// if pageToView == actualPage  refresh is not need
/*	if (pageToView == NULL) {
		guiPrintDbg( debug::DBG_INFO, "Page to refresh is NULL pointer!" );
		return;
	}
*/
	if ((pageToView != NULL) && ( (actualPage == NULL) || (actualPage->get() != pageToView->get()) ) && ( (pdf != NULL) || (actualPdf != NULL) ) ) {
		if ((actualPdf == NULL) || (pdf->get() != actualPdf->get())) {
			delete actualPdf;
			actualPdf = new QSPdf( pdf->get() , NULL );
		}
		delete actualPage;
		actualPage = new QSPage( pageToView->get() , NULL );
	} else {
		if ((actualPage == NULL) || (actualPdf == NULL) || (pageToView != NULL))
			return ;					// no page to refresh
		// else  need reload page ( changed zoom, ... )
	}

	// if actual page is removed then show new page at same position
	try {
		actualPagePos = actualPdf->getPagePosition( actualPage );
	} catch (PageNotFoundException) {
		delete actualPage;
		actualPage = NULL;

		refresh( actualPagePos );

		return;
	}

	// show actual page
	pageImage->showPage( actualPage->get() );

	// emit new page position
	emit changedPageTo( * actualPage, actualPagePos );

	// show actual information of position in document
	pageNumber->setText( QString(tr("%1 of %2"))
				.arg(actualPagePos)
				.arg(actualPdf->getPageCount()) );
}

void PageSpace::selectObjectOnPage ( const std::vector<boost::shared_ptr<PdfOperator> > & ops ) {
	if (selectionMode)
		selectionMode->setSelectedOperators ( ops );
}
void PageSpace::addSelectedObjectOnPage ( const std::vector<boost::shared_ptr<PdfOperator> > & ops ) {
	if (selectionMode)
		selectionMode->addSelectedOperators ( ops );
}
void PageSpace::unselectObjectOnPage ( ) {
	if (selectionMode)
		selectionMode->clearSelectedOperators();
}

void PageSpace::setSelectionMode( QString mode, QString scriptFncAtMouseRelease, QString drawingObject ) {
	selectionMode.reset(  PageViewModeFactory::create( mode, drawingObject, scriptFncAtMouseRelease ) );
	if (pageImage)
		pageImage->setSelectionMode( selectionMode );

	if (selectionMode) {
		connect( selectionMode.get(), SIGNAL( popupMenu(const QPoint &) ),
				this, SLOT( requestPopupMenu(const QPoint &) ) );
		connect( selectionMode.get(), SIGNAL( newSelectedOperators(const std::vector< boost::shared_ptr< PdfOperator > > &) ),
				this, SLOT( newSelection(const std::vector< boost::shared_ptr< PdfOperator > > &) ) );
		connect( selectionMode.get(), SIGNAL( executeCommand(QString) ),
				this, SIGNAL( executeCommand(QString) ) );
	}
}

void PageSpace::setSelectArea ( int left, int top, int right, int bottom ) {
	if (selectionMode)
		selectionMode->setSelectedRegion( QRegion( left, top, right-left+1, bottom-top+1 ) );
}

void PageSpace::newSelection ( const std::vector< boost::shared_ptr< PdfOperator > > & objects ) {
	emit changeSelection( objects );
}

bool PageSpace::isSomeoneSelected ( ) {
	if (selectionMode)
		return selectionMode->isSomeoneSelected();
	else
		return false;
}

//----------------------------------------------------------------------

bool PageSpace::saveImageWithDialog ( bool onlySelectedArea ) {
	QString filters = "";
	for (unsigned int i = 0; i < QImageIO::outputFormats().count() ;++i) {
		filters += QString("\n%1 (*.%2)")
				.arg(QImageIO::outputFormats().at(i))
				.arg(QString(QImageIO::outputFormats().at(i)).lower());
	}

	QString sf;
	QString filename = QFileDialog::getSaveFileName( "", filters, NULL, NULL, tr("Save as image ..."), & sf );
	sf = sf.left( sf.find(' ') );

	if (! filename.isNull())
		return saveImage( filename, sf, -1, onlySelectedArea );

	return false;
}

bool PageSpace::saveImage ( const QString & filename, const char * format, int quality, bool onlySelectedArea ) {
	guiPrintDbg( debug::DBG_DBG, "save");
	return pageImage->saveImage( filename, format, quality, onlySelectedArea );
}

void PageSpace::requestPopupMenu ( const QPoint & pagePos ) {
	emit popupMenu( pagePos );
}

//----------------------------------------------------------------------

int PageSpace::findText ( QString &text, bool startAtTop, double xStart, double yStart, double xEnd, double yEnd) {
	TextSearchParams tsp = TextSearchParams();
	tsp.startAtTop = startAtTop;
	tsp.xStart = xStart;
	tsp.yStart = yStart;
	tsp.xEnd = xEnd;
	tsp.yEnd = yEnd;

	std::vector<Rectangle> recs;
	int count = actualPage->get()->findText( text, recs, tsp );

	guiPrintDbg( debug::DBG_DBG, "Founded "<<count<<" items:");
	for (std::vector<Rectangle>::iterator it = recs.begin(); it != recs.end() ; ++it) {
		guiPrintDbg( debug::DBG_DBG, "   ("<<(*it)<<")");
	}

	unselectObjectOnPage();

	// get all text operators
	std::vector<boost::shared_ptr<PdfOperator> > foundedIn;
	std::vector<boost::shared_ptr<PdfOperator> > ops;
	std::vector<boost::shared_ptr<CContentStream> > ccs;
	actualPage->get()->getContentStreams(ccs);
	// loop through all of content streams of actual page
	for ( std::vector<boost::shared_ptr<CContentStream> >::iterator ccsIt = ccs.begin(); ccsIt != ccs.end() ; ++ccsIt ) {
		(*ccsIt)->getPdfOperators( ops );

		pdfobjects::TextOperatorIterator it (ops[0]);

		// loop through container of operators one of content stream of actual page
		for ( ; !it.isEnd() ; it.next() ) {
			boost::shared_ptr<PdfOperator> h_textOp ( it.getCurrent() );

			Rectangle bbox = h_textOp->getBBox();
			if (bbox.xleft > bbox.xright) {
				double h = bbox.xleft;
				bbox.xleft = bbox.xright;
				bbox.xright = h;
			}
			if (bbox.yleft > bbox.yright) {
				double h = bbox.yleft;
				bbox.yleft = bbox.yright;
				bbox.yright = h;
			}
			// reduce small diference in compare
			bbox.xleft += bbox.xleft / 1e10;
			bbox.xright -= bbox.xright / 1e10;
			bbox.yleft += bbox.yleft / 1e10;
			bbox.yright -= bbox.yright / 1e10;

			// loop through bboxes of founded text
			for ( std::vector<Rectangle>::iterator it_recs = recs.begin(); it_recs != recs.end() ; ++it_recs ) {
				if ((	( std::min( (*it_recs).xleft, (*it_recs).xright ) <= bbox.xleft ) &&	// if operators width is all inside founded area
						( std::max( (*it_recs).xleft, (*it_recs).xright ) >= bbox.xright ) &&
						(std::max( std::min( (*it_recs).yleft, (*it_recs).yright ) , bbox.yleft ) <=	// and has some height intersection
							std::min( std::max( (*it_recs).yleft, (*it_recs).yright ) , bbox.yright ))) ||
					(	( std::min( (*it_recs).yleft, (*it_recs).yright ) <= bbox.yleft ) &&	// or operators height is all inside founded area
						( std::max( (*it_recs).yleft, (*it_recs).yright ) >= bbox.yright ) &&
						(std::max( std::min( (*it_recs).xleft, (*it_recs).xright ) , bbox.xleft ) <=	// and has some width intersection
							std::min( std::max( (*it_recs).xleft, (*it_recs).xright ) , bbox.xright ))) )
				{
					std::string a;
					h_textOp->getStringRepresentation(a);
					guiPrintDbg( debug::DBG_DBG, "vybrane ("<< std::min(bbox.xleft,bbox.xright) <<","<<
																std::min(bbox.yleft, bbox.yright)<<","<<
																std::max(bbox.xright, bbox.xleft)<<","<<
																std::max(bbox.yleft, bbox.yright)<<")   "<<
																a);

					foundedIn.push_back( h_textOp );

					break;
				}
			}
		}
	}

	if (selectionMode)
		selectionMode->addSelectedOperators( foundedIn );	// TODO change mode to textmode

	return count;
}

//----------------------------------------------------------------------

double PageSpace::convertPixmapPosToPdfPos_x( double fromX, double fromY ) {
	if (pageImage) {
		double toX, toY;
		pageImage->convertPixmapPosToPdfPos( fromX, fromY, toX, toY );
		return toX;
	}

	return fromX;
}
double PageSpace::convertPixmapPosToPdfPos_y( double fromX, double fromY ) {
	if (pageImage) {
		double toX, toY;
		pageImage->convertPixmapPosToPdfPos( fromX, fromY, toX, toY );
		return toY;
	}

	return fromY;
}
void PageSpace::convertPixmapPosToPdfPos( const QPoint & pos, Point & pdfPos ) {
	if (pageImage) {
		double toX, toY;
		pageImage->convertPixmapPosToPdfPos( pos.x(), pos.y(), toX, toY );

		pdfPos.x = toX;
		pdfPos.y = toY;
		return ;
	}

	pdfPos.x = pos.x();
	pdfPos.y = pos.y();
}

double PageSpace::convertPdfPosToPixmapPos_x( double fromX, double fromY ) {
	if (pageImage) {
		double toX, toY;
		pageImage->convertPdfPosToPixmapPos( fromX, fromY, toX, toY );
		return toX;
	}

	return fromX;
}
double PageSpace::convertPdfPosToPixmapPos_y( double fromX, double fromY ) {
	if (pageImage) {
		double toX, toY;
		pageImage->convertPdfPosToPixmapPos( fromX, fromY, toX, toY );
		return toY;
	}

	return fromY;
}
void PageSpace::convertPdfPosToPixmapPos( const Point & pdfPos, QPoint & pos ) {
	if (pageImage) {
		double toX, toY;
		pageImage->convertPdfPosToPixmapPos( pdfPos.x, pdfPos.y, toX, toY );

		pos.setX( (int) toX );
		pos.setY( (int) toY );
		return ;
	}

	pos.setX( (int) pdfPos.x );
	pos.setY( (int) pdfPos.y );
}

//  ------------------------------------------------------  //
//  --------------------   ZOOM  -------------------------  //
//  ------------------------------------------------------  //
void PageSpace::setZoomFactor ( float set_zoomFactor ) {
	if (pageImage == NULL)
		return;

	if (pageImage->getZoomFactor() == set_zoomFactor)
		return;

	emit changedZoomFactorTo( pageImage->setZoomFactor( set_zoomFactor ) );
}

float PageSpace::getZoomFactor ( ) {
	if (pageImage == NULL)
		return 1;

	return pageImage->getZoomFactor();
}

void PageSpace::zoomTo ( int percentage ) {
	if (percentage < 1)
		percentage = 1;

	setZoomFactor ( percentage/100.0 );
}
void PageSpace::zoomIn ( float step ) {
	if (pageImage != NULL)
		setZoomFactor ( pageImage->getZoomFactor() + step );
}
void PageSpace::zoomOut ( float step ) {
	if (pageImage == NULL)
		return ;

	if (pageImage->getZoomFactor() - step < 0.01)
		setZoomFactor ( 0.01 );
	else
		setZoomFactor ( pageImage->getZoomFactor() - step );
}

// ------------------------- end  ZOOM

void PageSpace::showMousePosition ( double x, double y ) {
	QString pom;
	pom = pom.sprintf( format, x, y );
	mousePositionOnPage->setText( pom );
}

void PageSpace::firstPage ( ) {
	if (!actualPdf)
		return;
	QSPage p (actualPdf->get()->getFirstPage(),NULL);
	refresh( &p, actualPdf );
}
void PageSpace::prevPage ( ) {
	if (!actualPdf)
		return;
	if (!actualPage) {
		firstPage ();
		return;
	}

	if ( (actualPdf->get()->hasPrevPage( actualPage->get() )) == true) {
		QSPage p (actualPdf->get()->getPrevPage( actualPage->get() ) , NULL);
		refresh( &p, actualPdf );
	} else {
		// if actual page is removed then show new page at same position -1
		try {
			actualPdf->getPagePosition( actualPage );
		} catch (PageNotFoundException) {
			refresh( actualPagePos -1 );
		}
	}
}
void PageSpace::nextPage ( ) {
	if (!actualPdf)
		return;
	if (!actualPage) {
		firstPage ();
		return;
	}

	if ( (actualPdf->get()->hasNextPage( actualPage->get() )) == true) {
		QSPage p (actualPdf->get()->getNextPage( actualPage->get() ) , NULL);
		refresh( &p, actualPdf );
	} else {
		// if actual page is removed then show new page at same position
		try {
			actualPdf->getPagePosition( actualPage );
		} catch (PageNotFoundException) {
			refresh( actualPagePos );
		}
	}
}
void PageSpace::lastPage ( ) {
	if (!actualPdf)
		return;
	QSPage p (actualPdf->get()->getLastPage() , NULL);
	refresh( &p, actualPdf );
}
} // namespace gui
