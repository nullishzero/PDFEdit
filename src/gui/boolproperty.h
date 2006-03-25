#ifndef __BOOLPROPERTY_H__
#define __BOOLPROPERTY_H__

#include "property.h"
#include <qcheckbox.h>
#include <cobject.h>

using namespace pdfobjects;

class BoolProperty : public Property {
public:
 QSize sizeHint() const;
 void resizeEvent (QResizeEvent *e);
 BoolProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=0);
 virtual ~BoolProperty();
 void writeValue(IProperty *pdfObject);
 void readValue(IProperty *pdfObject);
protected:
 /** checkbox used for editing the value */
 QCheckBox *ed;
};

#endif
