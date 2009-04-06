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
 This class manages Menu, Toolbars and Shortcuts
 Menus, menu items, toolbars and toolbar items share the same namespace and often
 menu and toolbar items are interchangable (only difference is that toolbar item
 must have icon, while menu item can be without an icon)
 For more information about menus and toolbars, check doc/design/gui/menu.xml
 @author Martin Petricek
*/

#include "menu.h"
#include "iconcache.h"
#include "settings.h"
#include "toolbar.h"
#include "toolbutton.h"
#include "toolfactory.h"
#include "util.h"
#include <qfile.h>
#include <qlabel.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qmenudata.h>
#include <qpixmap.h>
#include QPOPUPMENU
#include <qregexp.h>
#include <qstring.h>
#include <qtextstream.h>
#include <utils/debug.h>
#include <assert.h>

namespace gui {

using namespace util;
using namespace std;

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
 seqId=0;
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
 QStringList::ConstIterator it=qs.begin();
 for (;it!=qs.end();++it) { //load all subitems
  loadItem(*it,menu,prev);
 }
 //All items loaded, now check items and their accelerators and maybe fill accels for items that don't have one
 optimizeItems(menu);
}

/**
 Add accelerator to items that do not have one
 @param menu Menu to process
*/
void Menu::optimizeItems(QMenuData *menu) {
 assert(menu);
 QString used=mAccel[menu];
 if (used.isNull()) used="";
 QRegExp filter("[^a-z]",false);
 for (unsigned int i=0;i<menu->count();i++) {
  int id=menu->idAt(i);
  //Get item text
  QString itemText=menu->text(id);
  if (itemText.isNull()) continue;//Some separator or alike ...
  if (getAccel(itemText)) continue; //This one already have accelerator
//  guiPrintDbg(debug::DBG_DBG,"Used chars: " << used << " Item " << itemText);
  //Filter out all non-letter characters and get filterText
  QString filterText=itemText;
  filterText.replace(filter,"");
  //Look for first usable letter
  int idx=filterText.find(QRegExp("[^"+used+"0]",false));
  if (idx<0) {
   guiPrintDbg(debug::DBG_INFO,"No accel for Item "  << Q_OUT(itemText) << " Used chars: " << Q_OUT(used));
   //No usable letter for accelerator found. How unfortunate ...
   continue;
  }
  char pAccel=filterText[idx].lower().latin1();
  //Find positoon of accel in original string
  idx=itemText.find(pAccel,0,false);
  assert(idx>=0);
//  guiPrintDbg(debug::DBG_DBG,"Replace from: " << itemText << " using " << pAccel << " index " << idx);
  //Add accel to string
  itemText.insert(idx,'&');
//  guiPrintDbg(debug::DBG_DBG,"Replace to: " << itemText);
  //Update item
  menu->changeItem(id,itemText);
  used+=pAccel;
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
  menuName=Settings::trUtf8(caption.utf8());
 } else {
  menuName=Settings::trUtf8(caption.utf8(),name);
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
  Q_PopupMenu *item=new Q_PopupMenu();
  QString menuName=parseName(line,name);
  loadItemsDef(line,item,prev);
  if (parent) {
   int itemId=parent->insertItem(menuName,item);
   addToMap(name,parent,itemId);
  }
  //Add this item to cache
  //TODO: need multimap for always-correct behavior -> or warn in documentation about diff. behavior
  mCache[name]=item;
  mCacheName[name]=menuName;
  itemText.replace(name,menuName);
 } else if (line.startsWith("item ")) { // A single item
  if (parent) addItem(line,parent,name);
 } else { //something invalid
  invalidItem(QObject::tr("menu item"),name,line,"list | item");
 }
}

/**
 Get accelerator for given description
 @param name Name of menu item
 @return accelerator (the character after '&')
 */
char Menu::getAccel(const QString &name) {
 int pos=name.find('&');
 if (pos==-1) return 0;
 return name[pos+1].lower().latin1();
}

/**
 Load one menu or toolbar item and insert it into parent menu or toolbar.
 If inserting into toolbar and name refers to special toobar item, that item is inserted and rest of parameters are ignored
 @param parentName Name of parent menu/toolbar. If NULL/empty, main menubar is the parent
 @param name Name of the item. Have to be unique. ("" or "-" will insert separator and ignore rest of parameters)
 @param caption Caption of item
 @param action Script to execute when the item is selected
 @param accel Keyboard Accelerator
 @param icon Name of icon
 @param classes Item classes
 */
void Menu::createItem(const QString &parentName,const QString &name,const QString &caption,const QString &action,const QString &accel/*=QString::null*/,const QString &icon/*=QString::null*/,const QStringList &classes/*=QStringList()*/) throw (InvalidMenuException) {
 //TODO: check for duplicate names (mapTool, mapMenu)
 QMenuData *parent=NULL;
 if (parentName.isNull() || parentName=="") {
  //Check for main menubar
  parent=menubar;
  assert(parent);
 } else if (mCache.contains(parentName)) {
  //Check for some specific submenu
  parent=mCache[parentName];
  assert(parent);
 }
 if (parent) {
  //Insert into menu
  if (name=="-" || name=="") {
   parent->insertSeparator();
  } else {
   addItem(parent,name,caption,action,accel,icon,classes);
   //Create accels if not specified for this last item
   optimizeItems(parent);
  }
 } else {
  ToolBar *tb=getToolbar(parentName);
  if (tb) {
   if (name=="-" || name=="") {
    tb->addSeparator();
    return;
   }
   //Try special item (this includes separator too)
   QWidget *spec_tb=ToolFactory::specialItem(tb,name,main);
   if (spec_tb) {
    //special toolbar buttons are added to list with their full parameters
    addToMap(name,spec_tb);
    return;
   }
   //Create ordinary item
   //ToolButton *tbutton=
   createToolBarItem(tb,name,caption,action,accel,icon,classes);
  } else {
   guiPrintDbg(debug::DBG_WARN,"Toolbar/menu to install into not found: " << Q_OUT(parentName));
  }
 }
}

/**
 Get first string from QStringList iterator and advance iterator one item further.
 Return QString::null if at ending position "end"
 @param it iterator
 @param end iterator marking ending position
 @return currrent iterator item or QString::null if after the end
*/
QString Menu::pop(QStringList::ConstIterator &it,const QStringList::ConstIterator &end) {
 if (it==end) return QString::null;
 QString res=*it;
 ++it;
 return res;
}

/**
 Add menu item specified by given data to parent
 @param line Line containing menu item specification
 @param parent parent menu in which this item will be appended
 @param name Name of this item (key in settings).
*/
void Menu::addItem(QString line,QMenuData *parent,const QString &name/*=QString::null*/) throw (InvalidMenuException) {
 line=line.remove(0,5);
 //Format: Caption, Action,[,accelerator, [,menu icon]]
 QStringList qs=explode(MENUDEF_SEPARATOR,line,true);
 if (qs.count()<2) invalidItem(QObject::tr("menu item"),name,line,QObject::tr("2 or more parameters in item definition"));
 QStringList::ConstIterator it=qs.begin();
 QStringList::ConstIterator it_end=qs.end();
 QString caption=pop(it,it_end);
 QString action=pop(it,it_end);
 QString accel=pop(it,it_end);
 QString icon=pop(it,it_end);
 QStringList classes;
 for(;it!=it_end;++it) {
  classes+=*it;
 }
 addItem(parent,name,caption,action,accel,icon,classes);
}

/**
 Return item text for given menu item
 Will translate the text
 @param caption Caption of the menu item
 @param name Name if the menu item
 @return localized item text
*/
QString Menu::menuItemText(const QString &caption,const QString &name) {
 QString captionTr;
 if (name.isNull()) {
  captionTr=Settings::trUtf8(caption.utf8());
 } else {
  captionTr=Settings::trUtf8(caption.utf8(),name);
 }
 itemText.replace(name,caption);
 return captionTr;
}

/**
 Add menu item specified by given data to parent
 @param parent parent menu in which this item will be appended
 @param name Name of the item. Key in settings, or specified from script. Have to be unique
 @param caption Caption of item (untranslated)
 @param action Script to execute when the item is selected
 @param accel Keyboard Accelerator
 @param icon Name of icon
 @param classes Item classes
 @return ID of item just added
*/
int Menu::addItem(QMenuData *parent,const QString &name,const QString &caption,const QString &action,const QString &accel/*=QString::null*/,const QString &icon/*=QString::null*/,const QStringList &classes/*=QStringList()*/) throw (InvalidMenuException) {
 int menu_id=addAction(action);
 QString captionTr=menuItemText(caption,name);
 accelText.replace(name,accel);
 char accelChar=getAccel(captionTr);
 if (accelChar) {
  //Some accelerator specified for item
  QString s=mAccel[parent];
  if (s.isNull()) s="";
  if (s.find(accelChar)>=0) {
   //Already used
   guiPrintDbg(debug::DBG_WARN,"Accelerator for " << Q_OUT(caption) << " is already used elsewhere!");
  } else {
   //Not yet used
   s=s+accelChar;
   mAccel[parent]=s;
  }
 }
 int itemId=parent->insertItem(captionTr,menu_id);
 addToMap(name,parent,itemId);
 if (!accel.isNull() && accel.length()>0) { //accelerator specified
  if (reserveAccel(accel,action)) parent->setAccel(QKeySequence(accel),menu_id);
 }
 if (!icon.isNull() && icon.length()>0) { //menu icon specified
  const QIconSet *iconSet=cache->getIconSet(icon);
  if (iconSet) {
   parent->changeItem(itemId,*iconSet,captionTr);
  } else {
   guiPrintDbg(debug::DBG_WARN, "Icon missing: " << Q_OUT(icon));
  }
 }
 if (classes.count()) { //Extra data - Item classes
  for (QStringList::ConstIterator it=classes.begin();it!=classes.end();++it) {
   addToMap("/"+*it,parent,itemId);
  }
 }
 return itemId;
}

/**
 Loads menubar from configuration bar, and return it<br>
 If menubar can't be loaded, InvalidMenuException will be thrown
 Missing menu icons are allowed (if it can't be loaded, there will be no pixmap), missing items in configuration are not.<br>
 @param parent QWidget that will contain the menubar
 @return loaded and initialized menubar
*/
QMenuBar* Menu::loadMenu(QWidget *parent) throw (InvalidMenuException) {
 //menubar can't be cached and must be separate for each window (otherwise weird things happen)
 menubar=new QMenuBar(parent);//Make new menubar
 loadItems(QString("MainMenu"),menubar);//create root menu
 return menubar;
}

/**
 Add accelerator to list of used accelerators and return true, if the accelerator was not taken before.
 Ensure only one item uses the accelerator at time
 @param accelDef Accelerator in string form
 @param action Action to reserve for this accelerator
 @return true if accelerator was not in list before, false otherwise
*/
bool Menu::reserveAccel(const QString &accelDef,const QString &action) {
 if (accels.contains(accelDef)) {
  if (accels[accelDef]!=action) {
   guiPrintDbg(debug::DBG_WARN,"Attempt to redefine accel " << Q_OUT(accelDef) << " from '" << Q_OUT(accels[accelDef]) << "' to '" << Q_OUT(action) << "'");
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
 if (item=="-" || item=="") {
  tb->addSeparator();
  return;
 }
 QWidget *spec_tb=ToolFactory::specialItem(tb,item,main);
 if (spec_tb) {
  //special toolbar buttons are added to list with their full parameters
  addToMap(item,spec_tb);
  return;
 }
 QString line=readItem(item);
 if (chopCommand(line,"item")) { //Format: Tooltip, Action,[,accelerator, [,icon]]
  QStringList qs=explode(MENUDEF_SEPARATOR,line,true);
  if (qs.count()<4) invalidItem(QObject::tr("toolbar item"),item,line,QObject::tr("4 parameters in item definition"));
  QStringList::ConstIterator it=qs.begin();
  QStringList::ConstIterator it_end=qs.end();
  QString caption=pop(it,it_end);
  QString action=pop(it,it_end);
  QString accel=pop(it,it_end);
  QString icon=pop(it,it_end);
  QStringList classes;
  for(;it!=it_end;++it) {
   classes+=*it;
  }
  createToolBarItem(tb,item,caption,action,accel,icon,classes);
 } else if (chopCommand(line,"label")) { //Format: Text
  if (!line.length()) invalidItem(QObject::tr("label item"),item,line);
  new QLabel(Settings::trUtf8(line.utf8(),item),tb);
 } else {
  invalidItem(QObject::tr("toolbar item"),item,line);
 }
}

/**
 Return tooltip text for given text
 Will translate the tooltip, remove any & characters that have meaning in menus and add accelerator if specified
 @param text Tooltip text
 @param name Name of the toolbar item for which the text is. Used for correctly determining possible translation
 @param accel Accelerator specified for the item (or QString::null if none defined or unknown)
 @return localized tooltip text
*/
QString Menu::toolTipText(const QString &text,const QString &name,QString accel/*=QString::null*/) {
 if (accel.isNull() && accelText.contains(name)) accel=accelText[name];
 itemText.replace(name,text);
 QString tooltip=Settings::trUtf8(text.utf8(),name);
 tooltip=tooltip.replace("&","");
 if (!accel.isNull() && accel.length()>0) { //accelerator specified
  tooltip+=" ("+accel+")";
 }
 accelText.replace(name,accel);
 return tooltip;
}

/**
 Create single toolbar button and add it to toolbar
 @param tb Toolbar for addition of item
 @param name Name of the item. Key in settings, or specified from script. Have to be unique
 @param text Tooltip of item (untranslated)
 @param action Script to execute when the item is selected
 @param accel Keyboard Accelerator
 @param icon Name of icon
 @param classes Item classes
 @return pointer to toolbutton just added
*/
ToolButton* Menu::createToolBarItem(ToolBar *tb,const QString &name,const QString &text,const QString &action, const QString &accel,const QString &icon, const QStringList &classes/*=QStringList()*/) {
 const QIconSet *iconSet=cache->getIconSet(icon);
 int menu_id=addAction(action);
 if (!iconSet) {
  guiPrintDbg(debug::DBG_WARN, "Icon missing: " << Q_OUT(icon));
 }
 QString tooltip=toolTipText(text,name,accel);
 ToolButton *tbutton=new ToolButton(iconSet,tooltip,menu_id,tb);
 addToMap(name,tbutton);
 if (!accel.isNull() && accel.length()>0) { //accelerator specified
  if (reserveAccel(accel,action)) tbutton->setAccel(QKeySequence(accel));
 }
 tb->addButton(tbutton);
 tbutton->show();
 if (classes.count()) { //Extra data - Item classes
  for (QStringList::ConstIterator it=classes.begin();it!=classes.end();++it) {
   addToMap("/"+*it,tbutton);
  }
 }
 return tbutton;
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
 guiPrintDbg(debug::DBG_INFO,"Loading toolbar:" << Q_OUT(name));
 if (!isList(line)) { // List of values - first is name, others are items in it
  invalidItem(QObject::tr("toolbar definition"),name,line,"list");
  return NULL;
 }
 QString tbName=parseName(line,name);
 ToolBar *tb=new ToolBar(tbName,main);
 tb->setName(name);
 QStringList qs=explode(MENULIST_SEPARATOR,line);
 QStringList::ConstIterator it=qs.begin();
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
 line=line.simplifyWhiteSpace();
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
 Return toolbar with given name.
 @param name Name of toolbar
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
#ifdef QT4
 QByteArray ba=main->saveState();
 globalSettings->write("gui/toolbarpos",QString(ba.toBase64()));
#else
 QString out;
 QTextStream qs(out,IO_WriteOnly);
 qs << *main;
 globalSettings->write("gui/toolbarpos",out);
#endif
}

/**
 Restore toolbar state of QMainWindow from configuration
*/
void Menu::restoreToolbars() {
 QString out=globalSettings->read("gui/toolbarpos");
 if (out.isNull()) return;
#ifdef QT4
 main->restoreState(QByteArray::fromBase64(out.toAscii()));
#else
 QTextStream qs(out,IO_ReadOnly);
 qs >> *main;
#endif
}

/**
 Add specified toolbar item to "all items" map
 @param name Name of item
 @param item The toolbutton or widget
 */
void Menu::addToMap(const QString &name,QWidget* item) {
 MapKey mk(name,seqId);
 seqId++;
 mapTool.insert(mk,item,false);
}

/**
 Add specified menu item to "all items" map
 @param name Name of item
 @param parent Parent menu
 @param itemId Id of item in parent menu
*/
void Menu::addToMap(const QString &name,QMenuData* parent,int itemId) {
 MapKey mk(name,seqId);
 seqId++;
 MenuItemsValue v(parent,itemId);
 mapMenu.insert(mk,v,false);
}

/**
 Enable or disable item in toolbar and/or menu, given its name
 @param name Name of item
 @param enable True to enable, false to disable
*/
void Menu::enableByName(const QString &name,bool enable) {
 //TODO: lookup is slow & linear, improve ....
 //TODO: need multimap
 for (ToolbarItems::ConstIterator it=mapTool.begin();it!=mapTool.end();++it) {
  if (it.key().first==name) {
   QWidget* el=it.data();
   el->setEnabled(enable);
//   guiPrintDbg(debug::DBG_DBG,"en/dis abling toolbar " << name << " " << enableItem);
  }
 }
 for (MenuItems::ConstIterator it=mapMenu.begin();it!=mapMenu.end();++it) {
  if (it.key().first==name) {
   MenuItemsValue el=it.data();
   QMenuData* md=el.first;
   int id=el.second;
   md->setItemEnabled(id,enable);
//   guiPrintDbg(debug::DBG_DBG,"en/dis abling menu " << name << " " << enableItem);
  }
 }
}

/**
 Check or uncheck item in toolbar and/or menu, given its name<br>
 Note: Toolbuttons will automatically convert to Togglable toolbuttons this way
       and will start togling itself automatically on each succesive click
 @param name Name of item
 @param check True to check, false to uncheck
*/
void Menu::checkByName(const QString &name,bool check) {
 for (ToolbarItems::ConstIterator it=mapTool.begin();it!=mapTool.end();++it) {
  if (it.key().first==name) {
   ToolButton* el=dynamic_cast<ToolButton*>(it.data());
   if (el) {
    el->setToggleButton(true);
    el->setOn(check);
   }
  }
 }
 for (MenuItems::ConstIterator it=mapMenu.begin();it!=mapMenu.end();++it) {
  if (it.key().first==name) {
   MenuItemsValue el=it.data();
   QMenuData* md=el.first;
   int id=el.second;
   md->setItemChecked(id,check);
  }
 }
}

/**
 Set text on toolbar or menu item with given name.
 If both text and menu item exist with same name, both are updated.
 Note: the text wilol be translated according to the translation file,
 so english text should be used and any translation should be put into the translation file
 @param name Name of the item
 @param newText text on the item
*/
void Menu::setTextByName(const QString &name,const QString &newText) {
 for (ToolbarItems::ConstIterator it=mapTool.begin();it!=mapTool.end();++it) {
  if (it.key().first==name) {
   ToolButton* el=dynamic_cast<ToolButton*>(it.data());
   if (el) {
    el->setTextLabel(toolTipText(newText,name));
   }
  }
 }
 for (MenuItems::ConstIterator it=mapMenu.begin();it!=mapMenu.end();++it) {
  if (it.key().first==name) {
   MenuItemsValue el=it.data();
   QMenuData* md=el.first;
   int id=el.second;
   md->changeItem(id,menuItemText(newText,name));
   optimizeItems(md);
  }
 }
}

/**
 Given name of toolbar or menu item, return its text
 This will return original (untranslated) text (in english)
 Setting it back with setTextByName will translate it according to the translation file
 @param name Name of the item
 @return text on the item
*/
QString Menu::getTextByName(const QString &name) {
 if (itemText.contains(name)) return itemText[name];
 return QString::null;
}

/**
 Show or hide item in toolbar, given its name.<br>
 Items in menu can't be show or hidden this way
 @param name Name of item
 @param show True to show, false to hide
*/
void Menu::showByName(const QString &name,bool show) {
 for (ToolbarItems::ConstIterator it=mapTool.begin();it!=mapTool.end();++it) {
  if (it.key().first==name) {
   QWidget* el=dynamic_cast<QWidget*>(it.data());
   if (el) {
    if (show) el->show(); else el->hide();
   }
  }
 }
}

} // namespace gui
