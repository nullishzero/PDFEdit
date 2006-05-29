#ifndef __FONTOPTION_H__
#define __FONTOPTION_H__

#include "dialogoption.h"
class QString;

namespace gui {

class FontOption : public DialogOption {
 Q_OBJECT
public:
 FontOption(const QString &_key=0,QWidget *parent=0,const QString &defValue=0);
 virtual void dialog(QString &value);
 virtual ~FontOption();
};

} // namespace gui

#endif
