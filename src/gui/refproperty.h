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

/**
 RefProperty - class for widget containing one editable property of type "Ref"<br>
 Represented by editable line and button allowing to pick reference target from the list<br>
 Used as one item in property editor
*/
class RefProperty : public Property {
Q_OBJECT
public:
 RefProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
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
