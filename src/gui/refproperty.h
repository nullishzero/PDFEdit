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
#ifndef __REFPROPERTY_H__
#define __REFPROPERTY_H__

#include "property.h"
#include <kernel/iproperty.h>
class QResizeEvent;
class QLineEdit;
class QPushButton;
namespace pdfobjects {
 class CPdf;
}

namespace gui {

using namespace pdfobjects;

/**
 RefProperty - class for widget containing one editable property of type "Ref"<br>
 Represented by editable line and button allowing to pick reference target from the list<br>
 Used as one item in property editor
 \brief Property editor widget for editing CRef
*/
class RefProperty : public Property {
Q_OBJECT
public:
 RefProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
 virtual QSize sizeHint() const;
 void resizeEvent(QResizeEvent *e);
 virtual ~RefProperty();
 virtual void setValue(IProperty *pdfObject);
 virtual void readValue(IProperty *pdfObject);
 virtual void setDisabled(bool disabled);
 virtual void applyReadOnly(bool _readonly);
 virtual bool isValid();
 IndiRef getValue();
 void setPdf(boost::weak_ptr<CPdf> _pdf);
protected slots:
 void selectRef();
 void emitChange();
 void enableChange(const QString &newText);
private:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
 /** Push button for GUI selection */
 QPushButton *pb;
 /** PDF stored for validating references */
 boost::weak_ptr<CPdf> pdf;
};

} // namespace gui

#endif
