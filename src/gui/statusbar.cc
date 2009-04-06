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
 StatusBar - class representing Status bar
 @author Martin Petricek
*/
#include "statusbar.h"
#include <qlabel.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qcolor.h>

namespace gui {

using namespace std;

/**
 Constructor of StatusBar
 @param parent Parent window of this dialog
 @param name Name of this window (used only for debugging)
*/
StatusBar::StatusBar(QWidget *parent/*=0*/,const char *name/*=0*/) : QStatusBar(parent,name) {
 //Add informational label to right
 info=new QLabel(" ",this,"status_right_label");
 info->setMargin(2);
 addWidget(info,1,true);
 //Add message label to left
 msgLabel=new QLabel(" ",this,"status_left_label");
 msgLabel->setMargin(2);
 addWidget(msgLabel,4,false);
 //Timer
 tm=new QTimer();
 connect(tm,SIGNAL(timeout()),this,SLOT(timeOut()));
}

 //TODO: add progressbar

/**
 Set text shown in informational widget
 (permanently, until replaced by other text)
 @param theMessage new text
*/
void StatusBar::receiveInfoText(const QString &theMessage) {
 info->setText(" "+theMessage+" ");
}

/**
 Slot handling time signal received from timer
*/
void StatusBar::timeOut() {
 msgLabel->setText(storedMsg);
 normCol();
}

/**
 Set color scheme of message label to "Normal"
*/
void StatusBar::normCol() {
 msgLabel->setPaletteForegroundColor(info->paletteForegroundColor());
 msgLabel->setPaletteBackgroundColor(info->paletteBackgroundColor());
}

/**
 Set color scheme of message label to "Warning"
*/
void StatusBar::warnCol() {
 msgLabel->setPaletteForegroundColor(info->paletteBackgroundColor());
 msgLabel->setPaletteBackgroundColor(info->paletteForegroundColor());
}

/**
 Set text shown in message label
 (permanently, until replaced by other text)
 @param theMessage new text
*/
void StatusBar::message(const QString &theMessage) {
 storedMsg=theMessage;
 msgLabel->setText(theMessage);
 normCol();
}

/**
 Receive warning and show it for some time
 @param theMessage Message
*/
void StatusBar::receiveWarnText(const QString &theMessage){
 tm->start(2500,true);
 msgLabel->setText(theMessage);
 warnCol();
}

/** default destructor */
StatusBar::~StatusBar() {
 delete tm;
}

} // namespace gui
