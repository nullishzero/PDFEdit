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
 AddItemDialog - class representing dialog for adding new item to Dictionary/array
 Dialog does not block focus of main window and you can have multiple of them opened simultaneously (even for same Dict / Array)
 @author Martin Petricek
*/
#include "additemdialog.h"
#include "boolproperty.h"
#include "dialog.h"
#include "intproperty.h"
#include "nameproperty.h"
#include "pdfutil.h"
#include "realproperty.h"
#include "refproperty.h"
#include "settings.h"
#include "stringproperty.h"
#include "util.h"
#include "version.h"
#include QBUTTONGROUP
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvalidator.h>

namespace gui {

using namespace std;
using namespace pdfobjects;
using namespace util;

/**
 constructor of AddItemDialog, creates window and fills it with elements, parameters are ignored
 @param parent Parent window of this dialog
 @param name Name of this window (used only for debugging
 */
AddItemDialog::AddItemDialog(QWidget *parent/*=0*/,const char *name/*=0*/)
 : SelfDestructiveWidget(parent,parent,name,Qt::WDestructiveClose | Qt::WType_TopLevel | Qt::WStyle_Minimize | Qt::WStyle_SysMenu | Qt::WStyle_Title | Qt::WStyle_Customize) {
 //Parent is also killer -> this is always toplevel widget
 settingName="add_item_dialog";
 globalSettings->restoreWindow(this,settingName);
 setCaption(tr("Add object"));
 layout=new QVBoxLayout(this);
 layout->setMargin(4);
 layout->setSpacing(4);

 //Lower frame with buttons
 qbox=new QFrame(this);
 QHBoxLayout *qblayout = new QHBoxLayout(qbox);
 QPushButton *ok=new QPushButton(QObject::tr("&Add object"), qbox);
 QPushButton *okclo=new QPushButton(QObject::tr("Add and c&lose"), qbox);
 QPushButton *cancel=new QPushButton(QObject::tr("&Cancel"), qbox);
 qblayout->addWidget(ok);
 qblayout->addWidget(okclo);
 qblayout->addWidget(cancel);
 QObject::connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
 QObject::connect(ok, SIGNAL(clicked()), this, SLOT(commit()));
 QObject::connect(okclo, SIGNAL(clicked()), this, SLOT(commitClose()));

 //Subpanel to select name or how to add to array - will be initialized later
 target=new QFrame(this);

 //Subpanel to edit content to be added
 items=new Q_ButtonGroup(2,Qt::Horizontal,tr("Type and value of new object"),this);
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
  if (props[i]) props[i]->setDisabled(true);
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
  if (props[selectedItem]) props[selectedItem]->setDisabled(true);
 }
 selectedItem=id;
 //Select new item;
 if (props[selectedItem]) props[selectedItem]->setDisabled(false);
}

//TODO: detect if dict no longer exists in document and close/disable dialog
//TODO: cont.unique()==true -> this is last copy, BUT what if two dialogs are opened for same dict/array?

/**
 Set item into which newly created objects will be added and also initialize items in this control<br>
 Must be called exactly once before showing the control and must not be called after that
 @param it Dict or Array into which properties will be added
*/
void AddItemDialog::setItem(boost::shared_ptr<IProperty> it) {
 item=it;
 pdf=it->getPdf().lock();

 //We must set PDF to property
 RefProperty* refProp=dynamic_cast<RefProperty*>(props[5]);
 assert(refProp);
 refProp->setPdf(pdf);

 CDict* dict=dynamic_cast<CDict*>(it.get());
 CArray* arr=dynamic_cast<CArray*>(it.get());
 QHBoxLayout *lu=new QHBoxLayout(target,4,4);
 if (dict) {		//initialize items for adding to Dict
  settingName="add_item_dialog_dict";
  globalSettings->restoreWindow(this,settingName);
  usingArray=false;
  lu->addWidget(new QLabel(tr("Property name"),target));
  propertyName=new QLineEdit("",target);
  lu->addWidget(propertyName);
  //No validation for entered name - kernel does validation (and correction), so it is unnecessary
  //Just the string should not be empty
 } else if (arr) {	//initialize items for adding to Array
  settingName="add_item_dialog_array";
  globalSettings->restoreWindow(this,settingName);
  usingArray=true;

  QFrame *qb_pos=new QFrame(this);
  QHBoxLayout *qb_pos_layout = new QHBoxLayout(qb_pos);

  posNum=new QRadioButton(tr("Add to position"),qb_pos);
  arrayPos=new QLineEdit("0",qb_pos);
  QIntValidator *vv=new QIntValidator(this);
  vv->setBottom(0);
  arrayPos->setValidator(vv);
  posEnd=new QRadioButton(tr("Append to end"),qb_pos);
  qb_pos_layout->addWidget(posNum);
  qb_pos_layout->addWidget(arrayPos);
  qb_pos_layout->addWidget(posEnd);

  layout->addWidget(qb_pos);
  posNum->setChecked(true);
  posNumSet(true);
  connect(posNum,SIGNAL(toggled(bool)),this,SLOT(posNumSet(bool)));
  connect(posEnd,SIGNAL(toggled(bool)),this,SLOT(posEndSet(bool)));
 } else {
  //Should never happen
  assert(0);
 }
 layout->addWidget(target);
 layout->addWidget(items);
 layout->addWidget(qbox);
 layout->addWidget(msg);
}

/**
 Called whenever item in main window is deleted. If the item matches the item in which we are adding, close the dialog
 @param it deleted item
*/
void AddItemDialog::itemDeleted(boost::shared_ptr<IProperty> it) {
 if (it==item) {
  //Our item in which we are adding stuff was deleted
  close();
 }
}
/**
 This slot is called when state of the "insert at position" radiobox is changed
 @param on True of it was checked as "on"
*/
void AddItemDialog::posNumSet(bool on) {
 if (!on) return;
 arrayPos->setEnabled(true);
 posEnd->setChecked(false);
}

/**
 This slot is called when state of the "insert at end" radiobox is changed
 @param on True of it was checked as "on"
*/
void AddItemDialog::posEndSet(bool on) {
 if (!on) return;
 arrayPos->setEnabled(false);
 posNum->setChecked(false);
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
 connect(parent,SIGNAL(itemDeleted(boost::shared_ptr<IProperty>)),ret,SLOT(itemDeleted(boost::shared_ptr<IProperty>)));
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
 connect(parent,SIGNAL(itemDeleted(boost::shared_ptr<IProperty>)),ret,SLOT(itemDeleted(boost::shared_ptr<IProperty>)));
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
 try {
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
    IndiRef refValue;
    RefProperty* refProp=dynamic_cast<RefProperty*>(props[selectedItem]);
    refProp->setPdf(pdf);
    assert(refProp);
    refValue=refProp->getValue();
    if (!isRefValid(pdf,refValue)) {
     error(tr("Reference target does not exist!"));
     return false;
    }
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
  if (props[selectedItem]) props[selectedItem]->setValue(property.get());
  //TODO: validate refproperty, if selected
  CDict* dict=dynamic_cast<CDict*>(item.get());
  if (dict) { //Add to dict
   string tex=util::convertFromUnicode(propertyName->text(),util::PDF);
   try {
    PropertyType pt=dict->getPropertyType(tex);
    QString msg=tr("Property '%1' already exist as %2").arg(propertyName->text(),getTypeName(pt));
    if (questionDialog(this,msg+"\n"+QObject::tr("Overwrite?"))) {
     dict->delProperty(tex);
    } else {
     error(msg);
     return false;
    }
   } catch (...) {
    //The property does not exist, so it is ok to add it
   }
   message(tr("Property '%1' added to dictionary").arg(propertyName->text()));
   dict->addProperty(tex,*(property.get()));
   return true;
  }
  CArray* arr=dynamic_cast<CArray*>(item.get());
  if (arr) { //Add to array
   if (posNum->isChecked()) {
    int array_pos=0;
    try {
     //Add to specific index
     array_pos=arrayPos->text().toInt();
     message(tr("Property added to position %1 in array").arg(array_pos));
     arr->addProperty(array_pos,*(property.get()));
     return true;
    } catch (ReadOnlyDocumentException &e) {
     //The document is read only, so we can't add anything in in
     error(QObject::tr("Document is read-only"));
     return false;
    } catch (...) {
     error(tr("Array index %1 out of range").arg(array_pos));
     return false;
    }
   } else {
    message(tr("Property added to end of array"));
    arr->addProperty(*(property.get()));
    return true;
   }
  }
 } catch (ReadOnlyDocumentException &e) {
  //The document is read only, so we can't add anything in in
  error(QObject::tr("Document is read-only"));
  return false;
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
 globalSettings->saveWindow(this,settingName);
}

} // namespace gui
