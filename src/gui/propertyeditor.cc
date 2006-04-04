/** @file
PropertyEditor - widget for editing properties of selected object
*/

#include "propertyeditor.h"
#include <utils/debug.h>
#include "util.h"
#include <qpoint.h>
#include <stdlib.h>
#include "stringproperty.h"
#include "intproperty.h"
#include "realproperty.h"
#include "boolproperty.h"
#include "propertyfactory.h"

using namespace std;

//TODO: add functions for display & editing ... 

/** Default constructor of property editor.
 On creation, no object is assigned to editor
 @param parent parent widget containing this control
 @param name name of widget (currently unused)
 */
PropertyEditor::PropertyEditor(QWidget *parent /*=0*/, const char *name /*=0*/) : QWidget(parent,name) {
 obj=NULL;
 //create list of properties in this editor;
 list=new QStringList();
 //create property dictionary
 items=new QDict<Property>();
 //create labels dictionary
 labels=new QDict<QLabel>();
 // create scrollview
 scroll=new QScrollView(this,"propertyeditor_scroll",0);
 //number of objects - empty
 nObjects=0;
 //create grid in scrollview
 grid=new QFrame(scroll,"propertyeditor_grid");
 gridl = new QGridLayout( grid, 1, 2 );
 scroll->addChild(grid);
 //set key column to be fixed and value column to be expandable
 gridl->setColStretch(0,0);
 gridl->setColStretch(1,1);
}

/** Called on resizing of property editor */
void PropertyEditor::resizeEvent (QResizeEvent *e) {
 scroll->setFixedSize(e->size());
 grid->setFixedWidth(scroll->visibleWidth());
}

/** remove and delete all properties from the editor */
void PropertyEditor::clear() {
 QObjectList *ygl=grid->queryList(0,0,FALSE,FALSE);
 printDbg(debug::DBG_DBG,"QUERYLIST BEFORE " << ygl->count() << " items");
 delete ygl;
 //clear properties in property dictionary
 QDictIterator<Property> itp(*items);
 for (;itp.current();++itp) {
  delete itp.current();
 }
 //clear labels in label dictionary
 QDictIterator<QLabel> itl(*labels);
 for (;itl.current();++itl) {
  delete itl.current();
 }
 //remove all items from grid if there is something left
 QObjectList *gl=grid->queryList(0,0,FALSE,FALSE);
 printDbg(debug::DBG_DBG,"QUERYLIST " << gl->count() << " items");
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
 //clear string list
 list->clear();
 obj=NULL;
 nObjects=0;
}

/** called on update of value from a property
 @param p property that was just updated in GUI
 */
void PropertyEditor::update(Property &p) {
 //TODO: read name from p
 //TODO: read value from p
 //TODO: update PDFObject 
 //TODO: redraw, etc ...
 //TODO: call from properties
}

/** Unset current object from property editor, leving it empty (use when current object deleted, etc ..) */
void PropertyEditor::unsetObject() {
 //TODO: unset object
 clear();
}

/** Add all internal properties to the widget */
void PropertyEditor::commitProperties() {
 //TODO: remove, not used
}

/** add single property
 @param prop property to be added to this widget
 */
void PropertyEditor::addProperty(Property *prop) {
 QString name=prop->getName();
 QLabel *label;
 label=new QLabel(name,grid);
 int labelHeight=label->sizeHint().height();
 int propHeight=prop->sizeHint().height();
 gridl->setRowSpacing(nObjects,MAX(labelHeight,propHeight));
 gridl->addWidget(label,nObjects,0);
 gridl->addWidget(prop,nObjects,1);
 nObjects++;
 list->append(name);
 items->insert(name,prop);
 labels->insert(name,label);
 prop->show();
 label->show();
}

/** add single property
 @param name property to be added to this widget
 */
void PropertyEditor::addProperty(QString name) {
//TODO: use Property factory and add type parameter
 Property *prop;
 prop=new StringProperty(name,grid);
 addProperty(prop);
}

/** Add single property to the widget
 @param name Name of property to be added
 @param value Value of property to be added
*/
void PropertyEditor::addProperty(const QString &name,boost::shared_ptr<IProperty> value) {
 Property *p=propertyFactory(value.get(),name,grid);//todo: flags
 printDbg(debug::DBG_DBG,"ADDP " << name);
 if (!p) return;	//check if editable
 printDbg(debug::DBG_DBG,"ADDP " << name << " not null");
 addProperty(p);
 p->show();
}

/** set IProperty object to be active (edited) in this editor
 @param pdfObject Object to set for editing in the widget
 */
void PropertyEditor::setObject(IProperty *pdfObject) {
 unsetObject();
 obj=pdfObject;
 if (!pdfObject) {
 //TODO: ! real code: get all names, sort all names, then add all properties
 //TODO: ! get_property_all & in loop get_property, get_property_type & get_property_flag
//TODO: debug example code begin
 Property *prop;
 prop=new IntProperty("Intproperty",grid);
 addProperty(prop);
 prop=new RealProperty("Realproperty",grid);
 addProperty(prop);
 prop=new BoolProperty("Boolproperty",grid);
 addProperty(prop);
 addProperty("StringProperty1");
 addProperty("StringProperty2");
 addProperty("StringProperty3");
//TODO: debug example code end
 } else {
 if (pdfObject->getType()==pDict) {	//Object is CDict -> edit its properties
  printDbg(debug::DBG_DBG,"CDICT->PropertyEditor");
  CDict *dict=(CDict*)pdfObject;
  vector<string> list;
  dict->getAllPropertyNames(list);
  vector<string>::iterator it;
  for( it=list.begin();it!=list.end();++it) { // for each property
   printDbg(debug::DBG_DBG,"Subproperty: " << *it);
   boost::shared_ptr<IProperty> property=dict->getProperty(*it);
   addProperty(*it,property);
  }
  grid->update();
 }
 }
 ///TODO: array
}

/** default destructor */
PropertyEditor::~PropertyEditor() {
 unsetObject();//remove current object from editor
 delete labels;
 delete items;
 delete list;
}
