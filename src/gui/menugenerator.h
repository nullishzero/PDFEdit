#ifndef __MENUGENERATOR_H__
#define __MENUGENERATOR_H__

#include <qsettings.h>

class MenuGenerator {
public:
 MenuGenerator();
 ~MenuGenerator();
 void addToolbar(const QString &id,const QString &name,const QString &data);
 void addItem(const QString &id,const QString &name,const QString &data);
 void addMenu(const QString &id,const QString &name,const QString &data);
 void generate();
private:
 /** Settings object used to load configuration */
 QSettings *set;
 /** Installed toolbars */
 QStringList tbs;
};
#endif
