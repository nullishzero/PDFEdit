#ifndef __SETTINGS_H__
#define __SETTINGS_H__
#include "config.h"
#include <qapplication.h>
#include <qsettings.h>
#include <qmenudata.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qpixmap.h>
#include "toolbar.h"


typedef QMap<QString, int> ActionMap;
typedef QMap<int, QString> ActionMapInverse;
typedef QMap<QString, QPixmap*> IconCache;
typedef QValueList<ToolBar*> ToolBarList;

/** Class managing settings and also loading configurable menus, toolbars and keyboard shortcuts */

class Settings {
 /** Cache storing loaded icons */
 IconCache iconCache;
 /** Settings object used to load configuration */
 QSettings *set;
 /** Mapping between menu IDs and actions */
 ActionMap action_map;
 /** Inverse mapping between menu IDs and actions */
 ActionMapInverse action_map_i;
 /** Incrementing action index for menu items */
 int action_index;
public:
 Settings();
 ~Settings();
 QMenuBar *loadMenu(QWidget *parent);
 QString getAction(int index);
 void flushSettings();
 ToolBarList loadToolBars(QMainWindow *parent);
private:
 int addAction(const QString action);
 void init();
 QString readItem(const QString key,const QString root="gui/items/");
 void loadItem(const QString name,QMenuData *parent=NULL,bool isRoot=FALSE);
 void initSettings();
 QPixmap *getIcon(const QString name);
 ToolBar *loadToolbar(const QString name,QMainWindow *parent);
 void loadToolBarItem(ToolBar *tb,QString item);

};

#endif
