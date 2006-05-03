/** @file
 OptionWindow - widget for editing program options
 Options are arranged to tabs and it is ensured, that only one dialog at once is active
 (via Private constructor and static method to invoke the dialog, which will focus on
 existing dialog if it exists, insetad of creating second one) 
 @author Martin Petricek
*/

#include "optionwindow.h"
#include <qlabel.h>
#include <qtabwidget.h> 
#include <qlayout.h>
#include <utils/debug.h>
#include "util.h"
#include "stringoption.h"
#include "realoption.h"
#include "combooption.h"
#include "booloption.h"
#include "intoption.h"
#include <qpoint.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qstylefactory.h>
#include <qapplication.h>
#include <qlayout.h>
#include "option.h"
#include "version.h"
#include "settings.h"
#include "option.h"
#include "menu.h"
#include "toolbar.h"

namespace gui {

using namespace std;

/** The only Option window - 
 Pointer to running OptionWindow instance or NULL if none active.
 */
OptionWindow *opt=NULL;

/** invoke option dialog. Ensure only one copy is running at time
@param msystem Menu system to 
*/
void OptionWindow::optionsDialog(Menu *msystem) {
 if (opt) { //the dialog is already active
  opt->setActiveWindow();
 } else { //create new dialog
  opt=new OptionWindow(msystem);
  opt->show();
 }
}

/** Default constructor of option window.
 The window is initially empty
 @param msystem Menu system (Needed for toolbar list)
 @param parent parent widget containing this control
 @param name name of widget (currently unused)
 */
OptionWindow::OptionWindow(Menu *msystem,QWidget *parent /*=0*/, const char *name /*=0*/) : QWidget(parent,name,WDestructiveClose || WType_TopLevel) {
 guiPrintDbg(debug::DBG_DBG,"Options creating ...");
 menuSystem=msystem;
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
 grl->addWidget(new QLabel(CONFIG_DIR,low),0,0);
 QPushButton* btOk=    new QPushButton(QObject::tr("&Ok"),low,"opt_ok");
 QPushButton* btApply= new QPushButton(QObject::tr("&Apply"),low,"opt_apply");
 QPushButton* btCancel=new QPushButton(QObject::tr("&Cancel"),low,"opt_cancel");
 grl->addWidget(btOk,0,1);
 grl->addWidget(btApply,0,2);
 grl->addWidget(btCancel,0,3);
 QObject::connect(btCancel, SIGNAL(clicked()), this, SLOT(close()));
 QObject::connect(btApply,  SIGNAL(clicked()), this, SLOT(apply()));
 QObject::connect(btOk,	    SIGNAL(clicked()), this, SLOT(ok()));
 globalSettings->restoreWindow(this,"options"); 
 init();
}

/** Called on pushing 'Apply' button */
void OptionWindow::apply() {
 //save settings
 QDictIterator<Option> it(*items);
 for (;it.current();++it) {
  Option* c=it.current();
  guiPrintDbg(debug::DBG_DBG,"Writing " << c->getName());
  c->writeValue();
 }
 //Write settings to disk (editor may crash or be killed, right?)
 globalSettings->flushSettings();
 applyLookAndFeel(true);
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
 @param otab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 @param defValue Default value if option not found in configuration
 */
void OptionWindow::addOption(QWidget *otab,const QString &caption,const QString &key,const QString &defValue/*=QString::null*/) {
 addOption(otab,caption,new StringOption(key,otab,defValue));
}

/** add Option to the window (type of option is string, edited by combobox)
 @param otab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 @param values List of allowed values for this combobox
 */
void OptionWindow::addOptionCombo(QWidget *otab,const QString &caption,const QString &key,const QStringList &values) {
 addOption(otab,caption,new ComboOption(values,key,otab));
}

/** add Option to the window (type of option is float)
 @param otab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 */
void OptionWindow::addOptionFloat(QWidget *otab,const QString &caption,const QString &key) {
 addOption(otab,caption,new RealOption(key,otab));
}

/** add Option to the window (type of option is bool)
 @param otab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 @param defValue Default value if option not found in configuration
 */
void OptionWindow::addOptionBool(QWidget *otab,const QString &caption,const QString &key,bool defValue/*=false*/) {
 addOption(otab,caption,new BoolOption(key,otab,defValue));
}

/** add Option to the window (type of option is int)
 @param otab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 @param defValue Default value if option not found in configuration
 */
void OptionWindow::addOptionInt(QWidget *otab,const QString &caption,const QString &key,int defValue/*=0*/) {
 addOption(otab,caption,new IntOption(key,otab,defValue));
}

/** Initialize window with options */
void OptionWindow::init() {
 setUpdatesEnabled( FALSE );

 QWidget *edit_tab=addTab(tr("Editor"));
 addOptionBool(edit_tab,tr("Advanced mode"),"mode/advanced");
 addText      (edit_tab,tr("Turning on advanced mode will allow more powerful (but also more dangerous) changes to edited document."));
 addText      (edit_tab,tr("<b>Note</b>: changing Advanced mode will affect only newly opened files"));
 finishTab    (edit_tab);

 QWidget *data_tab=addTab(tr("Paths"));
 addText  (data_tab,tr("You can use environment variables (for example $HOME) in settings on this page"));
 addOption(data_tab,tr("Icon Path"),"path/icon");
 addText  (data_tab,tr("<b>Note</b>: changing Icon Path will take effect on next program start"));//TODO: apply path now
 addOption(data_tab,tr("Console log file"),"path/console_log");
 finishTab(data_tab);

 QWidget *tree_tab=addTab(tr("Object tree"));
 addText      (tree_tab,tr("These objects will be shown in the object tree"));
 addOptionBool(tree_tab,tr("Document dictionary"),"tree/show_dict");
 addOptionBool(tree_tab,tr("Object dictionaries"),"tree/show_objdict");
 addOptionBool(tree_tab,tr("Outlines"),"tree/show_outline");
 addOptionBool(tree_tab,tr("Graphic objects"),"tree/show_graphic");
 addOptionBool(tree_tab,tr("Annotations"),"tree/show_annot");
 addOptionBool(tree_tab,tr("Pages"),"tree/show_page");
 addOptionBool(tree_tab,tr("Simple Objects"),"tree/show_simple");
 finishTab    (tree_tab);

 QWidget *misc_tab=addTab(tr("Misc"));
 addOptionBool(misc_tab,tr("Show return value of executed scripts in console"),"console/showretvalue");
 addText      (misc_tab,tr("<br>History-related options"));
 addOption    (misc_tab,tr("History file"),"gui/CommandLine/HistoryFile");
 addOptionInt (misc_tab,tr("Max. lines in history"),"gui/CommandLine/HistorySize");
 addOptionBool(misc_tab,tr("Remember path of last opened/saved file"),"history/save_filePath",true);
 finishTab    (misc_tab);

 QWidget *tool_tab=addTab(tr("Toolbars"));
 addText(tool_tab,tr("These toolbars will be shown:"));
 QStringList tbs=menuSystem->getToolbarList();
 for (unsigned int i=0;i<tbs.count();i++) {
  ToolBar* tb=menuSystem->getToolbar(tbs[i]);
  if (!tb) continue; //Someone put invalid toolbar in settings. Just ignore it
  addOptionBool(tool_tab,tb->label(),QString("toolbar/")+tbs[i],true);
 }
 finishTab(tool_tab);

 QWidget *laf_tab=addTab(tr("Look and Feel"));
 QStringList styles=QStyleFactory::keys();
 styles.prepend("");
 addText       (laf_tab,tr("You can set parameters of application font"));
 addOption     (laf_tab,tr("Font family"),"gui/font",QApplication::font().family());
 addOptionInt  (laf_tab,tr("Font size"),"gui/fontsize",QApplication::font().pointSize());
 addOptionBool (laf_tab,tr("Bold"),"gui/fontbold",QApplication::font().weight()>QFont::DemiBold);
 addOptionBool (laf_tab,tr("Italic"),"gui/fontitalic",QApplication::font().italic());
 addText       (laf_tab,tr("You can specify overall visual style"));
 addOptionCombo(laf_tab,tr("Style"),"gui/style",styles);
 addText       (laf_tab,tr("<b>Note</b>: changing style will take effect on next program start"));//TODO: apply style now
 finishTab     (laf_tab);

 setUpdatesEnabled( TRUE );
}


/** This is called on attempt to close window. */
void OptionWindow::closeEvent(__attribute__((unused)) QCloseEvent *e) {
 delete this;
}

/** default destructor */
OptionWindow::~OptionWindow() {
 globalSettings->saveWindow(this,"options"); 
 delete labels;
 delete items;
 delete list;
 opt=NULL;//No instance active now
 guiPrintDbg(debug::DBG_DBG,"Options closing ...");
}

/** applies look and feel settings from options
 @param notify If true, all fonts will be immediately changed in all widgets
 */
void applyLookAndFeel(bool notify) {
 QString font=globalSettings->read("gui/font",QApplication::font().family());
 int fontsize=globalSettings->readNum("gui/fontsize",QApplication::font().pointSize());
 if (fontsize==-1) fontsize=12;
 bool bold=globalSettings->readBool("gui/fontbold");
 bool italic=globalSettings->readBool("gui/fontitalic");
 QApplication::setFont(QFont(font,fontsize,bold?(QFont::Bold):(QFont::Normal),italic),notify);
}

} // namespace gui
