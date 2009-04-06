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
 QSMenu - class providing Popup menu with functions to create and show these menus<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsmenu.h"
#include <assert.h>
#include QPOPUPMENU
#include <qcursor.h>
#include "menu.h"
#include "util.h"
#include <utils/debug.h>
#include "basecore.h"

class QPixMap;
class QIcon;

namespace gui {

using namespace util;

/** Menu ID offset for temporary items */
const int TMP_OFFSET=0x3fffffff;

/**
 Construct popup menu containing the same items as some other already specified menu (which is used as template)
 @param _msys Menu system of window in which this popup menu will be shown
 @param _base Scripting base
 @param name Name of the menu as it is stored in settings. If null or not specified, menu will be initially empty
*/
QSMenu::QSMenu(Menu *_msys,BaseCore *_base,const QString &name/*=QString::null*/) : QSCObject ("Menu",_base) {
 msys=_msys;
 actionId=0;
 menu=new Q_PopupMenu();
 try {
  if (!name.isNull()) { //Menu template specified
   QString line=Menu::readItem(name);
   if (Menu::isList(line)) { //list given
    Menu::parseName(line);//Strip off the name
    QStringList qs=explode(MENUDEF_SEPARATOR,line,true);
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
  guiPrintDbg(debug::DBG_WARN, "Menu Exception:  " << Q_OUT(e.message()));
 } catch (...) {
  //Ignore any exception when adding item - the item was just not added
  guiPrintDbg(debug::DBG_WARN, "Unknown Exception");
 }
}

/**
 Invoke popup menu near position of mouse pointer, return selected action
 @return selected action (script), or null string if no action selected
*/

QString QSMenu::popup() {
 int id=menu->exec(QCursor::pos());
 guiPrintDbg(debug::DBG_DBG, "Menu-popup id " << id);
 QString action=getAction(id);
 if (action.isNull()) {  //NOOP sent
 guiPrintDbg(debug::DBG_DBG, "Menu-popup a NOOP");
  return "";
 }
 guiPrintDbg(debug::DBG_DBG, "Menu-popup a " << Q_OUT(action));
 return action;
}


/**
 For given ID get associated action (script)
 @param id ID of menu action
 @return script corresponding to the action
*/
QString QSMenu::getAction(int id) {
 if (id<0) return QString::null;//Menu cancelled
 if (id>=TMP_OFFSET) { //From temporary item
//  guiPrintDbg(debug::DBG_DBG, "qs action " << (id-TMP_OFFSET));
  return actions[id-TMP_OFFSET-1];
 } else {
//  guiPrintDbg(debug::DBG_DBG, "m  action " << id);
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
 if (def.isNull()) return;//Empty definition?
 guiPrintDbg(debug::DBG_DBG, "++" << Q_OUT(def));
 try {
  if (!def.startsWith("item ")) {
   guiPrintDbg(debug::DBG_WARN, "NOT item: " << Q_OUT(def));
   return;//Can add only items this way
  }
  actionId++;
  QString itemName=Menu::parseName(def);
  QStringList param=explode(MENUDEF_SEPARATOR,def,true);  //param = Action[,accelerator [,menu icon]]
  menu->insertItem(itemName,actionId+TMP_OFFSET);
  if (param.count()>1 && param[1].length()>0) { //accelerator specified
   menu->setAccel(QKeySequence(param[1]),actionId+TMP_OFFSET);
  }
  if (param.count()>2 && param[2].length()>0) { //menu icon specified
   const QIconSet *icon=msys->getIconSet(param[2]);
   if (icon) {
    menu->changeItem(actionId+TMP_OFFSET,*icon,itemName);
   } else {
    guiPrintDbg(debug::DBG_WARN, "Pixmap missing: " << Q_OUT(param[2]));
   }
  }
  if (param[0].isNull()) { //PrintDbg can't handle null string sent to it.
   guiPrintDbg(debug::DBG_WARN, "Menu append action: (NULL)");
  } else {
   guiPrintDbg(debug::DBG_DBG, "Menu append action: " << Q_OUT(param[0]));
  }
  actions.append(param[0]);
 } catch (InvalidMenuException &e) {
  //Ignore any exception when adding item - the item was just not added
  guiPrintDbg(debug::DBG_WARN, "Menu Exception:  " << Q_OUT(e.message()));
 } catch (...) {
  //Ignore any exception when adding item - the item was just not added
  guiPrintDbg(debug::DBG_WARN, "Unknown Exception");
 }
}

/** Append item (or list) to menu using its name (as specified in configuration).<br>
 Special: if the name is "-" or "", separator is added to menu<br>
 Exceptions are ignored and will result only in the item not added to menu
 @param name name of item (item or list)
*/
void QSMenu::addItem(const QString &name) {
// guiPrintDbg(debug::DBG_DBG, "+" << name);
 try {
  if (name=="-" || name=="") {
   menu->insertSeparator();
   return;
  }
  QString line=Menu::readItem(name);
  if (line.isNull()) {//Item does not exist
   guiPrintDbg(debug::DBG_WARN, "Menu item " << Q_OUT(name) << " doed not exist");
  }
  if (Menu::isList(line)) { //add as list
   msys->loadItem(name,menu);
  } else {//Add as item definition
   guiPrintDbg(debug::DBG_DBG, "Menu add as def: " << Q_OUT(name));
   addItemDef(line);
  }
 } catch (InvalidMenuException &e) {
  //Ignore any exception when adding item - the item was just not added
  guiPrintDbg(debug::DBG_WARN, "Menu Exception:  " << Q_OUT(e.message()));
 } catch (...) {
  //Ignore any exception when adding item - the item was just not added
  guiPrintDbg(debug::DBG_WARN, "Unknown Exception");
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

