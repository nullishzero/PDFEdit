/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __ABOUTWINDOW_H__
#define __ABOUTWINDOW_H__

#include <qwidget.h>

namespace gui {

/**
 Class representing about window.<br>
 Display authors, application name, version and some info.
 \brief Window displaying program version and authors 
*/
class AboutWindow : public QWidget {
Q_OBJECT
public:
 AboutWindow(QWidget *parent=0,const char *name=0);
 ~AboutWindow();
protected:
 void closeEvent(QCloseEvent *e);
};

} // namespace gui

#endif
