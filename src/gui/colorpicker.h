/** \file colorpicker.h Defines class for picking colors */
#ifndef ___COLORPICK__
#define ___COLORPICK__

#include <QWidget>
#include <QPainter>
#include <QColor>
#include "ui_colorPicker.h"
#include <vector>
#include <QColor>

/** \brief main class for color picking */
/** This class handles color picking. In contains following signals and slots: \n
 \n - setColor
 \n- setFromDialog
 */
class ColorPicker : public QWidget
{
	Q_OBJECT
		
private:
	Ui::colorPick ui;	 //tri spinboxy + 1 na vykreslovanie farby
public:
	/// constructor
	ColorPicker(QWidget * parent);
	/// selected colot
	QColor _color;
	/// red value of color
	int getR(); 
	/// green value of color
	int getG(); 
	/// blue value of color
	int getB(); 
signals:
	/// when color is changes, signal is emitted to notice all objects 
	void ValueChangedSignal(QColor);
	public slots:
		/** \brief sets color */
		/** This method is called when it is needed to set predefined color. When no color is set, default is  black*/
		void setColor(QColor color);
		/** \brief Gets set color */
		QColor getColor();
		/** \brief Sets dialog according to one picked in dialog */
		/** This method is called when user vlicked on the button */
		void setFromDialog();
};
#endif
