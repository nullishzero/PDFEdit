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
#include "rect2Darray.h"

// using namespace std;

namespace gui {

typedef struct { int labelWidth, labelHeight; } initStruct;
// TODO asi prepracovat
void Init( initStruct * is, const QString & s ) {
	QLabel pageNumber( s ,0);
	pageNumber.show();
	is->labelWidth = pageNumber.width();
	is->labelHeight = pageNumber.height();
}


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

	lastSelectedRect = QRect();
	requestsForSelecting.clear();
	actualSelectedObjects.clear();
	actualSelectedObjects.setAutoDelete( true );
	objectForSelecting.clear();
	objectForSelecting.setAutoDelete( true );

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

	selectionMode = PageView::SelectAllObjects;
	oldSelectionMode = selectionMode;
}

PageSpace::~PageSpace() {
	delete actualPdf;
	delete actualPage;
	delete actualPagePixmap;
}

void PageSpace::hideButtonsAndPageNumber ( ) {
	pageNumber->hide();
	mousePositionOnPage->hide();
}
void PageSpace::showButtonsAndPageNumber ( ) {
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

	pageImage->setResizingZone( globalSettings->readNum( PAGESPC + "ResizingZone" ) );

	connect( pageImage, SIGNAL( leftClicked(const QRect &) ), this, SLOT( newSelection(const QRect &) ) );
	connect( pageImage, SIGNAL( rightClicked(const QPoint &, const QRegion *) ),
			this, SLOT( requirementPopupMenu(const QPoint &, const QRegion *) ) );
	connect( pageImage, SIGNAL( selectionMoved(const QPoint &, const QPtrList<BBoxOfObjectOnPage> &) ),
			this, SLOT( moveSelection(const QPoint &, const QPtrList<BBoxOfObjectOnPage> &) ) );
	connect( pageImage, SIGNAL( selectionResized(const QRect &, const QRect &, const QPtrList<BBoxOfObjectOnPage> &) ),
			this, SLOT( resizeSelection(const QRect &, const QRect &, const QPtrList<BBoxOfObjectOnPage> &) ) );
	connect( pageImage, SIGNAL( newSelectedObjects(const QPtrList<BBoxOfObjectOnPage> &) ),
			this, SLOT( newSelection(const QPtrList<BBoxOfObjectOnPage> &) ) );
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

	setSelectionMode( selectionMode );
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

		actualSelectedObjects.clear();

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

	// create pixmap for page
	actualPage->get()->displayPage( output, displayParams );

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
	objectForSelecting.clear();

	emit changedPageTo( * actualPage, actualPdf->getPagePosition( actualPage ) );

	// reset selection mode
	int h = selectionMode;
	selectionMode = -1;
	setSelectionMode( h );
	//TODO actualSelectedObjects ?
}
#undef splashMakeRGB8

void PageSpace::keyPressEvent ( QKeyEvent * e ) {
	switch ( e->key() ) {
		case Qt::Key_Escape :
			if ( ! pageImage->escapeSelection() )
				e->ignore();
			break;
		default:
			e->ignore();
	}
}

bool PageSpace::refreshObjectsInPageImage() {
	pageImage->clearObjectsBBox();
	pageImage->unSelect();

	if (! objectForSelecting.isEmpty()) {
		boost::shared_ptr<PdfOperator> * it = objectForSelecting.first();
		for ( ; it ; it = objectForSelecting.next() ) {
			Rectangle bbox = it->get()->getBBox();
			QRect box ( (int) std::min(bbox.xleft,bbox.xright), (int) std::min(bbox.yleft, bbox.yright),
						std::abs((int)(bbox.xright - bbox.xleft))+1, std::abs((int)(bbox.yleft - bbox.yright))+1);

			pageImage->addObjectsBBox( box, it->get() );
		}
	}

	bool returnValue = pageImage->setSelectionMode( PageView::SelectText );

	if (! actualSelectedObjects.isEmpty()) {
		boost::shared_ptr<PdfOperator> * it = actualSelectedObjects.first();
		for ( ; it ; it = actualSelectedObjects.next() ) {
			Rectangle bbox = it->get()->getBBox();
			QRect box ( (int) std::min(bbox.xleft,bbox.xright), (int) std::min(bbox.yleft, bbox.yright),
						std::abs((int)(bbox.xright - bbox.xleft))+1, std::abs((int)(bbox.yleft - bbox.yright))+1);

			pageImage->addSelectedRegion( box, it->get() );
		}
	}

	return returnValue;
}

bool PageSpace::requestSelectArea ( int id ) {
	//TODO
	requestsForSelecting.push_back( id );
	if (selectionMode != PageView::SelectRect) {
		oldSelectionMode = selectionMode;
		if (! setSelectionMode( PageView::SelectRect ) ) {
			requestsForSelecting.pop_back();
			return false;
		}
	}
	return true;
}

void PageSpace::selectObjectOnPage ( std::vector<boost::shared_ptr<PdfOperator> > ops ) {
	lastSelectedRect = QRect();

	actualSelectedObjects.clear();
	for (std::vector<boost::shared_ptr<PdfOperator> >::iterator it = ops.begin(); it != ops.end() ; ++it) {
		Rectangle bbox;
		std::string name;
		std::string strr;
		(*it)->getOperatorName( name );
		(*it)->getStringRepresentation( strr );
		bbox = (*it)->getBBox( );

		QRect box ( (int) std::min(bbox.xleft,bbox.xright), (int) std::min(bbox.yleft, bbox.yright),
					std::abs((int)(bbox.xright - bbox.xleft))+1, std::abs((int)(bbox.yleft - bbox.yright))+1);
		lastSelectedRect |= box;

		pageImage->addSelectedRegion( box, (*it).get() );
		actualSelectedObjects.append( new boost::shared_ptr<PdfOperator> (*it) );
	}

	newSelection();
}

void PageSpace::unselectObjectOnPage ( ) {
	pageImage->unSelect();
	actualSelectedObjects.clear();
}

bool PageSpace::setSelectionMode( int mode ) {
	if (pageImage == NULL)
		return false;

	pageImage->clearObjectsBBox();
	pageImage->unSelect();

	bool returnValue = false;
	switch (mode) {
		case PageView::SelectRect : {
				returnValue = pageImage->setSelectionMode( PageView::SelectRect );
				if (mode == selectionMode)
					pageImage->setSelectedRegion( lastSelectedRect, NULL );
				break;
		}
		case PageView::SelectAllObjects : {
				objectForSelecting.clear();
				actualSelectedObjects.clear();
				returnValue = pageImage->setSelectionMode( PageView::SelectAllObjects );
				break;
		}
		case PageView::SelectText : {
				if (mode == selectionMode) {
					returnValue = refreshObjectsInPageImage();
				} else {
					// init objectForSelecting
					std::vector<boost::shared_ptr<PdfOperator> > ops;
					std::vector<boost::shared_ptr<CContentStream> > ccs;
					actualPage->get()->getContentStreams(ccs);
					for ( std::vector<boost::shared_ptr<CContentStream> >::iterator ccsIt = ccs.begin(); ccsIt != ccs.end() ; ++ccsIt ) {
						(*ccsIt)->getPdfOperators( ops );

						PdfOperator::Iterator *it=NULL;
						it = new TextOperatorIterator(ops[0]);
						for ( ; !it->isEnd() ; it->next() ) {
							boost::shared_ptr<PdfOperator> * h_textOp = new boost::shared_ptr<PdfOperator> (it->getCurrent());

							Rectangle bbox = (* h_textOp)->getBBox();
							QRect box ( (int) std::min(bbox.xleft,bbox.xright), (int) std::min(bbox.yleft, bbox.yright),
										std::abs((int)(bbox.xright - bbox.xleft))+1, std::abs((int)(bbox.yleft - bbox.yright))+1);

							objectForSelecting.append( h_textOp );
							pageImage->addObjectsBBox( box, (*h_textOp).get() );
						}
						delete it;
					}
				}
				returnValue = pageImage->setSelectionMode( PageView::SelectText );
				break;
			}
		default:
				guiPrintDbg( debug::DBG_DBG, "not set selection mode to "<<mode);
				guiPrintDbg( debug::DBG_DBG, "All objects = "<<(int)PageView::SelectAllObjects);
				guiPrintDbg( debug::DBG_DBG, "Text	  = "<<(int)PageView::SelectText);
				guiPrintDbg( debug::DBG_DBG, "Rect        = "<<(int)PageView::SelectRect);
				break;
	}

	selectionMode = mode;

	return returnValue;
}

void PageSpace::newSelection () {
	std::vector<boost::shared_ptr<PdfOperator> >	sops;
	
	boost::shared_ptr<PdfOperator> * it = actualSelectedObjects.first();
	for ( ; it ; it = actualSelectedObjects.next() ) {
		sops.push_back(*it);
	}

	emit changeSelection ( sops );
}

void PageSpace::newSelection ( const QPtrList<BBoxOfObjectOnPage> & objects ) {
	actualSelectedObjects.clear();

	QPtrListIterator<BBoxOfObjectOnPage> it_objs (objects);
	for ( ; *it_objs ; ++it_objs ) {
		const void * h = (*it_objs)->getPtrToObject();
		QPtrListIterator<boost::shared_ptr<PdfOperator> > it (objectForSelecting);
		for ( ; *it ; ++it ) {
			if ((*it)->get() == h) {
				actualSelectedObjects.append( new boost::shared_ptr<PdfOperator> (*(*it)) );
				break;
			}
		}
	}

	newSelection();
}

void PageSpace::newSelection ( const QRect & r) {
	lastSelectedRect = r;

	if (selectionMode == PageView::SelectAllObjects) {
		std::vector<boost::shared_ptr<PdfOperator> > ops;

		pageImage->unSelect();

		if (actualPage != NULL) {
			if ( r.topLeft() == r.bottomRight() ) {
				Point p;
				convertPixmapPosToPdfPos( r.topLeft(), p );
				actualPage->get()->getObjectsAtPosition( ops, Point( p.x, p.y ) );
			} else {
				Point p1, p2;
				convertPixmapPosToPdfPos( r.topLeft(), p1 );
				convertPixmapPosToPdfPos( r.bottomRight(), p2 );
				actualPage->get()->getObjectsAtPosition( ops, Rectangle( p1.x, p1.y, p2.x, p2.y ) );
			}
		}

		selectObjectOnPage( ops );
	} else
		if (! requestsForSelecting.empty()) {		// someone send request for selection
			int cur = requestsForSelecting.back();
			requestsForSelecting.pop_back();
			emit responseSelectArea ( cur, true, r.left(), r.top(), r.right(), r.bottom() );

			while (! requestsForSelecting.empty()) {
				cur = requestsForSelecting.back();
				requestsForSelecting.pop_back();
				emit responseSelectArea ( cur, false, r.left(), r.top(), r.right(), r.bottom() );
			}

			setSelectionMode( oldSelectionMode );
		}
}

bool PageSpace::isSomeoneSelected ( ) {
	return ( ! actualSelectedObjects.isEmpty() );
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

void PageSpace::requirementPopupMenu ( const QPoint & pagePos, const QRegion * /* r */) {
/*	if (r != NULL) {
	} else {
	}*/
	emit popupMenu( pagePos );
}
void PageSpace::moveSelection ( const QPoint & relativeMove, const QPtrList<BBoxOfObjectOnPage> & objects ) {
	lastSelectedRect.moveBy( relativeMove.x(), relativeMove.y() );
	// TODO
}
void PageSpace::resizeSelection ( const QRect &, const QRect &, const QPtrList<BBoxOfObjectOnPage> & ) {
	// TODO
}

void PageSpace::convertPdfPosToPixmapPos( const Point & pdfPos, QPoint & pos ) {
	if (actualPage == NULL) {
		pos.setX(0);
		pos.setY(0);
		return ;
	}
	pos.setX( (int) pdfPos.x );
	if (! displayParams.upsideDown)
		pos.setY( (int) (actualPagePixmap->height() - pdfPos.y) );
	else
		pos.setY( (int) pdfPos.y );
}

void PageSpace::convertPixmapPosToPdfPos( const QPoint & pos, Point & pdfPos ) {
	if (actualPage == NULL) {
		pdfPos.x = 0;
		pdfPos.y = 0;
		return ;
	}
	pdfPos.x = pos.x();
	if (! displayParams.upsideDown)
		pdfPos.y = actualPagePixmap->height() - pos.y();
	else
		pdfPos.y = pos.y();
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
	Point pdfPagePos;
	convertPixmapPosToPdfPos( pos, pdfPagePos );
	pom = pom.sprintf( format, pdfPagePos.x, pdfPagePos.y );
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
