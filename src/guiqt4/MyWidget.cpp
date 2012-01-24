#include "MyWidget.h"

#include <QApplication>
#include <QKeyEvent>


MyWidget::MyWidget( QWidget * parent) : QWidget(parent) {}

void MyWidget::keyPressEvent(QKeyEvent *event)
{
	throw "not implemented yet";
}