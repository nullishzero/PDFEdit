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
#ifndef __ADDITEMDIALOG_H__
#define __ADDITEMDIALOG_H__

#include "selfdestructivewidget.h"
#include "qtcompat.h"
#include <kernel/iproperty.h>
#include <kernel/cobject.h>
namespace pdfopbjects {
 class CPdf;
}

class QBoxLayout;
class QFrame;
class QRadioButton;
class Q_ButtonGroup;
class QLineEdit;
class QLabel;

namespace gui {

using namespace pdfobjects;

class Property;

/** Number of property types that can be added in AddItemDialog */
const int addDialogPropertyTypes=8;

/**
 Class representing dialog for adding new item to Dictionary/array<br>
 Dialog does not block focus of main window and you can have multiple of them opened simultaneously
 (even for same Dict / Array)<br>
 Dialog closes itself, when the relevant document is closed
 \brief Dialog for adding items to Dictionaries or Arrays
*/
class AddItemDialog : public SelfDestructiveWidget {
Q_OBJECT
public:
 ~AddItemDialog();
 static AddItemDialog* create(QWidget *parent,boost::shared_ptr<CDict> cont);
 static AddItemDialog* create(QWidget *parent,boost::shared_ptr<CArray> cont);
public slots:
 void itemDeleted(boost::shared_ptr<IProperty> it);
protected slots:
 void posNumSet(bool on);
 void posEndSet(bool on);
 void buttonSelected(int id);
 bool commit();
 void commitClose();
protected:
 void message(const QString &message);
 void error(const QString &message);
 AddItemDialog(QWidget *parent=0,const char *name=0);
 void setItem(boost::shared_ptr<IProperty> it);
private:
 /** Object (Dict or Array) into which any subitems will be added */
 boost::shared_ptr<IProperty> item;
 /** Array with properties */
 Property* props[addDialogPropertyTypes];
 /** Array with property names (selectors) */
 QRadioButton* labels[addDialogPropertyTypes];
 /** Frame containing controls to select name of new property (Dict) or end of array for appending (Array) */
 QFrame *target;
 /** Index of item that is selected for editing */
 int selectedItem;
 /** Name of new property, if adding to Dict */
 QLineEdit *propertyName;
 /** Adding to array? */
 bool usingArray;
 /** General layout of this widget */
 QBoxLayout *layout;
 /** Box holding buttons */
 QFrame *qbox;
 /** Group with content items */
 Q_ButtonGroup *items;
 /** Label used to show message */
 QLabel *msg;
 /** Settings used to save dialog position (dict/array) - this is useful, as these can have different sizes */
 QString settingName;
 /** Pdf document (for validating reference) */
 boost::shared_ptr<CPdf> pdf;
 /** Radiobutton for inserting at arbitrary position in array */
 QRadioButton *posNum;
 /** Radiobutton for appending at end of array */
 QRadioButton *posEnd;
 /** Edit box for editing array position at which the element should be inserted */
 QLineEdit *arrayPos;
};

} // namespace gui

#endif
