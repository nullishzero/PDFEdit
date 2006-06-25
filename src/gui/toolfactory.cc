/** @file
 ToolFactory class, factory that will create special toobar items
 @author Martin Petricek
*/

#include "toolfactory.h"
//The toolbar items to create
#include "colortool.h"
#include "pagetool.h"
#include "revisiontool.h"
#include "zoomtool.h"
//Other includes needed
#include "pagespace.h"
#include "pdfeditwindow.h"
#include "toolbar.h"
#include <qobject.h>
#include <qstring.h>
namespace pdfobjects {
 class CPdf;
}

namespace gui {

/**
 Check if given item name is a special item and load it if it is special item
 @param tb Toolbar for addition of item
 @param item Item name
 @param main Main window associated with this special item
 @return True, if special item was loaded and inserted into toolbar, false if item is not a special item
*/
bool ToolFactory::specialItem(ToolBar *tb,const QString &item,QMainWindow *main) {
 if (item=="-" || item=="") {
  tb->addSeparator();
  return true;
 }
 //All special items start with underscore character
 if (!item.startsWith("_")) return false;
 QString itemName=item.section(' ',0,0);
 QString itemParam=item.section(' ',1);
 if (itemName=="_revision_tool") {
  //Add RevisionTool to toolbar and return
  RevisionTool *tool =new RevisionTool(tb,"revision");
  QObject::connect(main,SIGNAL(documentChanged(pdfobjects::CPdf*)),tool,SLOT(setDocument(pdfobjects::CPdf*)));
  QObject::connect(main,SIGNAL(revisionChanged(int)),tool,SLOT(updateRevision(int)));
  QObject::connect(tool,SIGNAL(revisionChanged(int)),main,SLOT(changeRevision(int)));
  tool->show();
  return true;
 }
 if (itemName=="_zoom_tool") {
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
 if (itemName=="_page_tool") {
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
 if (itemName=="_color_tool") {
  //Add PageTool to toolbar and return
  ColorTool *tool =new ColorTool(itemParam,tb,"color");
  PdfEditWindow *pdfw=dynamic_cast<PdfEditWindow*>(main);
  assert(pdfw);
  pdfw->addColorTool(tool);
/*  PageSpace *pSpace=pdfw->getPageSpace();
  assert(pSpace);
  tool->updatePage(pdfw->pageNumber());
  QObject::connect(pSpace,SIGNAL(changedPageTo(const QSPage&,int)),tool,SLOT(updatePage(const QSPage&,int)));
  QObject::connect(tool,SIGNAL(pageSet(int)),pSpace,SLOT(refresh(int)));*/
  tool->show();
  return true;
 }
 return false;
}

} // namespace gui
