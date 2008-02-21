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
#ifndef __PROPERTYMODECONTROLLER_H__
#define __PROPERTYMODECONTROLLER_H__

#include <kernel/modecontroller.h>

class QString;

namespace gui {

using namespace configuration;

/**
 PropertyModeController - class providing wrapper around single ModeController
 instance and managing its configuration
 \brief Mode controller wrapper
*/
class PropertyModeController {
public:
 ModeController* get();
 static PropertyModeController* getInstance();
 PropertyMode mode(const QString &type,const QString &name);
 ~PropertyModeController();
private:
 PropertyModeController();
};

} // namespace gui

#endif
