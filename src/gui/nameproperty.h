#ifndef __NAMEPROPERTY_H__
#define __NAMEPROPERTY_H__

#include "stringproperty.h"

namespace gui {

using namespace pdfobjects;

/**
 Class for widget containing one editable property of type "Name"<br>
 Basically, CName is CString, but we might want differentiation in case we'll handle names specially in future<br>
 Used as one item type in property editor
 \brief Property editor widget for editing CName
*/
class NameProperty : public StringProperty {
Q_OBJECT
public:
 NameProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
 virtual ~NameProperty();
};

} // namespace gui 

#endif
