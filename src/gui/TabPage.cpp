#include "debug.h"
#include "TabPage.h"
#include "globalfunctions.h"
#include "typedefs.h"
#include <ctype.h>
//created files
#include "insertpagerange.h"
#include "tree.h"
#include "page.h"
#include "bookmark.h"
#include "globalfunctions.h"
#include "utils/types/coordinates.h"
#include "kernel/carray.h"
#include <float.h>
#include <vector>
#include <QRgb>
//QT$
#include <QMessageBox>
#include <QRect>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QScrollBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QClipboard>
#include <QTreeWidgetItem>
#include <QToolTip>
#include <QShortcut>
#include <QVariant>
#include "ui_convertPageRange.h"
#include <qthread.h>

//PDF
#include <kernel/pdfoperators.h>
#include <kernel/cannotation.h>
#include <typedefs.h>

#include "kernel/pdfwriter.h"
#include "openpdf.h"

#include <QProgressBar>
#include <xpdf/xpdf/SplashOutputDev.h>
#include "ui_aboutDialog.h"
/** \brief searching thread */
class MyThread : public QThread
{
	/// tabpage where should be performed searching 
	TabPage * _shared;
	/// string to be searched
	QString _str;
	/// flas for searching machine
	int _flags;
public:
	/// initialize seatrch thread
	void set(TabPage * pg, QString str, int fl)
	{ 
		_shared = pg;
		_str = str;
		_flags = fl; 
	}
	/// what should be done when the thread starts
	void run()
	{
		_shared->performSearch(_str, _flags & SearchForward);
		_shared->show();
	}
};
//operatory, ktore musim zaklonovat, ked chcem pohnut textom
#define ZOOM_AFTER_HACK 4

/// name of operators that are text-related - just informative
std::string nameInTextOperators[] = { "w","j","J","M","d","ri","i","gs", "CS","cs", "SC","SCN", "sc","scn", "G","g","RG","rg","k","K","Tc","Tw", "Tz", "TL", "Tf","Tr","Ts","Td","TD","Tm","T*" };

void TabPage::handleBookmark(QTreeWidgetItem* item, int) //nezaujima nas stlpec
{
	Bookmark * b = (Bookmark *)(item);
	emit addHistory(QString("Navigating to page ")+ QVariant(b->getDest()).toString() + ", zoom:" +QVariant(b->getZoom()).toString() +"\n" );
	int pIndex = b->getDest();
	if (pIndex <=0 || pIndex >_pdf->getPageCount())
		return;
	_page = _pdf->getPage(b->getDest());
	double z = b->getZoom();
	int index = z/0.5 -1;
	if (index < 0)
		index = 0;
	if (index >= ui.zoom->count())
		index = ui.zoom->count()-1;
	if(ui.zoom->currentIndex() == index )
		redraw();
	ui.zoom->setCurrentIndex(index);
	double x = b->getX();
	double y = b->getY();
	rotatePdf(displayparams, x,y,false);
	this->ui.scrollArea->ensureVisible(x,y);
}
/** \brief class for bridging progress set in pdf and in QT4 */
class PdfProgress : public pdfobjects::utils::IProgressBar
{
	/// QT$ bar that nadles the progress
	QProgressBar * _bar;
public:
	/// contructor
	PdfProgress(QProgressBar * bar): _bar(bar){};
	/// initialization
	virtual void start()
	{
		_bar->reset();
	}
	/// progress reached its end
	virtual void finish()
	{
		//hide it
		_bar->hide();
	};
	/// progress update. Step is the value that the bar should advance
	virtual void update(int step)
	{
		_bar->setValue(_bar->value() + step);
	};
	/// sets range for bar
	virtual void setMaxStep(int maxStep)
	{
		_bar->setRange(0,maxStep);
	};
};
bool TabPage::containsOperator(std::string wanted)
{
	std::vector<shared_ptr<CContentStream> > streams;
	_page->getContentStreams(streams);
	for ( int content = 0; content < streams.size() ; content++)
	{
		Ops ops;
		streams[content]->getPdfOperators(ops);
		for ( int i = 0; i < ops.size(); i++)
		{
			PdfOperator::Iterator iter = PdfOperator::getIterator(ops[i]);
			while(iter.valid())
			{
				std::string name;
				iter.getCurrent()->getOperatorName(name);
				if (name == wanted)
					return true;
				iter.next();
			}
		}
	}
	return false;
}
static SplashColor paperColor = {0xff,0xff,0xff};

TabPage::TabPage(OpenPdf * parent, QString name) : _name(name),_parent(parent),splash (splashModeBGR8, 4, gFalse, paperColor),aboutDialog(this),_stop(0)
{
	_pdf = boost::shared_ptr<pdfobjects::CPdf> ( pdfobjects::CPdf::getInstance (name.toAscii().data(), pdfobjects::CPdf::ReadWrite));
	_searchThread = new MyThread();
	debug::changeDebugLevel(10000);
	_page = boost::shared_ptr<pdfobjects::CPage> (_pdf->getPage(1)); //or set from last
	//displayparams.upsideDown = true;
	// init splash bitmap
	_acceptedType = OpTextName;
	_selected = false;
	_font = NULL;
	_cmts = NULL;
	ui.setupUi(this);
	shared_ptr<pdfobjects::utils::ProgressObserver> prog(new pdfobjects::utils::ProgressObserver(new PdfProgress(ui.progressBar)));
	_pdf->getPdfWriter()->registerObserver(prog);

	_labelPage = new DisplayPage(this);
	this->ui.stateLabel->hide();
	_cmts = new Comments(_parent->Author());
	aboutDialogUI.setupUi(&aboutDialog);
	_image = new InsertImage(NULL);
	this->ui.scrollArea->setWidget(_labelPage);	
	//this->ui.scrollArea->set
	//this->ui.displayManipulation->hide();
	
	_font = new FontWidget(NULL);
	for ( int i = BEGIN_ZOOM; i< MAX_ZOOM; i+= ZOOM_STEP)
	{
		QVariant s(i);
		this->ui.zoom->addItem( s.toString()+" %",s);
	}
	//////////////////////////////////////THREAD////////////////////////////////////
	connect( _searchThread, SIGNAL(finished()), this, SLOT(reportSearchResult()));
	//////////////////////////////////////////////////////////////////////////
	connect( this, SIGNAL(SetStateSignal(QString)), this->ui.stateLabel, SLOT(setText(QString)));
	connect( this, SIGNAL(SetStateSignal(QString)), this->ui.stateLabel, SLOT(show()));

	connect( this->ui.documentInfo, SIGNAL(pressed()), this, SLOT(about()));
	connect( this->ui.plusZoom, SIGNAL(pressed()), this, SLOT(addZoom()) );
 	connect(this->ui.minusZoom, SIGNAL(pressed()), this, SLOT(minusZoom()) );
 	connect(this->ui.commitButton, SIGNAL(pressed()), this, SLOT(commitRevision()));
	connect( this->ui.pageInfo, SIGNAL(returnPressed()),this, SLOT(setPageFromInfo()));
	connect (this->ui.firstPage, SIGNAL(pressed()), this, SLOT(setFirstPage()));
	connect (this->ui.lastPage, SIGNAL(pressed()), this, SLOT(setLastPage()));

	connect(this, SIGNAL(addHistory(QString)), this->ui.historyText, SLOT(append(QString)));
	//connect musi byt potom!!!->inak sa to zobrazi milion krat namiesto raz
	connect(this->ui.zoom, SIGNAL(currentIndexChanged(int)),this,SLOT(zoom(int)));
	//	_font->show();
	connect(_font, SIGNAL(getLastTm(libs::Point,float*)), this, SLOT(getPreviousTmInPosition(libs::Point,float*)));
	connect( _font, SIGNAL(FontClosedSignal()),this,SLOT(operationDone()));
	connect(_font, SIGNAL(fontInPage(std::string)), this, SLOT(addFontToPage(std::string)));
	connect(_font, SIGNAL(text(PdfOp)), this, SLOT(insertText(PdfOp)));
	connect(_font, SIGNAL(changeTextSignal(PdfOp)), this, SLOT(changeSelectedText(PdfOp)));
	connect(_font, SIGNAL(FindLastFontSignal()), this, SLOT(findLastFontMode()));
	connect(_font, SIGNAL(getLastFontSignal(libs::Point)), this, SLOT(getPreviousFontInPosition(libs::Point)));
	connect(_font, SIGNAL(convertTextFromUnicode(QString,std::string)), this, SLOT(checkCode(QString,std::string)));
	//show text button, hide everything else
	//--------------------------------Disabling------------------
	ui.showBookmark->setEnabled( _pdf->getDictionary()->containsProperty("Outlines"));
	//--------------------------------HIDING---------------------
	//hiding necessary
	this->ui.tree->hide();
	this->ui.historyText->hide();
	this->ui.analyzeTree->hide();
	this->ui.showAnalyzeButton->hide();
	this->ui.progressBar->hide();
	//------------------------------CONNECTIONS----------------------
	//connections
	//pridanie anotacie
	connect(this->ui.showBookmark, SIGNAL(clicked()), this, SLOT(checkLoadedBookmarks()));
	connect(_cmts,SIGNAL(textAnnotation(PdfAnnot)),this,SLOT(insertTextAnnot(PdfAnnot)));
	//connect(_cmts,SIGNAL(annotation(PdfAnnot)),this,SLOT(insertAnnotation(PdfAnnot)));
	connect(_cmts,SIGNAL(WaitForPosition(PdfAnnot)),this,SLOT(SetModePosition(PdfAnnot)));
	connect(_labelPage,SIGNAL(ShowAnnotation(int)),this,SLOT(showAnnotation(int)));

	//	connect(this,SIGNAL(pdfPosition(float,float,int,int)),_cmts,SLOT(setRectangle(float,float,int,int)));
	connect(this,SIGNAL(parsed(std::vector<float>)),_cmts,SLOT(setPoints(std::vector<float>)));

	connect (ui.previous,SIGNAL(clicked()),this,SLOT(previousPage()));
	connect (ui.next,SIGNAL(clicked()),this,SLOT(nextPage()));
	connect (_labelPage,SIGNAL(MouseClicked(QPoint)),this, SLOT(clicked(QPoint))); //pri selecte sa to disconnectne a nahrasi inym modom
	connect (this,SIGNAL(markPosition(QPoint)),_labelPage, SLOT(markPosition(QPoint))); //pri selecte sa to disconnectne a nahrasi inym modom
	connect (_labelPage,SIGNAL(MouseReleased(QPoint)),this, SLOT(mouseReleased(QPoint))); //pri selecte sa to disconnectne
	connect( this,SIGNAL(ChangePageModeSignal(PageDrawMode)), _labelPage, SLOT(setMode(PageDrawMode)));
	connect (_labelPage,SIGNAL(InsertTextSignal(QPoint)),this,SLOT(raiseInsertText(QPoint)));
	connect (_labelPage,SIGNAL(DeleteTextSignal()),this,SLOT(deleteSelectedText()));
	connect (_labelPage,SIGNAL(EraseTextSignal()),this,SLOT(eraseSelectedText()));
	connect (_labelPage,SIGNAL(ChangeTextSignal()),this,SLOT(raiseChangeSelectedText()));
	connect (_labelPage,SIGNAL(copySelectedSignal()), this, SLOT(copyTextToClipBoard()));
	connect (_labelPage,SIGNAL(DeleteImageSignal(QPoint)),this,SLOT(deleteImage(QPoint)));
	connect (_labelPage, SIGNAL(HandleLink(int)),this, SLOT(handleLink(int)));
	connect (_labelPage,SIGNAL(AnnotationSignal(QPoint)),this,SLOT(raiseAnnotation(QPoint)));
//	connect (_labelPage,SIGNAL(DeleteAnnotationSignal(QPoint)),this,SLOT(deleteAnnotation(QPoint)));
	//connect (_labelPage,SIGNAL(ChangeImageSignal(QPoint)),this,SLOT(raiseChangeImage(QPoint)));

	connect( _image, SIGNAL(ImageClosedSignal()),this,SLOT(operationDone()));
	connect( _image, SIGNAL(insertImage(PdfOp)),this,SLOT(insertImage(PdfOp)));
	connect( _image, SIGNAL(changeImage(PdfOp)),this,SLOT(changeSelectedImage(PdfOp)));
	connect( _image, SIGNAL(NeedDisplayParamsSignal()),this,SLOT(getDisplayParams()));

	connect( _cmts,SIGNAL(annotationTextMarkup(PdfAnnot)),this,SLOT(insertTextMarkup(PdfAnnot)));

	///////////////////////////BOOKMARKS///////////////////////////////////////////////

	connect(ui.tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(handleBookmark(QTreeWidgetItem*,int)));
	connect(ui.tree, SIGNAL(itemExpanded( QTreeWidgetItem*)), this, SLOT(loadBookmark(QTreeWidgetItem *)));
	connect(ui.analyzeTree, SIGNAL(itemExpanded( QTreeWidgetItem*)), this, SLOT(loadAnalyzeItem(QTreeWidgetItem *)));

	//////////////////////////////////////////////////////////////////////////
	initRevisions();
	connect(ui.revision, SIGNAL(currentIndexChanged(int)),this, SLOT(initRevision(int) ));
	connect(ui.branchRevision, SIGNAL(clicked()), this, SLOT(exportRevision()) );

	//////////////////////////////////////////////////////////////////////////
	//connect(_parent, SIGNAL(ModeChangedSignal(QString), this, SLOT(clearSelected())))
	//////////////////////////////////////////////////////////////////////////
	//redraw();
	this->ui.tree->setColumnCount(1); //only one columne
	_dataReady = false;
	//setZoom();
//HACK
	Mode oldMode = _parent->getMode();
	this->ui.zoom->setCurrentIndex(1);
	_dataReady = false;
	_parent->setMode(oldMode);
	//changeSelectedText();
	setPage(1);
	//search("[jm]e", Searf);
}
void TabPage::setState()
{
	QString message;
	if (_pdf->isLinearized())
		message += "Linearized!\n";
	if (_pdf->getMode() == CPdf::ReadOnly)
		message+="Read-only document\n";
	if (containsOperator("TJ"))
		message += "Contains unsupported TJ operator. Text may not work properly\n"; 
	if (_pdf->getRevisionsCount()!= this->ui.revision->currentIndex()+1)
		message = "Revision is not actual";
	if (!message.isEmpty())
		emit SetStateSignal(message);
}
void TabPage::about()
{
	//naplnime metadatom
	aboutDialogUI.info->clear();
	aboutDialogUI.info->setAlignment(Qt::AlignLeft);
	aboutDialog.setWindowTitle("Info about Pdf document");
	QString message("FullPath: ");
	message += _name;
	message += "\n\n";
	message += "Version";
	message += _pdf->getCXref()->getPDFVersion();
	message += "\n\n";
	//bookmarky
	message += "Contains bookmarks: ";
	if (_pdf->getDictionary()->containsProperty("Outlines"))
	{
		message += "Yes, number od top-level items: ";
		shared_ptr<CDict> dict = _pdf->getDictionary()->getProperty<CDict>("Outlines");
		QVariant v(utils::getIntFromDict("Count",dict));
		message += v.toString();
	}
	else
		message += "No";
	message += "\n\n";
	//informacie z metadat,
	if (_pdf->getDictionary()->containsProperty("Metadata"))
	{
		message += "Contains metadata:";
		shared_ptr<CStream> dict = _pdf->getDictionary()->getProperty<CStream>("Metadata");
		std::vector<std::string> conts;
		dict->getAllPropertyNames(conts);
		for ( int i = 0; i< conts.size(); i++)
		{
			message += conts[i].c_str();
			std::string val;
			dict->getProperty(conts[i])->getStringRepresentation(val);
			message += val.c_str();
			message += "\n";
		}
		message += QString("Data:");
		for ( int i =0; i < dict->getBuffer().size(); i++)
			message += dict->getBuffer()[i];
	}
	else
		message += "No metadata\n\n";
	//lenarozacia:
	message+= "Linearized: ";
	if (_pdf->isLinearized())
		message+= "yes\n";
	else
		message+= "No\n";
	message += "ReadOnly: ";
	if(_pdf->getMode()==CPdf::ReadOnly)
		message += "Yes\n";
	else
		message += "No\n";
	//pocet revizii
	message += "Number of revisions :";
	message +=QVariant(_pdf->getRevisionsCount()).toString();
	
	aboutDialogUI.info->setText(message);
	aboutDialog.exec();
}
void TabPage::showAnnotation(int level)
{
	//emit addHistory("Getting")
	std::string typ = utils::getNameFromDict("Subtype",_annots[level]->getDictionary());
	if (typ == "Text" || typ == "Highlight")
	{
		//show tooltip
		QString message;
		shared_ptr<CDict> dict = _annots[level]->getDictionary();
		if (dict->containsProperty("T"))
			message += QString::fromStdString(utils::getStringFromDict(dict,"T"));
		message += ":\n";
		if (dict->containsProperty("Contents"))
			message += QString::fromStdString(utils::getStringFromDict(dict,"Contents"));
		QToolTip::showText(QCursor::pos(), message);
	}
	//TODO pre link trosku inak - URI
}
void TabPage::initRevisions()
{
	QStringList list;
	ui.revision->clear();
	for ( size_t i = 0; i< _pdf->getRevisionsCount(); i++)
	{
		QVariant v(i);
		list.append(QString("Revision " + v.toString()));
	}
	ui.revision->addItems(list);
	ui.revision->setCurrentIndex(list.count()-1);
}
void TabPage::highLightAnnSelected()
{
	if (!CanBeSavedChanges(true))
		return;
	if (!_selected)
		return;//TODO musi byt tie non-empty
	_cmts->setIndex(AHighlight);
	_cmts->setHColor(_parent->getHColor());
	_cmts->insertMarkup();
	emit ("Inserted highlight annotation without comment");
}
void TabPage::raiseChangeSelectedImage()
{
	if (!CanBeSavedChanges(true))
		return;
	if(!_selected)
	{
		emit addHistory("No image was selected, ignoring");
		return;
	}
	BBox b = _selectedImage->getBBox();
	double scaleX = 72.0f/displayparams.vDpi;
	double scaleY = 72.0f/displayparams.hDpi;
	double x = min(b.xleft,b.xright);
	x*=scaleX;
	double y = (float)displayparams.DEFAULT_PAGE_RY;
	y-= (min(b.yleft,b.yright))*scaleY;
	//uz mame bbox
	rotatePdf(displayparams,x,y,false);
	float corr = 72.0f/displayparams.vDpi;
	_image->setPosition(x,y);
	_image->setSize(abs(b.xleft-b.xright)*corr, abs(b.xleft-b.xright)*corr);
	emit addHistory(QString("Raising image change at position ") + QVariant(abs(b.xleft-b.xright)).toString() + " " + QVariant(abs(b.xleft-b.xright)).toString() + "\n");
	_image->setImage( _selectedImage); //TODO x,y
	_image->show();
}

void TabPage::deleteImage(QPoint point)
{
	if (!CanBeSavedChanges(true))
		return;
	//zistime, co mame pod pointom;
	double x,y;
	toPdfPos(point.x(),point.y(),x,y);
	Ops ops;
	_page->getObjectsAtPosition(ops,libs::Rectangle(0,0,1000,1000));
	//hladame BI. Dictionary nevymazavame - bez operatora sa to stejne nezobrazi
	int i =0;
	InlineImageOperatorIterator it(ops.back(),false);
	PdfOp op;
	/*if (it.valid())
	{
	op = it.getCurrent();
	shared_ptr<InlineImageCompositePdfOperator> img = boost::dynamic_pointer_cast<InlineImageCompositePdfOperator>(op);
	BBox b = op->getBBox();
	PdfOperator::PdfOperators child;		
	it.next();
	}*/
	if (!it.valid())
		return;
	op = it.getCurrent();
	//mame iba obrazky
	//zmaz len ten, ktore je 'navrchu' -> je v ops posledny?
	op->getContentStream()->deleteOperator(op);
	emit addHistory("Inline image deleted \n");
	clearSelected();
	redraw();
}
void TabPage::raiseInsertImage(QRect rect)
{
	if (!CanBeSavedChanges(true))
		return;
	double w=rect.width()*72.0f/displayparams.vDpi,h =rect.height()*72.0f/displayparams.vDpi;
	//::rotate(displayparams.rotate,w,h);
	_image->setSize(w,h);
	_image->show();
}
void TabPage::zoom( int zoomscale )
{
	QVariant v = ui.zoom->itemData(zoomscale);
	float scale = v.toFloat()/100;
	if (scale >5) //TODO QWarning
	{
		assert(false);
		return;
	}
	float dpix=_labelPage->logicalDpiX();
	float dpiy =_labelPage->logicalDpiY();
	displayparams.hDpi = dpix * scale;
	displayparams.vDpi = dpiy * scale;
	emit addHistory( QString("Rezoomed to ") + QVariant(scale).toString() );
	clearSelected();
	redraw();
}
void TabPage::fillCoordinates(std::vector<float>& coordinates, float * dim)
{
	assert(_selected);
	TextData::iterator iter = sTextIt;
	while (true)
	{
		double x = iter->getPreviousStop();
		double y = iter->_ymax;
		rotatePdf(displayparams,x,y,true);
		displayparams.convertPixmapPosToPdfPos(iter->getPreviousStop(),iter->_ymax,x,y);
		coordinates.push_back(x);
		coordinates.push_back(y);

		x = iter->getNextStop();
		y = iter->_ymax;
		rotatePdf(displayparams,x,y,true);
		displayparams.convertPixmapPosToPdfPos(iter->getNextStop(),iter->_ymax,x,y);
		coordinates.push_back(x);
		coordinates.push_back(y);
		dim[1] = min(dim[1],y);

		x = iter->getPreviousStop();
		y = iter->_ymin;
		rotatePdf(displayparams,x,y,true);
		displayparams.convertPixmapPosToPdfPos(iter->getPreviousStop(),iter->_ymin,x,y);
		coordinates.push_back(x);
		coordinates.push_back(y); 
		dim[0] = min(dim[0],x);

		x = iter->getNextStop();
		y = iter->_ymin;
		rotatePdf(displayparams,x,y,true);
		displayparams.convertPixmapPosToPdfPos(iter->getNextStop(),iter->_ymin,x,y);
		coordinates.push_back(x);
		coordinates.push_back(y); 
		dim[2] = max(dim[2],x);
		dim[3] = max(dim[3],y);
		if ( iter == sTextItEnd )
			break;
		iter++;
	}
}
pdfobjects::IndiRef TabPage::createAppearanceHighlight( float * dim )
{
	addHistory("Creating appearance directory for highlighting..\n");
	shared_ptr<CStream> apStream = createAPStream(dim);
	PdfOperator::Operands operands;
	operands.push_back((PdfProperty(CNameFactory::getInstance("TransGs"))));
	PdfOp op = createOperator("gs",operands);
	apStream->addToBuffer(op);
	operands.clear();
	QColor color = _cmts->getHColor();
	operands.push_back((PdfProperty(CRealFactory::getInstance(color.redF()))));
	operands.push_back((PdfProperty(CRealFactory::getInstance(color.greenF()))));
	operands.push_back((PdfProperty(CRealFactory::getInstance(color.blueF()))));
	op = createOperator("rg",operands);
	apStream->addToBuffer(op);

	operands.clear();
	operands.push_back((PdfProperty(CRealFactory::getInstance(dim[0]))));
	operands.push_back((PdfProperty(CRealFactory::getInstance(dim[1]))));
	op = createOperator("m",operands);
	apStream->addToBuffer(op);

	operands.clear();
	operands.push_back((PdfProperty(CRealFactory::getInstance(dim[2]))));
	operands.push_back((PdfProperty(CRealFactory::getInstance(dim[1]))));
	op = createOperator("l",operands);
	apStream->addToBuffer(op);

	operands.clear();
	operands.push_back((PdfProperty(CRealFactory::getInstance(dim[2]))));
	operands.push_back((PdfProperty(CRealFactory::getInstance(dim[3]))));
	op = createOperator("l",operands);
	apStream->addToBuffer(op);

	operands.clear();
	operands.push_back((PdfProperty(CRealFactory::getInstance(dim[0]))));
	operands.push_back((PdfProperty(CRealFactory::getInstance(dim[3]))));
	op=createOperator("l",operands);
	apStream->addToBuffer(op);

	operands.clear();
	apStream->addToBuffer(createOperator("h",operands));
	apStream->addToBuffer(createOperator("f",operands));

	//std::string text("173.11566 660.86603 m 181.534 660.86603 l 181.534 639.63373 l 164.869 639.63373 l h f");
	apStream->validate();
	addHistory("Appearance directory created\n");
	return _pdf->addIndirectProperty(apStream);//hack
}
boost::shared_ptr<pdfobjects::CStream> TabPage::createAPStream(float * dim)
{
	addHistory("Creating appearance stream ...\n");
	boost::shared_ptr<pdfobjects::CStream> apStream(new CStream());
	apStream->addProperty("Type",*PdfProperty(CNameFactory::getInstance("XObject")));
	apStream->addProperty("SubType",*PdfProperty(CNameFactory::getInstance("Form")));
	apStream->addProperty("FormType",*boost::shared_ptr<CInt>(CIntFactory::getInstance(1)));
	{
		CArray rct;
		rct.addProperty(*(PdfProperty(CRealFactory::getInstance(dim[0]))));
		rct.addProperty(*(PdfProperty(CRealFactory::getInstance(dim[1]))));
		rct.addProperty(*(PdfProperty(CRealFactory::getInstance(dim[2]))));
		rct.addProperty(*(PdfProperty(CRealFactory::getInstance(dim[3]))));
		apStream->addProperty("BBox",rct);
	}
	{
		CArray arr;
		arr.addProperty(*PdfProperty(CRealFactory::getInstance(1)));
		arr.addProperty(*PdfProperty(CRealFactory::getInstance(0)));
		arr.addProperty(*PdfProperty(CRealFactory::getInstance(0)));
		arr.addProperty(*PdfProperty(CRealFactory::getInstance(1)));
		arr.addProperty(*PdfProperty(CRealFactory::getInstance(-dim[0])));
		arr.addProperty(*PdfProperty(CRealFactory::getInstance(-dim[1])));
		apStream->addProperty("Matrix",arr);
	}
	{
		//group pre transparency
		boost::shared_ptr<pdfobjects::CDict> group(CDictFactory::getInstance());
		group->addProperty("S",*PdfProperty(CNameFactory::getInstance("Transparency")));
		group->addProperty("Type",*PdfProperty(CNameFactory::getInstance("Group")));
		apStream->addProperty("Group",*group);
	}
	{
		//resource
		boost::shared_ptr<pdfobjects::CDict> resDict(CDictFactory::getInstance());
		boost::shared_ptr<pdfobjects::CDict> grDict(CDictFactory::getInstance());
		boost::shared_ptr<pdfobjects::CDict> g0Dict(CDictFactory::getInstance());

		CArray arr2;
		arr2.addProperty(*PdfProperty(CNameFactory::getInstance("PDF")));
		resDict->addProperty("ProcSet",arr2);


		g0Dict->addProperty("Type",*PdfProperty(CNameFactory::getInstance("ExtGState")));

		g0Dict->addProperty("AIS",*PdfProperty(CBoolFactory::getInstance(false)));
		g0Dict->addProperty("BM",*PdfProperty(CNameFactory::getInstance("Multiply")));
		grDict->addProperty("TransGs",*g0Dict);
		resDict->addProperty("ExtGState",*grDict);
		apStream->addProperty("Resources",*resDict);
	}
	{
		CArray rct;
		rct.addProperty(*(PdfProperty(CRealFactory::getInstance(0))));
		rct.addProperty(*(PdfProperty(CRealFactory::getInstance(0))));
		rct.addProperty(*(PdfProperty(CRealFactory::getInstance(1))));
		apStream->addProperty("Border",rct);
	}
	addHistory("Appearance stream created\n");
	return apStream;
}
#define ADD(q,op) q->push_back(op, getLastOperator(q))

pdfobjects::IndiRef TabPage::createAppearanceComment(float *dim)
{
	addHistory("Creating appearance stream for comment...\n");
	shared_ptr<CStream> apStream = createAPStream(dim);
	//pdfoperatory
	std::string b("q 1 1 1 rg 0 i 1 w 4 M 1 j 0 J []0 d /GS0 gs 1 0 0 1 9 5.0908 cm 7.74 12.616 m -7.74 12.616 l -8.274 12.616 -8.707 12.184 -8.707 11.649 c -8.707 -3.831 l -8.707 -4.365 -8.274 -4.798 -7.74 -4.798 c 7.74 -4.798 l 8.274 -4.798 8.707 -4.365 8.707 -3.831 c 8.707 11.649 l 8.707 12.184 8.274 12.616 7.74 12.616 c h f Q 0 G 1 1 0 rg 0 i 0.60 w 4 M 1 j 0 J []0 d  1 1 0 rg 0 G 0 i 0.59 w 4 M 1 j 0 J []0 d  1 0 0 1 9 5.0908 cm 0 0 m -0.142 0 -0.28 0.008 -0.418 0.015 c -2.199 -1.969 -5.555 -2.242 -4.642 -1.42 c -4.024 -0.862 -3.916 0.111 -3.954 0.916 c -5.658 1.795 -6.772 3.222 -6.772 4.839 c -6.772 7.509 -3.74 9.674 0 9.674 c 3.74 9.674 6.772 7.509 6.772 4.839 c 6.772 2.167 3.74 0 0 0 c 7.74 12.616 m -7.74 12.616 l -8.274 12.616 -8.707 12.184 -8.707 11.649 c -8.707 -3.831 l -8.707 -4.365 -8.274 -4.798 -7.74 -4.798 c 7.74 -4.798 l 8.274 -4.798 8.707 -4.365 8.707 -3.831 c 8.707 11.649 l 8.707 12.184 8.274 12.616 7.74 12.616 c b");
	apStream->setBuffer(b);
	
	PdfComp q = PdfComp(new pdfobjects::UnknownCompositePdfOperator( "q", "Q"));
	PdfOperator::Operands ops;
goto END;
	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));

	ADD(q,createOperator("rg",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	ADD(q,createOperator("i",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	ADD(q,createOperator("w",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(4)));
	ADD(q,createOperator("M",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	ADD(q,createOperator("j",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	ADD(q,createOperator("J",ops));
	ops.clear();

	ops.push_back(PdfProperty(CArrayFactory::getInstance()));
	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	ADD(q,createOperator("d",ops));
	ops.clear();

	ops.push_back(PdfProperty(CNameFactory::getInstance("GS0")));
	ADD(q,createOperator("gs",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(1)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(1)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(9)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(5.0908)));
	ADD(q,createOperator("cm",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(7.74)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(12.616)));
	ADD(q,createOperator("m",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-7.74)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-12.616)));
	ADD(q,createOperator("l",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-8.274)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(12.616)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(12.184)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(11.649)));
	ADD(q,createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-3.831)));
	ADD(q,createOperator("l",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.365)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-8.274)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.798)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-7.74)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.798)));
	ADD(q,createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(7.74)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.798)));
	ADD(q,createOperator("l",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance( 8.274)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.798)));
	ops.push_back(PdfProperty(CRealFactory::getInstance( 8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.365)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-3.831)));
	ADD(q,createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(11.649)));
	ADD(q,createOperator("l",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(12.184)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(8.274)));
	ops.push_back(PdfProperty(CRealFactory::getInstance( 12.616)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(7.74)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(12.616)));
	ADD(q,createOperator("c",ops));
	ops.clear();

	ADD(q,createOperator("h",ops));
	ADD(q,createOperator("f",ops));
	ADD(q,createOperator("Q",ops));

	apStream->addToBuffer(q);

	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("G",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("rg",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("i",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(0.60)));
	apStream->addToBuffer(createOperator("w",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(4)));
	apStream->addToBuffer(createOperator("M",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	apStream->addToBuffer(createOperator("j",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("J",ops));
	ops.clear();

	ops.push_back(PdfProperty(CArrayFactory::getInstance()));
	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("d",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("rg",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("G",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("i",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(0.59)));
	apStream->addToBuffer(createOperator("w",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(4)));
	apStream->addToBuffer(createOperator("M",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(1)));
	apStream->addToBuffer(createOperator("j",ops));
	ops.clear();

	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("J",ops));
	ops.clear();

	ops.push_back(PdfProperty(CArrayFactory::getInstance()));
	ops.push_back(PdfProperty(CIntFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("d",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(1)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(1)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(5.0908)));
	apStream->addToBuffer(createOperator("cm",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	apStream->addToBuffer(createOperator("m",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-0.142 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-0.28 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance( 0.008)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-0.418)));
	ops.push_back(PdfProperty(CRealFactory::getInstance( 0.015)));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-2.199 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-1.969)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-5.555 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-2.242)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.642)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-1.42)));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.024 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance( -0.862)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(  -3.916)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0.111)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-3.954)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0.916)));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-5.658 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(1.795)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-6.772 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(3.222)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-6.772)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(4.839)));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-6.772 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(7.509 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-3.74 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(9.674)));
	ops.push_back(PdfProperty(CRealFactory::getInstance (0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(9.674 )));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance( 3.74)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(9.674)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(6.772 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(7.509)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(6.772)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(4.839)));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(6.772 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance( 2.167)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(3.74 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(0 )));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance( 7.74 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(  12.616)));
	apStream->addToBuffer(createOperator("m",ops));
	ops.clear();
	ops.push_back(PdfProperty(CRealFactory::getInstance(-7.74 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(  12.616)));
	apStream->addToBuffer(createOperator("l",ops));
	ops.clear();


	ops.push_back(PdfProperty(CRealFactory::getInstance(-8.274 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(12.616 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance( -8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(12.184)));
	ops.push_back(PdfProperty(CRealFactory::getInstance( -8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(  11.649)));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-8.707 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-3.831 )));
	apStream->addToBuffer(createOperator("l",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-8.707 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.365 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance( -8.274)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.798)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-7.74 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance( -4.798)));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(-7.74 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance( -4.798)));
	apStream->addToBuffer(createOperator("l",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance( 8.274)));
	ops.push_back(PdfProperty(CRealFactory::getInstance( -4.798)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(-4.365)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(8.707 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(  -3.831)));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(11.649)));
	apStream->addToBuffer(createOperator("l",ops));
	ops.clear();

	ops.push_back(PdfProperty(CRealFactory::getInstance(8.707)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(12.184 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(8.274)));
	ops.push_back(PdfProperty(CRealFactory::getInstance( 12.616)));
	ops.push_back(PdfProperty(CRealFactory::getInstance(7.74 )));
	ops.push_back(PdfProperty(CRealFactory::getInstance(12.616 )));
	apStream->addToBuffer(createOperator("c",ops));
	ops.clear();

	apStream->addToBuffer(createOperator("b",ops));


	apStream->validate();
END:
	addHistory("Appearance stream for comment created\n");
	return _pdf->addIndirectProperty(apStream);
}
void TabPage::insertTextMarkup(PdfAnnot annot)
{
	if (!CanBeSavedChanges(true))
		return;
	if ( this->ui.revision->currentIndex()!= _pdf->getRevisionsCount()-1)
		return;
	//for all selected text
	/*for ()
	if (!_selected)
	{
		addHistory("Text not selected, aborting\n");
		return;
	}*/
	bool changing = false;
	float dim[4] = {FLT_MAX,FLT_MAX,0,0};
	for ( int i =0; i < _annots.size(); i++)
		if (_annots[i] == annot)
		{
			shared_ptr<CArray> arr = annot->getDictionary()->getProperty<CArray>("Rect"); 
			for ( int a =0; a< 4; a++)
			{
				if (arr->getProperty(a)->getType() == pInt)
					dim[a] = utils::getIntFromArray(arr,a);
				if (arr->getProperty(a)->getType() == pReal)
					dim[a] = utils::getDoubleFromArray(arr, a);
			}
			IndiRef indi = createAppearanceHighlight(dim);
			PdfProperty prop(pdfobjects::CRefFactory::getInstance(indi));
			boost::shared_ptr<pdfobjects::CDict> nDict(CDictFactory::getInstance());
			nDict->addProperty("N",*prop);
			annot->getDictionary()->setProperty("AP",*nDict);
			emit addHistory("Annotation changed\n");
			operationDone();
			return;
		}
	//toto je len pre highlighty, ktore este nemame
	_page->addAnnotation(annot);	
	emit addHistory("Annotation added\n");
	_page->getAllAnnotations(_annots);
	annot = _annots.back();//posledny pridany, na nom budeme pachat zmeny
	std::vector<float> coordinates;

	fillCoordinates(coordinates, dim);
	IndiRef indi = createAppearanceHighlight(dim);
	{
		CArray rect;
		for ( int i = 0; i < 4;i++)
			rect.addProperty(*(PdfProperty(CRealFactory::getInstance(dim[i]))));
		annot->getDictionary()->setProperty("Rect",rect); 
	}
	//vytvor ApearanceStream
	//boost::shared_ptr<pdfobjects::CDict> apDict(pdfobjects::CDictFactory::getInstance());

	CArray points;
	for ( int i = 0; i < coordinates.size();i++)
		points.addProperty(*(PdfProperty(CRealFactory::getInstance(coordinates[i]))));
	annot->getDictionary()->setProperty("QuadPoints",points);
	boost::shared_ptr<pdfobjects::CDict> nDict(CDictFactory::getInstance());

	PdfProperty prop(pdfobjects::CRefFactory::getInstance(indi));
	nDict->addProperty("N",*prop);
	annot->getDictionary()->setProperty("AP",*nDict);
	std::string m;
	//apDict->getStringRepresentation(m);
	//annot->getDictionary()->getProperty("AP")->getStringRepresentation(m);
	//fprintf(stderr,"$s\n",m.c_str());

	//najsi mi vsetky textove operatory a z nich vycuvam svojich 8 cisel, pravepodobne z gxfconfu a pozicie offset operatorov ( vektory )
	//TODO treba osetrit na to, ak chceme zvyraznit len jednu cast textoveho operatora
	redraw();
}
void TabPage::closeAnnotDiag()
{
	delete _cmts; //cleanup za anotacie
	_cmts = NULL;
	//_mode = ModeSelectText;
	//disconnect all?
}
void TabPage::insertTextAnnot(PdfAnnot a)
{
	if (!CanBeSavedChanges(true))
		return;
	if ( this->ui.revision->currentIndex()!= _pdf->getRevisionsCount()-1)
		return;
#ifdef _DEBUG
	std::string m;
	a->getDictionary()->getStringRepresentation(m);
#endif // _DEBUG
	_page->addAnnotation(a);
	emit addHistory("Text annotation added\n");
	_page->getAllAnnotations(_annots);
	a = _annots.back();
	float dim[]={0,0,18,18};
	boost::shared_ptr<pdfobjects::CDict> nDict(CDictFactory::getInstance());

	IndiRef indi = createAppearanceComment(dim);
	PdfProperty prop(pdfobjects::CRefFactory::getInstance(indi));
	nDict->addProperty("N",*prop);
	a->getDictionary()->setProperty("AP",*nDict);
	redraw();
}
void TabPage::insertAnnotation(PdfAnnot a)
{
	if ( !this->CanBeSavedChanges())
		return;
#ifdef _DEBUG
	std::string m;
	a->getDictionary()->getStringRepresentation(m);
#endif // _DEBUG
	_page->addAnnotation(a);
	emit addHistory("General annotation added\n");
	redraw();
	//	std::string m;
	//_pdf->addIndirectProperty(a->getDictionary())
	//	_annots[0]->getDictionary()->setProperty("AP",a->getDictionary()->getpo);
	//	_pdf->getIndirectProperty(i)->getStringRepresentation(m);
	//setFromSplash();
}
//void TabPage::deleteAnnotation(QPoint point)
//{
//	_page->getAllAnnotations(_annots);
//	for (int i =0; i < _annots.size(); i++)
//	{
//		shared_ptr<CArray> rect;
//		PdfProperty prop = _annots[i]->getDictionary()->getProperty("Rect");
//		rect = pdfobjects::IProperty::getSmartCObjectPtr<CArray>(prop);
//		float x1,x2,y1,y2;
//		x1 = utils::getSimpleValueFromArray<CReal>(rect,0);
//		y1 = utils::getSimpleValueFromArray<CReal>(rect,1);
//		x2 = utils::getSimpleValueFromArray<CReal>(rect,2);
//		y2 = utils::getSimpleValueFromArray<CReal>(rect,3);
//		//dostat annotecny rectangle
//		BBox b(x1,y1,x2,y2);
//		QRect convertedRect = getRectangle(b);
//		if (convertedRect.contains(point))
//		{
//			_page->delAnnotation(_annots[i]);
//			redraw();
//			return;
//		}
//	}
//	printf("Anotation was not deleted");
//}
void TabPage::raiseChangeSelectedText()
{
	if (!CanBeSavedChanges(true))
		return;
	if (!_selected)
		return;
	QString s,s1,s2,s3;
	TextData::iterator it = sTextIt;
	sTextIt->split(s1,s,s3);
	while (it != sTextItEnd)
	{//TODO medzera?
		it++;
		//musime vratane posledneho
		if (it != sTextItEnd)
			s+=it->_text;
		else
		{
			it->split(s1,s2,s3);
			s+=s2;
		}
	}//TODO set font height
	float corr = displayparams.vDpi/72;
	float h = sTextIt->_op->getFontHeight()*sTextIt->_width;//TODO fontmatrix, fontBBox
	
	double ret, dummy; //TODO conevrt page rotation!!
	//rotatePdf(displayparams,pos,ret,true);
	displayparams.convertPixmapPosToPdfPos(sTextIt->_origX, sTextIt->_ymin, dummy, ret);//TODO font matrix
	double pos = sTextIt->getPreviousStop();

	const double * fMatrix = sTextIt->_op->getCurrentFont()->getFontBBox();
	ret -= fMatrix[3]*h ; //TODO toto nemusi byt vyska BBoxu
//TODO checknut aj pre ostatne fonty, mozno treba fontmatrix sofistikovanejsiu
	_font->setPosition(pos/corr,ret);
	_font->setHeight(sTextIt->_op->getFontHeight());
	_font->setText(s);
	_font->setChange();
}
//void TabPage::waitForPosition(Annot _an)
//{
//	_annots.push_back(_an);
//	_mode = ModeEmitPosition;
//}
void TabPage::raiseAnnotation(QPoint point)//raise cpmment annotation
{
	if (!CanBeSavedChanges(true))
		return;
	//_mode = ModeEmitPosition;
	double x=point.x(),y=point.y();
	rotatePdf(displayparams ,x,y,true);
	//displayparams.convertPixmapPosToPdfPos(point.x(), point.y(),x,y);
	//float xdif = displayparams.vDpi/72;
	//float ydif = displayparams.hDpi/72;
	double d[] = {x, y};
	displayparams.convertPixmapPosToPdfPos(d[0],d[1],d[0],d[1]);
	_cmts->setRectangle(d[0],d[1],30,30);//pre zvysok sa to vyhodi a nahradi sadou anotacii	
	_cmts->setWindowFlags(Qt::Window);
	_cmts->setHColor(_parent->getHColor());
	_cmts->show();
}

void TabPage::createList()
{	
	emit addHistory("Recreating text list");
	_textList.clear();
	//get all pdf text operators in list
	Ops ops;
	libs::Rectangle rect(0,0,FLT_MAX,FLT_MAX); //dostaneme vsetky operatory
	_page->getObjectsAtPosition( ops, rect);
	//choose just test iterator
	Ops::iterator it = ops.begin();
	std::vector<GfxState *> states;
	{
		DisplayParams p;
		PDFRectangle pdfRect ( p.pageRect.xleft, p.pageRect.yleft, p.pageRect.xright, p.pageRect.yright );
		states.push_back(new GfxState(p.hDpi, p.vDpi, &pdfRect, p.rotate, p.upsideDown ));
	};
	while ( it != ops.end())
	{
		std::string n;
		(*it)->getOperatorName(n);
#ifdef _DEBUG
		if (n=="BDC")
		{
			std::string xx;
			(*it)->getStringRepresentation(xx);
			BBox b =(*it)->getBBox();
		}
#endif // _DEBUG

		if (!typeChecker.isType(OpTextName,n))
		{
			it++;
			continue;
		}
		OperatorData data(*it,displayparams);
		_textList.push_back(data);
		it++;
	}
	//sort list
	_textList.sort();
	sTextIt = sTextItEnd = sTextMarker = _textList.begin();
#ifdef _DEBUG
	if (_textList.empty())
		return;
	TextData::iterator iter = _textList.begin();
	TextData::iterator iter2 = _textList.begin();
	iter2++;
	while ( iter2!= _textList.end())
	{
		*iter < *iter2;
		iter++;
		iter2++;
	}
#endif // _DEBUG
}
void TabPage::raiseInsertText(QPoint point)
{
	if (!CanBeSavedChanges(true))
		return;
	double x,y;
	DisplayParams d = displayparams;
	d.pageRect = _page->getMediabox();
	d.convertPixmapPosToPdfPos(point.x(), point.y(), x, y); //TODO upravit aby to neblblo
	//Musime skontrolovat exte aky mediapage pouziva
	//toPdfPos(point.x(),point.y(),x,y);
	////do povodneho stavu .. hotfix
	//x/=displayparams.vDpi/72;
	//y/=displayparams.hDpi/72;
	_font->setPosition(x,y);
	_font->setInsert();
}
void TabPage::extractImage()
{
	if (!_selected || _selectedImage == NULL)
	{
		emit addHistory("No inline image was selected");
		return;
	}
	{
		shared_ptr<pdfobjects::CInlineImage> inIm =  boost::dynamic_pointer_cast<pdfobjects::CInlineImage>(_selectedImage);
		boost::shared_ptr<IProperty> prop = inIm->getProperty("CS");
		std::string cs = utils::getNameFromIProperty(prop);
		if (cs!="RGB")
		{
			QMessageBox::warning(this, "Unable to export inline image","Unsupported color space", QMessageBox::Ok,QMessageBox::Ok);
			return; 
		}
		int bpp = utils::getIntFromIProperty(inIm->getProperty("BPC"));
		if(bpp!=8)
		{
			emit addHistory("Failed to export an image\n");
			QMessageBox::warning(this, "Unable to export inline image","Wrong depth", QMessageBox::Ok,QMessageBox::Ok);
			return;
		}
		QImage image(QSize(inIm->width(),inIm->height()),QImage::Format_ARGB32);
		CStream::Buffer buffer = inIm->getBuffer();
		int index = 0;

		for ( int h=0; h<inIm->height();h++)
			for ( int w=0; w<inIm->width();w++)
			{ 
				QColor c((unsigned char)buffer[index],(unsigned char)buffer[index+1],(unsigned char)buffer[index+2],0);
				image.setPixel(w,h,c.rgb());
				index+=3;
			}

			QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), "", tr("Images (*.png *.jpg *.bmp))"));
			if (fileName.isEmpty())
				return;
			image.save(fileName);
			emit addHistory("Image saved\n");
	}
}
void TabPage::clearSelected()
{
	_selected = false;
	_parent->setPreviousMode();
	emit ChangePageModeSignal(ModeDrawNothing);
}
void TabPage::clicked(QPoint point) //resp. pressed, u select textu to znamena, ze sa vyberie prvy operator
{
	switch (_parent->getMode())
	{
	case ModeFindLastFont:
		{
			double x = point.x(),y = point.y();
			displayparams.convertPixmapPosToPdfPos(point.x(),point.y(),x,y);
			//rotatePdf(displayparams,x,y,false);//TODO check
			_font->addFont(getPreviousFontInPosition(libs::Point(x,y)));
			//find last TM
			float w[2];
			getPreviousTmInPosition(libs::Point(x,y),w);
			_font->addTm(w[0],w[1]);
			_font->show();
			emit addHistory("Font was selected");
			break;
		}
	case ModeChangeAnnotation:
		{
			if (!CanBeSavedChanges(true))
				return;
			int index = _labelPage->getPlace(point);
			if (index == -1)
				break;
			_cmts->loadAnnotation(_annots[index]);
			break;
		}
	case ModeInsertLinkAnnotation:
		{
			if (!CanBeSavedChanges(true))
				break;
			_labelPage->annotation();
			_cmts->setIndex(ALink);
			break;
		}
	case ModeInsertAnnotation:
		{
			if (!CanBeSavedChanges(true))
				break;
			_labelPage->annotation();
			_cmts->setIndex(0);
			break;
		}
	case ModeInsertText:
		{
			//show position
			if (!CanBeSavedChanges(true))
				break;
			_labelPage->drawCircle(point);
			raiseInsertText(point);
			break;
		}
	case ModeEmitPosition: //pre anotacie
		{
#ifdef _DEBUG
			std::string m;
			_linkAnnot->getDictionary()->getStringRepresentation(m);
#endif // _DEBUG
			double x = point.x();
			double y = point.y();//jelikoz to ide priamo do dictionary, musim prejst cez params.frompdf :-/
			displayparams.convertPixmapPosToPdfPos(x,y,x,y);
			rotatePdf(displayparams,x,y,true);
			if ( !this->CanBeSavedChanges())
				return;
			 _pdf->getPage(_oldPage)->addAnnotation(_linkAnnot);
			 _page->getAllAnnotations(_annots);
			 _cmts->addLink(_annots.back(), _page->getDictionary()->getIndiRef(),x,y);

			_parent->setModeInsertLinkAnotation();//boli sme v tomto 
#ifdef _DEBUG
			_linkAnnot->getDictionary()->getStringRepresentation(m);
			//_page->getAllAnnotations(_annots);
			//assert(_annots[0] == _linkAnnot);
#endif // _DEBUG
			emit addHistory("Position was selected");
			_page = _pdf->getPage(_oldPage);
			redraw();
			break;
		}
	case ModeImageSelected:
	case ModeSelectImage:
		{//vyber nejaky BI operator a daj ho to vybranych
			if (!CanBeSavedChanges(true))
				break;
			Ops ops;
			_mousePos = point;
			double px = point.x(), py=point.y();
			//rotatePdf(displayparams,px,py,true);
			//HACK - px, py su teraz v PDF poziciach
			//px, py musime ale zmenit vzhladom na to,ze sa u inline neberie do uvahy displayparams

			_page->getObjectsAtPosition(ops,libs::Rectangle(0,0,10000,10000));
			while(!ops.empty())
			{
				std::string name;
				ops.back()->getOperatorName(name);
				if (typeChecker.isType(OpImageName,name))
				{
					_selected = true;
					_selectedImage = ops.back();
					BBox bbox = _selectedImage->getBBox();
					QRect convertedRect = getRectangle(bbox);//PODIVNE
					_labelPage->drawAndTrackRectangle(convertedRect);
					_parent->setMode(ModeImageSelected);
					emit addHistory("Image was selected");
					break;
				}
				ops.pop_back();
			}
			if (ops.empty())
				emit addHistory("No inline image found");
			break;
		}
	case ModeHighlighComment:
	case ModeSelectText:
		{
			if (!CanBeSavedChanges(true))
				return;
			_labelPage->setMode(ModeTrackDrawingRect);
			//highlightText(point); //nesprav nic, pretoze to bude robit mouseMove
			break;
		}
	case ModeImagePartCopied:
	case ModeImagePart:
	case ModeInsertImage:
		{
			if (!CanBeSavedChanges(true))
				return;
			if (!_dataReady)
			{
				_mousePos = point;
				_dataReady = true;
				QRect r(_mousePos,QSize(1,1));
				_labelPage->drawAndTrackRectangle(r);
				_labelPage->setMode(ModeTrackDrawingRect);
				return;
			}
			{
				assert(false);
			}
			break;
		}
	case ImageMode:
		{
			//show only picture, all picture
			//get only operator
			double px, py;
			//convert
			toPdfPos(point.x(),point.y(), px, py);
			//find operattdisplayparamsors
			Ops ops;
			_page->getObjectsAtPosition(ops, libs::Point(px,py));
			//vsetky tieto objekty vymalujeme TODO zistit orientaciu
			for ( size_t i =0; i < ops.size(); i++)
			{
				//ukaz len povolene typy
				std::string s;
				ops[i]->getOperatorName(s); 
				if (!typeChecker.isType(OpImageName,s))
				{
					QClipboard *clipBoard = QApplication::clipboard();
					clipBoard->setImage(_labelPage->getImage().copy(getRectangle(ops[i])));
					break;
				}
			}
		}
	case ModeOperatorSelect:
		{
			_labelPage->unsetImg();
			showClicked(point.x(),point.y());//
			break;
		}
	default: //ukazuje celeho operatora
		{
			break;//do nothing
		}
	}
}

void TabPage::deleteSelectedImage()
{
	if (!_selected)
		return;
	std::string s;
	_selectedImage->getOperatorName(s);
	_selectedImage->getContentStream()->deleteOperator(_selectedImage);
	_selectedImage = PdfOp(); //empty
	_selected = false;
	clearSelected();
	redraw();
}
void TabPage::mouseReleased(QPoint point) //nesprav nic, pretoze to bude robit mouseMove
{
	switch (_parent->getMode())
	{
	case ModeDeleteHighLight:
		{
			if (!CanBeSavedChanges())
				break;
			int index = _labelPage->getPlace(point);
			if (index<0)
				break;
			PdfAnnot an = _annots[index];
			std::string type = utils::getNameFromDict("Subtype", an->getDictionary());
			if (type!= "Highlight")
				break;
			_page->delAnnotation(an);
			operationDone();
			break;
		}
	case ModeDeleteAnnotation:
		{
			if (!CanBeSavedChanges())
				break;
			int index = _labelPage->deleteAnnotation(point);
			if (index <0 )
				break;
			_page->delAnnotation(_annots[index]);
			_page->getAllAnnotations(_annots);
			operationDone();
			break;
		}
	case ModeHighlighComment:
		{
			if (!CanBeSavedChanges())
				break;
			highlightText(/*_mousePos, point*/);
			if (!_selected)
				break;
			_labelPage->annotation();
			_cmts->setIndex(AHighlight);
			break;	
		}
	case ModeSelectText:
		{
			highlightText(/*_mousePos, point*/);
			break;
		}
	case ModeInsertImage:
		{
			if (!CanBeSavedChanges())
				break;
			QRect r(min(_mousePos.x(),point.x()),min(_mousePos.y(),point.y()), 
				abs(_mousePos.x()-point.x()), abs(_mousePos.y()-point.y()));
			libs::Point p;
			p.x = r.x();
			p.y = r.y();
			//rotatePdf(displayparams, p.x, p.y,true);
			displayparams.convertPixmapPosToPdfPos(p.x,p.y,p.x,p.y);
			_image->setPosition(p.x,p.y);
			raiseInsertImage(r);
			_dataReady=false;
			break;
		}
	case ModeImagePartCopied:
	case ModeImagePart: //data ready
		{
			//copy to clipboard
			QClipboard *clipBoard = QApplication::clipboard();
			QRect rect(min(point.x(),_mousePos.x()), min(point.y(),_mousePos.y()), abs(point.x()-_mousePos.x()),abs(point.y()-_mousePos.y()));
			clipBoard->setImage(_labelPage->getImage().copy(rect));
			_parent->setMode(ModeImagePartCopied);
			emit addHistory("image was copied to the clipboard");
			operationDone();
			break;
		}
	case ModeImageSelected:
		{
			//vytvor nove cm a posun obrazok
			if (!CanBeSavedChanges())
				break;
			if(!_selected)
				break;
			if (abs(_mousePos.x()-point.x())<2 && abs(_mousePos.y()-point.y()) <2)
				return;//nebudeme pohybovat
			double x = point.x()-_mousePos.x(),y=_mousePos.y() - point.y();
			shared_ptr<InlineImageCompositePdfOperator> img = boost::dynamic_pointer_cast<InlineImageCompositePdfOperator>(_selectedImage);
			//get invert matrix & get actual matrix
			double invert[6];
			double actual[6];
			_image->getInvertMatrix(img,actual, invert);
			//convertuje podla matice
			double pomx =x, pomy =y;
			x = invert[0] * pomx + invert[2] * pomy;
			y = invert[1] * pomx + invert[3] * pomy;//ROTATE
			::rotate(_page->getRotation(),x,y);	
			PdfOp nw= createOperatorTranslation(x, y);//TODO zistit invertnu CM(bez poslednych POS suradnic)
			PdfOp prev = createOperatorTranslation(-x, -y);
			_selectedImage->getContentStream()->replaceOperator(_selectedImage,nw);
			nw->getContentStream()->insertOperator(nw,_selectedImage);
			_selectedImage->getContentStream()->insertOperator(_selectedImage,prev); //predchadzajuca pozicia
			_selected = false;
			operationDone();
			break;
		}
	default:
		_labelPage->setMode(ModeDrawNothing);
		break;
	}
	_labelPage->update();
	_dataReady = false;
}
void TabPage::highLightBegin(int x, int y) //nesprav nic, pretoze to bude robit mouseMove
{

	//najdi prvy operator, na ktory bolo kliknute
	Ops ops;
	_page->getObjectsAtPosition(ops, libs::Point(x,y)); //zaplnili sme operator
	//zistime, ze je to text
	std::string n;
	if (ops.empty())
		return;
#ifdef _DEBUG
	std::string na;
	int q = ops.size();
	for ( int i =0; i<ops.size(); i++)
		ops[i]->getStringRepresentation(na);
#endif // _DEBUG
	bool found = false;
	PdfOp op = getValidTextOp(ops,found);
	if (!found)
		return; //zoberieme iba posledny, viditelny, ak su na sebe
	_dataReady = true;
#if _DEBUG
	std::string m;
	op->getStringRepresentation(m);
	DEBUGLINE("Operator found");
#endif
	sTextIt = _textList.begin();
	setTextData(sTextIt,_textList.end(),op);
	sTextIt->clear();
	double px=x,py=y;
	rotatePdf(displayparams,px,py,true);
	sTextIt->setBegin(x);//zarovnane na pismenko
	sTextMarker = sTextItEnd = sTextIt;
}
void TabPage::setTextData(TextData::iterator & it, TextData::iterator end,shared_ptr< PdfOperator > op)
{
	for ( ; it!= end; it++)
	{
		if (op == it->_op)
			return;
	}
	throw "Unexpected operator, text is not present in tree, why, why? ";
}

void TabPage::highlightText() //tu mame convertle  x,y, co sa tyka ser space
{
	//double x = point.x(),y = point.y();
	QRect r = _labelPage->getRect();
	libs::Rectangle lRect(r.left(),  r.top(), r.right(), r.bottom());
	//rotatePdf(displayparams,x,y,true);
		//_mousePos = point;
		//_labelPage->unsetImg();
		//highLightBegin(x,y); //oznaci zaciatok
		//return;
	
	Ops ops;
	_page->getObjectsAtPosition(ops, lRect);
	bool found;
	StateUpdater::CheckTypes c;

	sTextIt = _textList.begin();
	int i = 0;
	if (ops.empty())
	{
		_selected = false;
		_labelPage->update();
		return;
	}
	for (; i < ops.size(); i++)
	{
		std::string s;
		ops[i]->getOperatorName(s);
		memcpy(c.name,s.c_str(),s.size());
		c.name[s.size()] = '\0';
		if (!isTextOp(c))
			continue;
		setTextData(sTextIt,_textList.end(), ops[i]);
		sTextMarker = sTextItEnd = sTextIt;
		break;
	}
	for(i; i<ops.size();i++)
	{
		std::string s;
		ops[i]->getOperatorName(s);
		memcpy(c.name,s.c_str(),s.size());
		c.name[s.size()] = '\0';
		if (!isTextOp(c))
			continue;
		OperatorData op(ops[i],displayparams);
		if ( op < *sTextIt )
		{
			sTextIt =_textList.begin();
			setTextData(sTextIt,_textList.end(),ops[i]);
		}
		if (*sTextItEnd < op)
			setTextData(sTextItEnd,_textList.end(),ops[i]);
	}
	for (TextData::iterator iter = sTextIt; iter!= sTextItEnd; iter++)
	{
		iter->clear();
	}
	sTextItEnd->clear();
	
	double xxPoint = r.left() , yyPoint =r.top();
	double xxOrig = r.right() , yyOrig= r.bottom();
	rotatePdf(displayparams,xxPoint,yyPoint,true);
	rotatePdf(displayparams,xxOrig,yyOrig,true);
	sTextIt->setBegin(min(xxOrig,xxPoint));
	sTextItEnd->setEnd(max(xxOrig,xxPoint));

	_selected =  true;
	highlight();
	//_dataReady = false;
}

void TabPage::highlight()
{
	if (!_selected)
		return;
	TextData::iterator first = sTextIt;
	TextData::iterator last = sTextItEnd; 
	bool forw = (*sTextIt) < (*sTextItEnd);
	double x1,x2,y1,y2;
	QList<QRect> region;
	assert(region.isEmpty());
	while (true)
	{
		x1= first->getPreviousStop();
		y1 = first->_ymin;
		//rotatePdf(displayparams,x1,y1,false);
		x2 = first->getNextStop();
		y2 = first->_ymax;

		DisplayParams d = displayparams;
		d.rotate*=-1;
		rotatePdf(d,x1,y1,true);
		rotatePdf(d,x2,y2,true);

		//rotatePdf(displayparams,x1,y1,false);
		//rotatePdf(displayparams,x2,y2,false);

		//rotatePdf(d,x1,y1,false);
		//rotatePdf(d,x2,y2,false);
		QRect r(min(x1,x2),min(y1,y2), fabs(x2-x1),fabs(y1-y2));
		region.append(r);
		if (first == sTextItEnd)
		{
			this->ui.scrollArea->ensureVisible(x2,y2);
			break;
		}
		inDirection(first, forw);
	}
	_labelPage->fillRect( region, _parent->getColor() );
}

//zatial len v ramci jednej stranky
//void TabPage::moveText(int difX, int difY) //on mouse event, called on mouse realease
//{
	//for each selected operator, move it accrding to position
	//if (!_selected) //spravne nastavene 
	//	return;
	////ostran z listu
	//TextData::iterator first = sTextIt; 
	//TextData::iterator last = sTextItEnd; 
	//bool forw = (*sTextIt) < (*sTextItEnd);
	//if (!forw)
	//{
	//	first = sTextItEnd;
	//	last = sTextIt;
	//}
	//QString s1,s2,s3,s4;
	////problem je, e to moze by tiez cast - jedna sa len o prve a posledne. To zmazeme, ponechame cast a insterime znova
	//_selected = false;
	//if (first==last)
	//{ //sprav to same len s jednym iteratorom
	//	//treba ho rozdelit na niekolko operatorov. Jelikoz je to jeden, tak na tri
	//	double b = first->_begin;
	//	double e = first->_end;
	//	first->replaceAllText(checkCode(s1,first->_op->getFontName())); //netreba nic mazat, je prave jedno
	//	createAddMoveString(first->_op,e+difX,first->_ymax+difY,s3);
	//	createAddMoveString(first->_op,b+difX,first->_ymax+difY,s2);
	//	return;
	//}
	//first->split(s1,s2,s3);	//zajima nas iba s2 -> od begin po end, s3 bude prazdna
	//float x = first->_begin , y=first->_ymax;
	//first->replaceAllText(checkCode(s1,first->_op->getFontName())); //tuto sa to pomeni
	//last->replaceAllText(checkCode(s3,first->_op->getFontName()));
	//last->split(s1,s3,s4); //last je end
	//createAddMoveString(first->_op,last->_begin+difX,last->_ymax+y,s2);
	//while (first!=last)
	//{
	//	PdfOp o = FontWidget::createTranslationTd(x+difX, y+difY);
	//	insertBefore(o, last->_op);
	//	last--;
	//	x = last->_begin;
	//	y = last->_ymax;
	//}
	//createAddMoveString(first->_op,first->_begin+difX,first->_ymax+y,s2);

	//_textList.sort();
//}
void TabPage::insertBefore(PdfOp op, PdfOp before)
{
	PdfOp clone = before->clone();
	before->getContentStream()->replaceOperator(before,op,false);
	op->getContentStream()->insertOperator(op,clone,true);
}

void TabPage::getPreviousTmInPosition( libs::Point p, float* size )
{
	double x = p.x;
	double y = p.y;
	//rotatePdf(displayparams,x,y,false);//TODO kontrola
	TextData::iterator iter = _textList.begin();
	TextData::iterator last = iter;
	float dist = 10e38;
	while (iter!= _textList.end())
	{
		float xdist = iter->_origX2 - p.x;//vzdialenesie
		float ydist = iter->_ymin - p.y;
		float distTest = xdist*xdist + ydist*ydist;
		if (dist<distTest)//nasli sme
			break;
		else
			dist = distTest;
		iter++;
	}//TODO do spolocneho priestoru
	float r[4] = {1,0,0,1};
	std::vector<float> ints;
	if (iter != _textList.end())
	{
		TextChangeOperatorIterator it = PdfOperator::getIterator<TextChangeOperatorIterator>(iter->_op,false);
		std::string name;
		while (it.valid())
		{
			it.getCurrent()->getOperatorName(name);
			if(name == "BT")
				break;
			if(name == "Tm")//chceme zistit, aka sirka sa na to pouzivala
			{
				PdfOperator::Operands operands;
				it.getCurrent()->getParameters(operands);
				for ( int a = 0; a < 4; a++)
					ints.push_back(utils::getValueFromSimple<CReal>(operands[a]));
			}
			it.prev();
		}
	}
	while(!ints.empty())
	{
		float pom[4];
		for (int i =3; i>=0; i--)
		{
			pom[i] = ints.back();
			ints.pop_back();
		}
		float a00 =pom[0]*r[0],
			  a02 =pom[0]*r[2],
			  a21 =pom[2]*r[1],
			  a23 =pom[2]*r[3],
			  a10 =pom[1]*r[0],
			  a12 =pom[1]*r[2],
			  a31 =pom[3]*r[1],
			  a33=pom[3]*r[3];
		r[0] = a00 + a21;
		r[1] = a02 + a23;
		r[2] = a10 + a31;
		r[3] = a12 + a33;
	}
	size[0] = r[0];//TODO toto moze sklamat, ak sa to este zvalstna skaluje, co nepredpokladam
	size[1] = r[3];
}
PdfOp TabPage::getPreviousFontInPosition(libs::Point pdfPos)//iba pre textove veci, bez upravy displayparam
{
	double x = pdfPos.x;
	double y = pdfPos.y;//toto sa vola  parametrom pozicie, kam chceme insertnut text
	{
		displayparams.convertPdfPosToPixmapPos(pdfPos.x,pdfPos.y,x,y);
		rotatePdf(displayparams,x,y,false);//TODO kontrola
	}
	TextData::iterator iter = _textList.begin();
	TextData::iterator last = iter;
	TextData::iterator found = iter;
	float dist = 10e38;
	while (iter!= _textList.end())
	{
		//float h = iter->_ymax - iter->_ymin;
		float distXTest =  x - iter->_origX;//vzdialenesie
		float distYTest = y - iter->_ymin;
		if ((distYTest > -1) && (distXTest >-1))//je to pred
		{
			float d = distXTest*distXTest + distYTest*distYTest;
			if ( d < dist)
			{
				found = iter;
				dist = d;
			}
		}
		iter++;
	}
	//najdi posledny TF
	FontOperatorIterator it = PdfOperator::getIterator<FontOperatorIterator>(found->_op,false);

	if (!it.valid())
		return shared_ptr<PdfOperator>((PdfOperator*)NULL);
	PdfOp op = it.getCurrent()->clone();
#if _DEBUG
	std::string m;
	op->getStringRepresentation(m);
#endif
	return op;
}
void TabPage::createAddMoveString(PdfOp bef, double x, double y, QString name)
{
	PdfOperator::Operands ops;
	ops.push_back(boost::shared_ptr<IProperty> (new CString(name.toAscii().data())));
	PdfOp p = createOperator("tj",ops);
	bef->getContentStream()->insertOperator(bef,p);
	OperatorData d(p,displayparams);
	_textList.push_back(d);
	PdfOp op = FontWidget::createTranslationTd(x,y);
	bef->getContentStream()->insertOperator(bef,p);	
}
void TabPage::inDirection(TextData::iterator& iter, bool forward)
{
	if (forward)
		iter++;
	else 
		iter--;
}
void TabPage::getAtPosition(Ops& ops, int x, int y )
{
	double px=x, py=y;
	rotatePdf(displayparams,px,py,true);
	//find operattors
	_page->getObjectsAtPosition(ops, libs::Point(px,py));
}
void TabPage::toPdfPos(int x, int y, double & x1, double &y1)
{
	displayparams.convertPixmapPosToPdfPos(x, y, x1, y1); //TODO upravit aby to neblblo
	//x1 = x1*displayparams.vDpi/72; //pretoze BBoxy su takto spravene
	//y1 = y1*displayparams.hDpi/72;
	//upside down
	//y1 = DisplayParams::DEFAULT_PAGE_RY*displayparams.vDpi/72 -y1;
	//bboxes su v pdf stejne podla pdicka
	//x1*= displayparams.vDpi/72; //pretoze BBoxy su takto spravene
	//y1*= displayparams.hDpi/72;
}
void TabPage::toPixmapPos(double x1, double y1, int & x, int &y)
{
	double x2, y2;
	displayparams.convertPdfPosToPixmapPos(x1, y1, x2, y2);
	x1 *= displayparams.hDpi/72;
	y1 *= displayparams.vDpi/72;
	x = x2;
	y = y2;
}

void TabPage::showClicked(int x, int y)
{
	double px=x, py=y;
	//convert
	//toPdfPos(x,y, px, py);
	//rotatePdf(displayparams,px,py,true);

	Ops ops;
	_page->getObjectsAtPosition(ops, libs::Point(px,py));

	for ( size_t i =0; i < ops.size(); i++)
	{
		//ukaz len povolene typy
		std::string s;
		ops[i]->getOperatorName(s);
		if (!typeChecker.isType(OpTextName,s))
			continue;
		std::wstring w;
		shared_ptr<TextSimpleOperator> txt= boost::dynamic_pointer_cast<TextSimpleOperator>(ops[i]);
		txt->getFontText(w);

		libs::Rectangle b = ops[i]->getBBox();

		QColor color(55, 55, 200,100);
		//rotatePdf(displayparams,b.xleft,b.yleft,false);
		//rotatePdf(displayparams,b.xright,b.yright,false);
		_labelPage->setPixmapFromImage( b.xleft, b.yleft, b.xright, b.yright, color );
		emit addHistory(QString("Selected operator ") + s.c_str() +"\n");
	}
}
QRect TabPage::getRectangle(PdfOp ops)
{
	QRect r;
	libs::Rectangle b = ops->getBBox();
	int x1,y1,x2,y2;

	toPixmapPos(b.xleft, b.yleft, x1,y1);
	toPixmapPos(b.xright, b.yright, x2, y2);
	//move according to page rotation
	/*int angle = page->getRotation();

	DEBUGLINE(y1);
	DEBUGLINE(y2);
	DEBUGLINE(angle);
	rotatePosition(x1,y1,x1,y1, angle);
	rotatePosition(x2,y2,x2,y2, angle);
	getc(stdin);*///TODO rotation when displaying

	r.setTop(min<float>(y1,y2));
	r.setBottom(max<float>(y1,y2));
	r.setLeft(min<float>(x1,x2));
	r.setRight(max<float>(x1, x2));
	return r;
}
void TabPage::updatePageInfoBar()
{
	//page changes
	QVariant nPages(_pdf->getPageCount());
	QVariant v(_page->getPagePosition());
	this->ui.pageInfo->setText( v.toString() + " / " + nPages.toString() );
}
void TabPage::pageUp()
{
	emit addHistory("Moving page up");
	if (_pdf->getPageCount()==1)
		return;
	int pos = _pdf->getPagePosition(_page);
	if(pos ==1)
		return;
	this->_pdf->removePage(pos);
	this->_pdf->insertPage(_page,pos-1);
	_page = _pdf->getPage(pos-1);
	updatePageInfoBar();
}
void TabPage::pageDown()
{
	emit addHistory("Moving page down");
	if (_pdf->getPageCount()==1)
		return;
	int pos = _pdf->getPagePosition(_page);
	if (pos == _pdf->getPageCount())
		return;
	this->_pdf->removePage(pos);
	this->_pdf->insertPage(_page,pos+1);
	_page = _pdf->getPage(pos+1);
	updatePageInfoBar();
}
void TabPage::setPage(int index)
{
	_page = _pdf->getPage(index);
	displayparams.rotate = _page->getRotation();
	this->redraw();
	setState();
	updatePageInfoBar();
}
bool TabPage::previousPage()
{
	if (_pdf->getPagePosition(_page) == 1)
		return false;
	int index = _pdf->getPagePosition(_page);
	setPage(index-1);
	return true;
}
bool TabPage::nextPage()
{
	if (_pdf->getPagePosition(_page) == _pdf->getPageCount())
		return false;
	int index = _pdf->getPagePosition(_page);
	setPage(index+1);
	return true;
}
void TabPage::getBookMarks()
{
	std::vector<shared_ptr<CDict> > outline;
	if (!_pdf->getDictionary()->containsProperty("Outlines"))
		return;
	emit addHistory("Getting bookmarks");
	shared_ptr<CDict> ol = _pdf->getDictionary()->getProperty<CDict>("Outlines");
	if (!ol->containsProperty("First"))
		return;

	ol = ol->getProperty<CDict>("First"); //musi obsahovat first
	while(ol->containsProperty("Next"))
	{
		outline.push_back(ol);
		PdfProperty p = utils::getReferencedObject(ol->getProperty("Next"));
		ol = p->getSmartCObjectPtr<CDict>(p);
	}
	outline.push_back(ol);

	this->ui.progressBar->show();
	this->ui.progressBar->setFormat("Loading bookmarks %p%");
	this->ui.progressBar->setValue(0);
	this->ui.progressBar->setMaximum(outline.size());

	for (size_t i =0; i< outline.size(); i++)
	{
		Bookmark * b = new Bookmark(ui.tree);
		setTree(outline[i],b); //TODO if "first" -> bude dam dalsia sekcia,add "dummy child"
		this->ui.tree->addTopLevelItem(b);
#ifdef _DEBUG
		std::string outText;
		outline[i]->getStringRepresentation(outText);
#endif // _DEBUG//utils::getStringFromDict (ip, "Title");
		std::string name = utils::getStringFromDict(outline[i],"Title");
		QString nm = QString::fromStdString(name);
		nm = nm.trimmed();
		QVariant v(nm);
		b->setText(0,v.toString());
		this->ui.progressBar->setValue(i);
	}
	this->ui.progressBar->hide();
}
void TabPage::setTree(shared_ptr<CDict> d, Bookmark * b)
{
	PdfProperty prop;
	if (!d->containsProperty("Dest"))
	{
		assert(d->containsProperty("A"));
		prop = utils::getReferencedObject(d->getProperty("A"));
#ifdef _DEBUG
		std::string m;
		prop->getStringRepresentation(m);
#endif // _DEBUG
		d = prop->getSmartCObjectPtr<CDict>(prop);
		prop = d->getProperty("S");
#ifdef _DEBUG
		d->getStringRepresentation(m);
#endif // _DEBUG
		if (utils::getNameFromDict("S",d) != "GoTo")
			return; //budeme mat invalid bookmark ze takuto akciu nevedieme
		prop = utils::getReferencedObject(d->getProperty("D"));//Toto by malo byt nase Dest ( Array)
	}
	else
	{
		prop = d->getProperty("Dest");
	}
	PropertyType type = prop->getType();
#ifdef _DEBUG
	std::string m;
	prop->getStringRepresentation(m);
#endif // _DEBUG
	if (type == pString)
		getDest(utils::getValueFromSimple<CString>(prop).c_str(),b);
	else if (type == pArray)
		getDestFromArray(prop,b);
	else
	{
		assert(false);
		return;
	}
	if (d->containsProperty("First"))
		b->setSubsection(utils::getRefFromDict("First",d));
}
//void TabPage::removeObjects() //vsetko, co je vo working
//{
//	for (size_t i = 0; i < workingOpSet.size(); i++ )
//	{
//		PdfOperator::Iterator it = PdfOperator::getIterator(workingOpSet[i]);
//		workingOpSet[i]->getContentStream()->deleteOperator(it,true);
//	}
//	workingOpSet.clear();
//}
void TabPage::changeSelectedImage(PdfOp op)
{
#ifdef _DEBUG
	std::string m;
	op->getStringRepresentation(m);
#endif // _DEBUG
	if (!_selected)
		return;
	_selectedImage->getContentStream()->replaceOperator(_selectedImage,op);
	clearSelected();
	emit addHistory("Inline image changed");
	redraw();
}
void TabPage::insertImage(PdfOp op) //positions
{
	if ( this->ui.revision->currentIndex()!= _pdf->getRevisionsCount()-1)
		return;
	Ops ops;
	ops.push_back(op);
	_page->addContentStreamToBack(ops);
	_parent->setMode(ModeSelectImage);
}
void TabPage::insertPageRangeFromExisting()
{ 
	if ( this->ui.revision->currentIndex()!= _pdf->getRevisionsCount()-1)
		return;
	QString s = QFileDialog::getOpenFileName(this, tr("Open File"),".",tr("Pdf files (*.pdf)"));
	if (s == NULL)
		return;
	boost::shared_ptr<pdfobjects::CPdf> pdf2 = boost::shared_ptr<pdfobjects::CPdf> ( 
		pdfobjects::CPdf::getInstance (s.toAscii().data(), pdfobjects::CPdf::ReadOnly));
	_page = _pdf->insertPage(pdf2->getPage(1),1);
}
void TabPage::deletePage()
{
	//removes actual page and displays the one after
	if (_pdf->getPageCount() <= 1)
		return;
	size_t i = _pdf->getPagePosition(_page);
	_pdf->removePage(i);
	if ( i > _pdf->getPageCount() ) //if removing last page..
		i =_pdf->getPageCount() ;
	setPage(i);
	emit addHistory(QString("page")+ QVariant(i).toString() + " deleted");
}
void TabPage::redraw()
{
	displayparams.pageRect = _page->getMediabox();
	JustDraw();
	_selected = false;
	createList();//TODO only in right mode
}
void TabPage::JustDraw()
{
	/*Ops oTest;
	page->getObjectsAtPosition(oTest,libs::Point(229,619));*/
	// display it = create internal splash bitmap
	_page->displayPage(splash, displayparams,-1,-1,-1,-1,true ); //vzdy reparsuj
	splash.clearModRegion();

	//( uchar * data, int width, int height, Format format )
	QImage image(splash.getBitmap()->getWidth(), splash.getBitmap()->getHeight(),QImage::Format_RGB32);	
	Guchar * p = new Guchar[3];
	for ( int i =0; i< image.width(); i++)
	{
		for ( int j =0; j < image.height(); j++)
		{
			splash.getBitmap()->getPixel(i,j,p);
			image.setPixel(i,j, qRgb(p[0],p[1],p[2]));
		}
	}
	delete[] p;
	showAnnotation();
	//image = image.scaled(QSize(max(x2,x1),max(y1,y2)));
	_labelPage->setImage(image);
	updatePageInfoBar();
}
#include <cmath>
void TabPage::resizeEvent(QResizeEvent * event)
{
	//_allowResize++;
	//if (_allowResize< ZOOM_AFTER_HACK)
	//	return;
	//_allowResize = ZOOM_AFTER_HACK;
	////this->resizeEvent(event);
	//float ratio = event->size().width();
	//ratio /= event->oldSize().width();
	//int ratioInt;
	//if (ratio < 1)
	//	ratioInt = ceil(ui.zoom->itemData(ui.zoom->currentIndex()).toInt()*ratio);
	//else 
	//	ratioInt = floor(ui.zoom->itemData(ui.zoom->currentIndex()).toInt()*ratio);
	//if (ratioInt ==1)
	//	return;
	//QVariant v(ratioInt);
	//int index = ui.zoom->findData(v);
	//if ( index == -1)
	//{
	//	index = ui.zoom->count();
	//	ui.zoom->addItem( QString( "Custom (" ) +v.toString() +")",v);
	//}
	//ui.zoom->setCurrentIndex(index);
}
void TabPage::wheelEvent( QWheelEvent * event ) //non-continuous mode
{
	QScrollBar * bar = this->ui.scrollArea->horizontalScrollBar();
	bar->setMinimum(0);
	bar->setMaximum(100);
	if (event->delta() > 0 )
	{
		//wheeling forward
		
		if (( bar->value() < event->delta()) && 
			(this->previousPage()))
		{
			ui.scrollArea->ensureVisible(0, _labelPage->height());
		}
	}
	else
	{
		if (( (bar->value() + event->delta()*-1)> bar->maximum() ) &&
			this->nextPage() )
		{
			ui.scrollArea->ensureVisible(0,0);
		}
	}
}
void TabPage::saveEncoded()
{
#if _DEBUG
	float dim[4] = {FLT_MAX,FLT_MAX,0,0};
	std::string out;
	//prop->getStringRepresentation(out);
	_pdf->saveDecoded("decoded");
	return;
#endif
#ifdef WIN32
	QString path = _name.mid(0,_name.indexOf("/"));
#else
	QString path = _name.mid(0,_name.size() - _name.indexOf("\\"));
#endif
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), path, tr("PdfFile Decoded (*.decoded)"));
	if (fileName.isEmpty())
		return;
	emit addHistory(QString("Saved decoded pdf to file") + fileName);
	_pdf->saveDecoded(fileName.toAscii().data());
}
void TabPage::save() //revision je inde
{
	_pdf->save(false);
	emit addHistory("PDF saved to hidden revision");
	initRevisions();
}
void TabPage::saveAs()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), _name, tr("PdfFile (*.pdf)"));
	savePdf(fileName.toAscii().data());
	emit addHistory(QString("Saved copy as ") + fileName);
	initRevisions();
}
bool TabPage::CanBeSavedChanges(bool raise)
{
	if(_pdf->getMode()==CPdf::ReadOnly)
	{
		if (raise)
			QMessageBox::warning(this, "ReadOnly pdf","Pdf cannot be changed", QMessageBox::Ok,QMessageBox::Ok); 
		return false;
	}
	return CanBeSaved(raise);
}
bool TabPage::CanBeSaved(bool raise)
{
	if (_pdf->isLinearized())
	{
		if (raise)
			QMessageBox::warning(this, "Linearized pdf","Pdf is linearized, cannot be saved", QMessageBox::Ok,QMessageBox::Ok); 
		return false;
	}
	if (ui.revision->currentIndex() != ui.revision->count()-1)
		if (QMessageBox::warning(this, "Revision mismatch","Cannot save older Revision\n Saving latest instead", QMessageBox::Ok | QMessageBox::Abort, QMessageBox::Ok)==QMessageBox::Abort)
			return false;
	return true;
}
void TabPage::savePdf(char * name)
{
	if (name == NULL)
	{
		_pdf->save();
		emit addHistory("Saved to latest revision");
		initRevisions();
		return;
	}
	emit addHistory(QString("PDF saved to new file") + name);
	_pdf->saveChangesToNew(name);
}
TabPage::~TabPage(void)	
{
	if (_searchThread->isRunning())
		_searchThread->terminate();
	delete _searchThread;
	//_pdf->
}

///---private--------
void TabPage::addRevision( int i )
{
	if ( i < 0)//remove last
	{
		ui.revision->removeItem(ui.revision->count()-1);
		return;//page was loaded
	}
	QVariant v(_pdf->getRevisionsCount()-1);
	//std::cout << "Adding revision "
	//std::cout << i << " " << pdf->getRevisionsCount() << std::endl;
	assert( (size_t)i < _pdf->getRevisionsCount() );
	ui.revision->addItem(QString(("Revision ") + v.toString()));
	ui.revision->setCurrentIndex(i);
}

///--------------------------------PRIVATE SLOTS----------------

void TabPage::initRevision(int revision) //snad su revizie od nuly:-/
{
	if (revision <0)
		return;
	size_t pos = _page->getPagePosition();
	_pdf->changeRevision(revision);
	if (pos > _pdf->getPageCount())
		pos = _pdf->getPageCount();
	_page = _pdf->getPage(pos);
	if (pos > _pdf->getPageCount())
		pos = _pdf->getPageCount();
	setState();
	redraw();
}

void TabPage::rotate(int angle) //rotovanie pages
{
	angle = _page->getRotation()+angle;
	if (angle < 0)
		angle += 360;
	if (angle > 360)
		angle %=360;
	_page->setRotation(angle);
	displayparams.rotate = _page->getRotation();
	emit addHistory("Page rotated\n");
	redraw();
	createList();
}
void TabPage::commitRevision()
{
	if (!CanBeSavedChanges(true))
		return;
	if (ui.revision->currentIndex() != ui.revision->count()-1)
	{
		QMessageBox::warning(this, "Cannot change","No change", QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
	//save revision to pdf
	this->_pdf->save(true);
	if (_pdf->getRevisionsCount() == (size_t)this->ui.revision->count()+1)
	{
		addRevision(this->ui.revision->count());
		emit addHistory("New revision created\n");
	}
	else
	{ 
		QMessageBox::warning(this, "Nothing to commit","File was not changed", QMessageBox::Ok, QMessageBox::Ok);
	}
}

void TabPage::exportRevision()
{
	QString name = getFile(false);
	if (name.isEmpty())
		return;
	//if exists, save it here
	FILE * f = fopen(name.toAscii().data(),"wb");
	//saving to new file
	if (!f)
	{
		QMessageBox::warning(this,"Cannot save","Unable to open handler", QMessageBox::Ok);
		return;
	}
	_pdf->clone(f);
	fclose(f);
	emit addHistory("Version exported\n");
}

QString TabPage::getFile(bool open, QFileDialog::FileMode flags)
{
	QString fileName;
	if (open)
	 fileName = QFileDialog::getOpenFileName(this, tr("Open pdf file"),_name,tr("PdfFiles (*.pdf)"));
	else
		fileName = QFileDialog::getSaveFileName(this, tr("Save pdf file"),_name,tr("PdfFiles (*.pdf)"));
	if (fileName == NULL)
		return NULL;//cancel pressed
	return fileName;
}

void TabPage::insertRange()
{
	if ( this->ui.revision->currentIndex()!= _pdf->getRevisionsCount()-1)
		return;
	//opens file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open pdf file"),".",tr("PdfFiles (*.pdf)"));
	if (fileName == NULL)
		return;//cancel pressed
	InsertPageRange range(_pdf,_pdf->getPagePosition(_page),this,fileName);
	//updateBar
	updatePageInfoBar();
}
void TabPage::addEmptyPage()
{
	//insert empty page
	boost::shared_ptr<pdfobjects::CDict> pageDict(pdfobjects::CDictFactory::getInstance());
	boost::shared_ptr<pdfobjects::CName> type(pdfobjects::CNameFactory::getInstance("Page"));
	pageDict->addProperty("Type", *type);
	boost::shared_ptr<pdfobjects::CPage> pageToAdd(new pdfobjects::CPage(pageDict));
	_pdf->insertPage(pageToAdd, _page->getPagePosition()+1);//insert after
	emit addHistory("Added empty page\n");
	updatePageInfoBar();
}
void TabPage::print()
{
	QPrinter printer(QPrinter::HighResolution);

	QPrintDialog dialog(&printer, this);
	dialog.setWindowTitle(tr("Print Document"));
	if (dialog.exec() != QDialog::Accepted)
		return;

	QPainter painter(&printer);

	SplashColor paperColor;
	paperColor[0] = paperColor[1] = paperColor[2] = 0xff;
	SplashOutputDev splash (splashModeBGR8, 4, gFalse, paperColor);
	Guchar * p = new Guchar[3];
	for (size_t pos = 1; pos <= _pdf->getPageCount(); ++pos) {

		// Use the painter to draw on the page.

		// display it = create internal splash bitmap
		_pdf->getPage(pos)->displayPage(splash, displayparams);
		splash.clearModRegion();

		QImage image(splash.getBitmap()->getWidth(), splash.getBitmap()->getHeight(),QImage::Format_RGB32);

		for ( int i =0; i< image.width(); i++)
		{
			for ( int j =0; j < image.height(); j++)
			{
				splash.getBitmap()->getPixel(i,j,p);
				image.setPixel(i,j, qRgb(p[0],p[1],p[2]));
			}
		}
		//we have the image
		QSize size(printer.pageRect().width(), printer.pageRect().height());
		QImage t = image.scaled(size);
		t.save("test resized.bmp");
		painter.drawImage(0,0,t);
		if (pos != _pdf->getPageCount())
			printer.newPage();
	}
	delete[] p;
	//sends to printer
	painter.end();
}

//void TabPage::draw() //change mode to drawing
//{
//	//	this->ui.content->_beginDraw();
//	_mode = DrawMode;
//}
//na kazdej stranke mozu byt anotacie, po kliknuti na ne vyskoci pop-up alebo sa inak spravi akcia
//page bude vediet o interaktovnyh miestach -> kvoli mouseMove
void TabPage::showAnnotation()
{
	_labelPage->clearLabels();
	std::string SupportedAnnotationNames[] = { ANNOTS(CREATE_ARRAY) };
	//akonahle sa zmeni stranka, upozornim page na to ze tam moze mat anotacie
	//dostan oblasti anotacii z pdf
	CPage::Annotations ann;
	_page->getAllAnnotations(ann);
	//v page nastav vsetky aktivne miesta - text only
	int iAnnot = -1;
	while( iAnnot< (int)ann.size()-1 )
	{
		iAnnot++;
		std::string name;
		name = utils::getNameFromDict("Subtype",ann[iAnnot]->getDictionary());
		int iter = 0;
		for ( ; !SupportedAnnotationNames[iter].empty(); iter++)
			if (SupportedAnnotationNames[iter] == name)
				break;
		if (SupportedAnnotationNames[iter].empty())
			continue;
#ifdef _DEBUG
		std::vector<std::string> names; //subtype == text
		ann[iAnnot]->getDictionary()->getAllPropertyNames(names);
#endif
		PdfProperty p = ann[iAnnot]->getDictionary()->getProperty("Subtype");
		std::string type = utils::getNameFromIProperty(p);
#ifdef _DEBUG 

#if 0
		if ( !_pdf->isLinearized() && type == "Popup")
		{
			_page->delAnnotation(ann[i]);
			return;
		}
#endif
		std::string m;
		/*for (int a = 0; a< names.size();a++)
		{
		_annots[i]->getDictionary()->getProperty(names[a])->getStringRepresentation(m);
		}*/
		ann[iAnnot]->getDictionary()->getStringRepresentation(m);
#endif // _DEBUG
		bool found = false;

		PdfProperty prop = ann[iAnnot]->getDictionary()->getProperty("Rect");
		shared_ptr<CArray>rect = pdfobjects::IProperty::getSmartCObjectPtr<CArray>(prop);
#ifdef _DEBUG
		ann[iAnnot]->getDictionary()->getStringRepresentation(m);
#endif // _DEBUG
		BBox b;
		//		if (ann[iAnnot]->getDictionary()->containsProperty("StructParent"))
		//		{
		//			int index = utils::getSimpleValueFromDict<CInt>(ann[iAnnot]->getDictionary(), "StructParent");
		//			PdfProperty myDict = utils::getReferencedObject(_pdf->getDictionary()->getProperty("StructTreeRoot"));
		//			assert(isDict(myDict));
		//			myDict =utils::getReferencedObject(myDict->getSmartCObjectPtr<CDict>(myDict)->getProperty("ParentTree"));
		//#ifdef _DEBUG
		//			std::string m;
		//			myDict->getStringRepresentation(m);
		//#endif // _DEBUG
		//			shared_ptr<CDict> parentDict = myDict->getSmartCObjectPtr<CDict>(myDict);
		//			//hladama parenta
		//			while(true)
		//			{
		//				shared_ptr<CDict> test;
		//				if (parentDict->containsProperty("Nums"))
		//					break;
		//				PdfProperty t = utils::getReferencedObject(parentDict->getProperty("Kids"));
		//				shared_ptr<CArray> c =t->getSmartCObjectPtr<CArray>(t);
		//				for ( int i =0; i < c->getPropertyCount(); i++)
		//				{
		//					test = IProperty::getSmartCObjectPtr<CDict>(utils::getReferencedObject(c->getProperty(i)));
		//					assert(test->containsProperty("Limits"));
		//					shared_ptr<CArray> r = IProperty::getSmartCObjectPtr<CArray>(test->getProperty("Limits"));
		//					if (index > utils::getSimpleValueFromArray<CInt>(r,0) && index < utils::getSimpleValueFromArray<CInt>(r,1))
		//					{
		//						parentDict = test;
		//						break;
		//					}
		//				}
		//			}
		//			shared_ptr<CArray> nums = IProperty::getSmartCObjectPtr<CArray>(parentDict->getProperty("Nums"));
		//#ifdef _DEBUG
		//			nums->getStringRepresentation(m);
		//#endif // _DEBUG
		//			int i=0;
		//			for ( i =0; i< nums->getPropertyCount(); i+=2)
		//				if (index == utils::getSimpleValueFromArray<CInt>(nums,i))
		//					break;
		//			if (i == nums->getPropertyCount())
		//			{
		//				assert(false);
		//				return;
		//			}
		//			myDict = utils::getReferencedObject(nums->getProperty(i+1)); //distaneme value
		//			assert(isDict(myDict));
		//			myDict = utils::getReferencedObject(IProperty::getSmartCObjectPtr<CDict>(myDict)->getProperty("K")); //distaneme value
		//			nums = IProperty::getSmartCObjectPtr<CArray>(myDict);
		//			myDict = IProperty::getSmartCObjectPtr<CDict>(utils::getReferencedObject(nums->getProperty(1)));
		//#ifdef _DEBUG
		//			myDict->getStringRepresentation(m);
		//#endif // _DEBUG
		//			int res = 8;
		//			//najdi 8 v 
		//		}
		//		else
		{
			double coor[4];
			float x1,x2,y1,y2;
			for ( int i =0; i<4; i++)
			{
				PdfProperty aProp = rect->getProperty(i);
				if (aProp->getType() == pInt)
					coor[i] = utils::getSimpleValueFromArray<CInt>(rect,i);
				else if (aProp->getType() == pReal)
					coor[i]=utils::getSimpleValueFromArray<CReal>(rect,i);
			}

			//dostat annotacny rectangle
			//musime rucne parametre pre annotation reprezentovat, pretoe to NEMA bbox, ale priano to sisame z dictionary
			DisplayParams dTemp = displayparams;
			dTemp.pageRect = _page->getMediabox();
			dTemp.convertPdfPosToPixmapPos(coor[0],coor[1],coor[0],coor[1]);
			dTemp.convertPdfPosToPixmapPos(coor[2],coor[3],coor[2],coor[3]);
			b = BBox(coor[0],coor[1],coor[2], coor[3]);
		}
		QRect convertedRect = getRectangle(b);//PODIVNE
		_labelPage->addPlace(convertedRect); //teraz vie o vsetkych miestach
		_annots.push_back(ann[iAnnot]);
	}
}

void TabPage::delAnnot(int i) //page to u seba upravi, aby ID zodpovedali
{
	_page->delAnnotation(_annots[i]);
	_annots[i] = _annots.back();
	 emit addHistory("Annotation deleted\n");
	_annots.pop_back();
}
QRect TabPage::getRectangle(BBox b)
{
	double x1=b.xleft,x2=b.xright,y1=b.yleft,y2=b.yright;
	//displayparams.convertPdfPosToPixmapPos( b.xleft, b.yleft, x1,y1);
	//displayparams.convertPdfPosToPixmapPos( b.xright, b.yright, x2,y2);
	//rotatePdf(displayparams,x1,y1,false);
	//rotatePdf(displayparams,x2,y2,false);
	QRect r(QPoint(min(x1,x2),min(y1,y2)),QPoint(max(x1,x2),max(y1,y2)));
	return r;
}
void TabPage::findLastFontMode()
{
	emit addHistory("Waiting for user to select font from page\n");
	_parent->setMode(ModeFindLastFont);//najde aj so Size, bude musiet emitovat aj Tm sirku a vysku
}
//void TabPage::loadFonts(FontWidget* fontWidget)
//{
//	//dostanme vsetky fontu, ktore su priamov pdf. bohuzial musime cez vsetky pages
//	CPage::FontList fontList;
//
//	CPageFonts::SystemFontList flist = CPageFonts::getSystemFonts();
//	for ( CPageFonts::SystemFontList::iterator i = flist.begin(); i != flist.end(); i++ )
//	{
//		fontWidget->addFont(*i,*i); //TODO zrusit a pridat do fontu, ktory si to moze naloadovat sam
//	}
//}
void TabPage::insertText( PdfOp op )
{
	if ( this->ui.revision->currentIndex()!= _pdf->getRevisionsCount()-1)
		return;
	Ops ops;
	ops.push_back(op);
	_page->addContentStreamToBack(ops);
	TextOperatorIterator iter(op);assert(iter.valid());
	_parent->setPreviousMode();
	emit addHistory("Text inserted\n");
	_labelPage->setMode(ModeDrawNothing);
	_labelPage->update();
	redraw();
}

//slot
void TabPage::changeSelectedText(PdfOp insertedText) //vsetko zosane na svojom mieste, akurat sa pridaju 
{
	if ( this->ui.revision->currentIndex()!= _pdf->getRevisionsCount()-1)
		return;
	assert(_selected);
	float corr = displayparams.vDpi/72;
	//float h = sTextIt->_op->getFontHeight();
	//const double * fMatrix = sTextIt->_op->getCurrentFont()->getFontBBox();
	//double y = displayparams.DEFAULT_PAGE_RY - sTextIt->_ymax/corr + fMatrix[3]*h ; //TODO toto nemusi byt vyska BBoxu
	double pos = sTextIt->getPreviousStop();
	//rotatePdf(displayparams,pos,y,true);
	//_font->setPosition(pos,y); //pretoze toto je v default user space
	//rozdelime na dva pripady - pokial je to roznake a pokial je zaciatok erozny od konca
	//if ( sTextIt==sTextItEnd ) //TODO co ak je s3 prazdne? -> Compact?:)
	{
		eraseSelectedText();
		insertText(insertedText);
		return;
	}
	//CHANGE COLOR ONLY
	//stale ame v operatoroch ulozene suradnice
	std::vector<PdfOp> operators;
	//prvy je vzdy stejny,ziadne Td
	PdfOperator::Operands operands;
	QString s1,s2,s3;
	operands.clear();
	TextData::iterator it = sTextIt;
	it->split(s1,s2,s3);
	float dist = it->getPreviousStop() - it->_origX/* - sTextItEnd->_charSpace*/;
	dist*= corr;
	float dx=sTextIt->getPreviousStop() - sTextIt->_origX;
	float dy=0;
	dx*=corr;
	assert(s3==""); //namiesto prveho TD bude 
	{
		operands.clear();
		operands.push_back(PdfProperty(CStringFactory::getInstance(s2.toAscii().data())));
		operators.push_back(createOperator("Tj",operands));
	}//spracovane prve
	float lastX = pos;
	float lastY = it->_ymin;
	while(it!=sTextItEnd) //spravi vratane poslendeho
	{
		it++;
		//td
		operands.clear();
		dist = it->_origX - lastX;
		dist /= corr;
		operands.push_back(PdfProperty(CRealFactory::getInstance(dist)));
		dist = it->_ymin - lastY;
		dist /= corr;
		operands.push_back(PdfProperty(CRealFactory::getInstance(dist)));
		operators.push_back(createOperator("Td",operands));
		//tj
		operands.clear();
		QString dummy1,dummy2, s = it->_text;
		if ( it == sTextItEnd )
			it->split(dummy1,s,dummy2);
		operands.push_back(PdfProperty(CStringFactory::getInstance(s.toAscii().data())));
		operators.push_back(createOperator("Tj",operands));
		lastX = it->_origX;
		lastY = it->_ymin;
	}
	//posledne zaverecne TD kvoli moznemu poslednemu rozbitiu polsendeho operatora
	{
		sTextItEnd->split(s1,s2,s3);
		assert(s1=="");
		operands.clear();
		dist = lastX - sTextItEnd->getPreviousStop()/*-sTextItEnd->_charSpace*/;
		dist /= corr;
		operands.push_back(PdfProperty(CRealFactory::getInstance(dist)));
		operands.push_back(PdfProperty(CRealFactory::getInstance(0)));
		operators.push_back(createOperator("Td",operands));
	}
	eraseSelectedText();
	_font->createFromMulti(operators);
	//vsetky ostatne Tj uchovame + musime zachovat ich rozostupenie - vytvorizme im nove Td na zaklade vzdialenost medzi nimi
	redraw();
	createList();
}
//float TabPage::findDistance(std::string s,TextData::iterator textIter)
//{
//	float res = 0;
//	shared_ptr<TextSimpleOperator> txt= boost::dynamic_pointer_cast<TextSimpleOperator>(textIter->_op);
//	for (int i=0; i< s.size(); i++)
//	{
//		res+=txt->getWidth(s[i], textIter->_scale );
//		res+=textIter->_charSpace;
//	}
//	return res;
//}
void TabPage::insertTextAfter(PdfOp opBehind, double td, double ymax, QString s)
{
	std::list<PdfOp> ops;
	opBehind->getContentStream()->getPdfOperators(ops);
	PdfOperator::Iterator iter(PdfOperator::getIterator(ops.front()));
	shared_ptr<pdfobjects::CompositePdfOperator> comp = findCompositeOfPdfOperator(iter,opBehind);
	_font->createBT();
	std::list<PdfOp> children;
	comp->getChildren(children);
	std::list<PdfOp>::iterator it = children.begin();
	while (*it != opBehind)
	{
		std::string n;
		(*it)->getOperatorName(n);
		if ( typeChecker.isType(OpTextName,n))
		{
			it ++;
			continue;
		}
		_font->addToBT((*it)->clone());
		it++;
	}
	_font->addParameters();
	PdfOp td1 = FontWidget::createTranslationTd(td, ymax);
	_font->addToBT(td1);
	//daj tam text
	{
		PdfOperator::Operands ops;
		ops.push_back(shared_ptr<IProperty>(new CString(s.toAscii().data())));
		PdfOp tj = createOperator("Tj", ops);
		_font->addToBT(tj);
	}
	comp->getContentStream()->insertOperator(comp, _font->createET());
}

/*
void TabPage::showTextAnnot(std::string name)
{
//TODO novy textbox
}
*/
//TODO ask if there should be deletion after what it found
void TabPage::replaceText( QString what, QString by)
{
	while ( true )
	{
		search(what,true);
		if (!_selected)
			break;
		//mame selectnute
		replaceSelectedText(by);
	}
}
void TabPage::setSelected(TextData::iterator& first, TextData::iterator& last)
{
	if (!_selected)
	{
		first = _textList.begin();
		last = _textList.end();
		return;
	}
	if (*last < *first)	
	{
		first = sTextItEnd;
		last = sTextIt;
	}
	else
	{
		first = sTextIt;
		last = sTextItEnd;
	}
}
//slot
void TabPage::stopSearch()
{
	_stop = true;
}

void TabPage::search(QString srch, int flags)
{
	assert(!_searchThread->isRunning());
	_stop = false;
	MyThread * m = (MyThread *) _searchThread;
	m->set(this,srch,flags);
	_searchEngine.setFlags(flags);
	_searchEngine.validateSearch(srch);
	m->start();
}

#include "splash/SplashGlyphBitmap.h"
#include "splash/SplashFont.h"

GlyphInfo TabPage::checkCode( QString s, std::string fontName )
{
	//get ID

	CPage::FontList contL;
	std::string t;
	_page->getFontIdsAndNames(contL);
	for ( int i =0; i< contL.size(); i++)
		if (contL[i].second == fontName)
		{
			fontName = contL[i].first;
			break;
		}

	GlyphInfo ret;
	ret.size = 0;
	bool raiseWarning = false;
	GfxFont * font = NULL;
	std::vector<shared_ptr<CContentStream> > cont;
	_page->getContentStreams(cont);
	for (int i =0; i<cont.size(); i++ )
	{
		font = cont[i]->getResources()->lookupFont(fontName.c_str());
		if (font)
			break;
	}
	assert(font);
	//TODO ak je truetype font, nepodporujeme ->return
	for (int i = 0; i< s.size(); i++)
	{
		Unicode u = s[i].unicode();
		char c = font->getCodeFromUnicode(&u,1);
		double temp[] = {1,0,0,1,0,0};
		SplashFont * fnt = splash.getFontById(font);
		if (!fnt)
			fnt = splash.getFont(font->getName(),temp);
		SplashGlyphBitmap tBitmap;
		assert(fnt);
		fnt->getGlyph(c,0,0,&tBitmap);
		if (tBitmap.w == 0)
			raiseWarning = true;
		else
		{
			CharCode code; int uLen; double dx,dy,originX,originY;
			font->getNextChar(&c, 1, &code,
				&u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
				&dx, &dy, &originX, &originY);

			ret.size += dx;
			ret.name+=c;
		}
	}
	if (raiseWarning)
	{
		int res = QMessageBox::warning(this, tr("Text truncation"),
			tr("Some characters are not supported.\n Truncated to ")+ret.name.c_str() + "\nContinue?",
			QMessageBox::Ok|QMessageBox::Discard,
			QMessageBox::Discard);
		if (res == QMessageBox::Discard)
			ret.name = "";
	}
	return ret;
}
bool TabPage::performSearch( QString srch, bool forw )
{
	QString s; //TODO do threadu
	for(int i = 0; i< _pdf->getPageCount()&&!_stop; i++)
	{
		float prev = 0;
		TextData::iterator iter;
		if (_textList.size() == 0)
			goto NextPage;
		if (!_searchEngine.setPattern(srch))
			return false; //vytvor strom, ktory bude hladat to slovo, pre kazdu stranku znova
		//vysviet prve, ktore najdes
		if (forw)
			iter = _textList.begin();
		else
		{
			iter = _textList.end();//posledne
			iter--;
		}
		iter->clear();
		if (_selected && forw)
			iter = sTextItEnd;//nic nemen v hladacom engine
		if (_selected && !forw)
			iter = sTextIt;//nic nemen v hladacom engine
		s = iter->_text;
		_searchEngine.setText(s);
		_searchEngine._begin = sTextIt->letters(sTextIt->_begin);

		prev = forw? iter->_end : iter->_begin;
		while (iter != _textList.end())
		{
			switch (_searchEngine.search())
			{
			case Tree::Next: //potrebuje dalsi token
				{
					if (forw)
						iter++;
					if (iter == _textList.end() || iter == _textList.begin())
						goto NextPage;
					if (!forw)
						iter--;
					shared_ptr<TextSimpleOperator> txt= iter->_op;
					//float dx = txt->getWidth(' ');//TODO zmenit na word
					//if (fabs(prev - iter->_begin) > iter->_w ) //from which space?
					//	_searchEngine.acceptSpace(); //space neberieme do uvahy, blbo sa s nimi pocita
					prev = forw? iter->_end : iter->_begin;
					break;
				}
			case Tree::Found:
				{
					_labelPage->unsetImg();
					//prev = iter->_end; //netreba
					double a, b;
					//ak je 
					if (forw)
						iter->setEnd(iter->position(_searchEngine._end));
					else
						iter->setBegin(iter->position(iter->_text.size() - _searchEngine._end-1));
					if (forw)
						sTextItEnd = iter;
					else
						sTextIt= iter;
					for ( int i = 0; i < _searchEngine._tokens; i++)
					{
						if (forw)
							iter--;
						else
							iter++;
						iter->clear();
					}
					if (forw)
					{
						sTextIt = iter;
						iter->setBegin(iter->position(_searchEngine._begin));
					}
					else
					{
						sTextItEnd = iter;
						iter->setEnd(iter->position(iter->_text.size() - _searchEngine._begin-1));
					}
					_selected = true; 
					emit addHistory("Found occurrence of word \"" +srch+"\" on page " + QVariant(_pdf->getPagePosition(_page)).toString());
					return true;
				}
			default:
				{
					throw "Unexpected t->search() token"; //pre developera
				}
			}
			QString s = iter->_text;
			_searchEngine.setText(s);
		}
		//next page, etreba davat do splashu
NextPage:
		int pg = _page->getPagePosition();
		//int p2 = _pdf->getPagePosition(_page);
		if (forw)
		{
			if ( pg == _pdf->getPageCount())
				_page = _pdf->getPage(1);
			else
				_page = _pdf->getNextPage(_page);
		}
		else
		{
			if ( pg == 1)
				_page = _pdf->getPage(1);
			else
				_page = _pdf->getPrevPage(_page);
		}
		emit addHistory("Trying next page " +QVariant(_pdf->getPagePosition(_page)).toString());
		redraw();
		//nastav nove _textbox, pretoze sme stejne v textovom rezime
	}
	return false;
}
PdfOperator::Iterator TabPage::findTdAssOp(PdfOperator::Iterator iter)
{
	iter.next(); //prve Tj
	while(iter.valid())
	{
		std::string name;
		iter.getCurrent()->getOperatorName(name);
		for ( int i =0; i < TextOperatorIterator::namecount; i++)
			if ( name == TextOperatorIterator::accepted_opers[i])
				return PdfOperator::Iterator(); //invalid
		if (name == "TD")
			return PdfOperator::Iterator(); //invalid
		if (name == "ET")
			return PdfOperator::Iterator(); //invalid
		if (name == "Td")
			return iter;
		iter.next();
	}
	return PdfOperator::Iterator(); //invalid
}
void TabPage::deleteSelectedText() //sucasne zarovna
{
	if (!_selected)
		return;
	PdfOp op;
	//PdfOp insertTdBeforeThis;
	//prvy replasni, ostatne vymaz, pridaj TD pred posledne
	//QString s[3];
	//sTextIt->split(s[0],s[1],s[2]);
	//PdfOperator::Operands operand;
	//operand.push_back(shared_ptr<IProperty>(new CString(s[0].toAscii().data())));;
	_selected = false;
	//PdfOp op = createOperator("Tj",operand);
	sTextIt->_op->setSubPartExclusive(sTextIt->letters(sTextIt->_begin), sTextIt->letters(sTextIt->_end));
	float distX = -sTextIt->getNextStop() + sTextItEnd->getPreviousStop(); //nesmie byt fabs!! 
	float distY =  sTextIt->_ymax - sTextItEnd->_ymax;
	if (sTextIt == sTextItEnd)
	{
		//setRawText, je to jediny operand
		assert(sTextIt == sTextItEnd);
		distX /= displayparams.vDpi/72;
		PdfOp tdop = FontWidget::createTranslationTd(distX,distY);
		sTextIt->_op->getContentStream()->insertOperator(sTextIt->_op, tdop);
		//insertBefore(op,);//insertneme ZA
		goto End;
	}

	/////////////////////////////////MANY OPERATORS///////////
	distX =  sTextIt->getPreviousStop() - sTextIt->_origX2;
	distX/= displayparams.vDpi/72;
	distY =  sTextIt->_ymin - sTextItEnd->_ymin;
	distY /= displayparams.hDpi/72;
	op = FontWidget::createTranslationTd(distX, 0);
	sTextIt->_op->getContentStream()->insertOperator(sTextIt->_op, op);

	distX =  sTextIt->_origX2 - sTextItEnd->getNextStop();
	distX /= displayparams.vDpi/72;
	distY =  sTextIt->_ymin - sTextItEnd->_ymin;
	distY /= displayparams.hDpi/72;
	op = FontWidget::createTranslationTd(distX, -distY);

	sTextIt++;
	while(sTextIt!=sTextItEnd)//delete also Td operators
	{
		sTextIt->_op->getContentStream()->deleteOperator(sTextIt->_op);
		sTextIt++;
	}
	//2Td
	op = FontWidget::createTranslationTd(distX, -distY);
	assert(sTextIt->_op->getContentStream());
	assert(sTextIt == sTextItEnd);
	/*sTextIt->split(s[0],s[1],s[2]);
	assert(s[0]=="");*/
	distX = sTextItEnd->_origX - sTextItEnd->getNextStop();
	distX /= displayparams.vDpi/72;
	{
		PdfOp op2 = FontWidget::createTranslationTd(distX,0);
		sTextIt->_op->getContentStream()->insertOperator(sTextIt->_op, op2);
		sTextIt->_op->setSubPartExclusive(sTextIt->letters(sTextIt->_begin), sTextIt->letters(sTextIt->_end));
		insertBefore(op, sTextIt->_op);
		assert(sTextIt->_op->getContentStream());
	}

End:
	_selected = false;
	createList();
	redraw();
}
void TabPage::replaceSelectedText(QString by)
{
	//TextData::iterator first, last;
	//setSelected(first, last);
	////delete the part
	//if (first != last)
	//{
	//	//ak nie su stejne, stejne to zopakuj len pre prve a zvysok zmaz
	//	QString s[3];
	//	TextData::iterator i1,i2;
	//	setSelected(i1,i2);
	//	i1->split(s[0],s[1],s[2]);
	//	i1->replaceAllText(checkCode(s[0]+s[1]+by,first->_op->getFontName()));
	//	i1++;
	//	TextData::iterator it = i1;
	//	for(; i1!=i2; i1++ )
	//	{
	//		i1->_op->getContentStream()->deleteOperator(i1->_op,true);
	//	}//ten dalsi bude mat asi spravne maticu, jelikoz je to tj-bud bude pokracovat -OK alebo vlastnu - OK
	//	createList();
	//	return;
	//}
	////zaciatok aj koniec je stejny
	//QString s[3];//iba s1 mame vymazat
	//first->split(s[0],s[1],s[2]);
	//first->replaceAllText(checkCode(s[0]+by+s[2],sTextIt->_op->getFontName()));
	////ostatne sa posunu, ak si v stejnom tj-> posunu sa s vlozenim. Ok nie su, maju maticu
}
void TabPage::eraseSelectedText()
{
	if (!_selected)
		return; //staci iba vymazat a vhodne pridat operator \Td

	float corr = 72.0f/displayparams.vDpi;
	if (sTextIt == sTextItEnd)
	{
		QString s[3];
		sTextIt->split(s[0],s[1],s[2]);
		//obkolesime vkladany operator s TD s prislusnou velkostou 
		float dist;
		if (!s[2].isEmpty()) //s2 not empty-> nejaky operator zostava potom 
		{
			PdfOp tClone = sTextIt->_op;
			tClone = tClone->clone();
			shared_ptr<TextSimpleOperator> textClone= boost::dynamic_pointer_cast<TextSimpleOperator>(tClone);
			
			//PdfOperator::Operands operands;
			//std::string e = checkCode(s[2],sTextIt->_op->getFontName());
			////s[2].toStdString();
			//if(e.empty())
			//	return;
			//operands.push_back(shared_ptr<IProperty>(CStringFactory::getInstance(e)));
			//PdfOp op = createOperator("Tj",operands); //stejny operator, stejny fot
			dist = sTextIt->getNextStop() - sTextIt->_origX;
			dist*=corr;
			PdfOp optd = FontWidget::createTranslationTd(-dist,0);
			//dist *= 72.0f/displayparams.hDpi;
			sTextIt->_op->getContentStream()->insertOperator(PdfOperator::getIterator(sTextIt->_op),optd);
			sTextIt->_op->getContentStream()->insertOperator(PdfOperator::getIterator(sTextIt->_op),textClone);
			
			textClone->setFontData(sTextIt->_op->getCurrentFont());
			textClone->setSubPartExclusive(0,sTextIt->letters(sTextIt->getNextStop())-1);

			optd = FontWidget::createTranslationTd(dist,0);
			int letters = sTextIt->letters(sTextIt->getPreviousStop());
			sTextIt->_op->setSubPartExclusive(letters,-1);//pre istotu
			sTextIt->_op->getContentStream()->insertOperator(PdfOperator::getIterator(sTextIt->_op),optd);
		}
		else
		{
			int letters = sTextIt->letters(sTextIt->getPreviousStop());
			sTextIt->_op->setSubPartExclusive(letters,-1);
		}
		emit addHistory("Text erased\n");
		_selected = false;
		return;
	}
	//	float distX1 =0;// -sTextIt->GetPreviousStop() + sTextIt->_origX2;//kolko sme zmazali
	float distX2 = sTextItEnd->getNextStop() - sTextItEnd->getPreviousStop()/*-sTextItEnd->_charSpace*/;
	distX2 *=corr;
	//najskor musim deletnut tie, ktore urcite nechceme, v dalsom kuse kodu robim replace a nema sa to rado
	TextData::iterator it = sTextIt;
	//std::string n1 = checkCode(s1,sTextIt->_op->getFontName());
	//std::string n2 = checkCode(s3,sTextIt->_op->getFontName());
	//if ((n1.empty() && !s1.isEmpty())|| (n2.empty()&& !s3.isEmpty()) )
	//	return;
	it++;
	while (true)
	{
		if(it==sTextItEnd)
			break;
		//delete operator
		it->_op->getContentStream()->deleteOperator(it->_op);
		it++;
	}

	{
		//uchovame to, co z praveho operatoru zostalo
#ifdef _DEBUG
		QString s1,s2,s3;
		sTextIt->split(s1,s2,s3); //splitneme
		assert(s3=="");
#endif // _DEBUG
		//sTextIt->replaceAllText(n1);
		int index = sTextIt->letters(sTextIt->getPreviousStop());
		sTextIt->_op->setSubPartExclusive(index,-1);
	}
	{
#ifdef _DEBUG
		QString s1,s2,s3;
		sTextItEnd->split(s1,s2,s3);
		assert(s1 == "");
#endif // _DEBUG
		PdfOp op = FontWidget::createTranslationTd(-distX2,0);
		sTextItEnd->_op->getContentStream()->insertOperator(sTextItEnd->_op,op);
		op = FontWidget::createTranslationTd(distX2,0);
		int index  = sTextItEnd->letters(sTextItEnd->getNextStop());
		//sTextItEnd->replaceAllText(n2);
		sTextItEnd->_op->setSubPartExclusive(0,index-1);
		insertBefore(op, sTextItEnd->_op);
	}
	_selected = false;
	emit addHistory("Text in multiple operators erased\n");
}
void TabPage::deleteText( QString text)
{
	replaceText(text,"");
}
void TabPage::exportText()
{
	//dialog na pocet stranok
	int beg = 1, end=2;
	if (_pdf->getPageCount() !=1)
	{
		QDialog * dialog = new QDialog(this);
		Ui::PageDialog pdialog;
		pdialog.setupUi(dialog);
		pdialog.begin->setMaximum(_pdf->getPageCount());
		pdialog.begin->setMinimum(1);
		pdialog.end->setMinimum(1);
		pdialog.end->setMaximum(_pdf->getPageCount());
		dialog->setWindowTitle("Select page range");
		dialog->exec();
		if (dialog->result() == QDialog::Rejected)
			return;
		beg = pdialog.begin->value();
		end = pdialog.end->value();
		end++;
		dialog->close();
		delete dialog;
	}
	int old = _page->getPagePosition();
	_page=_pdf->getPage(beg);
	QTextEdit * edit = new QTextEdit(this);
	//TODO nejaka inicializacia
	QString text;
	float dy = 0;
	for ( size_t i = beg; i < end; i++)
	{
		float prev =_textList.size() > 0  ? _textList.begin()->_begin : 0;
		float dy = _textList.size() > 0  ? _textList.begin()->_ymax : 0;
		for (TextData::iterator iter = _textList.begin(); iter != _textList.end(); iter++)
		{
			float xx = prev - iter->_origX;
			float yy = dy - iter->_ymax;
			float customSpace = iter->_op->getFontHeight()/4;
			float size2 = xx*xx + yy * yy;
			if (fabs(yy) > customSpace)
				text.append('\n');
			else if ( size2 > customSpace * customSpace) //TODO hack
				text.append(" ");
			std::wstring test;
			iter->_op->getFontText(test);
			text.append(iter->_text);
			prev = iter->_origX2;
			dy = iter->_ymax;
		}
		SetNextPageRotate();
		createList();
	}
	edit->setGeometry(0,0,600,700);
	edit->setText(text);
	edit->setReadOnly(true);
	edit->setGeometry(QRect(0,0,550,450));
	edit->setWindowFlags(Qt::Window);
	edit->setWindowModality(Qt::WindowModal);
	edit->setDocumentTitle("Exported text");
	//text += QString::from
	//edit->setText(text.latin());
	emit addHistory("Text exportedfrom page from " + QVariant(beg).toString() + " to " +QVariant(end).toString());
	edit->show();
	_page=_pdf->getPage(old);
	//delete edit;
}
void TabPage::SetNextPageRotate()
{
	size_t i = _pdf->getPagePosition(_page);
	if ( i == _pdf->getPageCount() )
		_page = _pdf->getPage(1);
	else
		_page = _pdf->getPage(i);
}
/*
//bolo kliknute na anotaciu, ideme ju vykonat
void TabPage::handleAnnotation(int id)
{
//mame identifikator
//_annots;
//Ukazeme widget s tymto textom
//ak je to link, tak rovno skocime
shared_ptr<CAnnotation> c = _annots[id];
//zistime, kam mame skocit
shared_ptr<IProperty> name = c->getDictionary()->getProperty("SubType");
std::string n = utils::getValueFromSimple<CName>(name);
if ( n == "Link")
{
//dostan page, na ktoru ma ist
if (c->getDictionary()->containsProperty("Dest"))
{
//ok,skaceme
shared_ptr< CArray > array; 
c->getDictionary()->getProperty("Dest")->getSmartCObjectPtr<CArray>(array);
page = pdf->getPage(utils::getSimpleValueFromArray<CInt>(array,0));
setFromSplash();
return;
}//alebo action
if (c->getDictionary()->containsProperty("A"))
{
shared_ptr<CDict> d; 
d = utils::getCDictFromDict(c->getDictionary(),"A"); //toto musi byt Launch, TODO overit
std::string nam = utils::getStringFromDict(d,"F");
//zistime lauch a jeho parametre
//TODO QDialog a launch
}
throw "Unsupported type";//TODO vymazat z anotacii alebo ich tam vobec nedavat
return;
}
//other Annotations
//tot by malo zobrazit pdfko
std::string name2 = utils::getStringFromDict(c->getDictionary(), "Contents");
showTextAnnot(name2);
return;
}
//rotate 
void TabPage::rotateObjects(int angle) //vsetky objekty wo workingOpSet
{
//selected operator will be removed and new QSTATE added
//let's look for Q
float rAngle = toRadians(angle); //su nastavene iterBegin a iterEnd, z neho vypraprarujeme working set
shared_ptr< PdfOperator > parent;
for ( int i =0; i < workingOpSet.size(); i++)
{
Ops ops;
workingOpSet[i]->getContentStream()->getPdfOperators(ops);
PdfOperator::Iterator it = 
PdfOperator::getIterator(ops.front());
parent = findCompositeOfPdfOperator(it, workingOpSet[i]);
//for graphical object
std::string opName;
parent->getOperatorName(opName);
for ( int i =0; i< opName.length(); i++)
opName[i] = tolower(opName[i]);
OpsList children;
if (strcmp(opName.c_str(),"q")==0)
{
//add cm rotation matrix
PdfOperator::Operands operands;
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(cos(rAngle))));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(sin(rAngle))));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(-sin(rAngle))));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(cos(rAngle))));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(0)));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(0)));
shared_ptr< PdfOperator > o = createOperator("cm",operands);
//addOperator
parent->getChildren(children);
parent->getContentStream()->insertOperator(PdfOperator::getIterator(children.front()),o,true);//FUJ, to snad ani nemoze fungovat..., a mozno to chce false
}
if (strcmp(opName.c_str(),"bt"))
{
PdfOperator::Operands operands;
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(cos(rAngle))));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(sin(rAngle))));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(-sin(rAngle))));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(cos(rAngle))));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(0)));
operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(0)));
shared_ptr< PdfOperator > o = createOperator("Tm",operands);
//addOperator
parent->getChildren(children);
parent->getContentStream()->insertOperator(PdfOperator::getIterator(children.front()),o,true);//FUJ, to snad ani nemoze fungovat...
}

}
}

//slot
void TabPage::riseSel()
{
//move only text operators

TextOperatorIterator it = PdfOperator::getIterator<TextOperatorIterator> (workingOpSet.front());
while (!it.isEnd())
{
//len Tj, potrebujeme pred pridat Ts, ak uz predty nejake ts nie je
// dostaneme composit
PdfOperator::Iterator bit = PdfOperator::getIterator<PdfOperator::Iterator> (workingOpSet.front());
shared_ptr< PdfOperator >  parent = findCompositeOfPdfOperator(bit,it.getCurrent());
//insert before, if full, first insert and then remove

PdfOperator::Operands intop;
intop.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(-5)));
parent->push_back( it.getCurrent()->clone(),it.getCurrent());//TODO must check if Tj is only one
parent->push_back( createOperator("Tj", intop),it.getCurrent());//TODO must check if Tj is only one

//split text acording to highlighted
}

}

*/
void TabPage::setImageOperator()
{
	_acceptedType = OpImageName;
}
void TabPage::setTextOperator()
{
	_acceptedType = OpTextName;
}
void TabPage::getSelected( int x, int y, Ops ops)
{
	//show only picture, all picture
	//get only operator
	double px, py;
	//convert
	toPdfPos(x,y, px, py);
	//find operattdisplayparamsors
	_page->getObjectsAtPosition(ops, libs::Point(px,py));

}

std::string TabPage::addFontToPage( std::string name )
{
	CPageFonts::FontList fonts;
	_page->getFontIdsAndNames(fonts);
	CPageFonts::FontList::const_iterator it=pdfobjects::findFont(fonts,name);
	if (it!=fonts.end())
		return it->first;
	std::string ret = _page->addSystemType1Font(name);
	return ret;
}

void TabPage::getDest( const char * nameToResolve, Bookmark *b )
{
#ifdef _DEBUG
	std::string m;
#endif
	PdfProperty pgl = _pdf->getDictionary()->getProperty("Names"); //MUST BE HERE - alebo dests
#ifdef _DEBUG
	pgl->getStringRepresentation(m);
#endif
	IndiRef ref = utils::getValueFromSimple<CRef>(pgl);
	pgl = _pdf->getIndirectProperty(ref);
#ifdef _DEBUG
	pgl->getStringRepresentation(m);
#endif
	shared_ptr<CDict> names = IProperty::getSmartCObjectPtr<CDict>(pgl);
	pgl = names->getProperty("Dests");
#ifdef _DEBUG
	pgl->getStringRepresentation(m);
#endif
	std::vector<IndiRef> refs;
	refs.push_back(utils::getValueFromSimple<CRef>(pgl));
	while (!refs.empty())
	{
		pgl = _pdf->getIndirectProperty(refs.back());
		names = IProperty::getSmartCObjectPtr<CDict>(pgl);
		refs.pop_back();
#ifdef _DEBUG
		int texxxxt = refs.size();
#endif // _DEBUG
		if (names->containsProperty("Kids"))
		{
			shared_ptr<CArray> arr;
			if (names->containsProperty("Limits"))
			{
				pgl = names->getProperty("Limits");
				arr = IProperty::getSmartCObjectPtr<CArray>(pgl);
				std::string minName = utils::getStringFromArray(arr,0);
				std::string maxName = utils::getStringFromArray(arr,1);
				if (minName.compare(nameToResolve) > 0 || maxName.compare(nameToResolve) < 0)
					continue;
			}
			pgl = names->getProperty("Kids");
#ifdef _DEBUG
			pgl->getStringRepresentation(m);
#endif
			arr = IProperty::getSmartCObjectPtr<CArray>(pgl);
			for (int i =arr->getPropertyCount()-1; i>=0 ; i--)
			{
				refs.push_back(utils::getRefFromArray(arr,i));
			}
		}
		else
		{
			assert(names->containsProperty("Names"));
			pgl = names->getProperty("Names");
#ifdef _DEBUG
			pgl->getStringRepresentation(m);
#endif
			//bohuzila je toto pole ktore ma X*2 hodnot key/value
			PropertyType t = pgl->getType();
			shared_ptr<CArray> arr = pgl->getSmartCObjectPtr<CArray>(pgl);
			for ( int keyId = 0; keyId < arr->getPropertyCount(); keyId+=2)
			{
				std::string name = utils::getSimpleValueFromArray<CString>(arr,keyId);
				if (strcmp(name.c_str(),nameToResolve))
					continue;
				pgl = utils::getReferencedObject(arr->getProperty(keyId+1));
				if ( pgl->getType() == pDict)
				{
					names = pgl->getSmartCObjectPtr<CDict>(pgl);
					pgl = names->getProperty("D"); 
				}
				assert(pgl->getType() == pArray);
#ifdef _DEBUG
				pgl->getStringRepresentation(m);
#endif // _DEBUG
				return getDestFromArray(pgl,b);

			}
		}
	}
	assert(refs.empty());
	assert(false);
	//TODO null would be better
}

void TabPage::getDestFromArray( PdfProperty pgl, Bookmark * ret )
{
	shared_ptr<CArray> destination = pgl->getSmartCObjectPtr<CArray>(pgl);
	pgl = utils::getReferencedObject(destination->getProperty(0));
#ifdef _DEBUG
	std::string m;
	destination->getStringRepresentation(m);
#endif // _DEBUG
	assert(isPage(pgl));
	shared_ptr<CDict> names = pgl->getSmartCObjectPtr<CDict>(pgl); //TODO boost rozoznava iva pointery, toto tu asi nebude byt
	for ( int i = 1; i<= _pdf->getPageCount(); i++)
	{
		shared_ptr<CDict> d = _pdf->getPage(i)->getDictionary(); //TODO mozy pages spravovat rovnaku dicionary?
		if (d == names)
		{
			ret->setPage(i);
			std::string type = destination->getProperty<CName>(1)->getValue();
			double t;
			double x(0),y(0);
			if (type == "XYZ")
			{
				if (destination->getProperty(2)->getType() == pInt)
					x = destination->getProperty<CInt>(2)->getValue();
				else if	(destination->getProperty(2)->getType() == pReal)
					x = destination->getProperty<CReal>(2)->getValue();
				if (destination->getProperty(3)->getType() == pInt)
					y = destination->getProperty<CInt>(3)->getValue();
				else if (destination->getProperty(3)->getType() ==pReal)
					y = destination->getProperty<CReal>(3)->getValue();
				t = 1;
				if (destination->getProperty(4)->getType() != pNull)
					t = destination->getProperty<CInt>(4)->getValue(); //TODO ho to get 0.5?
				ret->setZoom(t);
			}
			if (type == "FitH" || type == "FitBH")
			{
				if (destination->getProperty(2)->getType() == pInt)
					y = destination->getProperty<CInt>(2)->getValue();
				else
					y = destination->getProperty<CReal>(2)->getValue();
			}
			if (type == "FitV" || type == "FitBV")
			{
				if (destination->getProperty(2)->getType() == pInt)
					x = destination->getProperty<CInt>(2)->getValue();
				else
					x = destination->getProperty<CReal>(2)->getValue();
			}
			displayparams.convertPdfPosToPixmapPos(x,y,x,y);
			ret->setX(x);
			ret->setY(y);
			return;
		}
	}
	throw "Should not happen";
}

void TabPage::loadBookmark( QTreeWidgetItem * item )
{
	Bookmark * b = (Bookmark *) item;
	if (b->loaded())
		return;
	emit addHistory("Loading content of bookmark\n");
	IndiRef r = b->getIndiRef();
	PdfProperty p = _pdf->getIndirectProperty(r);
	assert(isDict(p));
	shared_ptr<CDict> dict = p->getSmartCObjectPtr<CDict>(p);
	Bookmark * n = NULL;
	while(dict->containsProperty("Next"))
	{
		n = new Bookmark(b);
		setTree(dict,n); //b ako parent
		b->addSubsection(n);
		QString nm = QString::fromStdString (utils::getStringFromDict("Title",dict));
		nm = nm.trimmed();
		n->setText(0,utils::getStringFromDict("Title",dict).c_str());
		p = utils::getReferencedObject( dict->getProperty("Next"));
		dict = p->getSmartCObjectPtr<CDict>(p);
	}
	n = new Bookmark(b);
	setTree(dict,n); //b ako parent
	b->addSubsection(n);
	n->setText(0,utils::getStringFromDict("Title",dict).c_str());
}

void TabPage::SetModePosition(PdfAnnot a)
{
	_linkAnnot = a;
	emit addHistory("Waiting for position to be set");
	_oldPage = _pdf->getPagePosition(_page);
	_parent->setMode(ModeEmitPosition);
}

void TabPage::handleLink( int level )
{
	std::string typ = _annots[level]->getDictionary()->getProperty<CName>("Subtype")->getValue();
	if (typ != "Link")
		return;
	Bookmark * b1 = new Bookmark((QTreeWidgetItem *)0);
#ifdef _DEBUG
	std::string m;
	_annots[level]->getDictionary()->getStringRepresentation(m);
#endif // _DEBUG
	setTree(_annots[level]->getDictionary(),b1);
	if (b1->getDest()<=0)
	{
		QMessageBox::warning(this, "not implemented","Link handler not found. Probably URI", QMessageBox::Ok,QMessageBox::Ok);
		return;
	}
	//Bookmark * b2 = (Bookmark*)b1->child(1);
	_page = _pdf->getPage(b1->getDest());
	double x = b1->getX();
	double y = b1->getY();
	int index=  b1->getZoom()*100/ZOOM_STEP -1;
	if (index < 0)
		index = 0;
	if(index >= ui.zoom->count())
		index = ui.zoom->count()-1;
	if (index == ui.zoom->currentIndex())
		redraw();
	else
		ui.zoom->setCurrentIndex(index);
	rotatePdf(displayparams,x,y,false);
	this->ui.scrollArea->ensureVisible(x,y);
	return;
}

bool TabPage::checkLinearization()
{
	if (!_pdf->isLinearized())
		return true;
	if ( QMessageBox::warning(this, "Pdf id linerized","Should it be delinerized?", QMessageBox::Ok|QMessageBox::Discard,QMessageBox::Ok) == QMessageBox::Ok)
		return false;
	return true;
}
#include "kernel/delinearizator.h"
void TabPage::delinearize( QString fileName )
{
	emit addHistory("Delinearization performed\n");
	shared_ptr<utils::Delinearizator> d = utils::Delinearizator::getInstance(_name.toAscii().data(), _pdf->getPdfWriter());
	d->delinearize(fileName.toAscii().data());
}

PdfOp TabPage::getValidTextOp( Ops& ops, bool & found )
{
	int id = -1; //museli sme kliknit na operator stejne
	found = false;
	for ( int i =0; i< ops.size(); i++)
	{
		std::string opName;
		ops[i]->getOperatorName(opName);
		for ( int a = 0; a < TextOperatorIterator::namecount; a++)
		{
			if (TextOperatorIterator::accepted_opers[a] == opName)
			{
#if _DEBUG
				std::string test;
				ops[i]->getStringRepresentation(test);
#endif
				found = true;
				id = i;
				//ops.back() = ops[i];
				return ops[i];
			}
		}
	}
	return PdfOp();
}

void TabPage::checkLoadedBookmarks()
{
	if (ui.tree->topLevelItemCount() == 0)
		getBookMarks();
}

void TabPage::copyTextToClipBoard()
{
	if (!_selected)
		return;
	QClipboard *clipBoard = QApplication::clipboard();
	TextData::iterator act = sTextIt;
	QString s[3];
	act->split(s[0],s[1],s[2]);
	if(sTextIt == sTextItEnd)
	{
		clipBoard->setText(s[1]);
		return;
	}
	assert(s[1].isEmpty());
	QString text=s[1];
	act++;
	while (act != sTextItEnd)
	{
		text += act->_text;
		act++;
	}
	sTextItEnd->split(s[0],s[1],s[2]);
	text+=s[1];
	clipBoard->setText(text);
	emit addHistory("Text was copied to clipboard\n");
}

void TabPage::operationDone()
{
	_labelPage->setMode(ModeDrawNothing);
	_parent->setPreviousMode();
	redraw();
}

void TabPage::initAnalyze()
{
	if (this->ui.analyzeTree->topLevelItemCount()!=0)
		return; //was initialized. TODO - inicializovat znova?
	emit addHistory("Tree for analyzation loaded");
	QStringList list;
	list << "Name" << "Type" << "Value" << "Reference";
	this->ui.analyzeTree->setHeaderLabels(list);
	//this->ui.analyzeTree->setColumnCount(4); //name type value, indirect prop
	AnalyzeItem * b = new AnalyzeItem(this->ui.analyzeTree, _pdf->getDictionary());
	this->ui.analyzeTree->addTopLevelItem(b);
	b->setText(0,"DocCatalog");
	this->ui.showAnalyzeButton->show();
}

void TabPage::loadAnalyzeItem( QTreeWidgetItem * item )
{
	AnalyzeItem * it = (AnalyzeItem *) item;
	it->load();
}

void TabPage::setFirstPage()
{
	_page = _pdf->getPage(1);
	redraw();
	updatePageInfoBar();
}

void TabPage::setLastPage()
{
	_page = _pdf->getPage(_pdf->getPageCount());
	redraw();
	updatePageInfoBar();
}

void TabPage::setPageFromInfo()
{
	QString text = this->ui.pageInfo->text();
	bool ok;
	QStringList list  = text.split('/');
	int pos = list[0].toInt(&ok);
	if (!ok || pos <=0 || pos > _pdf->getPageCount())
	{
		QMessageBox::warning(this, "Not a page","No such page exists",QMessageBox::Ok,QMessageBox::Ok);
		updatePageInfoBar();
		return;
	}
	setPage(pos);
}

void TabPage::addZoom()
{
	this->ui.plusZoom->setEnabled(false);
	int index = ui.zoom->currentIndex();
	index++;
	if (index > ui.zoom->count())
		return;
	ui.zoom->setCurrentIndex(index);
	this->ui.plusZoom->setEnabled(true);
}

void TabPage::minusZoom()
{
	this->ui.minusZoom->setEnabled(false);
	int index = ui.zoom->currentIndex();
	index--;
	if (index == -1)
		return;
	ui.zoom->setCurrentIndex(index);
	this->ui.minusZoom->setEnabled(true);
}

pdfobjects::DisplayParams TabPage::getDisplayParams()
{
	DisplayParams d = displayparams;
	d.pageRect = _page->getMediabox();
	return d;
}

void TabPage::reportSearchResult()
{
	if (_selected)
	{
		highlight();
		return;// nasiel, vsetko je OK
	}
	if (_stop)
		QMessageBox::warning(this,"Stopped","Searching was stopped");
	else
		QMessageBox::warning(this,"Not found","String was not found");

}

bool TabPage::changed()
{
	return _pdf->isChanged();
}

QString TabPage::getName()
{
	return _name;
}
