/** @file
 ToolBar class, ancestor of QToolBar.
 This toolbar can handle only buttons from ToolButton class
 (but any generic QWidget can be inserted)
 @author Martin Petricek
*/

#include "toolbar.h"
#include "toolbutton.h"
#include "revisiontool.h"
#include "zoomtool.h"
#include "pagetool.h"
#include <qstring.h>
#include "pdfeditwindow.h"
#include "pagespace.h"
namespace pdfobjects {
 class CPdf;
}

namespace gui {

/**
 Same constructor as QToolBar
 @param label Caption of toolbar
 @param mainWindow Main window in which toolbar will be managed
 */
ToolBar::ToolBar(const QString &label,QMainWindow *mainWindow) : QToolBar(label,mainWindow) {
 //Empty constructor
}

/**
 Same constructor as QToolBar
 @param label Name of toolbar
 @param mainWindow Main window in which toolbar will be managed
 @param parent Parent window containing toolbar
 @param newLine Dock toolbar in a new line? (default is false = dock in same line)
 @param name Parameter passed to QDockWindow
 @param f Parameter passed to QDockWindow
 */
ToolBar::ToolBar(const QString &label,QMainWindow *mainWindow,QWidget *parent,bool newLine/*=FALSE*/,const char * name/*=0*/,WFlags f/*=0*/)
 : QToolBar(label,mainWindow,parent,newLine,name,f) {
 //Empty constructor
} 

/**
 Same constructor as QToolBar
 @param parent Main window in which toolbar will be managed
 @param name Name of toolbar (for debugging)
 */
ToolBar::ToolBar(QMainWindow *parent,const char *name/*=0*/) : QToolBar(parent,name) {
 //Empty constructor
}

/**
 Add button to toolbar and link slots, so clicks from buttons are passed along with button ID to the application
 @param qb Button to add to this toolbar 
 */
void ToolBar::addButton(ToolButton *qb) {
 QObject::connect(qb,SIGNAL(clicked(int)),this,SLOT(slotClicked(int)));
 QObject::connect(qb,SIGNAL(helpText(const QString&)),this,SLOT(receiveHelpText(const QString&)));
}

/**
 slot that will emit clicked with ID of button
 @param id Id of button
*/
void ToolBar::slotClicked(int id) {
 emit itemClicked(id);
}

/**
 Check if given item name is a special item and load it if it is special item
 @param tb Toolbar for addition of item
 @param item Item name
 @return True, if special item was loaded, false if item is not a special item
 @param main Main window associated with this special item
*/
bool ToolBar::specialItem(ToolBar *tb,const QString &item,QMainWindow *main) {
 if (item=="-" || item=="") {
  tb->addSeparator();
  return true;
 }
 //All special items start with underscore character
 if (!item.startsWith("_")) return false;
 if (item=="_revision_tool") {
  //Add RevisionTool to toolbar and return
  RevisionTool *tool =new RevisionTool(tb,"revision");
  QObject::connect(main,SIGNAL(documentChanged(pdfobjects::CPdf*)),tool,SLOT(setDocument(pdfobjects::CPdf*)));
  QObject::connect(main,SIGNAL(revisionChanged(int)),tool,SLOT(updateRevision(int)));
  QObject::connect(tool,SIGNAL(revisionChanged(int)),main,SLOT(changeRevision(int)));
  tool->show();
  return true;
 }
 if (item=="_zoom_tool") {
  //Add ZoomTool to toolbar and return
  ZoomTool *tool =new ZoomTool(tb,"zoom");
  PdfEditWindow *pdfw=dynamic_cast<PdfEditWindow*>(main);
  assert(pdfw);
  PageSpace *pSpace=pdfw->getPageSpace();
  assert(pSpace);
  tool->updateZoom(pSpace->getZoomFactor());
  QObject::connect(pSpace,SIGNAL(changedZoomFactorTo(float)),tool,SLOT(updateZoom(float)));
  QObject::connect(tool,SIGNAL(zoomSet(float)),pSpace,SLOT(setZoomFactor(float)));
  tool->show();
  return true;
 }
 if (item=="_page_tool") {
  //Add PageTool to toolbar and return
  PageTool *tool =new PageTool(tb,"page");
  PdfEditWindow *pdfw=dynamic_cast<PdfEditWindow*>(main);
  assert(pdfw);
  PageSpace *pSpace=pdfw->getPageSpace();
  assert(pSpace);
  tool->updatePage(pdfw->pageNumber());
  QObject::connect(pSpace,SIGNAL(changedPageTo(const QSPage&,int)),tool,SLOT(updatePage(const QSPage&,int)));
  QObject::connect(tool,SIGNAL(pageSet(int)),pSpace,SLOT(refresh(int)));
  tool->show();
  return true;
 }
 return false;

}

/** 
 Signal called when receiving help message.
 Forward the message via helpText
 @param message Help message
*/
void ToolBar::receiveHelpText(const QString &message) {
 emit helpText(message);
}

} // namespace gui
