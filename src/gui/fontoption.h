#ifndef __FONTOPTION_H__
#define __FONTOPTION_H__

#include "dialogoption.h"
class QString;

namespace gui {

/**
 FontOption - class for widget containing one editable string setting,
 while allowing to either type (allowing things like copy/paste) the font
 description (name), or use dialog to pick it. User's choice<br>
 Used as one item type in option window<br>
*/
class FontOption : public DialogOption {
 Q_OBJECT
public:
 FontOption(const QString &_key=0,QWidget *parent=0,const QString &defValue=0);
 virtual void dialog(QString &value);
 virtual ~FontOption();
};

} // namespace gui

#endif
