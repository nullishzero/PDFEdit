/** @file
 QSMenu - class providing Popup menu with functions to create and show these menus<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsmenu.h"
#include <qpopupmenu.h>
#include <qcursor.h>
#include "menu.h"
#include "util.h"
#include <utils/debug.h>

class QPixMap;
class QIconSet;

namespace gui {

/** Menu ID offset for temporary items */
const int TMP_OFFSET=0x3fffffff;

/**
 Construct popup menu containing the same items as some other already specified menu (which is used as template)
 @param _msys Menu system of window in which this popup menu will be shown
 @param name Name of the menu as it is stored in settings. If null or not specified, menu will be initially empty
*/
QSMenu::QSMenu(Menu *_msys,const QString &name/*=QString::null*/) : QSCObject ("Menu") {
 msys=_msys;
 actionId=0;
 menu=new QPopupMenu();
 try {
  if (!name.isNull()) { //Menu template specified
   QString line=Menu::readItem(name);
   if (Menu::isList(line)) { //list given
    Menu::parseName(line);//Strip off the name
    QStringList qs=explode(',',line);  
    QStringList::Iterator it=qs.begin();
    for (;it!=qs.end();++it) { //load all subitems
     addItem(*it);
    }
   } else {//single item given
    addItem(name);
   }
  }
 } catch (InvalidMenuException &e) {
  //Ignore any exception when adding item - the item was just not added
  printDbg(debug::DBG_WARN, "Menu Exception:  " << e.message());
 } catch (...) {
  //Ignore any exception when adding item - the item was just not added
  printDbg(debug::DBG_WARN, "Unknown Exception");
 }
}

/**
 Invoke popup menu near position of mouse pointer, return selected action
 @return selected action (script), or null string if no action selected
*/

QString QSMenu::popup() {
 printDbg(debug::DBG_DBG, "Menu-popup");
 int id=menu->exec(QCursor::pos());
 printDbg(debug::DBG_DBG, "Menu-popup i " << id);
 QString action=getAction(id);
 printDbg(debug::DBG_DBG, "Menu-popup a " << action);
 return action;
}


/** For given ID get associated action (script)
 @param id ID of menu action
 @return script corresponding to 
*/
QString QSMenu::getAction(int id) {
 if (id<0) return QString::null;//Menu cancelled
 if (id>=TMP_OFFSET) { //From temporary item
  printDbg(debug::DBG_DBG, "qs action " << (id-TMP_OFFSET));
  return actions[id-TMP_OFFSET-1];
 } else {
  printDbg(debug::DBG_DBG, "m  action " << id);
  return msys->getAction(id);
 }
}

/** Append item to menu using its definition.<br>
 Exceptions are ignored and will result only in the item not added to menu<br>
 You can add items only, not lists in this way. If you want to add list,
 you must have it defined in configuration first.
 @param def Definition of menu item (same format as used in application menus)
*/
void QSMenu::addItemDef(QString def) {
 printDbg(debug::DBG_DBG, "++" << def);
 try {
  if (!def.startsWith("item ")) {
   printDbg(debug::DBG_WARN, "NOT item: " << def);
   return;//Can add only items this way
  }
  actionId++;
  printDbg(debug::DBG_DBG, "Menudef1: " << def);
  QString itemName=Menu::parseName(def);
  printDbg(debug::DBG_DBG, "Menudef2: " << def << " name " << itemName);
  QStringList param=explode(',',def);  //param = Action[,accelerator [,menu icon]]
  menu->insertItem(itemName,actionId+TMP_OFFSET);
  if (param.count()>1 && param[1].length()>0) { //accelerator specified
   menu->setAccel(QKeySequence(param[1]),actionId+TMP_OFFSET);
  }
  if (param.count()>2 && param[2].length()>0) { //menu icon specified
   const QIconSet *icon=msys->getIconSet(param[2]);
   if (icon) {
    menu->changeItem(actionId+TMP_OFFSET,*icon,itemName);
   } else {
    printDbg(debug::DBG_WARN, "Pixmap missing: " << param[2]);
   }
  }
  printDbg(debug::DBG_DBG, "Menu append action: " << param[0]);
  actions.append(param[0]);
 } catch (InvalidMenuException &e) {
  //Ignore any exception when adding item - the item was just not added
  printDbg(debug::DBG_WARN, "Menu Exception:  " << e.message());
 } catch (...) {
  //Ignore any exception when adding item - the item was just not added
  printDbg(debug::DBG_WARN, "Unknown Exception");
 }
}

/** Append item (or list) to menu using its name (as specified in configuration).<br>
 Special: if the name is "-" or "", separator is added to menu<br>
 Exceptions are ignored and will result only in the item not added to menu
 @param def name of item (item or list)
*/
void QSMenu::addItem(const QString &name) {
 printDbg(debug::DBG_DBG, "+" << name);
 try {
  if (name=="-" || name=="") {
   menu->insertSeparator();
   return;
  }
  QString line=Menu::readItem(name);
  if (line.isNull()) {//Item does not exist
   printDbg(debug::DBG_WARN, "Menu item " << name << " doed not exist");
  }
  if (Menu::isList(line)) { //add as list
   msys->loadItem(name,menu);
  } else {//Add as item definition
   addItemDef(line);
  }
 } catch (InvalidMenuException &e) {
  //Ignore any exception when adding item - the item was just not added
  printDbg(debug::DBG_WARN, "Menu Exception:  " << e.message());
 } catch (...) {
  //Ignore any exception when adding item - the item was just not added
  printDbg(debug::DBG_WARN, "Unknown Exception");
 }
}

/** Insert a separator in the menu */
void QSMenu::addSeparator() {
 addItem("-");
}

/** Reset the menu and remove all items from it */
void QSMenu::reset() {
 menu->clear(); //This will remove also all temporary items
 actionId=0;
 actions.clear(); 
}

/** destructor */
QSMenu::~QSMenu() {
 delete menu;
}

} // namespace gui

