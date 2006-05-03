#ifndef __REFPROPERTY_H__
#define __REFPROPERTY_H__

#include "property.h"
class QLineEdit;
class QPushButton;
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;

class RefProperty : public Property {
Q_OBJECT
public:
 RefProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=0);
 virtual QSize sizeHint() const;
 virtual void resizeEvent (QResizeEvent *e);
 virtual ~RefProperty();
 virtual void writeValue(IProperty *pdfObject);
 virtual void readValue(IProperty *pdfObject);
 virtual void setReadOnly(bool _readonly);
 virtual bool isValid();
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
