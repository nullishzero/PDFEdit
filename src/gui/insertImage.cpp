#include "insertImage.h"
#include "globalfunctions.h"
#include <QFileDialog>
#include <QMessageBox>
#include <boost/shared_ptr.hpp>
#include <kernel/pdfoperators.h>
#include <kernel/factories.h>
#include <kernel/cinlineimage.h>
#include <kernel/displayparams.h>
#include <QToolTip>

using namespace boost;
using namespace pdfobjects;

void InsertImage::closeEvent ( QCloseEvent * event )
{
	emit ImageClosedSignal();
}

InsertImage::InsertImage( QWidget * parent) : QWidget(parent)
{
	ui.setupUi(this);
	connect(this->ui.rotation, SIGNAL(sliderMoved(int)), this, SLOT(showAngleToolTip(int)));
#ifndef _DEBUG
	ui.positionX->hide();
	ui.positionY->hide();
#endif
}

void InsertImage::setSize(float w,float h)
{
	this->ui.imageChooseFrame->show();
	ui.sizeX->setValue(fabs(w));
	ui.sizeY->setValue(fabs(h));
	ui.rotation->setValue(0);
}
void InsertImage::setPosition(float pdfX,float pdfY)
{
	init();
	ui.positionX->setValue(pdfX);
	ui.positionY->setValue(pdfY);
}
void InsertImage::setImagePath()
{
	//open dialog and get file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "/home", tr("Images (*.png *.jpg *.bmp)"));
	if (!fileName.size() == 0)
		this->ui.lineEdit->setText(fileName);
}
//v tomto okamihu mame nazov obrazka
//void InsertImage::rotationCm(int angle)
//{
//	float radian = toRadians(angle);
//	float scale = 1; //this->ui.scale->value();
//	double cs = scale*cos(static_cast<double>(radian));
//	QVariant cs1(cs);
//	double sn = scale*sin(static_cast<double>(radian));
//	QVariant sn1(sn);
//	QVariant sn2(sn*-1);
//	ui.cm->setItem(0,0,new QTableWidgetItem(cs1.toString()));
//	ui.cm->setItem(0,1,new QTableWidgetItem(sn1.toString()));
//	ui.cm->setItem(1,0,new QTableWidgetItem(sn2.toString()));
//	ui.cm->setItem(1,1,new QTableWidgetItem(cs1.toString()));
//}
void InsertImage::createInlineImage()
{	
	if (ui.lineEdit->text().isEmpty())
	{
		close();
		return;
	}
	QImage image;
	if (!image.load(tr(ui.lineEdit->text().toAscii().data())))
	{
		QMessageBox::warning(this,tr("Unable to load file"),tr("Qt4 cannot load the image"),QMessageBox::Ok);
		return;
	}
	image = image.scaled(ui.sizeX->value(),ui.sizeY->value(),Qt::IgnoreAspectRatio); //proporcie
	CDict image_dict;
	image_dict.addProperty ("W", CInt (image.width())); 
	image_dict.addProperty ("H", CInt (image.height()));
	image_dict.addProperty ("CS", CName ("RGB"));
	image_dict.addProperty ("BPC", CInt (8)); //bits per component, kontanta, ine pdf nevie:)

	int depth = image.depth(); //8 - 8 bitov per pixel
	std::vector<char> imageData; //32 bitove sa musia pretypovat na QRGB
	
	for(int h = 0; h< image.height(); h++)
	{
		for ( int w = 0; w < image.width(); w++)
		{
			QRgb color = image.pixel(w,h);
			int r = qRed(color);
			int g = qGreen(color);
			int b = qBlue(color);
			imageData.push_back(r);
			imageData.push_back(g);
			imageData.push_back(b);
		}
	}
	PdfInlineImage im = boost::shared_ptr<CInlineImage>(new CInlineImage (image_dict,imageData));
	biOp = boost::shared_ptr<pdfobjects::InlineImageCompositePdfOperator>(new InlineImageCompositePdfOperator (im));
}
void InsertImage::init()
{	
	ui.positionX->setMaximum(DisplayParams::DEFAULT_PAGE_RX);//o kolko a poze posunut doprava
	ui.positionX->setMinimum(0);//o kolko a poze posunut doprava
	ui.positionX->setValue(0);

	ui.positionY->setMaximum(DisplayParams::DEFAULT_PAGE_RY);//o kolko a poze posunut doprava
	ui.positionY->setMinimum(0);//o kolko a poze posunut doprava
	ui.positionY->setValue(0);
}
void InsertImage::apply()
{
	bool create = false;
	if (biOp == NULL)
	{
		createInlineImage();
		create = true;
	}
	if (biOp == NULL)
		return;
	//add to buffer everything that is in image
	shared_ptr<UnknownCompositePdfOperator> q(new UnknownCompositePdfOperator("q", "Q"));
	
	int pixW = ui.sizeX->value();
	int pixH = ui.sizeY->value();
	float scaleX = (float)ui.scaleX->value()/(100.0f);
	float scaleY = (float)ui.scaleY->value()/(100.0f); //POCET KOMPONENT
	//mame maticy translacie & scale ( mozeme to dat dokopy))
	PdfOperator::Operands posOperands;
	if (create)
	{//translation rotation scale
		q->push_back(createOperatorTranslation(ui.positionX->value(),ui.positionY->value()-pixH ),getLastOperator(q));
		q->push_back(createOperatorRotation(toRadians(ui.rotation->value() )),getLastOperator(q));
		q->push_back(createOperatorScale(pixW*scaleX,pixH*scaleY ),getLastOperator(q));
	}
	else
	{
		q->push_back(_invertCm,getLastOperator(q));
		q->push_back(createOperatorTranslation(0,0),getLastOperator(q));
		q->push_back(createOperatorRotation(toRadians(ui.rotation->value())),getLastOperator(q));
		q->push_back(createOperatorScale(pixW*scaleX,pixH*scaleY ),getLastOperator(q));
	}
	//vyhod alpha channel	
	q->push_back(biOp,getLastOperator(q));
	PdfOperator::Operands o;
	q->push_back(createOperator("Q", o), getLastOperator(q));
	if (create)
		emit(insertImage(q));
	else
		emit(changeImage(q));
	biOp = boost::shared_ptr<InlineImageCompositePdfOperator>();
	this->close();
}
/** \brief pdf matrix convertor */
struct Cm
{
	/// matrix
	double matrix[6];
	/// constructor 
	Cm (double a=1.0f, double b=0.0f, double c=0.0f, double d=1.0f, double e=0.0f, double f=0.0f)
	{
		matrix[0] = a;matrix[1] = b;matrix[2] = c;
		matrix[3] = d;matrix[4] = e;matrix[5] = f;
	}
};
void InsertImage::getInvertMatrix( PdfOp ii, double * act, double * res )
{
	DisplayParams params = emit NeedDisplayParamsSignal();
	PDFRectangle pdfRect ( params.pageRect.xleft, params.pageRect.yleft, params.pageRect.xright, params.pageRect.yright );
	GfxState state(params.hDpi, params.vDpi, &pdfRect, params.rotate, params.upsideDown );
PdfOperator::Iterator iter = PdfOperator::getIterator(ii);
	iter = iter.next();
	bool addCm = false;
	std::vector<Cm> cms;
	int ballance =0; //kolko Q sme nasli ( neamju am byt )
	while (true)
	{
		iter = iter.prev();
		if (!iter.valid())
			break;
		std::string name;
		iter.getCurrent()->getOperatorName(name);
		if (name=="Q") //koniec nasho hladania
		{//zmaz poslednych 6
			ballance++;
			continue;
		}
		if (name == "q" && ballance>0)
		{
			ballance--;//uzatvara nejake Q
			continue;
		}
		if (name== "cm" && !ballance)
		{
			Cm cm;
			PdfOperator::Operands opers;
			iter.getCurrent()->getParameters(opers);
			assert(opers.size()==6);
			shared_ptr<CArray> rect;
			for ( int i = 0; i<6;i++)
			{
				cm.matrix[i] = utils::getDoubleFromIProperty(opers[i]);
			}
			cms.push_back(cm);
		}
	}
	while (cms.size())
	{
		Cm cm = cms.back();
		state.concatCTM(cm.matrix[0],cm.matrix[1],cm.matrix[2],cm.matrix[3],cm.matrix[4],cm.matrix[5]);
		cms.pop_back();
	}
	const double * d = state.getCTM();
	memcpy(act,d,sizeof(double)*6);
	double s = -d[3]*d[0] + d[1]*d[2];
	Cm c(-d[3]/s,d[1]/s,-d[2]/s,d[0]/s,0,0);//ziadne posuny
#if _DEBUG
	state.concatCTM(c.matrix[0],c.matrix[1],c.matrix[2],c.matrix[3],c.matrix[4],c.matrix[5]);
	d = state.getCTM();
	/*assert(fabs(d[0] - 1)<1e-4);
	assert(fabs(d[1])<1e-4);
	assert(fabs(d[2])<1e-4);
	assert(fabs(d[3]-1)<1e-4);*/
#endif
	/*c.matrix[0]=1;
	c.matrix[0]=0;
	c.matrix[0]=0;
	c.matrix[3]=1;*/
	memcpy(res,c.matrix,sizeof(double)*6);
}
void InsertImage::setImage(PdfOp ii)
{
	this->ui.imageChooseFrame->hide();
	biOp = boost::dynamic_pointer_cast<InlineImageCompositePdfOperator>(ii->clone());
	ui.sizeX->setValue(biOp->getWidth());
	ui.sizeY->setValue(biOp->getHeight());
	//najdeme inverznu maticu
	Cm c;
	double actual[6];
	getInvertMatrix(ii,actual, c.matrix);
	PdfOperator::Operands operands;
	for ( int i =0; i<6; i++)
		operands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(c.matrix[i])));
	_invertCm = createOperator("cm",operands);
	BBox b = ii->getBBox();
	//ui.positionX->setMaximum(DisplayParams::DEFAULT_PAGE_RX*_scale-min(b.xleft,b.xleft));//o kolko a poze posunut doprava
	//ui.positionX->setMinimum(-min(b.xleft,b.xleft));//o kolko a poze posunut doprava
	ui.positionX->setValue(0);

	//ui.positionY->setMaximum(DisplayParams::DEFAULT_PAGE_RY*_scale-min(b.yleft,b.yleft));//o kolko a poze posunut doprava
	//ui.positionY->setMinimum(-min(b.yleft,b.yleft));//o kolko a poze posunut doprava
	ui.positionY->setValue(0);
}

void InsertImage::showAngleToolTip( int value )
{
	QVariant v(value);
	QString str(QString("Angle:") + v.toString());
	QToolTip::showText( QCursor::pos() , str, this );
	this->ui.rotation->setToolTip(str);
	this->ui.angleText->setText(v.toString());
}
