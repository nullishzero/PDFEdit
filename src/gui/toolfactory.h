/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __TOOLFACTORY_H__
#define __TOOLFACTORY_H__

class QMainWindow;
class QString;
class QWidget;

namespace gui {

class ToolBar;

/**
 Factory class for creating special toolbar items
 \brief Special Toolbar item factory
 */
class ToolFactory {
public:
 static QWidget *specialItem(ToolBar *tb,const QString &item,QMainWindow *main);
};

} // namespace gui

#endif
