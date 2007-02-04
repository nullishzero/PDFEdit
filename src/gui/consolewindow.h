/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __CONSOLEWINDOW_H__
#define __CONSOLEWINDOW_H__

class QString;
class QStringList;

namespace gui {

class BaseConsole;

/**
 ConsoleWindow - class handling commandline mode
 \brief Commandline mode Main "window"
 */
class ConsoleWindow {
public:
 ConsoleWindow(const QStringList &params);
 void init();
 void run(const QString &scriptName);
 void runFile(const QString &scriptName);
 void eval(const QString &code);
 void call(const QString &func);
 ~ConsoleWindow();
private:
 /** Base used to host scripts */
 BaseConsole *base;
};

} // namespace gui

#endif
