#include "config.h"
#include <qapplication.h>
#include <qsettings.h>
#include <qmenudata.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qpixmap.h>

typedef QMap<QString, int> ActionMap;
typedef QMap<int, QString> ActionMapInverse;
typedef QMap<QString, QPixmap*> IconCache;

class Settings {
 /** Cache storingh loaded icons */
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
private:
 int addAction(QString action);
 void init();
 void fatalError(QString q);
 void loadItem(QString name,QMenuData *parent=NULL,bool isRoot=FALSE);
 void initSettings();
 void flushSettings();
 QPixmap *getIcon(QString name);
};

