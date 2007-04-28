/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __COLORTOOL_H__
#define __COLORTOOL_H__

#include <qwidget.h>
#include <qcolor.h>
#include <qstring.h>

class QResizeEvent;
class QPixmap;

namespace gui {

class ToolButton;

/**
 Toolbutton allowing to change color stored in the button.
 Scripts can read the color when needed
 Can be placed in toolbar in place of ordinary button
 \brief Toolbar widget for changing current color
*/
class ColorTool : public QWidget {
Q_OBJECT
public:
 ColorTool(const QString &cName,const QString &niceName,QWidget *parent=0,const char *name=NULL);
 ~ColorTool();
 QSize sizeHint() const;
 QString getName() const;
 QColor getColor() const;
 void setColor(const QColor &src);
 static QString niceName(const QString &id);
signals:
 /**
  Signal emitted when user changes the color
  @param name Name of the color tool
 */
 void clicked(const QString &name);
protected:
 void resizeEvent (QResizeEvent *e);
 void updateColor();
protected slots:
 void colorClicked();
protected:
 /** Button showing the color */
 ToolButton *pb;
 /** Pixmap showing the color  */
 QPixmap *pm;
 /** Color selected in the color tool */
 QColor color;
 /** Name of color in this widget */
 QString colorName;
};

} // namespace gui

#endif
