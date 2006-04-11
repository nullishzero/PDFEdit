#ifndef __MENU_H__
#define __MENU_H__

#include <qmenudata.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include "toolbar.h"
#include <qstringlist.h> 
#include <qstring.h>

namespace gui {

/** Action map type: mapping from to */
typedef QMap<QString, int> ActionMap;
/** Inverse action map: mapping from to  */
typedef QMap<int, QString> ActionMapInverse;
/** Icon Cache type: mapping from icon name to pixmap */
typedef QMap<QString, QPixmap*> IconCache;
/** Toolbar list type: mapping from toolbar name to toolbar object */
typedef QMap<QString, ToolBar*> ToolBarList;

class Menu {
public:
 Menu();
 ~Menu();
 QString getAction(int index);
 QMenuBar* loadMenu(QWidget *parent);
 ToolBarList loadToolBars(QMainWindow *parent);
 ToolBar* getToolbar(const QString &name);
 QStringList getToolbarList();
 void saveToolbar(QToolBar *tb,const QString &name,QMainWindow *main);
 void restoreToolbar(QToolBar *tb,const QString &name,QMainWindow *main);
 void saveToolbars(QMainWindow *main);
 void restoreToolbars(QMainWindow *main);
private:
 void initPaths();
 QString getIconFile(const QString &name);
 int addAction(const QString action);
 QPixmap* getIcon(const QString name);
 QString readItem(const QString name,const QString root="gui/items/");
 void loadItem(const QString name,QMenuData *parent=NULL,bool isRoot=FALSE);
 void loadToolBarItem(ToolBar *tb,QString item);
 ToolBar* loadToolbar(const QString name,QMainWindow *parent,bool visible=true);
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
};

} // namespace gui

#endif
