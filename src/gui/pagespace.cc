#include "pagespace.h"
#include "settings.h"
#include <stdlib.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpixmap.h>

namespace gui {

typedef struct { int labelWidth, labelHeight; } initStruct;
// TODO asi prepracovat
void Init( initStruct * is ) {
	QLabel pageNumber("00000",0);
	pageNumber.show();
	is->labelWidth = pageNumber.width();
	is->labelHeight = pageNumber.height();
}

QString PAGESPC = "gui/PageSpace/";
QString ICON = "icon/";

PageSpace::PageSpace(QWidget *parent /*=0*/, const char *name /*=0*/) : QWidget(parent,name) {
	initStruct is;
	Init( &is );

	vBox = new QVBoxLayout(this);
	
	scrollPageSpace = new QScrollView(this);
	vBox->addWidget(scrollPageSpace);
	
	hBox = new QHBoxLayout(vBox);

	actualPage = NULL;
	actualPagePixmap = NULL;
	actualSelectedObject = NULL;
	pageImage = NULL;
	newPageView();
//	scrollPageSpace->setBackGroundRole(QPalette::Dark); // TODO configurovatelna farba
	
//TODO: prepracovat vytvareni tlacitek tlacitka obrazek tlacitka nastavit ze skriptu (vcetne popup
	hBox->addStretch();
	bFirstPage = new QPushButton(this,"bFirstPage");
	bFirstPage->setPixmap( QPixmap( globalSettings->getFullPathName("icon", globalSettings->read( ICON +"FirstPage") ) ));
	hBox->addWidget(bFirstPage);
	bPrevPage = new QPushButton(this,"bPrevPage");
	bPrevPage->setPixmap( QPixmap( globalSettings->getFullPathName("icon", globalSettings->read( ICON +"PrevPage") ) ));
	hBox->addWidget(bPrevPage);
connect( bFirstPage, SIGNAL(clicked()), this, SLOT(refresh1()));/*TODO smazat*/
connect( bPrevPage, SIGNAL(clicked()), this, SLOT(refresh2()));/*TODO smazat*/
//TODO: pevna minimalna velikost (bez init)
	pageNumber = new QLabel( "0", this, "cisStr" );
	pageNumber->setMinimumWidth( is.labelWidth );
	pageNumber->setAlignment( AlignCenter | pageNumber->alignment() );
	//pageNumber->setNum( 0 );
	hBox->addWidget( pageNumber );

//TODO: prepracovat vytvareni tlacitek tlacitka obrazek tlacitka nastavit ze skriptu
	bNextPage = new QPushButton(this,"bNextPage");
	bNextPage->setPixmap( QPixmap( globalSettings->getFullPathName("icon", globalSettings->read( ICON +"NextPage") ) ));
	hBox->addWidget(bNextPage);
	bLastPage = new QPushButton(this,"bLastPage");
	bLastPage->setPixmap( QPixmap( globalSettings->getFullPathName("icon", globalSettings->read( ICON +"LastPage") ) ));
	hBox->addWidget(bLastPage);
	hBox->addStretch();
	hBox->setResizeMode(QLayout::Minimum);

r1=new QPixmap("obr/horse.png");r2=new QPixmap("obr/horse1.png"); /*TODO smazat*/
}

PageSpace::~PageSpace() {
	delete actualPagePixmap;
}

void PageSpace::hideButtonsAndPageNumber ( ) {
	bFirstPage->hide();
	bPrevPage->hide();
	bNextPage->hide();
	bLastPage->hide();
	pageNumber->hide();
}
void PageSpace::showButtonsAndPageNumber ( ) {
	bFirstPage->show();
	bPrevPage->show();
	bNextPage->show();
	bLastPage->show();
	pageNumber->show();
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
	connect( pageImage, SIGNAL( rightClicked(const QPoint &, const QRect &) ),
		this, SLOT( requirementPopupMenu(const QPoint &, const QRect &) ) );
	connect( pageImage, SIGNAL( selectionMovedTo(const QPoint &) ), this, SLOT( moveSelection(const QPoint &) ) );
	connect( pageImage, SIGNAL( selectionResized(const QRect &, const QRect &) ),
		this, SLOT( resizeSelection(const QRect &, const QRect &) ) );
}

void PageSpace::newPageView( QPixmap &qp ) {
	newPageView();
	pageImage->setPixmap( qp );
	pageImage->show();
	centerPageView();
}
void PageSpace::centerPageView( ) {
	bool reposition;
	int posX, posY;

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

//TODO smazat
void PageSpace::refresh1(/* CPage * = NULL*/) {
	newPageView( *r1 );
}
//TODO smazat
void PageSpace::refresh2(/* CPage * = NULL*/) {
	newPageView( *r2 );
}

void PageSpace::refresh ( CPage * pageToView ) {		// if pageToView is NULL, refresh actual page
								// if pageToView == actualPage  refresh is not need
	if ((pageToView != NULL) && (actualPage != pageToView)) {
		actualPage = pageToView;
		pageNumber->setNum( 0/*(int) pageToView->getPageNumber()*/ );//MP: po zmene kernelu neslo zkompilovat (TODO)
		actualSelectedObject = NULL;
		/* TODO ziskat pixmap z CPage a ulozit ju do actualPagePixmap*/
		/**/ refresh1( );
	} else {
		if ((actualPage == NULL) || (pageToView != NULL))
			return ;		// no page to refresh or refresh actual page is not need
		
		/* TODO zmazat */
		/**/ refresh2( );
	}
	/* TODO zobrazenie aktualnej stranky*/
	/* newPageView( *actualPagePixmap ); */
}

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

void PageSpace::newSelection ( const QRect & r) {
	// TODO
}
void PageSpace::requirementPopupMenu ( const QPoint & globalPos, const QRect & r) {
	// TODO
	printf("requirementPopupMenu\n");
	if (r.isEmpty())
		printf("empty\n");
	else
		printf("non empty\n");
}
void PageSpace::moveSelection ( const QPoint & relativeMove ) {
	// TODO
}
void PageSpace::resizeSelection ( const QRect &, const QRect & ) {
	// TODO
}

} // namespace gui
