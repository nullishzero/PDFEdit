/** @file
 RevisionTool - Toolbutton listing revisions of current PDF document, with ability to switch revisions
 @author Martin Petricek
*/

#include <utils/debug.h>
#include "revisiontool.h"
#include <qstring.h>
#include <cpdf.h>
#include <qcombobox.h>
#include "settings.h"

namespace gui {

/**
 Default constructor of RevisionTool
 @param parent Toolbar containing this control
 @param name Name of this widget (passed to QWidget constructor)
*/
RevisionTool::RevisionTool(QWidget *parent/*=0*/,const char *name/*=NULL*/) : QWidget (parent,name) {
 revList=new QComboBox(this,"revision_select");
 QObject::connect(revList,SIGNAL(activated(int)),this,SLOT(selectRevision(int)));
 setDocument(NULL);
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
 //TODO: better size guess
 tmp.setWidth(tmp.width()*2);
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
void RevisionTool::setDocument(CPdf *newDocument) {
 document=newDocument;
 revList->clear();
 if (document) {
  //Document loaded
  size_t revs=document->getRevisionsCount();
  CPdf::revision_t curr=document->getActualRevision();
  guiPrintDbg(debug::DBG_DBG,"Document revisions: " << revs);
  if (revs==0) {
   revList->insertItem(tr("No revisions"));
   revList->setEnabled(false);
   return;   
  }
  for(size_t i=0;i<revs;i++) {
   revList->insertItem(tr("Revision: ")+QString::number(i));  
  }
  revList->setCurrentItem(curr);
  revList->setEnabled(true);
 } else {
  //No document loaded
  revList->setEnabled(false);
 }
}

} // namespace gui
