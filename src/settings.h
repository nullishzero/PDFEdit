#include "config.h"
#include <qapplication.h>
#include <qsettings.h>
#include <qmenudata.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstring.h>

typedef QMap<QString, int> ActionMap;
typedef QMap<int, QString> ActionMapInverse;

class Settings {
 QSettings *set;
 ActionMap action_map;
 ActionMapInverse action_map_i;
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
};

