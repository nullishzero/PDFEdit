#ifndef __STRINGPROPERTY_H__
#define __STRINGPROPERTY_H__

#include "property.h"
#include <qlineedit.h>

class StringProperty : public Property {
public:
 QSize sizeHint() const;
 void resizeEvent (QResizeEvent *e);
 StringProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=0);
 virtual ~StringProperty();
 void writeValue(void *pdfObject);
 void readValue(void *pdfObject);
protected:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
};
#endif
