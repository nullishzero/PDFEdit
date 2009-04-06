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
#ifndef __STRINGOPTION_H__
#define __STRINGOPTION_H__

#include "option.h"
class QString;
class QLineEdit;
class QResizeEvent;

namespace gui {

/**
 Class for widget containing one editable setting of type string.<br>
 Also serves as a base class for editing some more specific types that can be represented in a string.
 \brief Widget to edit string option
*/
class StringOption : public Option {
 Q_OBJECT
public:
 StringOption(const QString &_key=0,QWidget *parent=0,const QString &defValue=QString::null);
 virtual ~StringOption();
 virtual void writeValue();
 virtual void readValue();
 virtual QSize sizeHint() const;
protected slots:
 void enableChange(const QString &newText);
protected:
 virtual void resizeEvent (QResizeEvent *e);
protected:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
};

} // namespace gui

#endif
