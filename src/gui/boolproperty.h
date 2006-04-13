#ifndef __BOOLPROPERTY_H__
#define __BOOLPROPERTY_H__

#include "property.h"
class QCheckBox;
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;

class BoolProperty : public Property {
Q_OBJECT
public:
 QSize sizeHint() const;
 void resizeEvent (QResizeEvent *e);
 BoolProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=0);
 virtual ~BoolProperty();
 void writeValue(IProperty *pdfObject);
 void readValue(IProperty *pdfObject);
protected slots:
 void emitChange();
protected:
 /** checkbox used for editing the value */
 QCheckBox *ed;
};

} // namespace gui

#endif
