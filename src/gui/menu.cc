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
#include <qpixmap.h>
#include "toolbar.h"
#include <qstring.h>

namespace gui {

/** 
 Exit with error message after encountering invalid menu/toolbar item
 @param type Type of offending item
 @param name Name of offending item
 @param line Line with offending item
 @param expected Optional "Expected" field hinting what is expected to make data valid
*/
void invalidItem(const QString &type,const QString &name,const QString &line,const QString &expected=QString::null) {
 QString err;
 err=QObject::tr("Invalid %1 in config:\nName: %2\nData: %3\n").arg(type,name,line);
 if (!expected.isNull()) err+="\n"+QObject::tr("Expected: ")+expected;
 fatalError(err);
}

/** Default constructor */
Menu::Menu() {
 action_index=1;
 initPaths();
}

/** Destructor */
Menu::~Menu() {
 QValueList<QString> pixmaps=iconCache.keys();
 //Delete all pixmaps from icon cache
 for (QValueList<QString>::Iterator it=pixmaps.begin();it!=pixmaps.end();++it) {
  QPixmap *rm=iconCache[*it];
  delete rm;
 }
 iconCache.clear();
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
// printDbg(debug::DBG_INFO,"Loading icon:" << name);
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

/**
 Load one GUI item from config file, exiting application with fatal error if item not found 
 @param name Name of the item to read
 @param root Root key to read from (default will be used if none specified)
 @return line from config file  
 */
QString Menu::readItem(const QString name,const QString root/*="gui/items/"*/) {
 QString line=globalSettings->read(root+name);
 line=line.simplifyWhiteSpace();
 if (line.length()==0) fatalError(QObject::tr("Missing item in config")+":\n"+root+name);
 return line; 
}

/**
 Load one menu item and insert it into parent menu. Recursively load subitems if item is a submenu.
 @param name name of item to be loaded from config file
 @param isRoot TRUE if main menubar is being loaded
 @param parent parent menu item (if isRoot == TRUE, this is the root menubar to add items to)
 @param prev String list containing names of all parents of this menu item (used for loop detection)
 */ 
void Menu::loadItem(const QString name,QMenuData *parent/*=NULL*/,bool isRoot/*=FALSE*/,QStringList prev/*=QStringList()*/) {
 //Check for cycles (unhandled cycle in menu = crash in QT)
 if (prev.contains(name)) {
  fatalError(QObject::tr("Cycle in menu detected")+":\n"+prev.join(" > ")+" > "+name);
 }
 prev+=name;
 //Check Menu cache
 if (mCache.contains(name)) {
  //This item is already in cache, insert it
  printDbg(debug::DBG_DBG,"MENU - Fetching from cache :" << name);
  parent->insertItem(mCacheName[name],mCache[name]);
  return;
 }
 QPopupMenu *item=NULL;
 if (name=="-" || name=="") { //separator
  parent->insertSeparator();
  return;
 }
 QString line=readItem(name);
 if (line.startsWith("list ")) { // List of values - a submenu, first is name of submenu, others are items in it
  QString add=globalSettings->read("gui/items_add/"+name);
  if (!add.isNull()) { //Read user-specified additional items and append them
   line+=",";
   line+=add;
  }
  if (!isRoot) {
   item=new QPopupMenu();
  }
  line=line.remove(0,5);
  QStringList qs=explode(',',line);  
  QStringList::Iterator it=qs.begin();
  if (it!=qs.end()) { //add itself as popup menu to parent with given name
   if (!isRoot) {
    QString menuName=Settings::tr(*it,name);
    parent->insertItem(menuName,item);
    //Add this item to cache
    mCache[name]=item;
    mCacheName[name]=menuName;
   }
   ++it;
  } else invalidItem(QObject::tr("menu definition"),name,line,QObject::tr("parameter (name of list)"));
  for (;it!=qs.end();++it) { //load all subitems
   if (!isRoot) loadItem(*it,item,false,prev); else loadItem(*it,parent,false,prev);
  }

 } else if (line.startsWith("item ")) { // A single item
  line=line.remove(0,5);
  //Format: Caption, Action,[,accelerator, [,menu icon]]
  QStringList qs=explode(',',line);
  if (qs.count()<2) invalidItem(QObject::tr("menu item"),name,line,QObject::tr("2 or more parameters in item definition"));
  int menu_id=addAction(qs[1]);
  qs[0]=Settings::tr(qs[0],name);
  parent->insertItem(qs[0],menu_id);
  if (qs.count()>=3 && qs[2].length()>0) { //accelerator specified
   if (reserveAccel(qs[2],qs[1])) parent->setAccel(QKeySequence(qs[2]),menu_id);
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
  invalidItem(QObject::tr("menu item"),name,line,"list | item");
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

/** Add accelerator to list of used accelerators and return true, if the accelerator was not taken before.
 Ensure only one item uses the accelerator at time
 @param accelDef Accelerator in string form
 @param action Action to reserve for this accelerator
 @return true if accelerator was not in list before, false otherwise
*/
bool Menu::reserveAccel(const QString &accelDef,const QString &action) {
 if (accels.contains(accelDef)) {
  if (accels[accelDef]!=action) {
   printDbg(debug::DBG_WARN,"Attempt to redefine accel " << accelDef << " from '" << accels[accelDef] << "' to '" << action << "'");
  }
  return false;
 }
 accels[accelDef]=action;
 return true;
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
  if (qs.count()<4) invalidItem(QObject::tr("toolbar item"),item,line,QObject::tr("4 parameters in item definition"));
  line=line.remove(0,5);
  QPixmap *pixmap=getIcon(qs[3]);
  int menu_id=addAction(qs[1]);
  if (!pixmap) {
   printDbg(debug::DBG_WARN, "Pixmap missing: " << qs[3]);
  }
  QString tooltip=qs[0];
  tooltip=tooltip.replace("&","");
  if (qs[2].length()>0) { //accelerator specified
   tooltip+=" ("+qs[2]+")";
  }
  ToolButton *tbutton =new ToolButton(pixmap,tooltip,menu_id,tb);
  if (qs[2].length()>0) { //accelerator specified
   if (reserveAccel(qs[2],qs[1])) tbutton->setAccel(QKeySequence(qs[2]));
  }
  tb->addButton(tbutton);
  tbutton->show();
 } else {
  invalidItem(QObject::tr("toolbar item"),item,line);
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
 if (!line.startsWith("list ")) { // List of values - first is name, others are items in it
  invalidItem(QObject::tr("toolbar definition"),name,line,"list");
  return NULL;
 }
 line=line.remove(0,5);
 QString add=globalSettings->read("gui/items_add/"+name);
 if (!add.isNull()) { //Read user-specified additional items and append them
  line+=",";
  line+=add;
 }
 QStringList qs=explode(',',line);
 ToolBar *tb=NULL;
 QStringList::Iterator it=qs.begin();
 if (it!=qs.end()) {
  tb=new ToolBar(Settings::tr(*it),parent);
  tb->setName(name);
  ++it;
 } else invalidItem(QObject::tr("toolbar definition"),name,line);

 for (;it!=qs.end();++it) { //load all subitems
  loadToolBarItem(tb,*it);
 }
 if (visible) tb->show();
  else        tb->hide();
 return tb;
}

/** Load all toolbars from configuration files and add them to parent window
  @param parent parent window for toolbars
 */
ToolBarList Menu::loadToolBars(QMainWindow *parent) {
 QString line=globalSettings->read("gui/toolbars");
 toolbarNames=explode(',',line);
 bool visible;
 for (unsigned int i=0;i<toolbarNames.count();i++) {
  visible=globalSettings->readBool(QString("toolbar/")+toolbarNames[i],true);
  toolbarList[toolbarNames[i]]=loadToolbar(toolbarNames[i],parent,visible);
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

/** Save toolbar state of given QMainWindow to configuration
 @param main Main application window */
void Menu::saveToolbars(QMainWindow *main) {
 QString out;
 QTextStream qs(out,IO_WriteOnly);
 qs << *main;
 globalSettings->write("gui/toolbarpos",out);
} 

/** Restore toolbar state of given QMainWindow to configuration
 @param main Main application window */
void Menu::restoreToolbars(QMainWindow *main) {
 QString out=globalSettings->read("gui/toolbarpos");
 if (out.isNull()) return;
 QTextStream qs(out,IO_ReadOnly);
 qs >> *main;
} 

} // namespace gui
