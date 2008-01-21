/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __CONSOLEWRITER_H__
#define __CONSOLEWRITER_H__

#include <qwidget.h>

class QString;

namespace gui {

/**
 Console Writer class providing console output
 \brief Abstract class for console output
*/
class ConsoleWriter  {
public:
 ConsoleWriter();
 virtual ~ConsoleWriter();

 //Abstract functions

 /**
  Print one line to console, followed by a newline
  @param line String to print
 */
 virtual void printLine(const QString &line)=0;
 /**
  Print one line with error message to console, followed by a newline
  Visual style may (or may not) be different from that of "ordinary" line
  @param line String to print
 */
 virtual void printErrorLine(const QString &line)=0;
 /**
  Print/echo/store command that is to be executed
  @param cmd Command string
 */
 virtual void printCommand(const QString &cmd)=0;
};

} // namespace gui

#endif
