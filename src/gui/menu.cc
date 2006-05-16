/** @file
 This class manages Menu, Toolbars and Shortcuts
 Menus, menu items, toolbars and toolbar items share the same namespace and often
 menu and toolbar items are interchangable (only difference is that toolbar item
 must have icon, while menu item can be without an icon)
 For more information about menus and toolbars, check doc/design/gui/menu.xml
 @author Martin Petricek
*/

#include <utils/debug.h>
#include <qfile.h>
#include "menu.h"
#include "toolbutton.h"
#include "util.h"
#include "settings.h"
#include <qtextstream.h> 
#include <qmenudata.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qpixmap.h>
#include "toolbar.h"
#include <qstring.h>
#include "iconcache.h"

namespace gui {

using namespace util;

/** root key for system menu items */
const QString ROOT="gui/items/";
/** root key for user menu items appended to system menu items (lists) */
const QString ROOT_ADD="gui/items_add/";

/** 
 Exit with error message after encountering invalid menu/toolbar item
 @param type Type of offending item
 @param name Name of offending item
 @param line Line with offending item
 @param expected Optional "Expected" field hinting what is expected to make data valid
*/
void Menu::invalidItem(const QString &type,const QString &name,const QString &line,const QString &expected/*=QString::null*/) throw (InvalidMenuException) {
 QString err;
 err=QObject::tr("Invalid %1 in config:\nName: %2\nData: %3\n").arg(type,name,line);
 if (!expected.isNull()) err+="\n"+QObject::tr("Expected: ")+expected;
 throw InvalidMenuException(err);
}

/**
 Contructor of menu system for one specific window
 @param _main Main application window
 */
Menu::Menu(QMainWindow *_main) {
 main=_main;
 action_index=1;
 cache=new IconCache();
}

/** Destructor */
Menu::~Menu() {
 delete cache;
}

/**
 Adds action to menu, returning newly allocated menu Id or existing menu id if action is already present
 @param action Name of action
 @return Menu ID of the specified action
*/
int Menu::addAction(const QString &action) {
 if (action_map.contains(action)) return action_map[action];
 action_index++;
 action_map[action]=action_index;
 action_map_i[action_index]=action;
 return action_index;
}

/**
 Return action string from given menu ID
 @param index Menu ID of action
 @return Name of the specified action
*/
QString Menu::getAction(int index) {
 return action_map_i[index];
}

/**
 Load one GUI item from config file, exiting application with fatal error if item not found 
 @param name Name of the item to read
 @return line from config file  
 */
QString Menu::readItem(const QString &name) throw (InvalidMenuException) {
 QString line=globalSettings->read(ROOT+name);
 if (line.isNull()) throw InvalidMenuException(QObject::tr("Missing item in config")+":\n"+ROOT+name);
 line=line.simplifyWhiteSpace();
 if (line.length()==0) throw InvalidMenuException(QObject::tr("Missing item in config")+":\n"+ROOT+name);
 if (isList(line)) { //Read user-specified additional items and append them if this is a list
  QString add=globalSettings->read(ROOT_ADD+name);
  if (!add.isNull()) { 
   add=add.simplifyWhiteSpace();
   line+=",";
   line+=add;
  }
 }
 return line; 
}

/**
 Check if menu item with given definition is a list
 @param line Definition of the item to check
 @return true if menu is a list
 */
bool Menu::isList(const QString &line) {
 return (line.startsWith("list "));
}

/**
 Load all needed items into existing menu list
 @param line comma separated list of childs
 @param menu menu in which to load items (menu bar or popup menu)
 @param prev String list containing names of all parents of this menu item (used for loop detection)
*/
void Menu::loadItemsDef(QString line,QMenuData *menu,QStringList prev/*=QStringList()*/) throw (InvalidMenuException) {
 QStringList qs=explode(MENULIST_SEPARATOR,line);
 QStringList::Iterator it=qs.begin();
 for (;it!=qs.end();++it) { //load all subitems
  loadItem(*it,menu,prev);
 }
}

/**
 Parse name of list or item from line.
 List is then modified to contain only list of its items, or in case if item the rest of parameters
 @param line Line with menu list definition
 @param name Name (key) of this menu item (for locatization)
 @return localized caption of the menu item
*/
QString Menu::parseName(QString &line, const QString &name/*=QString::null*/) throw (InvalidMenuException) {
 QString caption;
 if (line.startsWith("item ")) {
  line=line.remove(0,5);	//String "item "
  caption=getUntil(MENUDEF_SEPARATOR,line,true);
 } else if (line.startsWith("list ")) {
  line=line.remove(0,5);	//String "list "
  caption=getUntil(MENULIST_SEPARATOR,line);
 } else {
  invalidItem(QObject::tr("menu definition"),name,line);
 }
 QString menuName;
 if (name.isNull()) {
  menuName=Settings::tr(caption);
 } else {
  menuName=Settings::tr(caption,name);
 }
 return menuName;
}

/**
 Load all needed items into existing menu list
 @param name name of item to be loaded from config file
 @param menu menu in which to load items (menu bar or popup menu)
 @param prev String list containing names of all parents of this menu item (used for loop detection)
*/
void Menu::loadItems(const QString &name,QMenuData *menu,QStringList prev/*=QStringList()*/) throw (InvalidMenuException) {
 QString line=readItem(name);
 if (isList(line)) {
  parseName(line,name);//Name ignored
  loadItemsDef(line,menu,prev);
 } else { //We expect a list here
  invalidItem(QObject::tr("menu definition"),name,line,QObject::tr("Menu items list"));
 }
}

/**
 Load one menu item and insert it into parent menu. Recursively load subitems if item is a submenu.
 @param name name of item to be loaded from config file
 @param parent parent menu item. If NULL and the item is list, the item is loaded in cache, but not added anywhere
 @param prev String list containing names of all parents of this menu item (used for loop detection)
 */ 
void Menu::loadItem(const QString &name,QMenuData *parent/*=NULL*/,QStringList prev/*=QStringList()*/) throw (InvalidMenuException) {
 //Check for cycles (unhandled cycle in menu = crash in QT)
 if (prev.contains(name)) {
  throw InvalidMenuException(QObject::tr("Cycle in menu detected")+":\n"+prev.join(" > ")+" > "+name);
 }
 prev+=name;
 //Check Menu cache
 //There are currently some problems with the cache, so it is turned off
/*
 if (mCache.contains(name)) {
  //This item is already in cache, insert it in parent and return
  guiPrintDbg(debug::DBG_DBG,"MENU - Fetching from cache :" << name);
  if (parent) parent->insertItem(mCacheName[name],mCache[name]);
  return;
 }
*/
 //Separator requested - insert it and return
 if (name=="-" || name=="") {
  if (parent) parent->insertSeparator();
  return;
 }
 //Load items into list
 QString line=readItem(name);
 if (isList(line)) { // List of values - a submenu, first is name of submenu, others are items in it
  QPopupMenu *item=new QPopupMenu();
  QString menuName=parseName(line,name);
  loadItemsDef(line,item,prev);
  if (parent) parent->insertItem(menuName,item);
  //Add this item to cache
  mCache[name]=item;
  mCacheName[name]=menuName;
 } else if (line.startsWith("item ")) { // A single item
  if (parent) addItem(line,parent,name);
 } else { //something invalid
  invalidItem(QObject::tr("menu item"),name,line,"list | item");
 } 
}

/** Add menu item specified by given data to parent
 @param line Line containing menu item specification
 @param parent parent menu in which this item will be appended
 @param name Name of this item (key in settings).
 */
void Menu::addItem(QString line,QMenuData *parent,const QString &name/*=QString::null*/) throw (InvalidMenuException) {
 line=line.remove(0,5);
 //Format: Caption, Action,[,accelerator, [,menu icon]]
 QStringList qs=explode(MENUDEF_SEPARATOR,line,true);
 if (qs.count()<2) invalidItem(QObject::tr("menu item"),name,line,QObject::tr("2 or more parameters in item definition"));
 int menu_id=addAction(qs[1]);
 if (name.isNull()) {
  qs[0]=Settings::tr(qs[0]);
 } else {
  qs[0]=Settings::tr(qs[0],name);
 }
 parent->insertItem(qs[0],menu_id);
 if (qs.count()>=3 && qs[2].length()>0) { //accelerator specified
  if (reserveAccel(qs[2],qs[1])) parent->setAccel(QKeySequence(qs[2]),menu_id);
 }
 if (qs.count()>=4 && qs[3].length()>0) { //menu icon specified
  const QIconSet *icon=cache->getIconSet(qs[3]);
  if (icon) {
   parent->changeItem(menu_id,*icon,qs[0]);
  } else {
   guiPrintDbg(debug::DBG_WARN, "Icon missing: " << qs[3]);
  }
 }
}

/** Loads menubar from configuration bar, and return it<br>
 If menubar can't be loaded, InvalidMenuException will be thrown
 Missing menu icons are allowed (if it can't be loaded, there will be no pixmap), missing items in configuration are not.<br>
 @param parent QWidget that will contain the menubar
 @return loaded and initialized menubar
*/
QMenuBar* Menu::loadMenu(QWidget *parent) throw (InvalidMenuException) {
 //menubar can't be cached and must be separate for each window (otherwise weird things happen)
 QMenuBar *menubar=new QMenuBar(parent);//Make new menubar
 loadItems(QString("MainMenu"),menubar);//create root menu
 return menubar;
}

/** Add accelerator to list of used accelerators and return true, if the accelerator was not taken before.
 Ensure only one item uses the accelerator at time
 @param accelDef Accelerator in string form
 @param action Action to reserve for this accelerator
 @return true if accelerator was not in list before, false otherwise
*/
bool Menu::reserveAccel(const QString &accelDef,const QString &action) {
 if (accels.contains(accelDef)) {
  if (accels[accelDef]!=action) {
   guiPrintDbg(debug::DBG_WARN,"Attempt to redefine accel " << accelDef << " from '" << accels[accelDef] << "' to '" << action << "'");
  }
  return false;
 }
 accels[accelDef]=action;
 return true;
}

/**
 Try to remove command string from line.
 Return true, if command was found and removed from line,
 false if command was not found (and line is unchanged)
 @param line input line
 @param command Command to look for
 @return presence of command in line
*/
bool Menu::chopCommand(QString &line, const QString &command) {
 if (!line.startsWith(command+" ")) return false;
 line=line.remove(0,command.length()+1);
 return true;
}

/**
 Load single toolbar item and add it to toolbar
 @param tb Toolbar for addition of item
 @param item Item name in configuration file
*/
void Menu::loadToolBarItem(ToolBar *tb,const QString &item) throw (InvalidMenuException) {
 //Check for special item
 if (ToolBar::specialItem(tb,item,main)) return;
 QString line=readItem(item);
 if (chopCommand(line,"item")) { //Format: Tooltip, Action,[,accelerator, [,icon]]
  QStringList qs=explode(MENUDEF_SEPARATOR,line,true);
  if (qs.count()<4) invalidItem(QObject::tr("toolbar item"),item,line,QObject::tr("4 parameters in item definition"));
//  line=line.remove(0,5);
  const QIconSet *icon=cache->getIconSet(qs[3]);
  int menu_id=addAction(qs[1]);
  if (!icon) {
   guiPrintDbg(debug::DBG_WARN, "Icon missing: " << qs[3]);
  }
  QString tooltip=Settings::tr(qs[0],item);
  tooltip=tooltip.replace("&","");
  if (qs[2].length()>0) { //accelerator specified
   tooltip+=" ("+qs[2]+")";
  }
  ToolButton *tbutton =new ToolButton(icon,tooltip,menu_id,tb);
  if (qs[2].length()>0) { //accelerator specified
   if (reserveAccel(qs[2],qs[1])) tbutton->setAccel(QKeySequence(qs[2]));
  }
  tb->addButton(tbutton);
  tbutton->show();
 } else if (chopCommand(line,"label")) { //Format: Text
  if (!line.length()) invalidItem(QObject::tr("label item"),item,line);
  new QLabel(Settings::tr(line,item),tb);
 } else {
  invalidItem(QObject::tr("toolbar item"),item,line);
 }
}

/**
 Returns icon with given name from cache (loading if necessary)
 @param name Name of icon to get
 @return Pixmap containing specified icon
*/
const QPixmap* Menu::getIcon(const QString &name) {
 return cache->getIcon(name);
}

/**
 Returns icon set with given name from cache (creating if necessary)
 @param name Name of icon set to get
 @return specified Icon set
*/
const QIconSet* Menu::getIconSet(const QString &name) {
 return cache->getIconSet(name);
}

/**
 Load single toolbar from configuration file
 @param name Toolbar name in configuration file
 @param visible Will be toolbar initially visible?
 @return loaded toolbar
*/
ToolBar* Menu::loadToolbar(const QString &name,bool visible/*=true*/) throw (InvalidMenuException) {
 QString line=readItem(name);
 guiPrintDbg(debug::DBG_INFO,"Loading toolbar:" << name);
 if (!isList(line)) { // List of values - first is name, others are items in it
  invalidItem(QObject::tr("toolbar definition"),name,line,"list");
  return NULL;
 }
 QString tbName=parseName(line,name);
 ToolBar *tb=new ToolBar(tbName,main);
 tb->setName(name);
 QStringList qs=explode(MENULIST_SEPARATOR,line);
 QStringList::Iterator it=qs.begin();
 for (;it!=qs.end();++it) { //load all subitems
  loadToolBarItem(tb,*it);
 }
 if (visible) tb->show();
  else        tb->hide();
 return tb;
}

/**
 Load all toolbars from configuration files and add them to parent window
 If toolbar can't be loaded, InvalidMenuException will be thrown
*/
ToolBarList Menu::loadToolBars() throw (InvalidMenuException) {
 QString line=globalSettings->read("gui/toolbars");
 toolbarNames=explode(TOOLBARLIST_SEPARATOR,line);
 bool visible;
 for (unsigned int i=0;i<toolbarNames.count();i++) {
  visible=globalSettings->readBool(QString("toolbar/")+toolbarNames[i],true);
  toolbarList[toolbarNames[i]]=loadToolbar(toolbarNames[i],visible);
 }
 return toolbarList;
}

/**
 Return list of toolbar names
 @return Toolbar list
*/
QStringList Menu::getToolbarList() {
 return toolbarNames;
}

/**
 return toolbar with given name.
 @return Toolbar, or NULL if toolbar with given name was not found
*/
ToolBar* Menu::getToolbar(const QString &name) {
 if (!toolbarList.contains(name)) return NULL;
 return toolbarList[name];
}

/**
 Save toolbar state of QMainWindow to configuration
*/
void Menu::saveToolbars() {
 QString out;
 QTextStream qs(out,IO_WriteOnly);
 qs << *main;
 globalSettings->write("gui/toolbarpos",out);
} 

/**
 Restore toolbar state of QMainWindow from configuration
*/
void Menu::restoreToolbars() {
 QString out=globalSettings->read("gui/toolbarpos");
 if (out.isNull()) return;
 QTextStream qs(out,IO_ReadOnly);
 qs >> *main;
} 

} // namespace gui
