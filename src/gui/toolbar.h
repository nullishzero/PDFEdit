/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include <qtoolbar.h>

class QMainWindow;

namespace gui {

class ToolButton;

/**
 Class wrapping QToolBar, providing "click forwarding" from TButtons.
 Buttons must be added with addButton() for this to work properly
 \brief Application toolbar
 */
class ToolBar : public QToolBar {
 Q_OBJECT
public:
 ToolBar(const QString &label,QMainWindow *mainWindow);
 ToolBar(QMainWindow *parent,const char *name=0);
 void addButton(ToolButton *qb);
signals:
 /**
  Signal emitted when clicked on one of toolbar buttons.
  Send ID  number associated with it.
  @param id ID of button
 */
 void itemClicked(int id);
 /** 
  Send help message when mouse cursor enters/leaves on of toolbar buttons.
  Help message is sent on entering and QString::null on leaving.
  @param message Help message
 */
 void helpText(const QString &message);
public slots:
 void slotClicked(int id);
 void receiveHelpText(const QString &message);
};

} // namespace gui

#endif
