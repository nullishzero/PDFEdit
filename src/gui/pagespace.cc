#include <stdlib.h>
#include <math.h>

#include <qlabel.h>
#include <qstring.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qdockarea.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

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
QString RESIZINGZONE = "ResizingZone";
int DEFAULT__RESIZINGZONE = 2;
QString VIEWED_UNITS = "ViewedUnits";
QString DEFAULT__VIEWED_UNITS = "cm";
QString format = "x:%1 y:%2 %3";

TextLine::TextLine ()
	: QMainWindow ( NULL, NULL, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_StaysOnTop | Qt::WDestructiveClose )
{
	edit = new QLineEdit ( this, "textline" );
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

//  ------------------------------------------------------
//  --------------------   Units   -----------------------
//  ------------------------------------------------------
Units::Units( QString _defaultUnit ) : QObject() {
	units["pt"]		= 1;					// 1 Point
	units["in"]		= 1 / 0.013837;			// 1 Point == 0.013837 inch
	units["mil"]	= 0.001 * units["in"];	// 1 mil == 0.001 inch
	units["hand"]	= 4 * units["in"];		// 1 hand == 4 inches
	units["ft"]		= 12 * units["in"];		// 12 inches == 1 foot
	units["li"]		= 0.66 * units["ft"];	// 1 link == 0.66 foot
	units["yd"]		= 3 * units["ft"];		// 3 feet == 1 yard
	units["fathom"]	= 6 * units["ft"];		// 1 fathom == 6 feet
	units["rd"]		= 16.5 * units["ft"];	// 16.5 feet == 1 rod
	units["ch"]		= 66 * units["ft"];		// 1 chain == 66 feet
	units["fur"]	= 10 * units["ch"];		// 1 furlong == 10 chains
	units["cable"]	= 720 * units["ft"];	// 1 cable's length == 720 feet
	units["mi"]		= 8 * units["fur"];		// 1 mile == 8 furlongs
	units["land"]	= 3 * units["mi"];		// 1 league == 3 miles
	aliases[	"point"]	= "pt";
	aliases[tr("point")]	= "pt";		description["pt"]	= tr("pt : point");
	aliases[	"inch"]		= "in";
	aliases[	"inches"]	= "in";
	aliases[tr("inch")]		= "in";
	aliases[tr("inches")]	= "in";		description["in"]	= tr("in : inch");
										description["mil"]	= tr("mil :");
	aliases[	"hands"]	= "hand";
	aliases[tr("hands")]	= "hand";
	aliases[tr("hand")]		= "hand";	description["hand"]	= tr("hand : hand");
	aliases[	"foot"]		= "ft";
	aliases[	"feet"]		= "ft";
	aliases[tr("foot")] 	= "ft";
	aliases[tr("feet")] 	= "ft";		description["ft"]	= tr("ft : foot");
	aliases[	"link"]		= "li";
	aliases[	"links"]	= "li";
	aliases[tr("link")]		= "li";
	aliases[tr("links")]	= "li";		description["li"]	= tr("li : link");
	aliases[	"yard"]		= "yd";
	aliases[	"yards"]	= "yd";
	aliases[tr("yard")]		= "yd";
	aliases[tr("yards")]	= "yd";		description["yd"]	= tr("yd : yard");
	aliases[	"rod"]		= "rd";
	aliases[	"rods"]		= "rd";
	aliases[tr("rod")]		= "rd";
	aliases[tr("rods")]		= "rd";
	aliases[	"pole"]		= "rd";
	aliases[	"poles"]	= "rd";
	aliases[tr("pole")]		= "rd";
	aliases[tr("poles")]	= "rd";
	aliases[	"perch"]	= "rd";
	aliases[	"perches"]	= "rd";
	aliases[tr("perch")]	= "rd";
	aliases[tr("perches")]	= "rd";		description["rd"]	= tr("rd : rod (pole, perch)");
	aliases[	"fathoms"]	= "fathom";
	aliases[tr("fathom")]	= "fathom";
	aliases[tr("fathoms")]	= "fathom";	description["fathom"]	= tr("fathom :");								
	aliases[	"chain"]	= "ch";
	aliases[	"chains"]	= "ch";
	aliases[tr("chain")]	= "ch";
	aliases[tr("chains")]	= "ch";		description["ch"]	= tr("ch : chain");
	aliases[	"furlong"]	= "fur";
	aliases[	"furlongs"]	= "fur";
	aliases[tr("furlong")]	= "fur";
	aliases[tr("furlongs")]	= "fur";	description["fur"]	= tr("fur : furlong");
	aliases[tr("cable")]	= "cable";	description["cable"]	= tr("cable :");
	aliases[	"mile"]		= "mi";
	aliases[	"miles"]	= "mi";
	aliases[tr("mile")]		= "mi";
	aliases[tr("miles")]	= "mi";		description["mi"]	= tr("mi : mile");
	aliases[	"lands"]	= "land";
	aliases[tr("land")]		= "land";
	aliases[tr("lands")]	= "land";
	aliases[	"league"]	= "land";
	aliases[	"leagues"]	= "land";
	aliases[tr("league")]	= "land";
	aliases[tr("leagues")]	= "land";	description["land"]	= tr("land : land (league)");

	units["m"] = units["in"] / 0.0254;		// 1 inch = 2.54 cm
	units["Ym"] = 1e24 * units["m"];		// Y (yotta)	meter
	units["Zm"] = 1e21 * units["m"];		// Z (zetta)	meter
	units["Em"] = 1e18 * units["m"];		// E (exa)	meter
	units["Pm"] = 1e15 * units["m"];		// P (peta)	meter
	units["Tm"] = 1e12 * units["m"];		// T (tera)	meter
	units["Gm"] = 1e9 * units["m"];			// G (giga)	meter
	units["Mm"] = 1e6 * units["m"];			// M (mega)	meter
	units["km"] = 1e3 * units["m"];			// k (kilo)	meter
	units["hm"] = 1e2 * units["m"];			// h (hecto)	meter
	units["dam"] = 10 * units["m"];			// da (deka)	meter
	units["dm"] = 0.1 * units["m"];			// d (deci)	meter
	units["cm"] = 1e-2 * units["m"];		// c (centi)	meter
	units["mm"] = 1e-3 * units["m"];		// m (milli)	meter
	units["um"] = 1e-6 * units["m"];		// u (micro)	meter
	units["nm"] = 1e-9 * units["m"];		// n (nano)	meter
	units["pm"] = 1e-12 * units["m"];		// p (pico)	meter
	units["fm"] = 1e-15 * units["m"];		// f (femto)	meter
	units["am"] = 1e-18 * units["m"];		// a (atto)	meter
	units["zm"] = 1e-21 * units["m"];		// z (zepto)	meter
	units["ym"] = 1e-24 * units["m"];		// y (yocto)	meter

	description["m"]	= tr("m : meter");
	description["Ym"]	= tr("Ym : yotta meter");
	description["Zm"]	= tr("Zm : zetta meter");
	description["Em"]	= tr("Em : exa meter");
	description["Pm"]	= tr("Pm : peta meter");
	description["Tm"]	= tr("Tm : tera meter");
	description["Gm"]	= tr("Gm : giga meter");
	description["Mm"]	= tr("Mm : mega meter");
	description["km"]	= tr("km : kilo meter");
	description["hm"]	= tr("hm : hecto meter");
	description["dam"]	= tr("dam : deka meter");
	description["dm"]	= tr("dm : deci meter");
	description["cm"]	= tr("cm : centi meter");
	description["mm"]	= tr("mm : milli meter");
	description["um"]	= tr("um : micro meter");
	description["nm"]	= tr("nm : nano meter");
	description["pm"]	= tr("pm : pico meter");
	description["fm"]	= tr("fm : femto meter");
	description["am"]	= tr("am : atto meter");
	description["zm"]	= tr("zm : zepto meter");
	description["ym"]	= tr("ym : yocto meter");

	units["A"] = 0.1 * units["nm"];			// 1 angstrom = 0.1 nm	
	description["A"]	= tr("A : angstrom");

	units["AU"] = 149597870691.0 * units["m"];		// AU (astronomical unit)
	units["ly"] = 9460528404879.0 * units["km"];	// Light year
	units["pc"] = 3.26156378 * units["ly"];			// Parsec

	aliases[	"astronomical unit"]	= "AU";
	aliases[	"astronomical units"]	= "AU";
	aliases[tr("astronomical unit")]	= "AU";
	aliases[tr("astronomical units")]	= "AU";		description["AU"]	= tr("AU : astronomical unit");
	aliases[	"light year"]			= "ly";
	aliases[	"light years"]			= "ly";
	aliases[tr("light year")]			= "ly";
	aliases[tr("light years")]			= "ly";		description["ly"]	= tr("ly : light year");
	aliases[	"parsec"]				= "pc";
	aliases[tr("parsec")]				= "pc";		description["pc"]	= tr("pc : parsec");

	units["nautical mile"] = 1.852 * units["km"];	// 1 nautical mile == 1.852 km;

	aliases["nautical miles"]	= "nautical mile";
	aliases[tr("nautical mile")]	= "nautical mile";
	aliases[tr("nautical miles")]	= "nautical mile";	description["nautical mile"]	= tr("nautical mile :");


	setDefaultUnits( _defaultUnit );
}
Units::~Units()
	{}
void Units::getAllUnits( QStringList & names ) const {
	QMap<double,QString>					h_map;
	QMap<QString,double>::ConstIterator		it = units.begin();
	for ( ; it != units.end() ; ++it )
		if (description.contains( it.key() ))
			h_map[ it.data() ] = description[ it.key() ];
		else
			h_map[ it.data() ] = it.key();

	names.clear();
	names += h_map.values();
}

bool Units::setDefaultUnits( const QString dunits )
	{
		if (dunits.isNull()) {
			defaultUnit = "pt";
			return true;
		}
		if (units.contains( dunits )) {
			defaultUnit = dunits;
			return true;
		}
		if (aliases.contains( dunits )) {
			defaultUnit = aliases[ dunits ];
			return true;
		}
		if (description.values().contains( dunits )) {
			defaultUnit = description.keys()[description.values().findIndex( dunits )];
			return true;
		}

		return false;
	}
QString Units::getDefaultUnits( ) const
	{
		return defaultUnit;
	}
double Units::convertFromUnitsToPoint( double num, const QString & fromUnits ) const
	{
		bool hu_unit;
		bool ha_unit;
		if (fromUnits.isNull())
			return num * units[ defaultUnit ];
		if (! (	(hu_unit = units.contains( fromUnits )) ||
				(ha_unit = aliases.contains( fromUnits )) ||
				description.values().contains( fromUnits ) ))
			return num;
		// else
		if (hu_unit)
			return num * units[ fromUnits ];
		else if (ha_unit)
			return num * units[ aliases[ fromUnits ] ];
		else
			return num * units[ description.keys()[description.values().findIndex( fromUnits )] ];
	}
double Units::convertFromPointToUnits( double num, const QString & toUnits ) const
	{
		bool hu_unit;
		bool ha_unit;
		if (toUnits.isNull())
			return num / units[ defaultUnit ];
		if (! (	(hu_unit = units.contains( toUnits )) ||
				(ha_unit = aliases.contains( toUnits )) ||
				description.values().contains( toUnits ) ))
			return num;
		// else
		if (hu_unit)
			return num / units[ toUnits ];
		else if (ha_unit)
			return num / units[ aliases[ toUnits ] ];
		else
			return num / units[ description.keys()[description.values().findIndex( toUnits )] ];
	}
double Units::convertUnits( double num, const QString fromUnits, const QString toUnits ) const
	{
		return convertFromPointToUnits( convertFromUnitsToPoint( num, fromUnits ), toUnits );
	}


//  ----------------------------------------------------------
//  --------------------   PageSpace   -----------------------
//  ----------------------------------------------------------
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
	actualPage.reset();
	actualPagePos = -1;

	hBox->addStretch();
	pageNumber = new QLabel( QString(tr("%1 of %2")).arg(0).arg(0), this, "cisStr" );
	pageNumber->setAlignment( AlignCenter | pageNumber->alignment() );
	hBox->addWidget( pageNumber );
	hBox->addStretch();
        
 	actualUnits.setDefaultUnits( globalSettings->read( PAGESPC + VIEWED_UNITS, DEFAULT__VIEWED_UNITS ) );

	Init( &is , format + "00000000xx" );
	mousePositionOnPage = new QLabel( format.arg(0.0,0,'g',3).arg(0.0,0,'g',3).arg( actualUnits.getDefaultUnits() ), this );
	mousePositionOnPage->setMinimumWidth( is.labelWidth );
	mousePositionOnPage->setAlignment( AlignRight | mousePositionOnPage->alignment() );
	hBox->addWidget( mousePositionOnPage, 0, AlignRight);

	hBox->insertSpacing( 0, is.labelWidth );	// for center pageNumber

	hBox->setResizeMode(QLayout::Minimum);

	selectionMode.reset();

	textLine = new TextLine();
	textLine->hide();

	// if something use on page, take focus
	setFocusPolicy( WheelFocus );
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
	if ((pageToView != NULL) && ( (actualPage == NULL) || (actualPage != pageToView->get()) ) && ( (pdf != NULL) || (actualPdf != NULL) ) ) {
		if ((actualPdf == NULL) || (pdf->get() != actualPdf->get())) {
			delete actualPdf;
			actualPdf = new QSPdf( pdf->get() , NULL );
		}
		actualPage = pageToView->get();
	} else {
		if ((actualPage == NULL) || (actualPdf == NULL) || (pageToView != NULL))
			return ;					// no page to refresh
		if ((pageToView == NULL) && (pdf == NULL)) {
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
		guiPrintDbg( debug::DBG_INFO, tr("Resizing zone must be positive integer or null !") );
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
	for (unsigned int i = 0; i < QImageIO::outputFormats().count() ;++i) {
		filters += QString("\n%1 (*.%2)")
				.arg(QImageIO::outputFormats().at(i))
				.arg(QString(QImageIO::outputFormats().at(i)).lower());
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

	std::vector<Rectangle> recs;
	int count = actualPage->findText( text, recs, tsp );

	guiPrintDbg( debug::DBG_DBG, "Founded "<<count<<" items:");
	for (std::vector<Rectangle>::iterator it = recs.begin(); it != recs.end() ; ++it) {
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

	if (selectionMode) {
		selectionMode->addSelectedOperators( foundedIn );	// TODO change mode to textmode
		selectionMode->sendAllSelectedOperators ();
	}

	return count;
}

//----------------------------------------------------------------------
QStringList PageSpace::getAllUnits ( ) {
	QStringList		all_units;
	actualUnits.getAllUnits( all_units );

	return all_units;
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
	pom = format.arg( convertUnits(x,"pt") ,0,'g',3 )
				.arg( convertUnits(y,"pt") ,0,'g',3 )
				.arg( actualUnits.getDefaultUnits() );
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
	QSPage p (actualPdf->get()->getLastPage() , NULL);
	refresh( &p, actualPdf );
}
} // namespace gui
