#ifndef __MENUGENERATOR_H__
#define __MENUGENERATOR_H__

#include <qsettings.h>
#include <qmap.h>

class MenuGenerator {
public:
 MenuGenerator();
 ~MenuGenerator();
 void addToolbar(const QString &id,const QString &name,const QString &data);
 void addItem(const QString &id,const QString &name,const QString &data);
 void addMenu(const QString &id,const QString &name,const QString &data);
 void generate();
 void check();
 void translate();
private:
 void addLocString(const QString &id,const QString &name);
 void setAvail(const QString &name);
private:
 /** Settings object used to load configuration */
 QSettings *set;
 /** Installed toolbars */
 QStringList tbs;
 /** Translation data */
 QStringList trans;
 /** Reachable menu items */
 QMap<QString,int> avail;
};
#endif
