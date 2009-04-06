/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
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
