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
#include "pagespace.h"
#include QSCROLLVIEW
#include <stdlib.h>
#include <math.h>

#include <qlabel.h>
#include <qstring.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qimage.h>
//#include <qdockarea.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include QSTRLIST

#include "settings.h"
#include "kernel/cpdf.h"
#include "kernel/cpage.h"
#include "kernel/static.h"
#include "rect2Darray.h"

#include "pageviewmode.h"
#include "units.h"
#ifdef QT4
#include <QImageWriter>
#endif

/** Helpes struct for initializing width and height text labeles. */
typedef struct { /** lebel width */ int labelWidth, /** label height */ labelHeight; } initStruct;

/** Helpes init function for get correct width and height label of text.
 * @param is	Struct which will be updated.
 * @param s		Text which be in label.
 */
void Init( initStruct * is, const QString & s ) {
	QLabel pageNumber( s ,0);
	pageNumber.show();
	is->labelWidth = pageNumber.width();
	is->labelHeight = pageNumber.height();
}


// using namespace std;

namespace gui {

/** Path to setting, where are stored settings for PageSpace. */
QString PAGESPC = "gui/PageSpace/";
/** Name of setting for resizing zone. */
QString RESIZINGZONE = "ResizingZone";
/** Default value for resizing zone. */
int DEFAULT__RESIZINGZONE = 2;
/** Name of setting for viewed units. */
QString VIEWED_UNITS = "ViewedUnits";
/** Default value for viewed units. */
QString DEFAULT__VIEWED_UNITS = "cm";
/** Template for viewing mouse position on page. */
QString format = "x:%1 y:%2 %3";

TextLine::TextLine ()
	: QMainWindow ( NULL, NULL, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_StaysOnTop | Qt::WDestructiveClose )
{
	edit = new QLineEdit ( this, "textline" );
	edit->setFrame( false );
//	edit->setFocus ();
	setCentralWidget ( edit );
	setSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Minimum );

	connect( edit, SIGNAL( lostFocus() ), this, SLOT( lostFocus() ) );
	connect( edit, SIGNAL( returnPressed() ), this, SLOT( returnPressed() ) );
}
TextLine::~TextLine () {
}
void TextLine::keyReleaseEvent ( QKeyEvent * e ) {
	switch (e->key()) {
		case Qt::Key_Escape:
			emit escape();
			disconnect( SIGNAL( lostFocus( const QString & ) ) );
			disconnect( SIGNAL( returnPressed( const QString & ) ) );
			hide();
	}
}
void TextLine::setText( const QString & s ) {
	edit->setText( s );
}
void TextLine::lostFocus () {
	emit lostFocus ( edit->text() );
	disconnect( SIGNAL( lostFocus( const QString & ) ) );
	disconnect( SIGNAL( returnPressed( const QString & ) ) );
	hide();
}
void TextLine::returnPressed () {
	emit returnPressed ( edit->text() );
	disconnect( SIGNAL( lostFocus( const QString & ) ) );
	disconnect( SIGNAL( returnPressed( const QString & ) ) );
	hide();
}

//  ----------------------------------------------------------
//  --------------------   PageSpace   -----------------------
//  ----------------------------------------------------------
PageSpace::PageSpace(QWidget *parent /*=0*/, const char *name /*=0*/) : QWidget(parent,name) {
	initStruct is;

	vBox = new QVBoxLayout(this);
	
	scrollPageSpace = pageImage = new PageViewS(this);
	setFocusProxy(pageImage);
	connect( pageImage, SIGNAL( changeMousePosition(double, double) ), this, SLOT( showMousePosition(double, double) ) );
	connect( pageImage, SIGNAL( changeMousePosition(double, double) ), this, SIGNAL( changeMousePosition(double, double) ) );
	vBox->addWidget(scrollPageSpace);
//QObject *pomObj = this;
//while (pomObj->parent() !=NULL) pomObj = pomObj->parent();
//QDockArea * da = new QDockArea( Qt::Horizontal, QDockArea::Normal, this /*dynamic_cast<QWidget *>(pomObj)*/ );
//vBox->addWidget( da );
	
	hBox = new QHBoxLayout(vBox);

	actualPdf = NULL;
	actualPage.reset();
	actualPagePos = -1;

	hBox->addStretch();
	pageNumber = new QLabel( QString(tr("%1 of %2")).arg(0).arg(0), this, "cisStr" );
	pageNumber->setAlignment( Qt::AlignCenter | pageNumber->alignment() );
	hBox->addWidget( pageNumber );
	hBox->addStretch();

 	actualUnits.setDefaultUnits( globalSettings->read( PAGESPC + VIEWED_UNITS, DEFAULT__VIEWED_UNITS ) );

	Init( &is , format + "00000000xx" );
	mousePositionOnPage = new QLabel( format.arg(0.0,8,'g',6).arg(0.0,8,'g',6).arg( actualUnits.getDefaultUnits() ), this );
	mousePositionOnPage->setMinimumWidth( is.labelWidth );
	mousePositionOnPage->setAlignment( Qt::AlignRight | mousePositionOnPage->alignment() );
	hBox->addWidget( mousePositionOnPage, 0, Qt::AlignRight);

	hBox->insertSpacing( 0, is.labelWidth );	// for center pageNumber

	hBox->setResizeMode(QLayout::Minimum);

	selectionMode.reset();

	textLine = new TextLine();
	textLine->hide();

	// if something use on page, take focus
	setFocusPolicy( TheWheelFocus );
}

PageSpace::~PageSpace() {
	delete actualPdf;
	delete textLine;
}

QMainWindow * PageSpace::getTextLine( int x, int y, int fontsize, const QString & fontName ) {
	delete textLine;
	textLine = new TextLine();
	QFont font;
	if (fontName.isEmpty()) {
		font = textLine->font();
		font.setPointSize( fontsize );
	} else
		font = QFont( fontName, fontsize );
	textLine->setFont( font );
	textLine->adjustSize ();
	textLine->move ( x, y - textLine->height() );

	textLine->setText("");
	textLine->show ();
	textLine->setFocus ();

	return textLine;
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

	if (pageCount <= 0) {
		refresh( (QSPage *) NULL, (QSPdf *) NULL );
		return;
	}

	if (pageToView > pageCount)
			pageToView = pageCount;

	try {
		QSPage p( pdf->get()->getPage( pageToView ) , NULL );
		refresh( &p, pdf );
	} catch (PageNotFoundException) {
		refresh( (QSPage *) NULL, (QSPdf *) NULL );
	}
}

void PageSpace::refresh ( QSPage * pageToView, QSPdf * pdf ) {		// if pageToView is NULL, refresh actual page
									// if pageToView == actualPage  refresh is not need
/*	if (pageToView == NULL) {
		guiPrintDbg( debug::DBG_INFO, "Page to refresh is NULL pointer!" );
		return;
	}
*/
	if ((pageToView != NULL) && ( (actualPage == NULL) || (actualPage != pageToView->get()) ) && ( (pdf != NULL) || (actualPdf != NULL) ) ) {
		if ((actualPdf == NULL) || (pdf->get() != actualPdf->get())) {
			delete actualPdf;
			actualPdf = new QSPdf( pdf->get() , NULL );
		}
		actualPage = pageToView->get();
	} else {
		if ((actualPage == NULL) || (actualPdf == NULL) || (pageToView != NULL))
			return ;					// no page to refresh
		if (((pageToView == NULL) && (pdf == NULL)) || (actualPdf->getPageCount() == 0)) {
			actualPage.reset();
			delete actualPdf;
			actualPdf = NULL;
		}
		// else  need reload page ( changed zoom, ... )
	}


	// if actual page is removed then show new page at same position
	try {
		if (actualPdf)
			actualPagePos = actualPdf->get()->getPagePosition( actualPage );
		else
			actualPagePos = 0;
	} catch (PageNotFoundException) {
		actualPage.reset();

		refresh( actualPagePos );

		return;
	}

	// show actual page
	pageImage->showPage( actualPage );

	// emit new page position
	QSPage * qs_actualPage = new QSPage( actualPage, NULL );
	emit changedPageTo( *qs_actualPage, actualPagePos );
	delete qs_actualPage;

	// show actual information of position in document
	int pageCount = 0;
	if (actualPdf)
		pageCount = actualPdf->getPageCount();
	pageNumber->setText( QString(tr("%1 of %2"))
				.arg(actualPagePos)
				.arg(pageCount) );
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

void PageSpace::setResizingZone ( int width ) {
	if (width < 0) {
		guiPrintDbg( debug::DBG_INFO, "Resizing zone must be positive integer or null !" );
		width = 0;
	}
	if (selectionMode)
		selectionMode->setResizingZone( width );

	globalSettings->write( PAGESPC + RESIZINGZONE, width );
}
int PageSpace::getResizingZone() {
	if (selectionMode)
		return selectionMode->getResizingZone ();
	// else
	return globalSettings->readNum( PAGESPC + RESIZINGZONE, DEFAULT__RESIZINGZONE );
}
void PageSpace::setSelectionMode( QString mode,
									QString drawingObject,
									QString scriptFncAtMouseRelease,
									QString scriptFncAtMoveSelectedObjects,
									QString scriptFncAtResizeSelectedObjects ) {
	selectionMode.reset(
					PageViewModeFactory::create( mode,
												drawingObject,
												scriptFncAtMouseRelease,
												scriptFncAtMoveSelectedObjects,
												scriptFncAtResizeSelectedObjects ) );
	if (pageImage)
		pageImage->setSelectionMode( selectionMode );

	if (selectionMode) {
		selectionMode->setResizingZone( globalSettings->readNum( PAGESPC + RESIZINGZONE, DEFAULT__RESIZINGZONE ) );

		connect( selectionMode.get(), SIGNAL( popupMenu(const QPoint &) ),
				this, SLOT( requestPopupMenu(const QPoint &) ) );
		connect( selectionMode.get(), SIGNAL( newSelectedOperators(const std::vector< boost::shared_ptr< PdfOperator > > &) ),
				this, SLOT( newSelection(const std::vector< boost::shared_ptr< PdfOperator > > &) ) );
		connect( selectionMode.get(), SIGNAL( newSelectedAnnotations(const std::vector< boost::shared_ptr< CAnnotation > > &) ),
				this, SLOT( newSelection(const std::vector< boost::shared_ptr< CAnnotation > > &) ) );
		connect( selectionMode.get(), SIGNAL( executeCommand(QString) ),
				this, SIGNAL( executeCommand(QString) ) );
		connect( selectionMode.get(), SIGNAL( deleteSelection() ),
				this, SIGNAL( deleteSelection() ) );
	}
}

void PageSpace::setSelectArea ( int left, int top, int right, int bottom ) {
	if (selectionMode)
		selectionMode->setSelectedRegion( QRegion( left, top, right-left+1, bottom-top+1 ) );
}

void PageSpace::newSelection ( const std::vector< boost::shared_ptr< PdfOperator > > & objects ) {
	emit changeSelection( objects );
}
void PageSpace::newSelection ( const std::vector< boost::shared_ptr< CAnnotation > > & objects ) {
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

#ifdef QT4
	QStrList lst=QImageWriter::supportedImageFormats();
#else
	QStrList lst=QImageIO::outputFormats();
#endif
	for (unsigned int i = 0; i < lst.count() ;++i) {
		filters += QString("\n%1 (*.%2)")
				.arg(QString(lst.at(i)))
				.arg(QString(lst.at(i)).lower());
	}

	QString sf;
	QString filename = QFileDialog::getSaveFileName( "", filters, NULL, NULL, tr("Save as image ..."), & sf );
	sf = sf.left( sf.find(' ') );

	QFile f ( filename );
	if ( (! filename.isNull()) &&
		((! f.exists()) ||
			(0 == QMessageBox::question( this, QObject::tr("Overwrite?"),
                                                        QObject::tr("File called \"%1\" already exists. Do you want to overwrite it?").arg( filename ),
							QObject::tr("&Yes"), QObject::tr("&No"),
							QString::null, 0, 1 ) )) )
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

	std::vector<libs::Rectangle> recs;
	int count = actualPage->findText( util::convertFromUnicode(text,util::PDF), recs, tsp );

	guiPrintDbg( debug::DBG_DBG, "Founded "<<count<<" items:");
	for (std::vector<libs::Rectangle>::iterator it = recs.begin(); it != recs.end() ; ++it) {
		guiPrintDbg( debug::DBG_DBG, "   ("<<(*it)<<")");
	}

	unselectObjectOnPage();

	// get all text operators
	std::vector<boost::shared_ptr<PdfOperator> > foundedIn;
	std::vector<boost::shared_ptr<PdfOperator> > ops;
	std::vector<boost::shared_ptr<CContentStream> > ccs;
	actualPage->getContentStreams(ccs);
	// loop through all of content streams of actual page
	for ( std::vector<boost::shared_ptr<CContentStream> >::iterator ccsIt = ccs.begin(); ccsIt != ccs.end() ; ++ccsIt ) {
		(*ccsIt)->getPdfOperators( ops );

		pdfobjects::TextOperatorIterator it (ops[0]);

		// loop through container of operators one of content stream of actual page
		for ( ; !it.isEnd() ; it.next() ) {
			boost::shared_ptr<PdfOperator> h_textOp ( it.getCurrent() );

			libs::Rectangle bbox = h_textOp->getBBox();
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
			for ( std::vector<libs::Rectangle>::iterator it_recs = recs.begin(); it_recs != recs.end() ; ++it_recs ) {
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

	if (selectionMode) {
		selectionMode->addSelectedOperators( foundedIn );	// TODO change mode to textmode
		selectionMode->sendAllSelectedOperators ();
	}

	return count;
}

//----------------------------------------------------------------------
QStringList PageSpace::getAllUnits ( ) const {
	return actualUnits.getAllUnits( );
}

QStringList PageSpace::getAllUnitIds ( ) const {
	return actualUnits.getAllUnitIds( );
}
bool PageSpace::setDefaultUnits ( const QString dunits ) {
		bool h = actualUnits.setDefaultUnits ( dunits );

//Cannot write -> infinite loop
//		if (h)
//			globalSettings->write( PAGESPC + VIEWED_UNITS, actualUnits.getDefaultUnits() );

		return h;
}
QString PageSpace::getDefaultUnits ( ) const {
		return actualUnits.getDefaultUnits ();
}
QString PageSpace::getDescriptionForUnit( const QString _unit ) const {
		return actualUnits.getDescriptionForUnit( _unit );
}
QString PageSpace::getUnitFromAlias( const QString _unit ) const {
		return actualUnits.getUnitFromAlias( _unit );
}
double PageSpace::convertUnits ( double num, const QString fromUnits , const QString toUnits ) const {
		return actualUnits.convertUnits ( num, fromUnits, toUnits );
}
double PageSpace::convertFromUnitsToPoint ( double num, const QString & fromUnits ) const {
		return actualUnits.convertFromUnitsToPoint ( num, fromUnits );
}
double PageSpace::convertFromPointToUnits ( double num, const QString & toUnits ) const {
		return actualUnits.convertFromPointToUnits ( num, toUnits );
}

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
	pom = format.arg( convertUnits(x,"pt") ,8,'g',6 )
				.arg( convertUnits(y,"pt") ,8,'g',6 )
				.arg( actualUnits.getDefaultUnits() );
	mousePositionOnPage->setText( pom );
}

void PageSpace::firstPage ( ) {
	if (!actualPdf)
		return;
	try {
		QSPage p (actualPdf->get()->getFirstPage(),NULL);
		refresh( &p, actualPdf );
	} catch (PageNotFoundException) {
		//Pathological case when document have no pages
		//Do nothing, nothing to show
	}	
}
void PageSpace::prevPage ( ) {
	if (!actualPdf)
		return;
	if (!actualPage) {
		firstPage ();
		return;
	}

	if ( (actualPdf->get()->hasPrevPage( actualPage )) == true) {
		QSPage p (actualPdf->get()->getPrevPage( actualPage ) , NULL);
		refresh( &p, actualPdf );
	} else {
		// if actual page is removed then show new page at same position -1
		try {
			actualPdf->get()->getPagePosition( actualPage );
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

	if ( (actualPdf->get()->hasNextPage( actualPage )) == true) {
		QSPage p (actualPdf->get()->getNextPage( actualPage ) , NULL);
		refresh( &p, actualPdf );
	} else {
		// if actual page is removed then show new page at same position
		try {
			actualPdf->get()->getPagePosition( actualPage );
		} catch (PageNotFoundException) {
			refresh( actualPagePos );
		}
	}
}
void PageSpace::lastPage ( ) {
	if (!actualPdf)
		return;
	try {
		QSPage p (actualPdf->get()->getLastPage() , NULL);
		refresh( &p, actualPdf );
	} catch (PageNotFoundException) {
		//Pathological case when document have no pages
		//Do nothing, nothing to show
	}	
}

} // namespace gui
