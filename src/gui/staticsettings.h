/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#ifndef __STATICSETTINGS_H__
#define __STATICSETTINGS_H__

#include <qmap.h>
#include <qstring.h>

class QStringList;

namespace gui {
/**
 Class managing static settings, loading them from file
 \brief Loading static settings from file
 */
class StaticSettings {
public:
 QStringList entryList(const QString &prefix);
 bool tryLoad(const QString &dirName,const QString &fileName);
 bool tryLoad(const QString &fileName);
 QString readEntry(const QString &key,const QString &defValue=QString::null);
 StaticSettings();
 ~StaticSettings();
private:
 /** Map containing all settings */
 QMap<QString,QString> set;
};

} // namespace gui

#endif
