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
#ifndef __INTOPTION_H__
#define __INTOPTION_H__

#include "stringoption.h"
class QString;

namespace gui {

/**
 class for widget containing one editable setting of type integer<br>
 Used as one item type in option window<br>
 \brief Widget to edit integer option
*/
class IntOption : public StringOption {
 Q_OBJECT
public:
 IntOption(const QString &_key=0,QWidget *parent=0,int defValue=0);
 virtual ~IntOption();
};

} // namespace gui

#endif
