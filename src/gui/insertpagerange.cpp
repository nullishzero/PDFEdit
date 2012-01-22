#include "insertpagerange.h"
#include <QImage>

InsertPageRange::InsertPageRange(boost::shared_ptr<pdfobjects::CPdf> ref, size_t pos, QWidget *parent, QString name)
	: QDialog(parent), reference(ref), _pos(pos)
{
	ui.setupUi(this);
	//opens my pdf and shows it
	pdf = boost::shared_ptr<pdfobjects::CPdf> ( 
		pdfobjects::CPdf::getInstance (name.toAscii().data(), pdfobjects::CPdf::ReadWrite));
	this->ui.beginBox->setMinimum(1);
	this->ui.endbox->setMinimum(1);
	this->ui.beginBox->setMaximum(pdf->getPageCount());
	this->ui.endbox->setMaximum(pdf->getPageCount());
	this->ui.endbox->setValue(pdf->getPageCount());
	connect(this->ui.beginBox,SIGNAL(valueChaged(int)),this,SLOT(setPreviewPageTo(int)));

}
int InsertPageRange::getBegin()const { return this->ui.beginBox->value(); }
int InsertPageRange::getEnd()const { return this->ui.endbox->value(); }
void InsertPageRange::insertPageRange()
{
	//one less than the other
	if ( this->ui.beginBox->value() > this->ui.endbox->value())
		return;
	for ( int i = ui.beginBox->value(); i<ui.endbox->value(); i++ )
		reference->insertPage(pdf->getPage(i),_pos++);
}
void InsertPageRange::setPreviewToPage(int i) //like set from spash
{
	SplashColor paperColor;
	paperColor[0] = paperColor[1] = paperColor[2] = 0xff;
	SplashOutputDev splash (splashModeBGR8, 4, gFalse, paperColor);

	// display it = create internal splash bitmap
	boost::shared_ptr<pdfobjects::CPage> page = pdf->getPage(i);
	//displayParams as im label
	pdfobjects::DisplayParams displayparams;
	page->displayPage (splash, displayparams);
	splash.clearModRegion();

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

	this->ui.preview->setPixmap(QPixmap::fromImage(image));
	//image.save("mytest.bmp","BMP");
	this->ui.preview->adjustSize();
}

InsertPageRange::~InsertPageRange()
{

}
