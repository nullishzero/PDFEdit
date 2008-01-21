/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
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
