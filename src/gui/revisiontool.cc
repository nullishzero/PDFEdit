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
 RevisionTool - Toolbutton listing revisions of current PDF document, with ability to switch revisions
 @author Martin Petricek
*/

#include <utils/debug.h>
#include "revisiontool.h"
#include <qstring.h>
#include <kernel/cpdf.h>
#include <qcombobox.h>
#include "settings.h"
#include <qevent.h>

namespace gui {

using namespace pdfobjects;

/**
 Default constructor of RevisionTool
 @param parent Toolbar containing this control
 @param name Name of this widget (passed to QWidget constructor)
*/
RevisionTool::RevisionTool(QWidget *parent/*=0*/,const char *name/*=NULL*/) : QWidget (parent,name) {
 revList=new QComboBox(this,"revision_select");
 QObject::connect(revList,SIGNAL(activated(int)),this,SLOT(selectRevision(int)));
 setDocument(boost::shared_ptr<CPdf>());
}

/** default destructor */
RevisionTool::~RevisionTool() {
 delete revList;
}

/**
 return size hint of this control
 @return size hint from inner revision list
*/
QSize RevisionTool::sizeHint() const {
 QSize tmp=revList->sizeHint();
 QFont font=revList->font();
 QFontMetrics fm(font);
 //Enough space for the string, unless too many too big revisions
 tmp.setWidth(fm.width(tr("Revision: ")+"00 (1222333 "+tr("bytes")+")")+tmp.height());
 //We add tmp.height() because of the button on right side of the combobox.
 //It have approximately square shape (in most visual styles)
 return tmp;
}

/**
 Called on resizing of property editing control
 Will simply set the same fixed size to inner revision list
 @param e resize event
*/
void RevisionTool::resizeEvent (QResizeEvent *e) {
 revList->setFixedSize(e->size());
}

/**
 Called on selecting a new revision from menu
 @param revision selected revision
 */
void RevisionTool::selectRevision(int revision) {
 if (!document) return;
 guiPrintDbg(debug::DBG_DBG,"Document revision setting to : " << revision);
 int currentRevision=document->getActualRevision();
 if (revision==currentRevision) return;//no need to switch
 emit revisionChanged(revision);
}

/**
 Called to update control after selecting a new revision outside of this control
 @param revision selected revision
 */
void RevisionTool::updateRevision(int revision) {
 if (!document) return;
 revList->setCurrentItem(revision);
}

/**
 Called on loading of new document
 @param newDocument Reference to new PDF document
*/
void RevisionTool::setDocument(boost::shared_ptr<pdfobjects::CPdf> newDocument) {
 document=newDocument;
 revList->clear();
 if (document) {
  //Document loaded
  size_t revs=document->getRevisionsCount();
  CPdf::revision_t curr=document->getActualRevision();
  guiPrintDbg(debug::DBG_DBG,"Document revisions: " << revs);
  if (document->isLinearized()) {
   revList->insertItem(tr("Linearized PDF"));
   revList->setEnabled(false);
   return;
  }
  if (revs==0) {
   revList->insertItem(tr("No revisions"));
   revList->setEnabled(false);
   return;
  }
  for(size_t i=0;i<revs;i++) {
   size_t revSize=document->getRevisionSize(i,true);
   revList->insertItem(tr("Revision: ")+QString::number(i)+" ("+QString::number(revSize)+" "+tr("bytes")+")");
  }
  revList->setCurrentItem(curr);
  revList->setEnabled(true);
 } else {
  //No document loaded
  revList->setEnabled(false);
 }
}

} // namespace gui
