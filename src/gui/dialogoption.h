/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __DIALOGOPTION_H__
#define __DIALOGOPTION_H__

#include "option.h"
class QString;
class QLineEdit;
class QPushButton;

namespace gui {

/**
 Abstract class for widget containing one editable string setting,
 with nearby "..." button allowing to invoke dialog to change the setting
 in some possibly  more user-friently way<br>
 Subclasses should reimplement the dialog function to invoke some dialog and change the string value accordingly
 \brief Abstract class for option editable as string and pickable by dialog
*/
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
 void enableChange(const QString &newText);
 void invokeDialog();
protected:
 virtual void resizeEvent (QResizeEvent *e);
private:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
 /** Push button for invoking the dialog */
 QPushButton *pb;
};

} // namespace gui

#endif
