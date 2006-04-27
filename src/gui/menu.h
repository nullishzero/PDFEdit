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

namespace gui {

class ToolBar;

/** Action map type: mapping from to */
typedef QMap<QString, int> ActionMap;
/** Inverse action map: mapping from to  */
typedef QMap<int, QString> ActionMapInverse;
/** Icon Cache type: mapping from icon name to pixmap */
typedef QMap<QString, QPixmap*> IconCache;
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
 Menu();
 ~Menu();
 QString getAction(int index);
 QMenuBar* loadMenu(QWidget *parent) throw (InvalidMenuException);
 ToolBarList loadToolBars(QMainWindow *parent) throw (InvalidMenuException);
 ToolBar* getToolbar(const QString &name);
 QStringList getToolbarList();
 void saveToolbars(QMainWindow *main);
 void restoreToolbars(QMainWindow *main);
 static QString readItem(const QString name,const QString root="gui/items/");
private:
 void addItem(QString line,QString name,QMenuData *parent) throw (InvalidMenuException);
 void invalidItem(const QString &type,const QString &name,const QString &line,const QString &expected=QString::null) throw (InvalidMenuException);
 bool reserveAccel(const QString &accelDef,const QString &action);
 void initPaths();
 QString getIconFile(const QString &name);
 int addAction(const QString action);
 QPixmap* getIcon(const QString name);
 void loadItem(const QString name,QMenuData *parent=NULL,bool isRoot=FALSE,QStringList prev=QStringList()) throw (InvalidMenuException);
 void loadToolBarItem(ToolBar *tb,QString item) throw (InvalidMenuException);
 ToolBar* loadToolbar(const QString name,QMainWindow *parent,bool visible=true) throw (InvalidMenuException);
private:
 /** List with paths to application icons */
 QStringList iconPath;
 /** Cache storing loaded icons */
 IconCache iconCache;
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
};

} // namespace gui

#endif
