#ifndef __INTOPTION_H__
#define __INTOPTION_H__

#include <qstring.h>
#include "stringoption.h"

class IntOption : public StringOption {
 Q_OBJECT
public:
 IntOption(const QString &_key=0,QWidget *parent=0);
 virtual ~IntOption();
};

#endif
