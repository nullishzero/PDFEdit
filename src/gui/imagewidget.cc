/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 ImageWidget - simple widget showing a QPixmap
 @author Martin Petricek
*/

#include "imagewidget.h"
#include <qevent.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <assert.h>

namespace gui {

/**
 Constructor of ImageWidget
 @param p pixmap to show in the widget
 @param bgColor background color used to paint areas outside of image
 @param parent parent widget
*/
ImageWidget::ImageWidget(const QPixmap *p,QColor bgColor,QWidget *parent) : QWidget(parent) {
 img=p;
 assert(img);
 if (img){
  QSize imageSize=img->size();
  setMinimumSize(imageSize);
 }
 setPaletteBackgroundColor(bgColor);
// setBackgroundMode(FixedColor);
}

/** destructor */
ImageWidget::~ImageWidget() {
}

/**
 event handler called when image is to be repainted
*/
void ImageWidget::paintEvent(QPaintEvent *e) {
 QPainter p(this);
 if (img) {
  p.drawPixmap(0,0,*img);
 }
}

} //namespace gui
