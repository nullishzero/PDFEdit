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
 ColorTool - Toolbutton allowing to change current color
 @author Martin Petricek
*/

#include "colortool.h"
#include "settings.h"
#include "toolbutton.h"
#include <qcolordialog.h>
#include <qpixmap.h>
#include <utils/debug.h>
#include <qevent.h>

namespace gui {

/**
 Default constructor of ColorTool
 @param parent Toolbar containing this control
 @param name Name of this widget (passed to QWidget constructor)
 @param cName identifier of color in this widget
 @param niceName displayed name of color in this widget
*/
ColorTool::ColorTool(const QString &cName,const QString &niceName,QWidget *parent/*=0*/,const char *name/*=NULL*/) : QWidget (parent,name) {
 colorName=cName;
 //Reasonable defaults: yellow background color, black foreground
 if (cName=="bg") color=QColor(255,255,0);
 else color=QColor(0,0,0);
 pm=new QPixmap(20,20);
 QString toolTip=tr("Choose color")+": "+niceName;
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
 For given ID return it's displayable name
 @param id Identifier (name) of color
 @return string to display
*/
QString ColorTool::niceName(const QString &id){
 if (id=="fg") return tr("Foreground");
 if (id=="bg") return tr("Background");
 return id;
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
 Will simply set the same fixed size to inner button
 @param e resize event
*/
void ColorTool::resizeEvent (QResizeEvent *e) {
 pb->setFixedSize(e->size());
}

/**
 Called on clicking the color button
*/
void ColorTool::colorClicked() {
 QColor ret=QColorDialog::getColor(color,this,"std_color_dialog");
 if (!ret.isValid()) return;
 color=ret;
 updateColor();
 emit clicked(colorName);
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
