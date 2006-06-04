#ifndef __STRINGPROPERTY_H__
#define __STRINGPROPERTY_H__

#include "property.h"
class QLineEdit;
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;

/**
 Class for widget containing one editable property of type "String"<br>
 Used as one item type in property editor
*/
class StringProperty : public Property {
Q_OBJECT
public:
 virtual QSize sizeHint() const;
 virtual void resizeEvent (QResizeEvent *e);
 StringProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
 virtual ~StringProperty();
 virtual void writeValue(IProperty *pdfObject);
 virtual void readValue(IProperty *pdfObject);
 virtual void setDisabled(bool disabled);
 virtual void applyReadOnly(bool _readonly);
 virtual bool isValid();
protected slots:
 void emitChange();
 void enableChange(const QString &newText);
protected:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
};

} // namespace gui

#endif
