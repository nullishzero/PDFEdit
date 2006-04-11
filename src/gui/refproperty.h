#ifndef __REFPROPERTY_H__
#define __REFPROPERTY_H__

#include "property.h"
#include <qlineedit.h>
#include <qpushbutton.h>
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

class RefProperty : public Property {
Q_OBJECT
public:
 RefProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=0);
 QSize sizeHint() const;
 void resizeEvent (QResizeEvent *e);
 ~RefProperty();
 void writeValue(IProperty *pdfObject);
 void readValue(IProperty *pdfObject);
protected slots:
 void selectRef();
 void emitChange();
 void enableChange(const QString &newText);
private:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
 /** Push button for GUI selection */
 QPushButton *pb;
};

} // namespace gui

#endif
