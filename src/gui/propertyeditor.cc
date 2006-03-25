/** @file
PropertyEditor - widget for editing properties of selected object
*/

#include "propertyeditor.h"
#include "util.h"
#include <qpoint.h>
#include <stdlib.h>
#include "stringproperty.h"
#include "intproperty.h"


//TODO: add functions for display & editing ... 

/** Default constructor of property editor.
 On creation, no object is assigned to editor
 @param parent parent widget containing this control
 @param name name of widget (currently unused)
 */
PropertyEditor::PropertyEditor(QWidget *parent /*=0*/, const char *name /*=0*/) : QWidget(parent,name) {
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
 gridl = new QGridLayout( grid, 1, 1 );
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
 //remove all items from grid
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
 //clear labels dictionary
 labels->clear();
 //clear property dictionary
 items->clear();
 //clear string list
 list->clear();
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
 @param p property to be added to this widget
 */
void PropertyEditor::addProperty(Property *prop) {
 QString name=prop->getName();
 QLabel *label;
 label=new QLabel(name,grid);
 int labelHeight=label->sizeHint().height();
 int propHeight=prop->sizeHint().height();
 gridl->setRowSpacing(nObjects,max(labelHeight,propHeight));
 gridl->addWidget(label,nObjects,0);
 gridl->addWidget(prop,nObjects,1);
 nObjects++;

 list->append(name);
 items->insert(name,prop);
 labels->insert(name,label);
}

/** add single property
 @param p property to be added to this widget
 */
void PropertyEditor::addProperty(QString name) {
//TODO: use Property factory and add type parameter
 Property *prop;
 prop=new StringProperty(name,grid);
 addProperty(prop);
}

/** TODO: set PDF object to be active (edited) in this editor
 @param p pdfObject Objet to set for editing in the widget
 */
void PropertyEditor::setObject(IProperty *pdfObject) {
 unsetObject();
 //TODO: ! real code: get all names, sort all names, then add all properties
 //TODO: ! get_property_all & in loop get_property, get_property_type & get_property_flag
//TODO: debug example code begin
 Property *prop;
 prop=new IntProperty("Intproperty",grid);
 addProperty(prop);
 addProperty("Property1");
 addProperty("Property2");
 addProperty("Property3");
 addProperty("Property4");
 addProperty("Property5");
 addProperty("Property6");
 addProperty("Property7");
 addProperty("Property8");
 addProperty("Property12");
 addProperty("Property13");
 addProperty("Property14");
 addProperty("Property15");
 addProperty("Property16");
 addProperty("Property17");
 addProperty("Property18");
 addProperty("Property32");
 addProperty("Property33");
 addProperty("Property34");
 addProperty("Property35");
 addProperty("Property36");
 addProperty("Property37");
 addProperty("Property58");
 addProperty("Property52");
 addProperty("Property53");
 addProperty("Property54");
 addProperty("Property55");
 addProperty("Property66");
 addProperty("Property67");
 addProperty("Property68");
 addProperty("Property62");
 addProperty("Property73");
 addProperty("Property84");
 addProperty("Property185");
 addProperty("Property186");
 addProperty("Property197");
 addProperty("Property108");
//TODO: debug example code end
}

/** default destructor */
PropertyEditor::~PropertyEditor() {
 unsetObject();//remove current object from editor
 delete labels;
 delete items;
 delete list;
}
