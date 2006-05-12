#ifndef __MENU_H__
#define __MENU_H__

#include <qmap.h>
#include <qstringlist.h>
#include "invalidmenuexception.h"
class QMainWindow;
class QMenuBar;
class QMenuData;
class QPopupMenu;
class QPixmap;
class QWidget;
class QIconSet;

namespace gui {

class ToolBar;
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
typedef QMap<QString, QPopupMenu*> MenuCache;
/** Cache of already defined menu items (names) */
typedef QMap<QString, QString> MenuNames;

class Menu {
public:
 Menu(QMainWindow *_main);
 ~Menu();
 QString getAction(int index);
 QMenuBar* loadMenu(QWidget *parent) throw (InvalidMenuException);
 ToolBarList loadToolBars() throw (InvalidMenuException);
 ToolBar* getToolbar(const QString &name);
 QStringList getToolbarList();
 void saveToolbars();
 void restoreToolbars();
 static QString readItem(const QString &name) throw (InvalidMenuException);
 static bool isList(const QString &line);
 static QString parseName(QString &line, const QString &name=QString::null) throw (InvalidMenuException);
 const QPixmap* getIcon(const QString &name);
 const QIconSet* getIconSet(const QString &name);
 void loadItem(const QString &name,QMenuData *parent=NULL,QStringList prev=QStringList()) throw (InvalidMenuException);
 static bool chopCommand(QString &line, const QString &command);
 static void invalidItem(const QString &type,const QString &name,const QString &line,const QString &expected=QString::null) throw (InvalidMenuException);
private:
 void addItem(QString line,QMenuData *parent,const QString &name=QString::null) throw (InvalidMenuException);
 bool reserveAccel(const QString &accelDef,const QString &action);
 void initPaths();
 QString getIconFile(const QString &name);
 int addAction(const QString &action);
 void loadItemsDef(QString line,QMenuData *menu,QStringList prev=QStringList()) throw (InvalidMenuException);
 void loadItems(const QString &name,QMenuData *menu,QStringList prev=QStringList()) throw (InvalidMenuException);
 void loadToolBarItem(ToolBar *tb,const QString &item) throw (InvalidMenuException);
 ToolBar* loadToolbar(const QString &name,bool visible=true) throw (InvalidMenuException);
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
 /** Cache for menu items */
 MenuCache mCache;
 /** Cache for menu items - names */
 MenuNames mCacheName;
 /** Cache for icons */
 IconCache *cache;
 /** Main application window */
 QMainWindow *main;
};

} // namespace gui

#endif
