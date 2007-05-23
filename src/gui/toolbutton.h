/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __TOOLBUTTON_H__
#define __TOOLBUTTON_H__

#include "qtcompat.h"
#include <qtoolbutton.h>
class QPixmap;
class QString;
class QIcon;

namespace gui {

/**
 Class wrapping QToolButton, adding button ID and providing also
 a different constructors (with QPixmap or QIcon)<br>
 Also, emits clicked(id) signal in addition to normal clicked() signal
 \brief Pushbutton in application toolbar
 */
class ToolButton : public QToolButton {
 Q_OBJECT
public:
 ToolButton(const QIcon *iconSet, const QString tooltip, int id, QWidget *parent=0, const char *name="");
 ToolButton(const QPixmap *icon, const QString tooltip, int id, QWidget *parent=0, const char *name="");
protected slots:
 void slotClicked();
protected:
 virtual void enterEvent(QEvent *e);
 virtual void leaveEvent(QEvent *e);
private:
 /** ID number of this button */
 int b_id;   
signals:
 /**
  Signal emitted when clicked on this button.
  The button sends ID of itself.
  @param id ID of button
 */
 void clicked(int id);
 /** 
  Send help message when mouse cursor enters/leaves the button.
  Help message is sent on enter and QString::null on leave.
  @param message Help message
 */
 void helpText(const QString &message);
};

} // namespace gui

#endif
