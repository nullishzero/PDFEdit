/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
/** @file
 ConsoleWriterConsole - class handling console output by sending it to STDOUT
 @author Martin Petricek
*/

#include "consolewriterconsole.h"
#include "util.h"
#include <qstring.h>
#include <assert.h>
#include <iostream>

namespace gui {

using namespace std;

/**
 Default constructor of ConsoleWriter
*/
ConsoleWriterConsole::ConsoleWriterConsole() : ConsoleWriter() {
 //no extra initialization
}

/** default destructor */
ConsoleWriterConsole::~ConsoleWriterConsole() {
}

/**
 Print one line to console, followed by a newline
 @param line String to print
*/
void ConsoleWriterConsole::printLine(const QString &line) {
 cout << util::convertFromUnicode(line,util::CON) << endl;
}

/**
 Print one error line to console, followed by a newline
 @param line String to print
*/
void ConsoleWriterConsole::printErrorLine(const QString &line) {
 cout << "! " << util::convertFromUnicode(line,util::CON) << endl;
}

/**
 Print/echo/store command that is to be executed
 @param cmd Command string
*/
void ConsoleWriterConsole::printCommand(const QString &cmd) {
 cout << "> " << util::convertFromUnicode(cmd,util::CON) << endl;
}


} // namespace gui
