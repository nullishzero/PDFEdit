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
/** @file
 ConsoleWriterGui - class handling console output by sending it to command window
 @author Martin Petricek
*/

#include "consolewritergui.h"
#include "commandwindow.h"
#include "util.h"
#include <qstring.h>
#include <assert.h>

namespace gui {

/**
 Default constructor of ConsoleWriter
 @param _cmdLine CommandWindow widget used for output
*/
ConsoleWriterGui::ConsoleWriterGui(CommandWindow *_cmdLine) : ConsoleWriter() {
 cmdLine=_cmdLine;
 assert(cmdLine);
}

/** default destructor */
ConsoleWriterGui::~ConsoleWriterGui() {
}

/**
 Print one line to console, followed by a newline
 @param line String to print
*/
void ConsoleWriterGui::printLine(const QString &line) {
 cmdLine->addString(line);
}

/**
 Print one error line to console, followed by a newline
 @param line String to print
*/
void ConsoleWriterGui::printErrorLine(const QString &line) {
 cmdLine->addError(line);
}

/**
 Print/echo/store command that is to be executed
 @param cmd Command string
*/
void ConsoleWriterGui::printCommand(const QString &cmd) {
 cmdLine->addCommand(cmd);
}


} // namespace gui
