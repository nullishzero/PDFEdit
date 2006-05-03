/** @file
 AddItemDialog - class representing dialog for adding new item to Dictionary/array
 Dialog does not block focus of main window and you can have multiple of them opened simultaneously (even for same Dict / Array)
 @author Martin Petricek
*/
#include "additemdialog.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include "version.h"
#include "pdfutil.h"
#include "boolproperty.h"
#include "intproperty.h"
#include "realproperty.h"
#include "refproperty.h"
#include "stringproperty.h"
#include "nameproperty.h"
#include "settings.h"
#include "util.h"

namespace gui {

using namespace std;
using namespace pdfobjects;
using namespace util;

/**
 constructor of AddItemDialog, creates window and fills it with elements, parameters are ignored
 @param parent Parent window of this dialog
 @param name Name of this window (used only for debugging
 */
AddItemDialog::AddItemDialog(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name,WDestructiveClose || WType_TopLevel || WStyle_Minimize || WStyle_SysMenu || WStyle_Title || WStyle_Customize) {
 globalSettings->restoreWindow(this,"add_item_dialog"); 
 setCaption(tr("Add object"));
 l=new QVBoxLayout(this,4,4);
 qb=new QHBox(this,"additem_buttons");
 //Subpanel to select name or how to add to array - will be initialized later
 target=new QFrame(this,"additem_targetframe");
 //Subpanel to edit content to be added
 items=new QButtonGroup(2,Qt::Horizontal,tr("Type and value of new object"),this,"additem_itemframe");
 QPushButton *ok=new QPushButton(QObject::tr("&Add object"), qb);
 QPushButton *okclo=new QPushButton(QObject::tr("Add and c&lose"), qb);
 QPushButton *cancel=new QPushButton(QObject::tr("&Cancel"), qb);
 QObject::connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
 QObject::connect(ok, SIGNAL(clicked()), this, SLOT(commit()));
 QObject::connect(okclo, SIGNAL(clicked()), this, SLOT(commitClose()));
 int i=0;
 selectedItem=-1;
 //Start adding widgets for various types of editable properties
 labels[i]=new QRadioButton(getTypeName(pBool  ),items);props[i]=new BoolProperty("<new>",items);i++;
 labels[i]=new QRadioButton(getTypeName(pInt   ),items);props[i]=new IntProperty("<new>",items);i++;
 labels[i]=new QRadioButton(getTypeName(pReal  ),items);props[i]=new RealProperty("<new>",items);i++;
 labels[i]=new QRadioButton(getTypeName(pString),items);props[i]=new StringProperty("<new>",items);i++;
 labels[i]=new QRadioButton(getTypeName(pName  ),items);props[i]=new NameProperty("<new>",items);i++;
 labels[i]=new QRadioButton(getTypeName(pRef   ),items);props[i]=new RefProperty("<new>",items);i++;
 labels[i]=new QRadioButton(getTypeName(pDict  ),items);props[i]=NULL;i++;new QLabel(tr("New empty Dictionary"),items);
 labels[i]=new QRadioButton(getTypeName(pArray ),items);props[i]=NULL;i++;new QLabel(tr("New empty Array"),items);
 //Check if array dimensions are correct
 assert(i==addDialogPropertyTypes);
 for (i=0;i<addDialogPropertyTypes;i++) {
  if (props[i]) props[i]->setReadOnly(true);
  items->insert(labels[i],i);
 }
 QObject::connect(items,SIGNAL(clicked(int)),this,SLOT(buttonSelected(int)));
 //message label
 msg=new QLabel(" ",this);
}

/**
 Set given radiobutton as selected
 @param id Number of button
*/
void AddItemDialog::buttonSelected(int id) {
 guiPrintDbg(debug::DBG_DBG,"Selected button: " << id);
 if (selectedItem>=0) { //Something is selected -> deselect
  if (props[selectedItem]) props[selectedItem]->setReadOnly(true);
 }
 selectedItem=id;
 //Select new item;
 if (props[selectedItem]) props[selectedItem]->setReadOnly(false);
}

/** This is called on attempt to close window. */
void AddItemDialog::closeEvent(__attribute__((unused)) QCloseEvent* e) {
 delete this;
}

//TODO: detect if dict no longer exists in document and close/disable dialog
//TODO: cont.unique()==true -> this is last copy
//TODO: close on main window close or document destroy -> modify pdfeditwindow, add "close on close" widget dictionary

/**
 Set item into which newly created objects will be added and also initialize items in this control<br>
 Must be called exactly once before showing the control and must not be called after that
 @param it Dict or Array into which properties will be added
*/
void AddItemDialog::setItem(boost::shared_ptr<IProperty> it) {
 item=it;
 CDict* dict=dynamic_cast<CDict*>(it.get());
 CArray* arr=dynamic_cast<CArray*>(it.get());
 QHBoxLayout *lu=new QHBoxLayout(target,4,4);
 if (dict) {		//initialize items for adding to Dict
  usingArray=false;
  lu->addWidget(new QLabel(tr("Property name"),target));
  propertyName=new QLineEdit("",target);
  lu->addWidget(propertyName);
  //No validation for entered name - kernel does validation (and correction), so it is unnecessary
  //Just the string should not be empty
 } else if (arr) {	//initialize items for adding to Array
  usingArray=true;
  lu->addWidget(new QLabel(tr("Object will be appended at end of array"),target));
  //TODO: insert at arbitrary place in array
 } else {
  //Should never happen
  assert(0);
 }
 l->addWidget(target);
 l->addWidget(items);
 l->addWidget(qb);
 l->addWidget(msg);
}

/**
 Create dialog for adding objects (IProperty) to specified Dictionary<br>
 Returns immediately, but the dialog will remain opened.
 @param parent Parent window of this dialog
 @param cont Dict into which to add properties
 @return pointer to newly opened dialog instance
*/
AddItemDialog* AddItemDialog::create(QWidget *parent,boost::shared_ptr<CDict> cont) {
 AddItemDialog* ret=new AddItemDialog(parent);
 ret->setItem(cont);
 ret->show();
 return ret;
}

/**
 Create dialog for adding objects (IProperty) to specified Array
 Returns immediately, but the dialog will remain opened.
 @param parent Parent window of this dialog
 @param cont Array into which to add properties
 @return pointer to newly opened dialog instance
*/
AddItemDialog* AddItemDialog::create(QWidget *parent,boost::shared_ptr<CArray> cont) {
 AddItemDialog* ret=new AddItemDialog(parent);
 ret->setItem(cont);
 ret->show();
 return ret;
}

/** 
 Show informational message for this window
 @param message message to show
*/
void AddItemDialog::message(const QString &message) {
 msg->setPaletteForegroundColor(QColor(0,0,0));//Set black color
 msg->setText(message);
}

/** 
 Show error message for this window
 @param message message to show
*/
void AddItemDialog::error(const QString &message) {
 msg->setPaletteForegroundColor(QColor(255,0,0));//Set red color
 msg->setText(message);
}


/**
 Adds the item, but keep the window open,
 so user can easily modify value and add another similar item.
 @return true if successfull, false if cannot commit because of some errors in user-entered data 
*/
bool AddItemDialog::commit() {
 if (selectedItem<0) {
  error(tr("No object type selected"));
  return false;
 }
 if (props[selectedItem]) {	//Editable item type selected
  if (!props[selectedItem]->isValid()) {  
   error(tr("Entered property is not valid"));
   props[selectedItem]->setFocus();
   return false;
  }
 }
 if (!usingArray ) { //Adding to dict
  if (propertyName->text().length()==0) {
   error(tr("Name of new property must be specified"));
   propertyName->setFocus();
   return false;//No name entered for adding to Dict
  }
 }
 boost::shared_ptr<IProperty> property;
 switch (selectedItem) {
  case 0: {//pBool  
   property=boost::shared_ptr<IProperty>(new CBool(true));
   break;
  }
  case 1: {//pInt     
   property=boost::shared_ptr<IProperty>(new CInt(0));
   break;
  }
  case 2: {//pReal  
   property=boost::shared_ptr<IProperty>(new CReal(0));
   break;
  }
  case 3: {//pString
   property=boost::shared_ptr<IProperty>(new CString(""));
   break;
  }
  case 4: {//pName  
   property=boost::shared_ptr<IProperty>(new CName(""));
   break;
  }
  case 5: {//pRef   
   property=boost::shared_ptr<IProperty>(new CRef());
   break;
  }
  case 6: {//pDict  
   property=boost::shared_ptr<IProperty>(new CDict());
   break;
  }
  case 7: {//pArray 
   property=boost::shared_ptr<IProperty>(new CArray());
   break;
  }
  default:
   assert(0);//Should not get here
 }
 //Write value of new property
 if (props[selectedItem]) props[selectedItem]->writeValue(property.get());
 //TODO: check names
 //TODO: validate refproperty, if selected
 CDict* dict=dynamic_cast<CDict*>(item.get());
 if (dict) { //Add to dict
  message(tr("Property '%1' added to dictionary").arg(propertyName->text()));
  string tex=propertyName->text();
  dict->addProperty(tex,*(property.get()));
  return true;
 }
 CArray* arr=dynamic_cast<CArray*>(item.get());
 if (arr) { //Add to array
  message(tr("Property added to end of array"));
  arr->addProperty(*(property.get()));
  return true;
 }
 assert(0);//Should never happen 
 return false;
}

/** Adds the item and closes window */
void AddItemDialog::commitClose() {
 if (commit()) close();
}

/** default destructor */
AddItemDialog::~AddItemDialog() {
 globalSettings->saveWindow(this,"add_item_dialog"); 
}

} // namespace gui
