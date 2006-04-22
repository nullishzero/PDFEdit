#ifndef __REALOPTION_H__
#define __REALOPTION_H__

#include "stringoption.h"
class QString;

namespace gui {

class RealOption : public StringOption {
 Q_OBJECT
public:
 RealOption(const QString &_key=0,QWidget *parent=0,double defValue=0);
 virtual ~RealOption();
};

} // namespace gui

#endif
