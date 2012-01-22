#include "openpdf.h"
#include "TabPage.h"
#include <QFileDialog>
#include <QTabWidget>
#include "insertpagerange.h"
#include "kernel/exceptions.h"
#include <QMessageBox>
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

void OpenPdf::extractImage()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->extractImage();
}
void OpenPdf::about()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->about();
}
void OpenPdf::setModeDeleteHighLight()
{
	setMode(ModeDeleteHighLight);
}
void OpenPdf::stopSearch()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->stopSearch();
}
void OpenPdf::setModeOperator()
{
	setMode(ModeOperatorSelect);
}
void OpenPdf::initAnalyze()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->initAnalyze();
}
void OpenPdf::pdfChanged()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	this->setTabText( currentIndex(), QString("*") + this->tabText(currentIndex() )); //TODO zmenit na coot?
}
void OpenPdf::setModeChangeAnnotation()
{
	setMode(ModeChangeAnnotation);
}
void OpenPdf::setModeDeleteAnnotation()
{
	setMode(ModeDeleteAnnotation);
}
QColor OpenPdf::getHColor()
{
	_highlightColor = emit GetActualHColorSignal();
	return _highlightColor;
}
OpenPdf::OpenPdf(QWidget * centralWidget) :QTabWidget(centralWidget),_mode(ModeDoNothing),_previous(ModeDoNothing), _color(255,0,0,50), _highlightColor(0,255,0),_author("Unknown")
{
	char name[256];
#ifdef _WIN32
	DWORD size = sizeof(name);
	GetUserNameA(name,&size);

#else
	FILE * f; 
	f = fopen("whoami", "r");
	fgets(name, sizeof(name), f);
	fclose(f);
#endif
	_author = name;
#if _DEBUG
	open("./zadani.pdf");
#endif
	setModeDefault(); //default mode
}
void OpenPdf::search(QString s, int v)
{
	if (s.isEmpty())
	{
		QMessageBox::warning(this, "Empty string","No string to search", QMessageBox::Ok,QMessageBox::Ok); 
		return;
	}
	if (this->count()==0)
		return;
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->search(s,v);
}
void OpenPdf::changeSelectedImage()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->raiseChangeSelectedImage();
}
void OpenPdf::eraseSelectedText()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->eraseSelectedText();
	page->redraw();
	page->createList();
}
void OpenPdf::highlightSelected()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->highLightAnnSelected();
}

void OpenPdf::changeSelectedText()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->raiseChangeSelectedText();
}
void OpenPdf::deleteSelectedImage()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->deleteSelectedImage();
}
void OpenPdf::deleteSelectedText()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->deleteSelectedText();
}
void OpenPdf::setModeInsertLinkAnotation()
{
	setMode(ModeInsertLinkAnnotation);
}
void OpenPdf::setModeInsertAnotation()
{
	setMode(ModeInsertAnnotation);
}
void OpenPdf::setModeImagePart()
{
	setMode(ModeImagePart);
}
void OpenPdf::setModeInsertImage()
{
	setMode(ModeInsertImage);
}
void OpenPdf::setModeDefault()
{
	setModeView();
}
void OpenPdf::setModeSelectImage()
{
	setMode(ModeSelectImage);
}
void OpenPdf::setModeInsertText()
{
	setMode(ModeInsertText);
}
void OpenPdf::setModeSelectText()
{
	setMode(ModeSelectText);
}
OpenPdf::~OpenPdf(void){}

void OpenPdf::getText()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->exportText();
}
void OpenPdf::derotate()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->rotate(-90);
}
void OpenPdf::rotate()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->rotate(90);
}
void OpenPdf::checkClose()
{
	for ( int i = 0; i< count(); i++)
	{
		TabPage * page = (TabPage *)this->widget(i); //how to get exact tab?
		if (!page->changed())
			continue;
		if (QMessageBox::warning(this, "File not saved!",QString("File ")+page->getName()+ "was not changed. Save?", QMessageBox::Ok|QMessageBox::Discard, QMessageBox::Ok) == QMessageBox::Ok)
			page->save();
	}
}
void OpenPdf::setModeSetting()
{
	setMode(ModeSettings);
}
void OpenPdf::closeAndRemoveTab(int i)
{
	TabPage * page = (TabPage *)this->widget(i); //how to get exact tab?
	this->removeTab(i);
	delete page;
}
void OpenPdf::saveEncoded()
{
	try
	{
		TabPage * page = (TabPage *)this->widget(this->currentIndex());
		page->saveEncoded();
	}
	catch (...)
	{
		QMessageBox::warning(this, "Unable to save",QString("Unable to save to decoded file"), QMessageBox::Ok, QMessageBox::Ok);
	}
}
void OpenPdf::saveAs()
{
	try
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),".", tr("PDF files (*.pdf)"));
		if (fileName == "")
			return;
		TabPage * page = (TabPage *)this->widget(this->currentIndex());
		page->savePdf(fileName.toAscii().data());
	}
	catch (...)
	{
		QMessageBox::warning(this, "Error occured",QString("Unable to save file"), QMessageBox::Ok, QMessageBox::Ok);
	}
}

#include "kernel/delinearizator.h"

void OpenPdf::open(QString name)
{
	TabPage * page = NULL;
#ifndef _DEBUG
	try
#endif
	{
		page = new TabPage(this, name);
		QIcon icon(":/images/enabled.png");
#ifdef WIN32
		int index = name.lastIndexOf('/');
#else
		int index = name.lastIndexOf('\\');
#endif // WIN32
		this->addTab(page,icon,name.mid(index));
		emit OpenSuccess(name);
		setCurrentIndex(count() -1);
		//pri kazdon otvoreni sa spytaj, ci je to potreba delinerizovat(co noveho suboru), inak sa nebude dat savovat
		if (!page->CanBeSavedChanges(false))
		{
			QIcon i2(":/images/disabled.png");
			this->setTabIcon(this->count()-1,i2);
		}
		if (page->checkLinearization())
			return;
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save pdf file"),".",tr("PdfFiles (*.pdf)"));
		if (fileName == "")
			return;
		page->delinearize(fileName);
		
		//close the old file & open delinearized version
		this->closeAndRemoveTab(this->count()-1);

		page = new TabPage(this,fileName);
		this->addTab(page,fileName);
		assert(page->checkLinearization());
		setCurrentIndex(count() -1);
		setMode(ModeDoNothing);
	}
#ifndef _DEBUG
	catch (MalformedFormatExeption e)
	{
		QMessageBox::warning(this, "Unable to load",QString("Pdf is corrupted"), QMessageBox::Ok, QMessageBox::Ok);
	}
	catch (PdfOpenException e)
	{
		QMessageBox::warning(this, "Pdf library unable to perform action",QString("Reason") + QString(e.what()), QMessageBox::Ok, QMessageBox::Ok);

	}
	catch (PdfException e)
	{
		QMessageBox::warning(this, "Pdf library unable to perform action",QString("Reason : ") + QString(e.what()), QMessageBox::Ok, QMessageBox::Ok);

	//	return;
	}
	catch (PermissionException)
	{
		QMessageBox::warning(this, "Encrypted document",QString("Unable to open file ( it is ecnrypted )") , QMessageBox::Ok, QMessageBox::Ok);
	}
	catch (std::exception e)
	{
		QMessageBox::warning(this, "Unexpected exception",QString("Reason : ") + QString(e.what()), QMessageBox::Ok, QMessageBox::Ok);
	}
#endif // _DEBUG

}
//void OpenPdf::resizeEvent(QResizeEvent *event)
//{
//	if (event->oldSize().width()<0)
//		return;
//	for (int i =0; i < this->count(); i++)
//	{
//		TabPage * t = (TabPage *)this->widget(i);
//		t->rezoom(event);
//	}
//}
void OpenPdf::openAnotherPdf()
{
	QFileDialog d(this);
	d.setFilter("All PDF files (*.pdf)");
	if (!d.exec())
		return;
	QStringList fileNames = d.selectedFiles();
	for ( int i =0; i < fileNames.size(); i++)
		open(fileNames[i]);

}
void OpenPdf::save()
{
	TabPage * t = (TabPage *)this->widget(this->currentIndex());
	if (!t->CanBeSavedChanges())
		return;
	t->savePdf(NULL);
	//this->setTabText(currentIndex(), this->tabText(currentIndex()).mid(1));
}

void OpenPdf::pageUp()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->pageUp();
}

void OpenPdf::pageDown()
{
	/*timer.setSingleShot(false);
	timer.*/
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->pageDown();
}
void OpenPdf::redraw()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->redraw();
}
void OpenPdf::insertRange()
{
	//calls insert range specific to active tab
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->insertRange();
}
void OpenPdf::insertEmpty()
{
	//calls insert range specific to active tab
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->addEmptyPage();
}
void OpenPdf::print()
{
	throw "Not implemented";
	//calls insert range specific to active tab
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->print();
}
QColor OpenPdf::getColor()
{
	QColor c = GetActualColorSignal();
	c.setAlpha(100);
	return c;
}
void OpenPdf::deletePage()
{
	TabPage * page = (TabPage *)this->widget(currentIndex());
	page->deletePage();
}
static const HelptextIcon helper[] = { MODES(EARRAY) };

bool PermanentMode(Mode mode)
{
	return mode == ModeChangeAnnotation ||
		mode == ModeDeleteAnnotation ||
		mode == ModeDoNothing ||
		mode == ModeImagePart ||
		mode == ModeImagePartCopied ||
		mode == ModeImageSelected ||
		mode == ModeInsertAnnotation ||
		mode == ModeInsertLinkAnnotation ||
		mode == ModeInsertText ||
		mode == ModeSelectText ||
		mode == ModeSelectImage ||
		mode == ModeInsertImage ||
		mode == ModeOperatorSelect ;
}
void OpenPdf::setMode( Mode mode )
{
	_previous = _mode;
	if (PermanentMode(mode))
		_previous = mode;
	_mode =  mode;
	emit ModeChangedSignal(helper[mode]);
}

void OpenPdf::setPreviousMode()
{
	setMode(_previous);
}

void OpenPdf::setColor( QColor color )
{
	_color = color;
	_color.setAlpha(150);
}

void OpenPdf::setHColor( QColor color)
{
	_highlightColor =color;
}

void OpenPdf::setHighlighCommentText()
{
	setMode(ModeHighlighComment);
}

std::string OpenPdf::Author() const
{
	return _author;
}

void OpenPdf::setModeView()
{
	setMode(ModeDoNothing);
}
