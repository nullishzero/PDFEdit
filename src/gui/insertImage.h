/** \file insertImage.h  defines basic classes for operating eith images */
#ifndef ___IMAGE_MODULE__
#define ___IMAGE_MODULE__
#include <QWidget>
#include "ui_insertImage.h"
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cinlineimage.h>
#include <boost/shared_ptr.hpp>
#include <kernel/pdfoperators.h>
#include <xpdf/xpdf/GfxState.h>
#include "typedefs.h"
#include "kernel/displayparams.h"

/** \brief class handling images */
/** this class handles creation image and changing it. */
class InsertImage : public QWidget
{
	Q_OBJECT

		/** inverted matrix founs after request */
	PdfOp _invertCm;
	Ui::Image ui;
	typedef boost::shared_ptr<pdfobjects::CInlineImage> PdfInlineImage;
	boost::shared_ptr<pdfobjects::InlineImageCompositePdfOperator> biOp;
	
	/** \brief creates inline image from file */
	/* supported file are png and BMP, jpeg is somehow weird */
	void createInlineImage();
	/** \brief initialize insertImage */
	/** c;ears all necessary fields */
	void init();
protected:
	/** \brief emits closeEvent */
	/** this method is needed for correct repainting all helpful squares etc.. Thi means fail, because after insert is everytnih repainted */
	virtual void closeEvent ( QCloseEvent * event );
public:
	/** \brief constructor */
	InsertImage(QWidget * parent);

	/** \brief sets the image not from file but from inlinr image */
	/** this means that we want to change the BI operator, so everything will be set accordingly */
	void setImage(PdfOp image);//TODO x,y

	/** \brief founds out inverted matrix for correctly changing */
	void getInvertMatrix(PdfOp op, double * act, double * res);
public slots:
	/** \brief triggers creating inline image or changing it */
	/** this is reaction t=on the apply button */
	void apply();
	//void rotationCm(int angle);

	/** \brief sets image path*/
	/** this path will be used when creating inline image. It must be existing */
	void setImagePath();

	/** \brief sets position of the inserted inline image*/
	/** this is here for creating correct BI inline image. According to this values, cm position is created*/;
	void setPosition(float f1, float f2);

	/** \brief sets size of the image */
	/** this size is in PDF unit according to current MediaBox */
	void setSize(float w,float h);
	/** \brief after actovating this slot, value of angle currently set will be shown */
	void showAngleToolTip( int value );
signals:
	/** \brief emits this signal in case everything was checked and considered worthy of applying */
	void insertImage(PdfOp op); 
	/** \brief emits signal with the chaged BI operator */
	void changeImage(PdfOp q);

	/** \brief emits this signal when this widget is closed */
	/** the parent widget should catch this signal and clear all mess after steps that were taken as a preparation for inserting ( changing ) BI image */
	void ImageClosedSignal();

	/** \brief gets the actual displayparameters that page uses */
	/** this is used when creating invert matrix. To create real invert matrix, it is needed to know allsteps that state took ( all cm operators ) for that we need correct PageRect*/
	pdfobjects::DisplayParams NeedDisplayParamsSignal();
};
#endif

