/** @file
 ColorTool - Toolbutton allowing to change current color
 @author Martin Petricek
*/

#include "colortool.h"
#include "settings.h"
#include "toolbutton.h"
#include <qcolordialog.h>
#include <qpixmap.h>
#include <utils/debug.h>

namespace gui {

/**
 Default constructor of ColorTool
 @param parent Toolbar containing this control
 @param name Name of this widget (passed to QWidget constructor)
 @param cName name of color in this widget
*/
ColorTool::ColorTool(const QString &cName,QWidget *parent/*=0*/,const char *name/*=NULL*/) : QWidget (parent,name) {
 colorName=cName;
 color=QColor(0,0,0);
 pm=new QPixmap(20,20);
 QString toolTip=tr("Choose color")+": "+cName;
 //TODO: localize some standard cName's
 pb=new ToolButton(pm,toolTip,-1,this);
 updateColor();
 QObject::connect(pb,SIGNAL(clicked()),this,SLOT(colorClicked()));
 //parent MUST be toolbar
 //TODO: similar help texts also in zoom, page and revision tool
 QObject::connect(pb,SIGNAL(helpText(const QString&)),parent,SLOT(receiveHelpText(const QString&)));
}

/** default destructor */
ColorTool::~ColorTool() {
 delete pm;
}

/**
 return size hint of this control
 @return size hint
*/
QSize ColorTool::sizeHint() const {
 return pb->sizeHint();
}

/**
 Called on resizing of property editing control
 Will simply set the same fixed size to inner Zoom list
 @param e resize event
*/
void ColorTool::resizeEvent (QResizeEvent *e) {
 pb->setFixedSize(e->size());
}

/**
 Called on clicking the color button
 @param newZoom
*/
void ColorTool::colorClicked() {
 QColor ret=QColorDialog::getColor(color,this,"std_color_dialog");
 if (!ret.isValid()) return;
 color=ret; 
 updateColor();
}

/**
 Return color inside this control
*/
QColor ColorTool::getColor() const {
 return color;
}

/**
 Return name of the color
*/
QString ColorTool::getName() const {
 return colorName;
}

/**
 Set color inside this control
 @param src new color
*/
void ColorTool::setColor(const QColor &src) {
 color=src;
 updateColor();
}

/**
 Called to update control after selecting new color
 */
void ColorTool::updateColor() {
 pm->fill(color);
 pb->setIconSet(*pm);
}

} // namespace gui
