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
 TreeItemContentStream - class holding CContentStream object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include "treeitemcontentstream.h"
#include "treeitemcontentstreamobserver.h"
#include "multitreewindow.h"
#include "qscontentstream.h"
#include "treedata.h"
#include "treeitempage.h"
#include "treeitempdfoperator.h"
#include "util.h"
#include <kernel/ccontentstream.h>

namespace gui {

class TreeData;

using namespace std;

/**
 constructor of TreeItemContentStream - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListView in which to put this item
 @param pdfObj ContentStream contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemContentStream::TreeItemContentStream(TreeData *_data,Q_ListView *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 obj=pdfObj;
 init(name);
 mode=All;
}

/**
 constructor of TreeItemContentStream - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent Q_ListViewItem under which to put this item
 @param pdfObj ContentStream contained in this item
 @param nameId Internal name of this item
 @param name Caption of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemContentStream::TreeItemContentStream(TreeData *_data,Q_ListViewItem *parent,boost::shared_ptr<CContentStream> pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItemAbstract(nameId,_data,parent,after) {
 obj=pdfObj;
 init(name);
}

/**
 initialize captions of this item from name
 @param name Caption of item
 */
void TreeItemContentStream::init(const QString &name) {
 mode=All;
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,QObject::tr("Content Stream"));
 reload();
 initObserver();
 showMode();
}

/** Update tree item accdording to what mode is selected */
void TreeItemContentStream::showMode() {
 setText(2,"");
 if (parent()) return;	//Not in separate tree, which mean no modes
 if (mode==All) setText(2,QObject::tr("Showing all","mode"));
 else if (mode==Text) setText(2,QObject::tr("Showing text. op.","mode"));
 else if (mode==Font) setText(2,QObject::tr("Showing font. op.","mode"));
 else if (mode==Graphic) setText(2,QObject::tr("Showing graphic. op.","mode"));
 else {
  assert(0);
  setText(2,"?");
 }
}

/**
 Set mode of this tree item, i.e. what to show as children
 @param newMode new Mode
*/
void TreeItemContentStream::setMode(TreeItemContentStreamMode newMode) {
 mode=newMode;
 showMode();
 reload();
}

/** \copydoc setMode(TreeItemContentStreamMode) */
void TreeItemContentStream::setMode(const QString &newMode) {
 QString lMode=newMode.lower();
 if (lMode=="all") setMode(All);
 else if (lMode=="text") setMode(Text);
 else if (lMode=="font") setMode(Font);
 else if (lMode=="graphic") setMode(Graphic);
}

/**
 Get mode of this item as a string
 (that can be passed to setMode)
*/
QString TreeItemContentStream::getMode() {
 switch(mode) {
  case All:	return "all";
  case Text:	return "text";
  case Font:	return "font";
  case Graphic: return "graphic";
  default:	return "?";
 }
}

/** default destructor */
TreeItemContentStream::~TreeItemContentStream() {
 uninitObserver();
 //Deactivate the tab with this contentstream, if it is opened
 data->multi()->deactivate(obj);
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemContentStream::createChild(const QString &name,__attribute__((unused)) ChildType typ,Q_ListViewItem *after/*=NULL*/) {
 size_t position=name.toUInt();
 return new TreeItemPdfOperator(data,this,op[position],obj,name,after);
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemContentStream::getChildType(__attribute__((unused)) const QString &name) {
 return 1;//Just one type : PDF Operator
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemContentStream::getChildNames() {
 return util::countList(op.size());
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemContentStream::getQSObject() {
 return new QSContentStream(obj,data->base());
}

//See TreeItemAbstract for description of this virtual method
void TreeItemContentStream::remove() {
 // Do nothing
 // Contentstream can't just delete itself, no such function in either CPage or CContentstream.
 return;
}

//See TreeItemAbstract for description of this virtual method
void TreeItemContentStream::reloadSelf() {
 //Iterator to filter items
 PdfOperator::Iterator *it=NULL;
 //Reload list of pdf operators
 obj->getPdfOperators(op);
 if (!op.size()) return;//Nothing in here
 if (mode==Text) {
  // "Show only text operators" mode
  it=new TextOperatorIterator(op[0]);
 } else if (mode==Font){
  // "Show only font operators" mode
  it=new FontOperatorIterator(op[0]);
 } else if (mode==Graphic){
  // "Show only graphical operators" mode
  it=new GraphicalOperatorIterator(op[0]);
 } else {
  // "Show everything we got" mode -> no filtering is done
  return;
 }
 //We have the iterator, now clear the vector and populate it with ... something else
 op.clear();
 while (!it->isEnd()) {
  op.push_back(it->getCurrent());
  it->next();
 }
 delete it;
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemContentStream::haveChild() {
 return op.size()>0;
}

/**
 Slot that will be called when item is opened/closed
 @param open True if item is being opened, false if closed
*/
void TreeItemContentStream::setOpen(bool open) {
 if (parent() && open) { //Activate/open new tree
  //Check for parent if it is a page and try to "invent" some nice text for the tab and the item
  TreeItemPage* parentPage=dynamic_cast<TreeItemPage*>(parent());
  QString pName=text(0);
  QString pToolTip=pName;
  if (parentPage) { //Parent tree item is a page
   pName+=" (";
   pName+=parentPage->text(0);
   pName+=")";
   pToolTip=QObject::tr("Page")+" "+parentPage->text(0)+" - "+pToolTip;
  }
  data->multi()->activate(obj,pName,pToolTip);
  return;//Do not open
 }
 TreeItemAbstract::setOpen(open);
}

/**
 Return content stream inside this tree items
 @return Content stream in this tree item
*/
boost::shared_ptr<CContentStream> TreeItemContentStream::getObject() {
 return obj;
}

/** Sets observer for this item */
void TreeItemContentStream::initObserver() {
 guiPrintDbg(debug::DBG_DBG,"Set Observer");
 observer=boost::shared_ptr<TreeItemContentStreamObserver>(new TreeItemContentStreamObserver(this));
 REGISTER_SHAREDPTR_OBSERVER(obj, observer);
 guiPrintDbg(debug::DBG_DBG,"UC PRE" << observer.use_count() << " " << (intptr_t)(&(*observer)));
}

/** Unsets observer for this item */
void TreeItemContentStream::uninitObserver() {
 if (observer) {
  guiPrintDbg(debug::DBG_DBG,"UC POST" << observer.use_count() << " " << (intptr_t)(&(*observer)));
  observer->deactivate();
  UNREGISTER_SHAREDPTR_OBSERVER(obj, observer);
  observer.reset();
  guiPrintDbg(debug::DBG_DBG,"UnSet Observer");
 } else {
  //Because of some strange reason, the observer was destroyed
  guiPrintDbg(debug::DBG_ERR,"Observer does not exist anymore!");
 }
}

//See TreeItemAbstract for description of this virtual method
bool TreeItemContentStream::validChild(const QString &name,Q_ListViewItem *oldChild) {
 size_t i=name.toUInt();
 TreeItemPdfOperator *it=dynamic_cast<TreeItemPdfOperator*>(oldChild);
 assert(it);
 //Same address = same item
 //Different address = probably different item
 return op[i]==it->getObject();
}


} // namespace gui
