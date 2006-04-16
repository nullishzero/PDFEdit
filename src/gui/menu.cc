/** @file
 This class manages Menu, Toolbars and Shortcuts
 Menus, menu items, toolbars and toolbar items share the same namespace and often
 menu and toolbar items are interchangable (only difference is that toolbar item
 must have icon, while menu item can be without an icon)
*/

#include <utils/debug.h>
#include <qfile.h>
#include "toolbutton.h"
#include "menu.h"
#include "util.h"
#include "settings.h"
#include <qtextstream.h> 
#include <qmenudata.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include "toolbar.h"
#include <qstringlist.h> 
#include <qstring.h>

namespace gui {

/** Default constructor */
Menu::Menu() {
 action_index=1;
 initPaths();
}

/** Destructor */
Menu::~Menu() {
}

/** Given name of the icon, finds and returns full path to the icon,
     considering all relevant settings (icon path ..) */
QString Menu::getIconFile(const QString &name) {
 if (name.startsWith("/")) { //absolute path -> no change
  return name;
 }
 QString absName;
 for(QStringList::Iterator it=iconPath.begin();it!=iconPath.end();++it) {
  absName=*it+"/"+name;
//  printDbg(debug::DBG_DBG,"Looking for " <<name << " in: " << *it << " as " << absName);
  if (QFile::exists(absName)) return absName;
 }
 printDbg(debug::DBG_WARN,"Icon file not found: " << name);
 return name;
}

/** load and expand various PATHs from config file.
  Must be done after config defaults are set */
void Menu::initPaths() {
 iconPath=globalSettings->readPath("icon");
}

/** Adds action to menu, returning newly allocated menu Id or existing menu id if action is already present
 
 @param action Name of action
 @return Menu ID of the specified action
 */
int Menu::addAction(const QString action) {
 if (action_map.contains(action)) return action_map[action];
 action_index++;
 action_map[action]=action_index;
 action_map_i[action_index]=action;
 return action_index;
}

/** return action string from given menu ID
 
 @param index Menu ID of action
 @return Name of the specified action
 */
QString Menu::getAction(int index) {
 return action_map_i[index];
}

/** returns icon with given name, loading if necessary and caching for later use
 @param name Name of icon to load
 @return Pixmap containing specified icon
 */
QPixmap* Menu::getIcon(const QString name) {
 printDbg(debug::DBG_INFO,"Loading icon:" << name);
 if (iconCache.contains(name)) return iconCache[name];
 QString absName=getIconFile(name);
 QFile f(absName);
 if (!f.open(IO_ReadOnly)) {
  printDbg(debug::DBG_WARN,"File not found:" << absName);
  return NULL;//file not found or unreadable or whatever ...
 }
 QByteArray qb=f.readAll();
 f.close();
 QPixmap *pix=new QPixmap();
 pix->loadFromData (qb,0,0);
 iconCache[name]=pix;
 return pix; 
}

/** load one GUI item from config file, exiting application with fatal error if item not found 
 
 @param name Name of the item to read
 @param root Root key to read from (default will be used if none specified)
 @return line from config file  
 */
QString Menu::readItem(const QString name,const QString root/*="gui/items/"*/) {
 QString line=globalSettings->read(root+name);
 line=line.simplifyWhiteSpace();
 if (line.length()==0) fatalError("Missing item in config:\n"+root+name);
 return line; 
}

//TODO: keyboard shortcuts sdilet, instalovat jen jednou
//TODO: 

/**
 load one menu item and insert it into parent menu. Recursively load subitems if item is a submenu.

 @param name name of item to be loaded from config file
 @param isRoot TRUE if main menubar is being loaded
 @param parent parent menu item (if isRoot == TRUE, this is the root menubar to add items to)
 */ 
void Menu::loadItem(const QString name,QMenuData *parent/*=NULL*/,bool isRoot/*=FALSE*/) {
 QPopupMenu *item=NULL;
 if (name=="-" || name=="") { //separator
  parent->insertSeparator();
  return;
 }
 QString line=readItem(name);
 if (line.startsWith("list ")) { // List of values - a submenu, first is name of submenu, others are items in it
  if (!isRoot) {
   item=new QPopupMenu();
  }
  line=line.remove(0,5);
  QStringList qs=explode(',',line);  
  QStringList::Iterator it=qs.begin();
  if (it!=qs.end()) { //add itself as popup menu to parent with given name
   if (!isRoot) parent->insertItem(Settings::tr(*it,name),item);
   ++it;
  } else fatalError("Invalid menu list in config:\n"+line);
  for (;it!=qs.end();++it) { //load all subitems
   if (!isRoot) loadItem(*it,item); else loadItem(*it,parent);
  }
 } else if (line.startsWith("item ")) { // A single item
  line=line.remove(0,5);
  //Format: Caption, Action,[,accelerator, [,menu icon]]
  QStringList qs=explode(',',line);
  if (qs.count()<2) fatalError("Invalid menu item in config:\n"+line);
  int menu_id=addAction(qs[1]);
  qs[0]=Settings::tr(qs[0],name);
  parent->insertItem(qs[0],menu_id);
  if (qs.count()>=3 && qs[2].length()>0) { //accelerator specified
   parent->setAccel(QKeySequence(qs[2]),menu_id);
  }
  if (qs.count()>=4 && qs[3].length()>0) { //menu icon specified
   QPixmap *pixmap=getIcon(qs[3]);
   if (pixmap) {
    parent->changeItem(menu_id,*pixmap,qs[0]);
   } else {
    printDbg(debug::DBG_WARN, "Pixmap missing: " << qs[3]);
   }
  }
 } else { //something invalid
  fatalError("Invalid menu list/item in config:\n"+line);
 } 
}

/** Loads menubar from configuration bar, and return it
 
 If menubar can't be loaded, the application is terminated
 Missing menu icons are allowed (if it can't be loaded, there will be no pixmap), missing items in configuration are not.

 @param parent QWidget that will contain the menubar
 @return loaded and initialized menubar
 */
QMenuBar* Menu::loadMenu(QWidget *parent) {
 //menubar can't be cached and must be separate for each window (otherwise weird things happen)
 QMenuBar *menubar=new QMenuBar(parent);//Make new menubar
 loadItem(QString("MainMenu"),menubar,TRUE);//create root menu
 return menubar;
}

/** Load single toolbar item and add it to toolbar
 @param tb Toolbar for addition of item
 @param item Item name in configuration file
 */
void Menu::loadToolBarItem(ToolBar *tb,QString item) {
 if (item=="-" || item=="") {
  tb->addSeparator();
  return;
 }
 QString line=readItem(item);
 if (line.startsWith("item ")) { //Format: Tooltip, Action,[,accelerator, [,icon]]
  line=line.remove(0,5);  
  QStringList qs=explode(',',line);
  if (qs.count()<4) fatalError("Invalid toolbar item in config (must have 4 fields):\n"+line);
  line=line.remove(0,5);
  QPixmap *pixmap=getIcon(qs[3]);
  int menu_id=addAction(qs[1]);
  if (!pixmap) {
   printDbg(debug::DBG_WARN, "Pixmap missing: " << qs[3]);
  }
  ToolButton *tbutton =new ToolButton(pixmap,qs[0],menu_id,tb);
  if (qs[2].length()>0) { //accelerator specified
   tbutton->setAccel(QKeySequence(qs[2]));
  }
  tb->addButton(tbutton);
  tbutton->show();
 } else {
  fatalError("Invalid toolbar item in config:\n"+line);  
 }
}

/**
 Load single toolbar from configuration file

 @param name Toolbar name in configuration file
 @param parent Parent window
 @param visible Will be toolbar initially visible?
 @return loaded toolbar
 */
ToolBar* Menu::loadToolbar(const QString name,QMainWindow *parent,bool visible/*=true*/) {
 QString line=readItem(name);
 printDbg(debug::DBG_INFO,"Loading toolbar:" << name);
 if (line.startsWith("list ")) { // List of values - first is name, others are items in it
  line=line.remove(0,5);
  QStringList qs=explode(',',line);
  ToolBar *tb=NULL;
  QStringList::Iterator it=qs.begin();
  if (it!=qs.end()) {
   tb=new ToolBar(Settings::tr(*it),parent);
   tb->setName(name);
   ++it;
  } else fatalError("Invalid toolbar item in config:\n"+line);
  for (;it!=qs.end();++it) { //load all subitems
   loadToolBarItem(tb,*it);
  }
  if (visible) tb->show();
   else        tb->hide();
  return tb;
 } else {
  fatalError("Invalid toolbar item in config:\n"+line);
  return NULL;
 }
}

/** Load all toolbars from configuration files and add them to parent window
  @param parent parent window for toolbars
 */
ToolBarList Menu::loadToolBars(QMainWindow *parent) {
// ToolBarList list;
 QString line=globalSettings->read("gui/toolbars");
 toolbarNames=explode(',',line);
 bool visible;
 for (unsigned int i=0;i<toolbarNames.count();i++) {
  visible=globalSettings->readBool(QString("toolbar/")+toolbarNames[i],true);
  toolbarList[toolbarNames[i]]=loadToolbar(toolbarNames[i],parent,visible);//was ** += **
 }
 return toolbarList;
}

/** return list of toolbars
 @return Toolbar list
 */
QStringList Menu::getToolbarList() {
 return toolbarNames;
}

/** return toolbar with given name.
 @return Toolbar, or NULL if toolbar not found
 */
ToolBar* Menu::getToolbar(const QString &name) {
 if (!toolbarList.contains(name)) return NULL;
 return toolbarList[name];
}

/** Save Toolbar state to configuration 
 @param tb Toolbar to save state
 @param name Name of toolbar
 @param main Main application window
*/
void Menu::saveToolbar(QToolBar *tb,const QString &name,QMainWindow *main) {
// printDbg(debug::DBG_DBG,"save toolbar " << name);
 Qt::Dock dck;
 int index;
 bool nl;
 int ofs;
 if (!main->getLocation(tb,dck,index,nl,ofs)) return; //Toolbar not found
 QString dock=QString::number(dck)+","+QString::number(index)+","+QString::number(nl)+","+QString::number(ofs);
 globalSettings->write("gui/toolbarstate/"+name,dock);
}

/** Restore Toolbar state from configuration 
 @param tb Toolbar to restore state
 @param name Name of toolbar
 @param main Main application window
*/
void Menu::restoreToolbar(QToolBar *tb,const QString &name,QMainWindow *main) {
// printDbg(debug::DBG_DBG,"restore toolbar " << name);
 QString dock=globalSettings->read("gui/toolbarstate/"+name);
 if (dock.isNull()) return;  //Nothing saved
 QStringList tbs=QStringList::split(",",dock);
 if (tbs.count()!=4) return; //Invalid data
 Qt::Dock dck=(Qt::Dock)tbs[0].toInt();
 int index=tbs[1].toInt();
 bool nl=tbs[2].toInt();
 int ofs=tbs[3].toInt();
 main->moveDockWindow(tb,dck,index,nl,ofs);
}

void Menu::saveToolbars(QMainWindow *main) {
 QString out;
 QTextStream qs(out,IO_WriteOnly);
 qs << *main;
 globalSettings->write("gui/toolbarpos",out);
} 

void Menu::restoreToolbars(QMainWindow *main) {
 QString out=globalSettings->read("gui/toolbarpos");
 if (out.isNull()) return;
 QTextStream qs(out,IO_ReadOnly);
 qs >> *main;
} 

} // namespace gui
