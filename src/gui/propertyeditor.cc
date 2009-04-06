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
 PropertyEditor - widget for editing properties of selected object
 Can edit Dict and Array containing primitive types and also primitive type as one items itself
 Primitive types are: int, bool, string, name, real, ref
 @author Martin Petricek
*/

#include "propertyeditor.h"
#include <utils/debug.h>
#include "util.h"
#include <kernel/cobject.h>
#include <qapplication.h>
#include "main.h"
#include "propertyfactory.h"
#include "property.h"
#include QSCROLLVIEW
#include <qstringlist.h>
#include <qlabel.h>
#include <qgrid.h>
#include <qlayout.h>
#include "pdfutil.h"
#include "settings.h"
#include "main.h"
#include "propertymodecontroller.h"
#include <qevent.h>

namespace gui {

using namespace std;
using namespace util;

/**
 Default constructor of property editor.
 On creation, no object is assigned to editor
 @param parent parent widget containing this control
 @param name name of widget (currently unused)
 */
PropertyEditor::PropertyEditor(QWidget *parent /*=0*/, const char *name /*=0*/) : QWidget(parent,name) {
 //Mode Controller
 mode=PropertyModeController::getInstance();
 //create list of properties in this editor;
 list=new QStringList();
 //create property dictionary
 items=new Q_Dict<Property>();
 //create IProperty dictionary
 props=new QMap<QString,boost::shared_ptr<IProperty> > ();
 //create labels dictionary
 labels=new Q_Dict<QLabel>();
 //number of objects - empty
 nObjects=0;
 //Set some minimum height to be at least somewhat visible
 setMinimumHeight(40);
 // create scrollview
 scroll=new Q_ScrollView(this,"propertyeditor_scroll",0);
 scroll->setHScrollBarMode(Q_ScrollView::AlwaysOff);
 scroll->setVScrollBarMode(Q_ScrollView::AlwaysOn);//TODO:CHECK
 //create grid in scrollview
 grid=new QFrame(scroll);

 createLayout();
 checkOverrides();
}

/** Delete internal layout */
void PropertyEditor::deleteLayout() {
 delete gridl; gridl=NULL;
}

/** Create internal layout */
void PropertyEditor::createLayout() {
 gridl=new QGridLayout(grid,1,2);
 gridl->setSpacing(3);
 gridl->setMargin(3);
 scroll->addChild(grid);
 //set key column to be fixed and value column to be expandable
 gridl->setColStretch(0,0);
 gridl->setColStretch(1,1);
}

/**
 Called on resizing of property editor
 @param e resize event data (new size)
 */
void PropertyEditor::resizeEvent (QResizeEvent *e) {
 scroll->setFixedSize(e->size());
 //TODO: may not be correct
 grid->setFixedWidth(scroll->visibleWidth());//TODO:CHECK
// grid->setFixedWidth(scroll->viewport()->width());//TODO:CHECK
}

/** remove and delete all properties from the editor, unset current object */
void PropertyEditor::clear() {
 currentOp.reset();
 currentObj.reset();
 rowNum.clear();
 propLabel.clear();
 //clear properties in property dictionary
 Q_DictIterator<Property> itp(*items);
 for (;itp.current();++itp) {
  gridl->remove(itp.current());
  delete itp.current();
 }
 //clear labels in label dictionary
 Q_DictIterator<QLabel> itl(*labels);
 for (;itl.current();++itl) {
  gridl->remove(itl.current());
  delete itl.current();
 }
 //remove all items from grid if there is something left just to be sure its empty
 QObjectList *gl=grid->queryList(0,0,FALSE,FALSE);
 QObjectListIterator it_g(*gl);
 QObject *obj;
 while ((obj=it_g.current())!=0) {
  if (obj->isWidgetType()) { //if it is a widget (not layout, etc ...)
   grid->removeChild(obj);//remove property or label from grid
  }
  ++it_g;
 }
 delete gl;
 //clear labels dictionary
 labels->clear();
 //clear property dictionary
 items->clear();
 props->clear();
 //clear string list
 list->clear();
 obj=NULL;
 nObjects=0;

 //Needed, grid will never shrink
 deleteLayout();
 createLayout();
}

/** called on update of value from a property
 @param p property that was just updated in GUI
 */
void PropertyEditor::update(Property *p) {
 QString pname=p->getName();
 guiPrintDbg(debug::DBG_DBG,"Updating property" << Q_OUT(pname));
 assert(props->contains(pname));
 boost::shared_ptr<IProperty> obj=(*props)[pname];
 try {
  p->setValue(obj.get());
 } catch (ReadOnlyDocumentException &e) {
  //Read only, cannot change the property
  emit warnText(QObject::tr("Document is read-only"));
  return;
 }
 emit propertyChanged(obj.get());
}

/**
 Add single property
 @param prop property to be added to this widget
 @param value Value of property to be added
 */
void PropertyEditor::addProperty(Property *prop,boost::shared_ptr<IProperty> value) {
 QString name=prop->getName();
 QLabel *label;
 label=new QLabel(QString(" ")+name+" ",grid);
 label->setTextFormat(Qt::PlainText);
 int labelHeight=label->sizeHint().height();
 int propHeight=prop->sizeHint().height();
 gridl->setRowSpacing(nObjects,MAX(labelHeight,propHeight));
 gridl->addWidget(label,nObjects,0);
 gridl->addWidget(prop,nObjects,1);
 rowNum.insert(prop,nObjects);
 propLabel.insert(prop,label);
 nObjects++;
 list->append(name);
 items->insert(name,prop);
 props->insert(name,value);
 labels->insert(name,label);
 connect(prop,SIGNAL(propertyChanged(Property*)),this,SLOT(update(Property*)));
 connect(prop,SIGNAL(infoText(const QString&)),this,SLOT(receiveInfoText(const QString&)));
 connect(prop,SIGNAL(warnText(const QString&)),this,SLOT(receiveWarnText(const QString&)));
 prop->initLabel(label);
 prop->override(showHidden,editReadOnly);//Will show or hide
 fixPropertyHeight(prop);
}

/**
 Fix height of row containing specified property, after it was shown or hidden
 @param pr Affected property
*/
void PropertyEditor::fixPropertyHeight(Property *pr) {
 int row=rowNum[pr];
 QLabel *label=propLabel[pr];
 if (pr->isHidden()) {
  gridl->setRowSpacing(row,1);
 } else {
  int labelHeight=label->sizeHint().height();
  int propHeight=pr->sizeHint().height();
  gridl->setRowSpacing(row,MAX(labelHeight,propHeight));
 }
}
/** Add single property to the widget
 @param name Name of property to be added
 @param value Value of property to be added
 @return true if this property is editable and was added. false if this property is uneditable (unknown type)
*/
bool PropertyEditor::addProperty(const QString &name,boost::shared_ptr<IProperty> value) {
 Property *p=propertyFactory(value.get(),name,grid);
 if (!p) return false;	//check if editable
 p->readValue(value.get());
 addProperty(p,value);//Will add and show the property
 return true;
}

/**
 Append message as next "property"
 @param message Message string
*/
void PropertyEditor::appendMessage(const QString &message) {
 QString name="the_label";
 QLabel *label=new QLabel(message,grid);
 label->setTextFormat(Qt::RichText);
 int labelHeight=label->sizeHint().height();
 gridl->setRowSpacing(nObjects,labelHeight);
 gridl->addMultiCellWidget(label,nObjects,nObjects,0,1);
 nObjects++;
 list->append(name);
 labels->insert(name,label);
 label->show();
}

/** Show some message inside property editor instead of its usual contents
 (like "this is not editable", "this object does not have properties", etc ...)
 @param message Message to show
 */
void PropertyEditor::setObject(const QString &message) {
 setUpdatesEnabled( FALSE );
 clear();
 appendMessage(message);
 setUpdatesEnabled( TRUE );
}

/**
 Unset object from property editor
*/
void PropertyEditor::unsetObject() {
 setObject(tr("No object selected"));
}

/**
 Sort the list, if settings are set to sort it
 @param list list to conditionally sort
*/
void PropertyEditor::sortList(std::vector<std::string> &list) {
 if (globalSettings->readBool("tree/show_dict_sort",false)) {
  sort(list.begin(),list.end());
 }
}

/**
 Set IProperty object to be active (edited) in this editor
 @param pdfObject Object to set for editing in the widget
*/
void PropertyEditor::setObject(boost::shared_ptr<IProperty> pdfObject) {
 setUpdatesEnabled( FALSE );
 clear();
 currentObj=pdfObject;
 int complex_properties=0; //Number of complex (directly uneditable) properties
 if (!pdfObject.get()) {
  unsetObject();
 } else if (pdfObject->getType()==pDict) {	//Object is CDict -> edit its properties
  CDict *dict=dynamic_cast<CDict*>(pdfObject.get());
  vector<string> list;
  dict->getAllPropertyNames(list);
  sortList(list);
  vector<string>::iterator it;
  int i=0;
  for( it=list.begin();it!=list.end();++it) { // for each property
   i++;
   boost::shared_ptr<IProperty> property=dict->getProperty(*it);
   if (!addProperty(*it,property)) complex_properties++;
  }
  //complex_message(complex_properties);
  if (!i) { //No subproperties at all
   setObject(tr("This %1 is empty").arg(tr("dictionary","property type")));
  } else if (!nObjects) { //No editable subproperties
   setObject(tr("This %1 does not have any directly editable properties").arg(tr("dictionary","property type")));
  }
  grid->update();
 } else if (pdfObject->getType()==pStream) {	//Object is CCtream -> edit its properties.
  //Basically the same as CDict  ....
  CStream *cstream=dynamic_cast<CStream*>(pdfObject.get());
  vector<string> list;
  cstream->getAllPropertyNames(list);
  sortList(list);
  vector<string>::iterator it;
  int i=0;
  for( it=list.begin();it!=list.end();++it) { // for each property
   i++;
   boost::shared_ptr<IProperty> property=cstream->getProperty(*it);
   if (!addProperty(*it,property)) complex_properties++;
  }
  //complex_message(complex_properties);
  if (!i) { //No subproperties at all
   setObject(tr("This %1 is empty").arg(tr("stream","property type")));
  } else if (!nObjects) { //No editable subproperties
   setObject(tr("This %1 does not have any directly editable properties").arg(tr("stream","property type")));
  }
  grid->update();
 } else if (pdfObject->getType()==pArray) {	//Object is CArray
  CArray *ar=dynamic_cast<CArray*>(pdfObject.get());
  size_t n=ar->getPropertyCount();
  QString name;
  size_t i=0;
  for(;i<n;i++) { //for each property
   boost::shared_ptr<IProperty> property=ar->getProperty(i);
   name.sprintf("[ %4d ]",(int)i);
   if (!addProperty(name,property)) complex_properties++;
  }
  //complex_message(complex_properties);
  if (!i) { //No subproperties at all
   setObject(tr("This array is empty"));
  } else if (!nObjects) { //No editable subproperties
   setObject(tr("This array does not have any directly editable elements"));
  }
 } else { //Simple or unknown type
  setObject(tr("This type of object does not have any properties")+" ("+getTypeName(pdfObject)+")");
 }
 setUpdatesEnabled( TRUE );
}

/**
 Set PdfOperator to be active (edited) in this editor.
 Operands will be edited
 @param pdfOp Object to set for editing in the widget
*/
void PropertyEditor::setObject(boost::shared_ptr<PdfOperator> pdfOp) {
 setUpdatesEnabled( FALSE );
 clear();
 currentOp=pdfOp;
 if (!pdfOp.get()) {
  unsetObject();
 } else {
  int complex_properties=0; //Number of complex (directly uneditable) properties
  PdfOperator::Operands list;
  pdfOp->getParameters(list);
  PdfOperator::Operands::iterator it;
  int i=0;
  for(it=list.begin();it!=list.end();++it) { // for each property
   i++;
   if (!addProperty(QObject::tr("Parameter")+" "+QString::number(i),*it)) complex_properties++;
  }
  //complex_message(complex_properties);
  if (!i) { //No subproperties at all
   setObject(tr("This operator does not have any parameters"));
  } else if (!nObjects) { //No editable subproperties
   setObject(tr("This operator does not have any directly editable parameters"));
  }
  grid->update();
 }
 setUpdatesEnabled( TRUE );
}

/**
 Display additional message informing about number of complex
 properti4es that are not editable and thus not show
 @param num_complex Number of complex properties
*/
void PropertyEditor::complex_message(int num_complex) {
 if (!num_complex) return; /// No message to show
 appendMessage(countString(num_complex,"complex item is not shown","complex items are not shown"));
}

/** set single IProperty object to be active (edited) in this editor
 This is variant for editing single object - from parent, only this object will be shown and editable
 @param name Name under which will be this property shown in editor
 @param pdfObject Object to set for editing in the widget
 */
void PropertyEditor::setObject(const QString &name,boost::shared_ptr<IProperty> pdfObject) {
 if (!pdfObject.get()) {
  setObject(pdfObject);//NULL object? Pass along
 } else if (pdfObject->getType()==pDict || pdfObject->getType()==pArray || pdfObject->getType()==pStream) {
  //Object is CDict or CArray -> call original function
  setObject(pdfObject);
 } else { //Simple or unknown type -> we try to edit it as single item
  setUpdatesEnabled( FALSE );
  clear();
  //Editing dictionary from parent, but showing only one object
  if (!addProperty(name,pdfObject)) { //Unknown type
   setObject(tr("This type of object does not have any properties")+" ("+getTypeName(pdfObject)+")");
  }
  grid->update();
  setUpdatesEnabled( TRUE );
 }
}

/**
 Commit currently edited property in property editor,
 if that property is edited in current property editor
*/
void PropertyEditor::commitProperty() {
 //Find active widget
 QWidget* propNow=q_App->focusWidget();//This is probably a Linedit if edited control is Property
 if (!propNow) return;
// guiPrintDbg(debug::DBG_DBG,"commitProperty classname=" << propNow->className() << " name=" << propNow->name());
 while(propNow && !dynamic_cast<Property*>(propNow)) { //Look for parent until Property is found
  propNow=propNow->parentWidget();
  if (!propNow) return;//No property is active
//  guiPrintDbg(debug::DBG_DBG,"commitProperty traverse classname=" << propNow->className() << " name=" << propNow->name());
 }
 Property* propCurrent=dynamic_cast<Property*>(propNow);
 //Check if it is Property
 if (!propCurrent) return;//No property is active
// guiPrintDbg(debug::DBG_DBG,"property is current");
 //Check if it is property from this property editor
 if (propCurrent->parentWidget()==grid) {
//  guiPrintDbg(debug::DBG_DBG,"property is our");
  //Property from this property editor -> update it
  update(propCurrent);
 }
}

/**
 Receive and resend informational text
 @param message Message
*/
void PropertyEditor::receiveInfoText(const QString &message) {
 emit infoText(message);
}

/**
 Receive and resend warning message
 @param message Message
*/
void PropertyEditor::receiveWarnText(const QString &message){
 emit warnText(message);
}

/** default destructor */
PropertyEditor::~PropertyEditor() {
 clear();//remove current object from editor
 delete labels;
 delete items;
 delete list;
 delete props;
}

/**
 Apply overrides bypassing limitation set by modecontroller
 @param _showHidden Show hidden properties
 @param _editReadOnly Edit read-only properties
*/
void PropertyEditor::override(bool _showHidden,bool _editReadOnly) {
 Q_DictIterator<Property> itp(*items);
 for (;itp.current();++itp) {
  itp.current()->override(_showHidden,_editReadOnly);
  fixPropertyHeight(itp.current());
 }
}

/**
 Look in settings for overrides bypassing limitation set by modecontroller.
*/
void PropertyEditor::checkOverrides() {
 showHidden=globalSettings->readBool("editor/show_hidden");
 editReadOnly=globalSettings->readBool("editor/edit_readonly");
 override(showHidden,editReadOnly);
}

/**
 Reload item, if it is appropriate for the item currently shown in editor
*/
void PropertyEditor::reloadItem() {
 if (currentObj) {
  setObject(currentObj);
 }
 if (currentOp) {
  setObject(currentOp);
 }
}

} // namespace gui
