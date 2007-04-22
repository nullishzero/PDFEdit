/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __SELFDESTRUCTIVEWIDGET_H__
#define __SELFDESTRUCTIVEWIDGET_H__

#include <qwidget.h>
#include "qtcompat.h"

namespace gui {

/**
 Widget class that will destruct
 when appropriate signal from killer window is sent<br>
 Killer window can be any window, but must emit signal
 "selfDestruct()" for this widget to work<br>
 Used for helper dialogs that should close themselves
 when file/item they are editing is also closed<br>
 \brief Widget destroying itself on certain signal
*/
class SelfDestructiveWidget : public QWidget {
Q_OBJECT
public:
#ifdef QT4
 SelfDestructiveWidget(QWidget *killer,QWidget *parent=0, Qt::WindowFlags f=0);
 SelfDestructiveWidget(QWidget *killer,QWidget *parent=0, const char *name=0, Qt::WindowFlags f=0);
#else
 SelfDestructiveWidget(QWidget *killer,QWidget *parent=0, const char *name=0, WFlags f=0);
#endif
 ~SelfDestructiveWidget();
};

} // namespace gui

#endif
