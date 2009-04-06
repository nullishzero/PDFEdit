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
 SelectTool - Toolbutton with drop-down combobox with selectable items
 @author Martin Petricek
*/

#include "selecttool.h"
#include "settings.h"
#include <utils/debug.h>
#include <qevent.h>
#include <qtooltip.h>

namespace gui {

/**
 Default constructor of SelectTool
 @param parent Toolbar containing this control
 @param name Name of this widget (passed to QWidget constructor)
 @param cName name of text in this widget
*/
SelectTool::SelectTool(const QString &cName,QWidget *parent/*=0*/,const char *name/*=NULL*/) : QWidget (parent,name) {
 textName=cName;
 text="";
 ed=new QComboBox(false,this,"SelectTool");
 ed->setDuplicatesEnabled(false);
 QObject::connect(ed,SIGNAL(activated(const QString &)),this,SLOT(textEntered(const QString &)));
 QObject::connect(this,SIGNAL(helpText(const QString&)),parent,SLOT(receiveHelpText(const QString&)));
}

/** default destructor */
SelectTool::~SelectTool() {
}

/**
 Return size hint of this control
 @return size hint
*/
QSize SelectTool::sizeHint() const {
 return ed->sizeHint();
}

/**
 Set tooltip to be show for this tool
 @param t text to be used as tooltip
*/
void SelectTool::setTooltip(const QString &t) {
 tip=t;
 QToolTip::remove(this);
 QToolTip::add(this,t);
}

/**
 Event handler for mouse cursor entering tool.
 Sends its tooltip as help text.
 @param e Event
*/
void SelectTool::enterEvent(QEvent *e) {
 emit helpText(tip);
 QWidget::enterEvent(e);
}

/**
 Event handler for mouse cursor leaving tool.
 Sends empty helptext, thus disabling it
 @param e Event
*/
void SelectTool::leaveEvent(QEvent *e) {
 emit helpText(QString::null);
 QWidget::leaveEvent(e);
}

/**
 Set list of predefined values
 @param predefs List of predefined values for quick selecting
*/
void SelectTool::setPredefs(const QStringList &predefs) {
 //Get old text
 QString oldText=ed->currentText();
 //Update choices
 ed->clear();
 ed->insertStringList(predefs);
 text=predefs[0];
 choices=predefs;
 //Try to select back original text if possible
 setText(oldText);
}

/**
 Set list of predefined values
 @param predefs List of predefined values for quick selecting
 @param separator separator of values in list, by default comma
*/
void SelectTool::setPredefs(const QString &predefs,const QString &separator/*=","*/) {
 QStringList plist=QStringList::split(separator,predefs,true);
 setPredefs(plist);
}

/**
 Called on resizing of property editing control
 Will simply set the same fixed size to inner editbox
 @param e resize event
*/
void SelectTool::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/**
 Called whenever text in the control changes
 @param str New text
*/
void SelectTool::textEntered(const QString &str) {
 text=str;
 emit clicked(textName);
}

/**
 Return text inside this control
*/
QString SelectTool::getText() const {
 return text;
}

/**
 Return name of the text edit box
*/
QString SelectTool::getName() const {
 return textName;
}

/**
 Set text inside this control
 @param newText new text to set
*/
void SelectTool::setText(const QString &newText) {
 for (int i=0;i<(int)(choices.count());i++) {
  if (choices[i]==newText) {
   text=newText;
   ed->setCurrentItem(i);
   return;
  }
 }
}

} // namespace gui
