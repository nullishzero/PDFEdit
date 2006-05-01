/** @file
 AddItemDialog - class representing dialog for adding new item to Dictionary/array
 Dialog does not block focus of main window and you can have multiple of them opened simultaneously (even for same Dict / Array)
 @author Martin Petricek
*/
#include "additemdialog.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include "version.h"

namespace gui {

using namespace std;
using namespace pdfobjects;

/**
 constructor of AddItemDialog, creates window and fills it with elements, parameters are ignored
 @param parent Parent window of this dialog
 @param name Name of thisd window (used only for debugging
 */
AddItemDialog::AddItemDialog(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name,WDestructiveClose || WType_TopLevel || WStyle_Minimize || WStyle_SysMenu || WStyle_Title || WStyle_Customize) {
 setCaption(tr("Add object"));
 QBoxLayout *l=new QVBoxLayout(this);
 QHBox *qb=new QHBox(this,"additem_vbox");
 QLabel *lb=new QLabel(QString("Add item: TODO"), this);
 QPushButton *ok=new QPushButton(QObject::tr("&Add item"), qb);
 QPushButton *okclo=new QPushButton(QObject::tr("Add and c&lose"), qb);
 QPushButton *cancel=new QPushButton(QObject::tr("&Cancel"), qb);
 //TODO
 QObject::connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
 QObject::connect(ok, SIGNAL(clicked()), this, SLOT(commit()));
 QObject::connect(okclo, SIGNAL(clicked()), this, SLOT(commitClose()));
 l->addWidget(lb);
 l->addWidget(qb);
 ok->show();
 lb->show();
}

/** This is called on attempt to close window. */
void AddItemDialog::closeEvent(QCloseEvent *e) {
 delete this;
}

//TODO: detect if dict no longer exists in document and close/disable dialog
//TODO: cont.unique()==true -> this is last copy
//TODO: close on main window close or document destroy -> modify pdfeditwindow, add "close on close" widget dictionary

/**
 Set item into which newly created objects will be added and also initialize items in this control
 @param it Dict or Array into which properties will be added
*/
void AddItemDialog::setItem(boost::shared_ptr<IProperty> it) {
 item=it;
 CDict* dict=dynamic_cast<CDict*>(it.get());
 CArray* arr=dynamic_cast<CArray*>(it.get());
 if (dict) {		//initialize items for adding to Dict
  //TODO
 } else if (arr) {	//initialize items for adding to Array
  //TODO
 } else {
  //Should never happen
  assert(0);
 }
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
 Adds the item, but keep the window open,
 so user can easily modify value and add another similar item.
*/
void AddItemDialog::commit() {
 //TODO
}

/** Adds the item and closes window */
void AddItemDialog::commitClose() {
 commit();
 close();
}

/** default destructor */
AddItemDialog::~AddItemDialog() {
}

} // namespace gui
