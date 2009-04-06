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
 ToolFactory class, factory that will create special toobar items
 @author Martin Petricek
*/

#include "toolfactory.h"
//The toolbar items to create
#include "colortool.h"
#include "edittool.h"
#include "numbertool.h"
#include "pagetool.h"
#include "selecttool.h"
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
 @return Pointer to button, if special item was loaded and inserted into toolbar,
 NULL if item is not a special item
*/
QWidget *ToolFactory::specialItem(ToolBar *tb,const QString &item,QMainWindow *main) {
 //All special items start with underscore character
 if (!item.startsWith("_")) return NULL;
 QString itemName=item.section(' ',0,0);
 QString itemParam=item.section(' ',1);
 if (itemName=="_revision_tool") {
  //Add RevisionTool to toolbar and return
  RevisionTool *tool =new RevisionTool(tb,"revision");
  QObject::connect(main,SIGNAL(documentChanged(boost::shared_ptr<pdfobjects::CPdf>)),tool,SLOT(setDocument(boost::shared_ptr<pdfobjects::CPdf>)));
  QObject::connect(main,SIGNAL(revisionChanged(int)),tool,SLOT(updateRevision(int)));
  QObject::connect(tool,SIGNAL(revisionChanged(int)),main,SLOT(changeRevision(int)));
  tool->show();
  return tool;
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
  return tool;
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
  return tool;
 }
 if (itemName=="_color_tool") {
  //Add ColorTool to toolbar and return
  ColorTool *tool =new ColorTool(itemParam,ColorTool::niceName(itemParam),tb,"color");
  PdfEditWindow *pdfw=dynamic_cast<PdfEditWindow*>(main);
  assert(pdfw);
  pdfw->addColorTool(tool);
  tool->show();
  return tool;
 }
 if (itemName=="_edit_tool") {
  //Add EditTool to toolbar and return
  EditTool *tool =new EditTool(itemParam,tb,"edit");
  PdfEditWindow *pdfw=dynamic_cast<PdfEditWindow*>(main);
  assert(pdfw);
  pdfw->addEditTool(tool);
  tool->show();
  return tool;
 }
 if (itemName=="_number_tool") {
  //Add NumberTool to toolbar and return
  NumberTool *tool =new NumberTool(itemParam,tb,"edit");
  PdfEditWindow *pdfw=dynamic_cast<PdfEditWindow*>(main);
  assert(pdfw);
  pdfw->addNumberTool(tool);
  tool->show();
  return tool;
 }
 if (itemName=="_select_tool") {
  //Add SelectTool to toolbar and return
  SelectTool *tool =new SelectTool(itemParam,tb,"edit");
  PdfEditWindow *pdfw=dynamic_cast<PdfEditWindow*>(main);
  assert(pdfw);
  pdfw->addSelectTool(tool);
  tool->show();
  return tool;
 }
 return NULL;
}

} // namespace gui
