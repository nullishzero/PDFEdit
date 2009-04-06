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
#ifndef __MENU_H__
#define __MENU_H__

#include "qtcompat.h"
#include <qmap.h>
#include <qstringlist.h>
#include QICON
#include "invalidmenuexception.h"
class QMainWindow;
class QMenuBar;
class QMenuData;
class Q_PopupMenu;
class QPixmap;
class QWidget;
class QWidget;

namespace gui {

class ToolBar;
class ToolButton;
class IconCache;

/** Separator of fields in menu definition */
const char MENUDEF_SEPARATOR=',';
/** Separator of items in menu list */
const char MENULIST_SEPARATOR=',';
/** Separator of items in list of existing toolbars */
const char TOOLBARLIST_SEPARATOR=',';

/** Action map type: mapping from to */
typedef QMap<QString, int> ActionMap;
/** Inverse action map: mapping from to  */
typedef QMap<int, QString> ActionMapInverse;
/** Toolbar list type: mapping from toolbar name to toolbar object */
typedef QMap<QString, ToolBar*> ToolBarList;
/** Mapping from accel name to widget that have this accel */
typedef QMap<QString, QString> AccelList;
/** Cache of already defined menu items (some menus may be used on more than one place) */
typedef QMap<QString, Q_PopupMenu*> MenuCache;
/** List of already used accelerators for specific popup menu */
typedef QMap<QMenuData*, QString> MenuAccels;
/** Cache of already defined menu items (names) */
typedef QMap<QString, QString> MenuNames;
/** Value type for MenuItems type*/
typedef std::pair<QMenuData*,int> MenuItemsValue;
/** Key type for MenuItems and ToolbarItems */
typedef std::pair<QString,int> MapKey;
/** Pointer to of all items by item or "item class" name - menu items */
typedef QMap<MapKey,MenuItemsValue> MenuItems;
/** Pointer to of all items by item or "item class" name - toolbar items */
typedef QMap<MapKey,QWidget*> ToolbarItems;

/**
 This class manages Menu, Toolbars and Shortcuts.<br>
 Menus, menu items, toolbars and toolbar items share the same namespace and often
 menu and toolbar items are interchangable (only difference is that toolbar item
 must have icon, while menu item can be without an icon)<br>
 For more information about menus and toolbars, check doc/design/gui/menu.xml
 \brief Menu and Toolbar management
*/
class Menu {
public:
 Menu(QMainWindow *_main);
 ~Menu();
 QString getAction(int index);
 QMenuBar* loadMenu(QWidget *parent) throw (InvalidMenuException);
 ToolBarList loadToolBars() throw (InvalidMenuException);
 ToolBar* getToolbar(const QString &name);
 QStringList getToolbarList();
 static QString pop(QStringList::ConstIterator &it,const QStringList::ConstIterator &end);
 void saveToolbars();
 void restoreToolbars();
 static char getAccel(const QString &name);
 static QString readItem(const QString &name) throw (InvalidMenuException);
 static bool isList(const QString &line);
 static QString parseName(QString &line, const QString &name=QString::null) throw (InvalidMenuException);
 const QPixmap* getIcon(const QString &name);
 const QIconSet* getIconSet(const QString &name);
 void loadItem(const QString &name,QMenuData *parent=NULL,QStringList prev=QStringList()) throw (InvalidMenuException);
 static bool chopCommand(QString &line, const QString &command);
 static void invalidItem(const QString &type,const QString &name,const QString &line,const QString &expected=QString::null) throw (InvalidMenuException);
 void enableByName(const QString &name,bool enable);
 void checkByName(const QString &name,bool check);
 void showByName(const QString &name,bool show);
 void createItem(const QString &parentName,const QString &name,const QString &caption,const QString &action,const QString &accel=QString::null,const QString &icon=QString::null,const QStringList &classes=QStringList()) throw (InvalidMenuException);
 void setTextByName(const QString &name,const QString &newText);
 QString getTextByName(const QString &name);
private:
 QString menuItemText(const QString &caption,const QString &name);
 QString toolTipText(const QString &text,const QString &name,QString accel=QString::null);
 ToolButton* createToolBarItem(ToolBar *tb,const QString &name,const QString &text,const QString &action, const QString &accel,const QString &icon, const QStringList &classes=QStringList());
 void addToMap(const QString &name,QWidget* item);
 void addToMap(const QString &name,QMenuData* parent,int itemId);
 void addItem(QString line,QMenuData *parent,const QString &name=QString::null) throw (InvalidMenuException);
 int addItem(QMenuData *parent,const QString &name,const QString &caption,const QString &action,const QString &accel=QString::null,const QString &icon=QString::null,const QStringList &classes=QStringList()) throw (InvalidMenuException);
 bool reserveAccel(const QString &accelDef,const QString &action);
 int addAction(const QString &action);
 void loadItemsDef(QString line,QMenuData *menu,QStringList prev=QStringList()) throw (InvalidMenuException);
 void loadItems(const QString &name,QMenuData *menu,QStringList prev=QStringList()) throw (InvalidMenuException);
 void loadToolBarItem(ToolBar *tb,const QString &item) throw (InvalidMenuException);
 ToolBar* loadToolbar(const QString &name,bool visible=true) throw (InvalidMenuException);
 void optimizeItems(QMenuData *menu);
private:
 /** Mapping between menu IDs and actions */
 ActionMap action_map;
 /** Inverse mapping between menu IDs and actions */
 ActionMapInverse action_map_i;
 /** Incrementing action index for menu items */
 int action_index;
 /** List of loaded toolbars names */
 QStringList toolbarNames;
 /** List of loaded toolbars */
 ToolBarList toolbarList;
 /** List of installed accelerators */
 AccelList accels;
 /** Text for item by its name */
 QMap<QString, QString> itemText;
 /** Accelerator for item by its name */
 QMap<QString, QString> accelText;
 /** Cache for menu items */
 MenuCache mCache;
 /** Cache for menu items - names */
 MenuNames mCacheName;
 /** List of used menu accels */
 MenuAccels mAccel;
 /** Cache for icons */
 IconCache *cache;
 /** Main application window */
 QMainWindow *main;
 /** Pointer to of all items by item or "item class" name - menu items */
 MenuItems mapMenu;
 /** Pointer to of all items by item or "item class" name - toolbar items */
 ToolbarItems mapTool;
 /** Sequenc id for map key*/
 int seqId;
 /** The main menu bar */
 QMenuBar *menubar;
};

} // namespace gui

#endif
