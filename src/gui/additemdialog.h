#ifndef __ADDITEMDIALOG_H__
#define __ADDITEMDIALOG_H__

#include "selfdestructivewidget.h"
#include <iproperty.h>
#include <cobject.h>

class QBoxLayout;
class QFrame;
class QRadioButton;
class QButtonGroup;
class QLineEdit;
class QHBox;
class QLabel;

namespace gui {

using namespace pdfobjects;

class Property;

/** Number of property types that can be added in AddItemDialog */
const int addDialogPropertyTypes=8;

class AddItemDialog : public SelfDestructiveWidget {
Q_OBJECT
public:
 ~AddItemDialog();
 static AddItemDialog* create(QWidget *parent,boost::shared_ptr<CDict> cont);
 static AddItemDialog* create(QWidget *parent,boost::shared_ptr<CArray> cont);
protected slots:
 void buttonSelected(int id);
 bool commit();
 void commitClose();
protected:
 void message(const QString &message);
 void error(const QString &message);
 AddItemDialog(QWidget *parent=0,const char *name=0);
 void setItem(boost::shared_ptr<IProperty> it);
 void closeEvent(QCloseEvent* e);
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
 QBoxLayout *l;
 /** Box holding buttons */
 QHBox *qb;
 /** Group with content items */
 QButtonGroup *items;
 /** Label used to show message */
 QLabel *msg;
 /** Settings used to save dialog position (dict/array) - this is useful, as these can have different sizes */
 QString settingName;
};

} // namespace gui

#endif
