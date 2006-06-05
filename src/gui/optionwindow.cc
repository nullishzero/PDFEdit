/** @file
 OptionWindow - widget for editing program options
 Options are arranged to tabs and it is ensured, that only one dialog at once is active
 (via Private constructor and static method to invoke the dialog, which will focus on
 existing dialog if it exists, instead of creating second one) 
 @author Martin Petricek
*/

#include "optionwindow.h"
#include <qdir.h>
#include <qlabel.h>
#include <qtabwidget.h> 
#include <qlayout.h>
#include <utils/debug.h>
#include "util.h"
#include "stringoption.h"
#include "realoption.h"
#include "fileoption.h"
#include "fontoption.h"
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
#include "config.h"
#include <assert.h>

namespace gui {

using namespace std;

/**
 The only Option window<br>
 Pointer to running OptionWindow instance or NULL if none active.
*/
OptionWindow *opt=NULL;

/**
 Invoke option dialog. Ensure only one copy is running at time
 @param msystem Menu system reference for given option window (needed to get toolbar list)
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
 grl->addWidget(new QLabel("~/" CONFIG_DIR "/pdfeditrc",low),0,0);
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

/**
 Add empty tab to option dialog
 @param name Label of the new tab
 @param makeSegments Make it possible to split tab in parts, at cost of using an extra widget
 @return Inner widget of the new tab
 */
QWidget* OptionWindow::addTab(const QString name,bool makeSegments/*=false*/) {
 QFrame* grid;
 if (makeSegments) {
  QFrame* mGrid=new QFrame(tab);
  tab->addTab(mGrid,name);
  QGridLayout* mGridLayout=gridl[mGrid]=new QGridLayout(mGrid,1,1);
  grid=new QFrame(mGrid);
  masterGrid[grid]=mGrid;
  mGridLayout->addWidget(grid,0,0);
  nObjects[mGrid]=1;
 } else {
  grid=new QFrame(tab);
  tab->addTab(grid,name);
 }
 initGridFrame(grid);
 return grid;
}

/**
 Initialize grid for tab widget, so controls can be added there.
 @param grid Widget to initialize with grid layout
*/
void OptionWindow::initGridFrame(QWidget *grid) {
 QGridLayout* grl=gridl[grid]=new QGridLayout(grid,1,2);
 nObjects[grid]=0;
 grl->setSpacing(5);
 grl->setMargin(8);
 //set key column to be fixed and value column to be expandable
 grl->setColStretch(0,0);
 grl->setColStretch(1,1);
}

/**
 Add break to the option tab, breaking the column alignment at this point.<br>
 In fact, break the tab into two separate parts (but only second part can be splitted again).<br>
 Can be only applied to tabs that were created with makeSegments=true
 \see addTab
 @param otab Tab to break
 @return new value for tab - the lower half
*/
QWidget* OptionWindow::addBreak(QWidget *otab) {
 QFrame* mGrid=masterGrid[otab];
 assert(mGrid);
 QFrame* grid=new QFrame(mGrid);
 gridl[mGrid]->addWidget(grid,nObjects[mGrid],0);
 nObjects[mGrid]++;
 initGridFrame(grid);
 masterGrid.remove(otab);
 masterGrid[grid]=mGrid;
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

/**
 add Option to the window (type of option is file)
 @param otab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 @param defValue Default value if option not found in configuration
 */
void OptionWindow::addOptionFile(QWidget *otab,const QString &caption,const QString &key,const QString &defValue/*=QString::null*/) {
 addOption(otab,caption,new FileOption(key,otab,defValue));
}

/**
 add Option to the window (type of option is font)
 @param otab Tab holding that option
 @param caption Label for this option
 @param key Key of the given option
 @param defValue Default value if option not found in configuration
 */
void OptionWindow::addOptionFont(QWidget *otab,const QString &caption,const QString &key,const QString &defValue/*=QString::null*/) {
 addOption(otab,caption,new FontOption(key,otab,defValue));
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
 addOptionBool(edit_tab,tr("Show hidden properties"),"editor/show_hidden");
 addOptionBool(edit_tab,tr("Allow editing read-only properties"),"editor/edit_readonly");
 addOptionBool(edit_tab,tr("Remember path of last opened/saved file"),"history/save_filePath",true);
 finishTab    (edit_tab);

 QWidget *data_tab=addTab(tr("Paths"));
 addText  (data_tab,tr("You can use environment variables (for example $HOME) in settings on this page"));
 addOption(data_tab,tr("Icon Path"),"path/icon");
 addText  (data_tab,tr("<b>Note</b>: changing Icon Path will take effect on next program start"));//TODO: apply path now
 addOption(data_tab,tr("Script Path"),"path/script");
 addOption(data_tab,tr("Scripts to run on start"),"script/init");
 addOptionFile(data_tab,tr("Console log file"),"path/console_log");
 finishTab(data_tab);

 QWidget *tree_tab=addTab(tr("Object tree"));
 addText      (tree_tab,tr("These objects will be shown in the object tree"));
 addOptionBool(tree_tab,tr("Document dictionary"),"tree/show_dict",true);
 addOptionBool(tree_tab,tr("Object dictionaries"),"tree/show_objdict",true);
 addOptionBool(tree_tab,tr("Graphic objects"),"tree/show_graphic",true);
 addOptionBool(tree_tab,tr("Annotations"),"tree/show_annot",true);
 addOptionBool(tree_tab,tr("Pages"),"tree/show_page",true);
 addOptionBool(tree_tab,tr("Outlines"),"tree/show_outline",true);
 addOptionBool(tree_tab,tr("Content Streams"),"tree/show_stream",true);
 addOptionBool(tree_tab,tr("Simple Objects"),"tree/show_simple",true);
 addText      (tree_tab,"<br>"+tr("Options affecting object tree and property editor"));
 addOptionBool(tree_tab,tr("Sort dictionary items alphabetically"),"tree/show_dict_sort");
 finishTab    (tree_tab);

 QWidget *misc_tab=addTab(tr("Commandline"),true);
 addText      (misc_tab,tr("Commandline options"));
 addOptionBool(misc_tab,tr("Show return value of executed scripts in console"),"console/showretvalue");
 addOptionBool(misc_tab,tr("Show errors from event handlers"),"console/show_handler_errors");
 addOptionBool(misc_tab,tr("Show command editor"),"gui/CommandLine/CmdShowEditor");
 addOptionBool(misc_tab,tr("Show command line"),"gui/CommandLine/CmdShowHistory");
 addOptionBool(misc_tab,tr("Editable command line"),"gui/CommandLine/CmdShowLine");
 misc_tab=addBreak(misc_tab);
 addText      (misc_tab,"<br>"+tr("History options"));
 addOptionFile(misc_tab,tr("History file"),"gui/CommandLine/HistoryFile");
 addOptionInt (misc_tab,tr("Max. lines in history"),"gui/CommandLine/HistorySize");

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
 //Get list of styles
 QStringList styles=QStyleFactory::keys();
 styles.prepend("");
 //Get list of icon themes
 QStringList iconPath=globalSettings->readPath("icon");
 QStringList iconThemes;
 for (unsigned int pth=0;pth<iconPath.count();pth++) {
  QDir dir(iconPath[pth]);
  if (dir.isReadable()) {
   iconThemes+=dir.entryList(QDir::Dirs);
  }
 }
 //Remove . and .. directories
 iconThemes.remove(".");
 iconThemes.remove("..");
 //theme cannot be named 'default'
 iconThemes.remove("default");
 //Sort list
 qHeapSort(iconThemes);
 //Remove duplicates
 QString lastTheme;
 QStringList iconThemesUnique;
 for (QStringList::Iterator it=iconThemes.begin();it!=iconThemes.end();++it) {
  if (*it!=lastTheme) {
   lastTheme=*it;
   iconThemesUnique+=lastTheme;
  }
 }
 //prepend 'default' to list
 iconThemesUnique.prepend("default");
 addText       (laf_tab,tr("You can set fonts used in application"));
 addOptionFont (laf_tab,tr("Application font"),"gui/font_main",QApplication::font().toString());
 addOptionFont (laf_tab,tr("Console font"),"gui/font_con",QApplication::font().toString());
 addText       (laf_tab,tr("You can specify overall visual style"));
 addOptionCombo(laf_tab,tr("Style"),"gui/style",styles);
 addOptionCombo(laf_tab,tr("Icon theme"),"icon/theme/current",iconThemesUnique);
//TODO: scan icon directories and pick the theme from a combobox
 addText       (laf_tab,tr("<b>Note</b>: changing style or icon theme will take effect on next program start"));//TODO: apply style & icon theme now
 addOptionBool (laf_tab,tr("Use big icons"),"icon/theme/big");
 finishTab     (laf_tab);

 setUpdatesEnabled( TRUE );
}


/**
 This is called on attempt to close window.
 The window will delete itself
 @param e Close event (unused)
*/
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
 QFont fontMain=QApplication::font();
 fontMain.fromString(globalSettings->read("gui/font_main",QApplication::font().toString()));
 QFont fontConsole=QApplication::font();
 fontConsole.fromString(globalSettings->read("gui/font_con",QApplication::font().toString()));
 QApplication::setFont(fontMain,notify);
 QApplication::setFont(fontConsole,notify,"gui::CommandWindow");
}

} // namespace gui
