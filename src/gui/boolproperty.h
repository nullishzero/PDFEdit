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
#ifndef __BOOLPROPERTY_H__
#define __BOOLPROPERTY_H__

#include "property.h"
class QCheckBox;
class QResizeEvent;
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;

/**
 BoolProperty - class for widget containing one editable property of type "Bool"
 (represented by checkbox)<br>
 Used as one item type in property editor
 \brief Property editor widget for editing CBool
*/
class BoolProperty : public Property {
Q_OBJECT
public:
 virtual QSize sizeHint() const;
 virtual void resizeEvent (QResizeEvent *e);
 BoolProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
 virtual ~BoolProperty();
 virtual void setDisabled(bool disabled);
 virtual void applyReadOnly(bool _readonly);
 virtual void setValue(IProperty *pdfObject);
 virtual void readValue(IProperty *pdfObject);
 virtual bool isValid();
protected slots:
 void emitChange();
protected:
 /** checkbox used for editing the value */
 QCheckBox *ed;
};

} // namespace gui

#endif
