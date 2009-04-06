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
#ifndef __BOOLOPTION_H__
#define __BOOLOPTION_H__

#include "option.h"
class QString;
class QCheckBox;
class QResizeEvent;

namespace gui {

/**
 Class for widget containing one editable setting of type bool<br>
 Used as one item type in option window<br>
 \brief Widget to edit boolean option
*/
class BoolOption : public Option {
 Q_OBJECT
public:
 BoolOption(const QString &_key,QWidget *parent=0,bool _defValue=false);
 virtual ~BoolOption();
 virtual QSize sizeHint() const;
 virtual void writeValue();
 virtual void readValue();
protected:
 virtual void resizeEvent (QResizeEvent *e);
protected slots:
 void boolChange();
protected:
 /** checkbox used for editing the value */
 QCheckBox *ed;
 /** Default value if option not found */
 bool defValue;
};

} // namespace gui

#endif
