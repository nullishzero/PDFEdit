#ifndef __STRINGPROPERTY_H__
#define __STRINGPROPERTY_H__

#include "property.h"
#include <qlineedit.h>
#include <cobject.h>

using namespace pdfobjects;

class StringProperty : public Property {
//ADDED functions begin
//ADDED functions end
Q_OBJECT
public:
 QSize sizeHint() const;
 void resizeEvent (QResizeEvent *e);
 StringProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=0);
 virtual ~StringProperty();
 void writeValue(IProperty *pdfObject);
 void readValue(IProperty *pdfObject);
protected slots:
 void emitChange();
 void enableChange(const QString &newText);
protected:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
};
#endif
