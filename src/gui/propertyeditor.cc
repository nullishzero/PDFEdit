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
 //number of objects - empty
 nObjects=0;
 // create scrollview
 scroll=new QScrollView(this,"propertyeditor_scroll",0);
 scroll->setHScrollBarMode(QScrollView::AlwaysOff);
 scroll->setVScrollBarMode(QScrollView::AlwaysOn);
 //create grid in scrollview
 grid=new QFrame(scroll,"propertyeditor_grid");

 createLayout();
}

/** Delete internal layout */
void PropertyEditor::deleteLayout() {
 delete gridl; gridl=NULL;
}

/** Create internal layout */
void PropertyEditor::createLayout() {
//grid->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum));
 gridl = new QGridLayout( grid, 1, 2 );
 gridl->setSpacing(3);
 gridl->setMargin(3);
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
 //clear properties in property dictionary
 QDictIterator<Property> itp(*items);
 for (;itp.current();++itp) {
  gridl->remove(itp.current());
  delete itp.current();
 }
 //clear labels in label dictionary
 QDictIterator<QLabel> itl(*labels);
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
 labels->insert(QString(" ")+name+" ",label);
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
 if (!p) return;	//check if editable
 p->readValue(value.get());
 addProperty(p);//Will add and show the property
}

/** set IProperty object to be active (edited) in this editor
 @param pdfObject Object to set for editing in the widget
 */
void PropertyEditor::setObject(IProperty *pdfObject) {
 setUpdatesEnabled( FALSE );
 unsetObject();
 obj=pdfObject;
 //TODO: need property flags/mode
 if (!pdfObject) { //TODO: debug example
  Property *prop;
  prop=new IntProperty("Intproperty",grid);
  addProperty(prop);
  prop=new RealProperty("Realproperty",grid);
  addProperty(prop);
  prop=new BoolProperty("Boolproperty",grid);
  addProperty(prop);
  addProperty("StringProperty1");
  addProperty("StringProperty2");
 } else if (pdfObject->getType()==pDict) {	//Object is CDict -> edit its properties
  CDict *dict=(CDict*)pdfObject;
  vector<string> list;
  dict->getAllPropertyNames(list);
  vector<string>::iterator it;
  for( it=list.begin();it!=list.end();++it) { // for each property
   boost::shared_ptr<IProperty> property=dict->getProperty(*it);
   addProperty(*it,property);
  }
  grid->update();
 } else if (pdfObject->getType()==pArray) {	//Object is CArray
  CArray *ar=(CArray*)pdfObject;
  size_t n=ar->getPropertyCount();
  QString name;
  for(size_t i=0;i<n;i++) { //for each property
   boost::shared_ptr<IProperty> property=ar->getProperty(i);
   name.sprintf("[ %4d ]",i);
   addProperty(name,property);
  }
 }
 setUpdatesEnabled( TRUE );
}

/** default destructor */
PropertyEditor::~PropertyEditor() {
 unsetObject();//remove current object from editor
 delete labels;
 delete items;
 delete list;
}
