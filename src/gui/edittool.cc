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
 EditTool - Toolbutton allowing editing of arbitrary text
 @author Martin Petricek
*/

#include "edittool.h"
#include "settings.h"
#include <utils/debug.h>
#include <qevent.h>

namespace gui {

/**
 Default constructor of EditTool
 @param parent Toolbar containing this control
 @param name Name of this widget (passed to QWidget constructor)
 @param cName name of color in this widget
*/
EditTool::EditTool(const QString &cName,QWidget *parent/*=0*/,const char *name/*=NULL*/) : QWidget (parent,name) {
 textName=cName;
 text="";
 ed=new QLineEdit(this,"edittool");
 QObject::connect(ed,SIGNAL(returnPressed()),this,SLOT(textEntered()));
 QObject::connect(ed,SIGNAL(lostFocus()),this,SLOT(textEntered()));
}

/** default destructor */
EditTool::~EditTool() {
}

/**
 return size hint of this control
 @return size hint
*/
QSize EditTool::sizeHint() const {
 return ed->sizeHint();
}

/**
 Called on resizing of property editing control
 Will simply set the same fixed size to inner editbox
 @param e resize event
*/
void EditTool::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/**
 Called on pressing enter or loosing focus
*/
void EditTool::textEntered() {
 text=ed->text();
 emit clicked(textName);
}

/**
 Return text inside this control
*/
QString EditTool::getText() const {
 return text;
}

/**
 Return name of the text edit box
*/
QString EditTool::getName() const {
 return textName;
}

/**
 Set text inside this control
 @param newText new text to set
*/
void EditTool::setText(const QString &newText) {
 text=newText;
 ed->setText(text);
}

} // namespace gui
