#ifndef __FILEOPTION_H__
#define __FILEOPTION_H__

#include "dialogoption.h"
class QString;

namespace gui {

class FileOption : public DialogOption {
 Q_OBJECT
public:
 FileOption(const QString &_key=0,QWidget *parent=0,const QString &defValue=0);
 virtual void dialog(QString &value);
 virtual ~FileOption();
};

} // namespace gui

#endif
