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
#ifndef __ICONCACHE_H__
#define __ICONCACHE_H__

#include "qtcompat.h"

class QString;
class QPixmap;
class QIcon;

namespace gui {

/**
 Class responsible for loading and caching icons<br>
 There can be multiple instances of this class at once,
 but all of them share common cache
 \brief Class caching loaded icons
*/
class IconCache {
public:
 IconCache();
 ~IconCache();
 QPixmap* getIcon(const QString &name);
 QIcon* getIconSet(const QString &name);
private:
 QString getIconFile(const QString &name);
};

} // namespace gui

#endif
