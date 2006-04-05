/** @file
OptionWindow - widget for editing program options
*/

#include "optionwindow.h"
#include <utils/debug.h>
#include "util.h"
#include <qpoint.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include "option.h"
#include "version.h"

using namespace std;

/** Default constructor of option window.
 The window is initially empty
 @param parent parent widget containing this control
 @param name name of widget (currently unused)
 */
OptionWindow::OptionWindow(QWidget *parent /*=0*/, const char *name /*=0*/) : QWidget(parent,name,WDestructiveClose || WType_TopLevel) {
 printDbg(debug::DBG_DBG,"Options creating ...");
 setCaption(QString(APP_NAME)+" - "+tr("options"));
 //create list of properties in this editor;
 list=new QStringList();
 //create property dictionary
 items=new QDict<Option>();
 //create labels dictionary
 labels=new QDict<QLabel>();

 QGridLayout* grl_up=new QGridLayout(this,2,1);
 grl_up->setRowStretch(0,1);
 grl_up->setRowStretch(1,0);

 //create tab widget
 tab=new QTabWidget(this,"option_tab");
 grl_up->addWidget(tab,0,0);
 //Bottom part
 QFrame *low=new QFrame(this);
 grl_up->addWidget(low,1,0);
 QGridLayout* grl=new QGridLayout(low,1,4);
 grl->setColStretch(0,10);
 grl->setColStretch(1,1);
 grl->setColStretch(2,1);
 grl->setColStretch(3,1);
 grl->setSpacing(16);
 grl->setMargin(8);
 grl->addWidget(new QLabel("Options",low),0,0);
 QPushButton* btOk=    new QPushButton(tr("&Ok"),low,"opt_ok");
 QPushButton* btApply= new QPushButton(tr("&Apply"),low,"opt_apply");
 QPushButton* btCancel=new QPushButton(tr("&Cancel"),low,"opt_cancel");
 grl->addWidget(btOk,0,1);
 grl->addWidget(btApply,0,2);
 grl->addWidget(btCancel,0,3);
 QObject::connect(btCancel, SIGNAL(clicked()), this, SLOT(close()));
 QObject::connect(btApply,  SIGNAL(clicked()), this, SLOT(apply()));
 QObject::connect(btOk,	    SIGNAL(clicked()), this, SLOT(ok()));
 printDbg(debug::DBG_DBG,"Options pre-init ...");
 init();
 printDbg(debug::DBG_DBG,"Options post-init ...");
}

/** Called on pushing 'Apply' button */
void OptionWindow::apply() {
 //save settings
 QDictIterator<Option> it(*items);
 for (;it.current();++it) {
  Option* c=it.current();
  printDbg(debug::DBG_DBG,"Writing " << c->getName());
  c->writeValue();
 }
 //Write settings to disk (editor may crash or be killed, right?)
 Settings::getInstance()->flushSettings();
}

/** Called on pushing 'OK' button */
void OptionWindow::ok() {
 apply();	//apply settings
 close();	//and close window
}

/** Add empty tab to option dialog
 @param name Label of the new tab
 @return Inner widget of the new tab
 */
QWidget* OptionWindow::addTab(const QString name) {
 QFrame* grid=new QFrame(tab);
 tab->addTab(grid,name);
 QGridLayout* grl=gridl[grid]=new QGridLayout(grid,1,2);
 nObjects[grid]=0;
 grl->setSpacing(5);
 grl->setMargin(8);
 //set key column to be fixed and value column to be expandable
 grl->setColStretch(0,0);
 grl->setColStretch(1,1);
 return grid;
}

/** add Option to the window
 @param otab Tab holding that option
 @param caption Label for this option
 @param prop Option to be added to this widget
 */
void OptionWindow::addOption(QWidget *otab,const QString &caption,Option *opt) {
 QString key=opt->getName();
 QLabel *label;
 label=new QLabel(caption,otab);
 int labelHeight=label->sizeHint().height();
 int optHeight=opt->sizeHint().height();
 int lineHeight=MAX(labelHeight,optHeight);
 gridl[otab]->setRowSpacing(nObjects[otab],lineHeight);
 gridl[otab]->addWidget(label,nObjects[otab],0);
 gridl[otab]->addWidget(opt,nObjects[otab],1);
 label->setFixedHeight(lineHeight);
 opt->setFixedHeight(lineHeight);
 nObjects[otab]++;
 list->append(key);
 items->insert(key,opt);
 labels->insert(caption,label);
 opt->readValue();
 opt->show();
 label->show();
}

/** add Option to the window (type of option is string)
 @param tab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 */
void OptionWindow::addOption(QWidget *otab,const QString &caption,const QString &key) {
 addOption(otab,caption,new Option(key,otab));
}

/** Initialize window with options */
void OptionWindow::init() {
 setUpdatesEnabled( FALSE );

 //TODO: this is just a stub
 //tab 1
 QWidget *tab1=addTab("tab1");
 addOption(tab1,tr("Icon Path"),"path/icon");
 addOption(tab1,"Dummy 1","dummy1");
// finishTab(tab1);

 //tab 2
 QWidget *tab2=addTab("tab2");
 addOption(tab2,"Dummy 2a","dummy2a");
 addOption(tab2,"Dummy 2b","dummy2b");
// finishTab(tab2);

 setUpdatesEnabled( TRUE );
}

/** The only Option window */
OptionWindow *opt=NULL;

/** This is called on attempt to close window. */
void OptionWindow::closeEvent(QCloseEvent *e) {
 delete this;
}

/** default destructor */
OptionWindow::~OptionWindow() {
 delete labels;
 delete items;
 delete list;
 opt=NULL;
 printDbg(debug::DBG_DBG,"Options closing ...");
}
