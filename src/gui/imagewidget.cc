/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
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
