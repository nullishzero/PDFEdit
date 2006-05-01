#ifndef __ADDITEMDIALOG_H__
#define __ADDITEMDIALOG_H__

#include <qwidget.h>
#include <iproperty.h>
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

class AddItemDialog : public QWidget {
Q_OBJECT
public:
 ~AddItemDialog();
 static AddItemDialog* create(QWidget *parent,boost::shared_ptr<CDict> cont);
 static AddItemDialog* create(QWidget *parent,boost::shared_ptr<CArray> cont);
protected slots:
 void commit();
 void commitClose();
protected:
 AddItemDialog(QWidget *parent=0,const char *name=0);
 void setItem(boost::shared_ptr<IProperty> it);
 void closeEvent(QCloseEvent *e);
private:
 /** Object (Dict or Array) into which any subitems will be added */
 boost::shared_ptr<IProperty> item;
};

} // namespace gui

#endif
