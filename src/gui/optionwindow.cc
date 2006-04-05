/** @file
OptionWindow - widget for editing program options
*/

#include "optionwindow.h"
#include <utils/debug.h>
#include "util.h"
#include "stringoption.h"
#include "realoption.h"
#include "booloption.h"
#include "intoption.h"
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
 Settings::getInstance()->restoreWindow(this,"options"); 
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
 @param opt Option to be added to this widget
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

/** add any widget to option (typically some label)
 The widget will take one line
 @param otab Tab holding that option
 @param elem element to add
 */
void OptionWindow::addWidget(QWidget *otab,QWidget *elem) {
// int lineHeight=elem->sizeHint().height();
// gridl[otab]->setRowSpacing(nObjects[otab],lineHeight);
 gridl[otab]->addMultiCellWidget(elem,nObjects[otab],nObjects[otab],0,1);
// elem->setFixedHeight(lineHeight);
 nObjects[otab]++;
 elem->show();
}

/** add description text to option 
 @param otab Tab holding that option
 @param text Text to add. Using rich text format, so basically you can use HTML here
 */
void OptionWindow::addText(QWidget *otab,const QString &text) {
 QLabel *txt=new QLabel(text,otab);
 txt->setTextFormat(RichText);
 addWidget(otab,txt);
}

/** Add padding to pad the tab window
 @param otab Tab holding that option
 */
void OptionWindow::finishTab(QWidget *otab) {
 QFrame *pad=new QFrame(otab);
 pad->setMinimumHeight(1);
 pad->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);
 addWidget(otab,pad);
}

/** add Option to the window (type of option is string)
 @param tab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 */
void OptionWindow::addOption(QWidget *otab,const QString &caption,const QString &key) {
 addOption(otab,caption,new StringOption(key,otab));
}

/** add Option to the window (type of option is float)
 @param tab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 */
void OptionWindow::addOptionFloat(QWidget *otab,const QString &caption,const QString &key) {
 addOption(otab,caption,new RealOption(key,otab));
}

/** add Option to the window (type of option is bool)
 @param tab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 */
void OptionWindow::addOptionBool(QWidget *otab,const QString &caption,const QString &key) {
 addOption(otab,caption,new BoolOption(key,otab));
}

/** add Option to the window (type of option is int)
 @param tab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 */
void OptionWindow::addOptionInt(QWidget *otab,const QString &caption,const QString &key) {
 addOption(otab,caption,new IntOption(key,otab));
}

/** Initialize window with options */
void OptionWindow::init() {
 setUpdatesEnabled( FALSE );

 QWidget *edit_tab=addTab(tr("Editor"));
 addOptionBool(edit_tab,tr("Advanced mode"),"mode/advanced");
 addText(edit_tab,tr("Turning advanced mode on will allow more powerful (but also potentially more destructive) changes to edited document."));
 addText(edit_tab,tr("<b>Note</b>: this will affect only newly opened files"));
 finishTab(edit_tab);

 QWidget *data_tab=addTab(tr("Data files"));
 addText(data_tab,tr("Paths to data files"));
 addOption(data_tab,tr("Icon Path"),"path/icon");
 addText(data_tab,tr("<b>Note</b>: this will take effect on next program start"));//TODO: apply path now
 finishTab(data_tab);

 QWidget *tree_tab=addTab(tr("Tree View"));
 addText(tree_tab,tr("You can specify what kind of objects will be displayed in the tree view"));
 addOptionBool(tree_tab,tr("Document dictionary"),"mode/show_dict");
 addOptionBool(tree_tab,tr("Object dictionaries"),"mode/show_objdict");
 addOptionBool(tree_tab,tr("Outlines"),"mode/show_outline");
 addOptionBool(tree_tab,tr("Graphic objects"),"mode/show_graphic");
 addOptionBool(tree_tab,tr("Annotations"),"mode/show_annot");
 addOptionBool(tree_tab,tr("Pages"),"mode/show_page");
 addOptionBool(tree_tab,tr("Simple Objects"),"mode/show_simple");
 finishTab(tree_tab);

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
 Settings::getInstance()->saveWindow(this,"options"); 
 delete labels;
 delete items;
 delete list;
 opt=NULL;
 printDbg(debug::DBG_DBG,"Options closing ...");
}
