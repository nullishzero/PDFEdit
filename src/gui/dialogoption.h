#ifndef __DIALOGOPTION_H__
#define __DIALOGOPTION_H__

#include "option.h"
class QString;
class QLineEdit;
class QPushButton;

namespace gui {

class DialogOption : public Option {
 Q_OBJECT
public:
 DialogOption(const QString &_key=0,QWidget *parent=0,const QString &defValue=QString::null);
 virtual ~DialogOption();
 virtual void writeValue();
 virtual void readValue();
 virtual QSize sizeHint() const;
 //Abstract functions
 /**
  Invoke the dialog to alter the edited value
  Reference to edited value is passed as parameter.
  This function should return after the dialog ended
  and may update the value as it seems approprieate
  based on user's interaction with the dialog
  @param value Reference to edited string
 */
 virtual void dialog(QString &value)=0;
protected slots:
 void invokeDialog();
protected:
 virtual void resizeEvent (QResizeEvent *e);
protected:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
 /** Push button for invoking the dialog */
 QPushButton *pb;
};

} // namespace gui

#endif
