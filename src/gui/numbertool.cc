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
 NumberTool - Toolbutton allowing editing of arbitrary number, with helper drop-down box
 @author Martin Petricek
*/

#include "numbertool.h"
#include "settings.h"
#include <qvalidator.h>
#include <utils/debug.h>
#include <qevent.h>
#include <qtooltip.h>

namespace gui {

/**
 Default constructor of NumberTool
 @param parent Toolbar containing this control
 @param name Name of this widget (passed to QWidget constructor)
 @param cName name of color in this widget
*/
NumberTool::NumberTool(const QString &cName,QWidget *parent/*=0*/,const char *name/*=NULL*/) : QWidget (parent,name) {
 textName=cName;
 num=0;
 ed=new QComboBox(true,this,"NumberTool");
 val=new QDoubleValidator(this);
 val->setBottom(0);
 ed->setValidator(val);
 QObject::connect(ed,SIGNAL(activated(const QString &)),this,SLOT(textEntered(const QString &)));
 QObject::connect(this,SIGNAL(helpText(const QString&)),parent,SLOT(receiveHelpText(const QString&)));
}

/** default destructor */
NumberTool::~NumberTool() {
}

/**
 return size hint of this control
 @return size hint
*/
QSize NumberTool::sizeHint() const {
 return ed->sizeHint();
}

/**
 Set list of predefined values
 @param predefs List of predefined values for quick selecting
 @param separator separator of values in list, by default comma
*/
void NumberTool::setPredefs(const QString &predefs,const QString &separator/*=","*/) {
 QStringList plist=QStringList::split(separator,predefs,true);
 setPredefs(plist);
}

/**
 Set list of predefined values
 @param predefs List of predefined values for quick selecting
*/
void NumberTool::setPredefs(const QStringList &predefs) {
 ed->clear();
 ed->insertStringList(predefs);
}

/**
 Called on resizing of property editing control
 Will simply set the same fixed size to inner editbox
 @param e resize event
*/
void NumberTool::resizeEvent(QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/**
 Called whenever text in the control changes
 @param str New text
*/
void NumberTool::textEntered(const QString &str) {
 num=str.toDouble();
 emit clicked(textName);
}

/**
 Return text inside this control
*/
double NumberTool::getNum() const {
 return num;
}

/**
 Return name of the text edit box
*/
QString NumberTool::getName() const {
 return textName;
}

/**
 Set tooltip to be show for this tool
 @param t text to be used as tooltip
*/
void NumberTool::setTooltip(const QString &t) {
 tip=t;
 QToolTip::remove(this);
 QToolTip::add(this,t);
}

/**
 Event handler for mouse cursor entering tool.
 Sends its tooltip as help text.
 @param e Event
*/
void NumberTool::enterEvent(QEvent *e) {
 emit helpText(tip);
 QWidget::enterEvent(e);
}

/**
 Event handler for mouse cursor leaving tool.
 Sends empty helptext, thus disabling it
 @param e Event
*/
void NumberTool::leaveEvent(QEvent *e) {
 emit helpText(QString::null);
 QWidget::leaveEvent(e);
}

/**
 Set number inside this control
 @param newNum new text to set
*/
void NumberTool::setNum(double newNum) {
 num=newNum;
 ed->setCurrentText(QString::number(newNum));
}

} // namespace gui
