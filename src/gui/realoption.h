#ifndef __REALOPTION_H__
#define __REALOPTION_H__

#include <qstring.h>
#include "stringoption.h"

namespace gui {

class RealOption : public StringOption {
 Q_OBJECT
public:
 RealOption(const QString &_key=0,QWidget *parent=0);
 virtual ~RealOption();
};

} // namespace gui

#endif
