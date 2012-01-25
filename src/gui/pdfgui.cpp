#include <iostream>
#include <QImage>
#include <QColor>
#include <QKeyEvent>
#include <QMenu>
#include <QIcon>
#include <QShortcut>
#include "pdfgui.h"
#include "typedefs.h"
#include <QWidget>

pdfGui::pdfGui(QWidget *parent, Qt::WFlags flags) 
: QMainWindow(parent, flags),/*_textFrame(this), _imageFrame(this), _annotationFrame(this), _debugFrame(this),*/ init()
{
	init.fontDir = ".";
	int argc = 0;
	char ** argv;
	if ( 0 != pdfedit_core_dev_init(&argc, &argv, &init))
		throw "Unable to init PdfEdit library";

	GlobalParams::initGlobalParams(NULL)->setEnableT1lib("no");
	GlobalParams::initGlobalParams(NULL)->setEnableFreeType("yes");
	GlobalParams::initGlobalParams(NULL)->setErrQuiet(gFalse);
	GlobalParams::initGlobalParams(NULL)->setAntialias("yes");
	//GlobalParams::initGlobalParams(NULL)->setupBaseFonts("D:\\Work\\winPdfEdit\\winPdfEdit\\pdfedit-0.4.5\\projects\\output"); //? Where are my fonts?

	ui.setupUi(this);
	ui.textFrame->hide();
	ui.annotationFrame->hide();
	ui.imageFrame->hide();
	ui.debugFrame->hide();

	ui.settingFrame->hide();
	aboutDialogUI.setupUi(&aboutDialog);

	_search = new Search(this);//neptraia k tomuto mimiokienku
	_searchShortCut = new QShortcut(QKeySequence(tr("Ctrl+F", "Find texts")),this);

	connect(this->ui.openedPdfs, SIGNAL(GetActualHColorSignal()), this->ui.hcolor, SLOT(getColor()));
	connect(this->ui.openedPdfs, SIGNAL(GetActualColorSignal()), this->ui.color, SLOT(getColor()));

	connect(_search, SIGNAL(stopSignal()), this->ui.openedPdfs, SLOT(stopSearch()));
	connect(_searchShortCut, SIGNAL(activated()), _search, SLOT(show())); //TODO showYourself
	connect(_searchShortCut, SIGNAL(activated()), _search, SLOT(raise()));
	//connect(_searchShortCut, SIGNAL(activated()), _search, SLOT(activateWindow()));

//////////////////////////////////MENU////////////////////////////////////////

	connect(this->ui.actionDocument,SIGNAL(triggered()), this->ui.openedPdfs, SLOT(about()));
	
	connect( this->ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect( this->ui.actionFullScreen, SIGNAL(triggered()), this, SLOT(showFullScreened()) );
	connect( this->ui.actionOpen, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(openAnotherPdf()));

	connect( this->ui.actionAbout, SIGNAL(triggered()), &aboutDialog, SLOT(open()));
	connect( this->ui.actionPage_down, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(pageDown()));
	connect( this->ui.actionPage_up, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(pageUp()));
	connect( this->ui.actionRotate_left, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(rotate()));
	connect( this->ui.actionRotate_right, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(derotate()));
	connect( this->ui.actionDelete_Page, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(deletePage()));
	connect( this->ui.actionAnalyze, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(initAnalyze()));
	connect( this->ui.actionAdd_page, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(insertEmpty()));
	connect( this->ui.actionExport_text, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(getText()));
	connect( this->ui.actionSave, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(save()));
	connect( this->ui.actionSaveCopy, SIGNAL(triggered()), this->ui.openedPdfs, SLOT(saveAs()));

//////////////////////////////////////////////////////////////////////////

	connect( this->ui.extractButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(extractImage()));
	connect( this->ui.settingsButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setModeSetting()));
	connect( this->ui.debugButton, SIGNAL(toggled(bool)), ui.debugFrame, SLOT(setVisible(bool)));
	connect( this->ui.searchButton, SIGNAL(pressed()), _search, SLOT(show()));
	connect( this->ui.openButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(openAnotherPdf()));
	connect( this->ui.saveButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(save()));
//	connect( this->ui.openButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(openAnotherPdf()));
//	connect( this->ui.saveButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(save()));
//	connect( this->ui.saveAsButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(saveAs()));
	connect( ui.saveEncodedButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(saveEncoded()));
	connect( this->ui.viewButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setModeView()));
//	connect( this->ui.exportButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(getText()));
	connect( this->ui.insertImageButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setModeInsertImage()));
	connect( ui.analyzeButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(initAnalyze()));
//	connect( this->ui.lastOpenedButton, SIGNAL(pressed()), this, SLOT(lastOpenedPdfs()));
	connect( ui.opSelect, SIGNAL(pressed()), this->ui.openedPdfs,SLOT(setModeOperator()));
	//connect( this->ui.rotateButton, SIGNAL(pressed()), this->ui.openedPdfs,SLOT(rotate()));
	//connect( this->ui.derotateButton, SIGNAL(pressed()), this->ui.openedPdfs,SLOT(derotate()));
	connect( this->ui.insertTextButton,SIGNAL(pressed()),this->ui.openedPdfs, SLOT(setModeInsertText()));
	connect( this->ui.deleteButton,SIGNAL(pressed()),this->ui.openedPdfs,SLOT(deleteSelectedText()));
	connect( this->ui.changeButton,SIGNAL(pressed()),this->ui.openedPdfs,SLOT(changeSelectedText()));
	connect( this->ui.eraseButton,SIGNAL(pressed()),this->ui.openedPdfs,SLOT(eraseSelectedText()));
	connect( this->ui.highlightButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(highlightSelected()));
	connect( this->ui.textHighlightButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(highlightSelected()));
	connect( this->ui.selectTextButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setModeSelectText()));
	
	connect( _search, SIGNAL(search(QString,int)),this->ui.openedPdfs, SLOT(search(QString, int)));
	connect( this->ui.selectImageButton,SIGNAL(pressed()),this->ui.openedPdfs, SLOT(setModeSelectImage()));
	connect( this->ui.deleteImageButton,SIGNAL(pressed()),this->ui.openedPdfs, SLOT(deleteSelectedImage()));
	connect( this->ui.changeImageButton,SIGNAL(pressed()),this->ui.openedPdfs, SLOT(changeSelectedImage()));
	connect( this->ui.imagePartButton,SIGNAL(pressed()),this->ui.openedPdfs, SLOT(setModeImagePart()));
	
	connect( this->ui.insertAnotation, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setModeInsertAnotation()));
	connect( this->ui.linkAnnotation, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setModeInsertLinkAnotation()));
	connect( this->ui.changeAnnotationButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setModeChangeAnnotation()) );
	connect( this->ui.delAnnotation,SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setModeDeleteAnnotation()));
	connect( this->ui.openedPdfs, SIGNAL(ModeChangedSignal(HelptextIcon)), this,SLOT(handleModeChange(HelptextIcon)));
	connect( this->ui.highlightButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setHighlighCommentText()));

	connect( ui.refreshButton, SIGNAL(pressed()), ui.openedPdfs, SLOT(redraw()));
	connect(ui.deleteHighlightButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(setModeDeleteHighLight()));
	connect(ui.exportTextButton, SIGNAL(pressed()), this->ui.openedPdfs, SLOT(getText()));
//	connect( this->ui.pageUp, SIGNAL(pressed()), ui.openedPdfs, SLOT(pageUp()));
//	connect( this->ui.pageDown, SIGNAL(pressed()), ui.openedPdfs, SLOT(pageDown()));
//	connect( this->ui.insertEmpty, SIGNAL(pressed()), ui.openedPdfs, SLOT(insertEmpty()));
//	connect( this->ui.deletePage, SIGNAL(pressed()), ui.openedPdfs, SLOT(deletePage()));

	connect( this->ui.hcolor, SIGNAL(ValueChangedSignal(QColor)), ui.openedPdfs, SLOT(setHColor(QColor)));
	connect( this->ui.color, SIGNAL(ValueChangedSignal(QColor)), ui.openedPdfs, SLOT(setColor(QColor)));
	connect( this->ui.openedPdfs, SIGNAL(OpenSuccess(QString)), this, SLOT(appendToLast(QString)));
	////////////////////////////////////////VISIBILITY connect//////////////////////////////////
	connect(this->ui.textButton, SIGNAL(toggled(bool)), ui.textFrame, SLOT(setVisible(bool)));
	connect(this->ui.imageButton, SIGNAL(toggled(bool)), ui.imageFrame, SLOT(setVisible(bool)));
	connect(this->ui.annotationButton, SIGNAL(toggled(bool)), ui.annotationFrame, SLOT(setVisible(bool)));
	connect( this->ui.openedPdfs, SIGNAL(currentChanged(int)), this, SLOT(disableMainPanel(int)));

	connect(this->ui.textButton, SIGNAL(pressed()), this->ui.selectTextButton, SLOT(click()));
	connect(this->ui.imageButton, SIGNAL(pressed()), this->ui.selectImageButton, SLOT(click()));
	connect(this->ui.annotationButton, SIGNAL(pressed()), this->ui.insertAnotation, SLOT(click()));

	disableMainPanel(0);
	//////////////////////////////////////////////////////////////////////////
	this->ui.openedPdfs->setMode(ModeDoNothing);
	//load settings
	ui.color->setColor(QColor(255,0,0,50));
	ui.hcolor->setColor(QColor(0,255,0));
	_lastOpenedButtonMenu = this->ui.menuOpen_recent;

	FILE * f = fopen("config","r");
	if (!f)
		return;
	char buffer[256];
	int i=0;
	while (!feof(f))
	{
		//dostan riadok
		char c = getc(f);
		if ( c != '\n')
		{
			buffer[i] = c;
			i++;
			continue;
		}

		buffer[i] ='\0';
		char * s = strchr(buffer,'=');
		*s='\0';
		if (!strcmp(buffer,"hcolor"))
		{
			s++;//budu to vzdy len 3 hodnoty
			QVariant str(s);
			ui.hcolor->setColor((QRgb)str.toUInt());
		}
		if(!strcmp(buffer,"color"))
		{
			s++;//budu to vzdy len 3 hodnoty
			QVariant str(s);
			ui.color->setColor((QRgb)str.toUInt());
		}
		if(!strcmp(buffer,"file"))
		{
			s++;
			_lastOpenedButtonMenu->addAction(s,this,SLOT(openLastFile()));
		}
		if(!strcmp(buffer,"dash"))
		{
			s++;
			QVariant v(s);
			_search->setFlags(v.toInt());
		}
		i=0;
	}
	//--------------------
	fclose(f);
}
void pdfGui::showFullScreened()
{
	if (isFullScreen())
		showNormal();
	else
		showFullScreen();
}
void pdfGui::closeEvent( QCloseEvent *event )
{
	_search->close();
	this->ui.openedPdfs->checkClose();
	/*_debugFrame.close();
	_textFrame.close();
	_imageFrame.close();
	_annotationFrame.close();*/

	FILE * f = fopen("config","w");
	if (!f)
		return;
	//napis hcolor a color + 3 cislice wo forma te XX bez medzery za tym
	char buffer[256];
	sprintf(buffer,"color=%u\n",ui.openedPdfs->getColor().rgb());
	fwrite(buffer,sizeof(char),strlen(buffer),f);
	sprintf(buffer,"hcolor=%u\n",ui.hcolor->getColor().rgb());
	fwrite(buffer,sizeof(char),strlen(buffer),f);
	QList<QAction *> actions = _lastOpenedButtonMenu->actions();
	for (int i =0; i< actions.size(); i++)
	{
		if(actions[i]->text().isEmpty())
			continue; //hidden "nothing"?
		sprintf(buffer,"file=%s\n",actions[i]->text().toAscii().data());
		fwrite(buffer,sizeof(char),strlen(buffer),f);
	}
	sprintf(buffer,"dash=%d\n",_search->getFlags());
	fwrite(buffer,sizeof(char),strlen(buffer),f);
	fclose(f);
}
pdfGui::~pdfGui()
{
	delete _search;
	pdfedit_core_dev_destroy();
}

void pdfGui::lastOpenedPdfs()//pressed
{
	if (_lastOpenedButtonMenu->actions().size()==0)
		return;
	_lastOpenedButtonMenu->exec(QCursor::pos());
}

void pdfGui::openLastFile()
{
	QAction * action = (QAction *)sender();
	this->ui.openedPdfs->open(action->text());
}

void pdfGui::appendToLast( QString s )
{
// check if this is opened
	QList<QAction *> actions = _lastOpenedButtonMenu->actions();
	for ( int i =0; i < actions.size(); i++)
		if (actions[i]->text() == s)
			return;
	_lastOpenedButtonMenu->addAction(s,this,SLOT(openLastFile()));
#ifdef _DEBUG
	QString name = _lastOpenedButtonMenu->actions().back()->text();
	assert(name == s);
#endif // _DEBUG
}

void pdfGui::handleModeChange( HelptextIcon str )
{
	this->ui.Help->setText(str.helpText);
	this->ui.actualButton->setIcon(QIcon(QString(":images/")+str.icon));
	this->ui.actualButton->setText(str.description);
}

void pdfGui::disableMainPanel( int )
{
	bool enable = this->ui.openedPdfs->count()>0;
	this->ui.fileFrame->setEnabled(enable);
	this->ui.actionSaveCopy->setEnabled(enable);
	this->ui.actionSave->setEnabled(enable);
	this->ui.actionRotate_right->setEnabled(enable);
	this->ui.extractButton->setEnabled(enable);
	this->ui.actionPage_up->setEnabled(enable);
	this->ui.actionPage_down->setEnabled(enable);
	this->ui.actionAdd_page->setEnabled(enable);
	this->ui.actionAnalyze->setEnabled(enable);
	this->ui.actionDocument->setEnabled(enable);
	this->ui.actionRotate_left->setEnabled(enable);
	this->ui.actionDelete_Page->setEnabled(enable);
	this->ui.actionExport_text->setEnabled(enable);
}
