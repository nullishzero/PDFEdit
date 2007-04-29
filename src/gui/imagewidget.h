/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __IMAGEWIDGET_H__
#define __IMAGEWIDGET_H__

#include <qwidget.h>

class QPaintEvent;
class QPixmap;

namespace gui {

/**
 Simple widget, that will show image in it
 Will not allow to resize itself to be smaller than the image
*/

class ImageWidget: public QWidget {
Q_OBJECT 
public:
 ImageWidget(const QPixmap *p,QColor bgColor,QWidget *parent);
 virtual ~ImageWidget();
protected:
 virtual void paintEvent(QPaintEvent *e);
private:
 /** Image that is shown in the widget */
 const QPixmap *img;
};

} //namespace gui

#endif
