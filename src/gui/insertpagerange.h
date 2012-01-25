/** \file insertpagerange.h gui for inserting another page range from different pdf */
#ifndef INSERTPAGERANGE_H
#define INSERTPAGERANGE_H

#define NO_CMAP
#include "xpdf/GlobalParams.h"
#undef NO_CMAP
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <splash/Splash.h>
#include <splash/SplashBitmap.h>	
#include <xpdf/SplashOutputDev.h>

#include <QDialog>
#include <QString>
#include "ui_insertpagerange.h"

/** \brief this class is not used */
class InsertPageRange : public QDialog
{
	Q_OBJECT

public:
	/// contructor
	InsertPageRange(boost::shared_ptr<pdfobjects::CPdf> pdf, size_t pos = 0, QWidget *parent = 0, QString name = NULL);
	/// destructor
	~InsertPageRange();

	/// pdf the pdf ithat should be changes
	boost::shared_ptr<pdfobjects::CPdf> pdf;
	/// pdf the we are getting the pages from
	boost::shared_ptr<pdfobjects::CPdf> reference;
	/// position of the page where pages whoue be inserted
	size_t _pos;

private:
	/// GUI fot this class
	Ui::InsertPageRangeClass ui;

public:
	/// id of the forst page to be inserted
	int getBegin()const;
	/// id of the last page to be inserted
	int getEnd()const;

public slots:
	/// created preview of the first page
	void setPreviewToPage(int i);
	/// performs the action
	void insertPageRange();
};

#endif // INSERTPAGERANGE_H
