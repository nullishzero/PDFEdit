#ifndef __INTOPTION_H__
#define __INTOPTION_H__

#include "stringoption.h"
class QString;

namespace gui {

class IntOption : public StringOption {
 Q_OBJECT
public:
 IntOption(const QString &_key=0,QWidget *parent=0,int defValue=0);
 virtual ~IntOption();
};

} // namespace gui

#endif
