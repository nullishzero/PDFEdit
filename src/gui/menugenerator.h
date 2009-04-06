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
