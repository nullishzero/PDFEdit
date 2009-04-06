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
#ifndef __OPTION_H__
#define __OPTION_H__

#include <qwidget.h>
class QString;

namespace gui {

/**
 Class for widget containing one editable setting<br>
 Abstract class, must be subclassed for editing specific option types (integer, string ...)
 \brief Abstract class for option editing widget
*/
class Option : public QWidget{
 Q_OBJECT
public:
 Option(const QString &_key=0,QWidget *parent=0);
 virtual QString getName();
 virtual ~Option();
 //Abstract functions
 /** write edited value to settings (using key specified in contructor) */
 virtual void writeValue()=0;
 /** read value from settings for editing (using key specified in contructor) */
 virtual void readValue()=0;
protected:
 /** Name of this option */
 QString key;
 /** Was the value changed? */
 bool changed;
};

} // namespace gui

#endif
