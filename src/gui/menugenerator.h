/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __MENUGENERATOR_H__
#define __MENUGENERATOR_H__

#include "staticsettings.h"
#include <qmap.h>

/**
 Generator, checker and translation generator of menu configuration<br>
 This is helper utility used to:<br>
  - check menus for translatable strings and write them to .menu-trans.h,
    so they will be found by lupdate utility<br>
  - check menus for unreferenced items<br>
  - generate initial menu (now obsolete)<br>
*/
class MenuGenerator {
public:
 MenuGenerator();
 ~MenuGenerator();
 void check();
 void translate();
 static bool special(const QString &itemName);
private:
 void addLocString(const QString &id,const QString &name);
 void setAvail(const QString &name);
private:
 /** Settings object used to load configuration */
 gui::StaticSettings *set;
 /** Installed toolbars */
 QStringList tbs;
 /** Translation data */
 QStringList trans;
 /** Reachable menu items */
 QMap<QString,int> avail;
};

#endif
