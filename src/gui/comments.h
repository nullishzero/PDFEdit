/** \file comments.h containing class handling annotations */
#ifndef __COMMENTS___
#define __COMMENTS___

#include <QWidget>
#include <QRect>
#include <QColor>
#include <vector>
#include "ui_comments.h"
//PDF
#include "typedefs.h"
#include "kernel/cannotation.h"
#include "kernel/cdict.h"
#include "kernel/carray.h"

/** \brief main class handling Annotation insert */
/** this class is responsible only for filling correct annotation fields. Appearance stream must be createn in the page with the document */
class Comments : public  QWidget
{
	Q_OBJECT

	/// annotation dictionary
	typedef boost::shared_ptr<pdfobjects::CDict> ADictionary;
	/// points that determine active place of the annotation 
	boost::shared_ptr<pdfobjects::CArray> points;
	/// user interface
	Ui::Comments ui;
	/// destination array. One of the properties of the pdf
	boost::shared_ptr<pdfobjects::CArray> arr;
	/// annotation to ne inserted or change
	PdfAnnot _an;
	/// bounding rectangle for annotation
	libs::Rectangle _rect;
	typedef boost::shared_ptr<pdfobjects::utils::IAnnotInitializator> CAInit;
	struct InitName
	{
		std::string name;
		CAInit init;
		InitName(std::string n,CAInit i):name(n),init(i){};
	};
	/// initializtor for all types
	std::vector<InitName> _inits;
	/// type of annotation to be created
	int _index;
	/// flag if annotation should be changed or created
	bool _change;
	/// default name of the autor
	std::string _name;
public:
	/** \brief constructor */
	Comments(std::string name);

signals:
	/** \brief signal for inserting pdf annotation */
	void textAnnotation (PdfAnnot);
	/** \brief signal for inserting markup ( highlight annoatation ) */
	void annotationTextMarkup(PdfAnnot);
	
	//void annotation(PdfAnnot);
	/** \brief Signal raised when inserting link annotation*/
	/** this signal is raised only when link annotation is inserted. annotation to be inserted is also emmited after this signal */ 
	void WaitForPosition(PdfAnnot);
protected:
	/** \brief sets the title to Add annotation */
	/** this will perform default behaviour. It can be changed later by setting change */
	void showEvent ( QShowEvent * event );

public slots:
	/** \brief Sets index of the annotation */
	/** \param i is type of the annotation that should be inserted. According to this type, proper windows will be selected */
	void setIndex(int i);

	/** \brief Sets active rectangle of the annotation */
	/** since this class has no other means how to set this, it must be set efore window appears. Otherwise the createannotation wouldn't be valid */
	void setRectangle(libs::Rectangle rectangle);
	
	/** \brief Sets active rectangle of the annotation */
	/** since this class has no other means how to set this, it must be set efore window appears. Otherwise the createannotation wouldn't be valid */
	void setRectangle(float pdfx, float pdfy, int pdfwidth, int pdfheight);

	/** \brief sets annotation active points */
	/** these points must be multiplication of 8 according to the PDF specification. it is used only when highlighting to determine where is the active place */ 
	void setPoints(std::vector<float> flts);

	/// sets detination for link annotation 
	void setDestination(pdfobjects::IndiRef ref);
	/** \brief checks if the shown windows are correct */
	/** every time annotation type changes, this method ensures that correct windows will be visible */
	void onChange(int index);

	/** \brief What should happen after apply */
	/** at the end of this method, annotation will be created and signal with it will be emitted. All mandatory properties will be set during this metod */
	void apply();

	/** \brief asks this method for creating annotation with pre-defined color for highlighting */
	/** this is basically the same as "apply" with highlight annotation set. It creates annotaion and emmits the signal*/ 
	void insertMarkup();

	/** \brief fills all the necessary value fom link annotation*/
	/** this is here as convertor mthos between bookmark and annotation */
	void addLink( PdfAnnot an, pdfobjects::IndiRef ref, float x, float y );

	/** \brief fills all necessary value from annotation */
	void fromDict( boost::shared_ptr<pdfobjects::CDict> annDict );

	/** \brief loads annotation from page */
	void loadAnnotation( PdfAnnot _annots );

	/** \brief gets annoation highlight color */
	QColor getHColor();

	/** \brief sets annotation highlight color */
	void setHColor( QColor color );
};

#endif
