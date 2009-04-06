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
#ifndef __COMBOOPTION_H__
#define __COMBOOPTION_H__

#include "option.h"
class QString;
class QComboBox;
class QStringList;
class QResizeEvent;

namespace gui {

/**
 Class for widget containing one editable setting of type string, selectable from combobox<br>
 If current setting specify item not in the list, the first item in list is shown instead<br>
 User is unable to specify string not in the list<br>
 Used as one item type in option window<br>
 \brief Widget to edit option by picking from list of choices
*/
class ComboOption : public Option {
 Q_OBJECT
public:
 ComboOption(const QStringList &_values,const QStringList &_valueDesc,const QString &_key=0,QWidget *parent=0);
 ComboOption(const QStringList &_values,const QString &_key=0,QWidget *parent=0);
 ~ComboOption();
 virtual void writeValue();
 virtual void readValue();
 virtual QSize sizeHint() const;
 void setCaseSensitive(bool value);
protected:
 virtual void resizeEvent (QResizeEvent *e);
private:
 void init(const QStringList &_values, const QStringList &_desc);
protected slots:
 void itemActivated(int index);
protected:
 /** edit control used for editing the value (combo box) */
 QComboBox *ed;
 /** List of values in the control. */
 QStringList values;
 /** Are items case sensitive? */
 bool caseSensitive;
 /** Index of selected item */
 int idx;
};

} // namespace gui

#endif
