#include "pagespace.h"
#include "settings.h"
#include <stdlib.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpixmap.h>
#include "cpage.h"
#include "static.h"
#include "QOutputDevPixmap.h"
#include <qdockarea.h>
#include <qfiledialog.h>
#include <math.h>
#include <cpdf.h>
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
	
	scrollPageSpace = new QScrollView(this);
	vBox->addWidget(scrollPageSpace);
//QObject *pomObj = this;
//while (pomObj->parent() !=NULL) pomObj = pomObj->parent();
QDockArea * da = new QDockArea( Qt::Horizontal, QDockArea::Normal, this /*dynamic_cast<QWidget *>(pomObj)*/ );
vBox->addWidget( da );
	
	hBox = new QHBoxLayout(vBox);

	displayParams = DisplayParams();
	actualPdf = NULL;
	actualPage = NULL;

	pageImage = NULL;
	actualPagePixmap = new QPixmap();
	newPageView( *actualPagePixmap );
	delete actualPagePixmap;
	actualPagePixmap = NULL;
//	scrollPageSpace->setBackGroundRole(QPalette::Dark); // TODO configurovatelna farba
	
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

	basePpP		= QPaintDevice::x11AppDpiX() / 72.0;
	zoomFactor	= 1;

	selectionMode.reset();
	setSelectionMode( "new_object","script","line" );
	setSelectionMode( "text_selection","script","line" );
}

PageSpace::~PageSpace() {
	delete actualPdf;
	delete actualPage;
	delete actualPagePixmap;
}

void PageSpace::hidePageNumberAndPosition  ( ) {
	pageNumber->hide();
	mousePositionOnPage->hide();
}
void PageSpace::showPageNumberAndPosition  ( ) {
	pageNumber->show();
	mousePositionOnPage->show();
}

void PageSpace::newPageView() {
	if (pageImage != NULL) {
		scrollPageSpace->removeChild( pageImage );
		delete pageImage;
	}
	pageImage = new PageView(scrollPageSpace->viewport());
	scrollPageSpace->addChild(pageImage);

	pageImage->setSelectionMode( selectionMode );

	connect( pageImage, SIGNAL( changeMousePosition(const QPoint &) ), this, SLOT( showMousePosition(const QPoint &) ) );
}

void PageSpace::newPageView( QPixmap &qp ) {
	if (qp.isNull()) {
		scrollPageSpace->removeChild( pageImage );
		delete pageImage;
		return;
	}
	newPageView();
	pageImage->setPixmap( qp );
	pageImage->show();
	centerPageView();
}
void PageSpace::centerPageView( ) {
	bool reposition;
	int posX, posY;

	// If no page is shown, don't center
	if (pageImage == NULL)
		return;

	// When page is refreshing, pageImage->width() is not correct the page's width (same height)
	if (pageImage->pixmap() == NULL) {
		posX = pageImage->width();
		posY = pageImage->height();
	} else {
		posX = pageImage->pixmap()->width();
		posY = pageImage->pixmap()->height();
	}

	// Calculation topLeft position of page on scrollPageSpace
	if (reposition = (posX = (scrollPageSpace->visibleWidth() - posX) / 2) < 0 )
		posX = 0;
	if ((posY = (scrollPageSpace->visibleHeight() - posY) / 2) < 0 )
		posY = 0;
	else
		reposition = false;
	// If scrollPageSpace is smaller then page and page's position on scrollPageSpace is not set to (0,0), must be changed
	reposition = reposition && (0 == (scrollPageSpace->childX(pageImage) + scrollPageSpace->childY(pageImage)));

	if (! reposition) {
		// move page to center of scrollPageSpace
		scrollPageSpace->moveChild( pageImage, posX, posY );
	}
}

void PageSpace::resizeEvent ( QResizeEvent * re) {
	this->QWidget::resizeEvent( re );

	centerPageView();
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

#define splashMakeRGB8(to, r, g, b) \
	  (to[3]=0, to[2]=((r) & 0xff) , to[1]=((g) & 0xff) , to[0]=((b) & 0xff) )

void PageSpace::refresh ( QSPage * pageToView, QSPdf * pdf ) {		// if pageToView is NULL, refresh actual page
									// if pageToView == actualPage  refresh is not need
	if ((pageToView != NULL) && ( (actualPage == NULL) || (actualPage->get() != pageToView->get()) ) && ( (pdf != NULL) || (actualPdf != NULL) ) ) {
		if ((actualPdf == NULL) || (pdf->get() != actualPdf->get())) {
			delete actualPdf;
			actualPdf = new QSPdf( pdf->get() , NULL );
		}
		delete actualPage;
		actualPage = new QSPage( pageToView->get() , NULL );

		pageNumber->setText( QString(tr("%1 of %2"))
					.arg(actualPdf->getPagePosition( actualPage ))
					.arg(actualPdf->getPageCount()) );
	} else {
		if ((actualPage == NULL) || (actualPdf == NULL) || (pageToView != NULL))
			return ;					// no page to refresh
		// else  need reload page ( changed zoom, ... )
	}

	// initialize create pixmap for page
	SplashColor paperColor;
	splashMakeRGB8(paperColor, 0xff, 0xff, 0xff);
	QOutputDevPixmap output ( paperColor );

	// -------------- update display parameters --------------

	// update upsideDown
	displayParams.upsideDown = output.upsideDown();

	// update mediabox
	try {
		Rectangle mb = actualPage->get()->getMediabox();
		displayParams.pageRect = mb;
	} catch (ElementNotFoundException) {
		// TODO find mediabox in parent
		displayParams.pageRect = DisplayParams().pageRect;
	}

	// update rotate
	try {
		displayParams.rotate = actualPage->get()->getRotation();
	} catch (ElementNotFoundException) {
		// TODO find rotate in parent
		displayParams.rotate = 0;
	}
	// TODO  cropbox, ...

	// -----end------ update display parameters --------------


	// create pixmap for page
	actualPage->get()->displayPage( output, displayParams );

	// initialize work operators in mode
	initializeWorkOperatorsInMode();

	// get created pixmap
	delete actualPagePixmap;
	QImage img = output.getImage();
	if (img.isNull())
		actualPagePixmap = new QPixmap();
	else {
		actualPagePixmap = new QPixmap( img );
	}

	// show new pixmap
	newPageView( * actualPagePixmap );

	emit changedPageTo( * actualPage, actualPdf->getPagePosition( actualPage ) );
}
#undef splashMakeRGB8

void PageSpace::initializeWorkOperatorsInMode() {
	if (selectionMode && actualPage) {
		selectionMode->clearWorkOperators();

		std::vector< boost::shared_ptr< PdfOperator > > ops;

		std::vector< boost::shared_ptr< CContentStream > > ccs;
		actualPage->get()->getContentStreams( ccs );

		for ( std::vector<boost::shared_ptr<CContentStream> >::iterator ccsIt = ccs.begin(); ccsIt != ccs.end() ; ++ccsIt ) {
			(*ccsIt)->getPdfOperators( ops );

			selectionMode->addWorkOperators( ops );
		}

		// actualize selected operators in mode
		selectionMode->actualizeSelection();
	}
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

	// initialize work operators in mode
	initializeWorkOperatorsInMode();
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

double PageSpace::convertPixmapPosToPdfPos_x( double fromX, double fromY ) {
	if (actualPage == NULL)
		return 0;

	double * ctm /*[6]*/;
	double h;
	PDFRectangle pdfRect ( displayParams.pageRect.xleft, displayParams.pageRect.yleft,
							displayParams.pageRect.xright, displayParams.pageRect.yright );
	GfxState state (displayParams.hDpi, displayParams.vDpi, &pdfRect, displayParams.rotate, displayParams.upsideDown );
	ctm = state.getCTM();

	h = (ctm[0]*ctm[3] - ctm[1]*ctm[2]);

	assert( h != 0 );

	return  (fromX*ctm[3] - ctm[2]*fromY + ctm[2]*ctm[5] - ctm[4]*ctm[3]) / h;
}
double PageSpace::convertPixmapPosToPdfPos_y( double fromX, double fromY ) {
	if (actualPage == NULL)
		return 0;

	double * ctm /*[6]*/;
	double h;
	PDFRectangle pdfRect ( displayParams.pageRect.xleft, displayParams.pageRect.yleft,
							displayParams.pageRect.xright, displayParams.pageRect.yright );
	GfxState state (displayParams.hDpi, displayParams.vDpi, &pdfRect, displayParams.rotate, displayParams.upsideDown );
	ctm = state.getCTM();

	h = (ctm[0]*ctm[3] - ctm[1]*ctm[2]);

	assert( h != 0 );

	return  (ctm[0]*fromY + ctm[1]*ctm[4] - ctm[0]*ctm[5] - ctm[1]*fromX) / h;
}

void PageSpace::convertPdfPosToPixmapPos( const Point & pdfPos, QPoint & pos ) {
	pos.setX( (int) convertPdfPosToPixmapPos_x( pdfPos.x, pdfPos.y) );
	pos.setY( (int) convertPdfPosToPixmapPos_y( pdfPos.x, pdfPos.y) );
}

double PageSpace::convertPdfPosToPixmapPos_x( double fromX, double fromY ) {
	if (actualPage == NULL)
		return 0;

	double hx, hy;
	PDFRectangle pdfRect ( displayParams.pageRect.xleft, displayParams.pageRect.yleft,
							displayParams.pageRect.xright, displayParams.pageRect.yright );
	GfxState state (displayParams.hDpi, displayParams.vDpi, &pdfRect, displayParams.rotate, displayParams.upsideDown );

	state.transform( fromX, fromY, &hx, &hy );
	return hx;
}
double PageSpace::convertPdfPosToPixmapPos_y( double fromX, double fromY ) {
	if (actualPage == NULL)
		return 0;

	double hx, hy;
	PDFRectangle pdfRect ( displayParams.pageRect.xleft, displayParams.pageRect.yleft,
							displayParams.pageRect.xright, displayParams.pageRect.yright );
	GfxState state (displayParams.hDpi, displayParams.vDpi, &pdfRect, displayParams.rotate, displayParams.upsideDown );

	state.transform( fromX, fromY, &hx, &hy );
	return hy;
}
void PageSpace::convertPixmapPosToPdfPos( const QPoint & pos, Point & pdfPos ) {
	pdfPos.x = convertPixmapPosToPdfPos_x( pos.x(), pos.y());
	pdfPos.y = convertPixmapPosToPdfPos_y( pos.x(), pos.y());
}

//  ------------------------------------------------------  //
//  --------------------   ZOOM  -------------------------  //
//  ------------------------------------------------------  //
void PageSpace::setZoomFactor ( float set_zoomFactor ) {
	if (zoomFactor == set_zoomFactor)
		return;

	emit changedZoomFactorTo(set_zoomFactor);

	if (displayParams.useMediaBox == gFalse) {
		float pom = set_zoomFactor / zoomFactor;
		zoomFactor = set_zoomFactor;
		// TODO
		return ;
	}

	zoomFactor = set_zoomFactor;
	displayParams.hDpi = basePpP * zoomFactor * 72;
	displayParams.vDpi = basePpP * zoomFactor * 72;
	refresh ();
}

float PageSpace::getZoomFactor ( ) {
	return zoomFactor;
}

void PageSpace::zoomTo ( int percentage ) {
	if (percentage < 1)
		percentage = 1;

	float pom = percentage;
	pom=pom/100;
	fprintf(stderr,"%f\n",pom);
	setZoomFactor ( pom );
}
void PageSpace::zoomIn ( float step ) {
	setZoomFactor ( zoomFactor + step );
}
void PageSpace::zoomOut ( float step ) {
	if (zoomFactor - step < 0.01)
		setZoomFactor ( 0.01 );
	else
		setZoomFactor ( zoomFactor - step );
}

// ------------------------- end  ZOOM

void PageSpace::showMousePosition ( const QPoint & pos ) {
	QString pom;
	pom = pom.sprintf( format, (double)pos.x(), (double)pos.y() );
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
	}
}
void PageSpace::lastPage ( ) {
	if (!actualPdf)
		return;
	QSPage p (actualPdf->get()->getLastPage() , NULL);
	refresh( &p, actualPdf );
}
} // namespace gui
