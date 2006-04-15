#ifndef __COMBOOPTION_H__
#define __COMBOOPTION_H__

#include "option.h"
class QString;
class QComboBox;

namespace gui {

class ComboOption : public Option {
 Q_OBJECT
public:
 ComboOption(const QStringList &values,const QString &_key=0,QWidget *parent=0);
 ~ComboOption();
 virtual void writeValue();
 virtual void readValue();
 virtual QSize sizeHint() const;
protected:
 virtual void resizeEvent (QResizeEvent *e);
protected:
 /** edit control used for editing the value (combo box) */
 QComboBox *ed;
};

} // namespace gui

#endif
