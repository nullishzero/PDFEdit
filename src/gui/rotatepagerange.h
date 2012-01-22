/** \file rotatepagerange.h handles mass rotation */
#ifndef ROTATEPAGERANGE_H
#define ROTATEPAGERANGE_H

#include <QWidget>
#include "ui_rotatepagerange.h"

/** \brief class for rotating page range */
class rotatePageRange : public QWidget
{
	Q_OBJECT

public:
	/// constructor 
	rotatePageRange(QWidget *parent = 0);
	/// destructor
	~rotatePageRange();

private:
	/// graphic interface 
	Ui::rotatePageRangeClass ui;
};

#endif // ROTATEPAGERANGE_H
