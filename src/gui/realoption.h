#ifndef __REALOPTION_H__
#define __REALOPTION_H__

#include "stringoption.h"
class QString;

namespace gui {

/**
 RealOption - class for widget containing one editable setting of type float
 (internally represented as a double)<br>
 Used as one item in option window
*/
class RealOption : public StringOption {
 Q_OBJECT
public:
 RealOption(const QString &_key=0,QWidget *parent=0,double _defValue=0);
 virtual ~RealOption();
};

} // namespace gui

#endif
