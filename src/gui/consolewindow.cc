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
 ConsoleWindow - class representing "commandline window"
*/
#include "baseconsole.h"
#include "consolewindow.h"
#include "settings.h"
#include "util.h"
#include <qstring.h>
#include <utils/debug.h>

namespace gui {

using namespace std;
using namespace util;

/**
 constructor of ConsoleWindow
 @param params Commandline Parameters
*/
ConsoleWindow::ConsoleWindow(const QStringList &params) {
 //Base for scripting
 base=new BaseConsole(params);
}

/**
 Run initscripts
*/
void ConsoleWindow::init() {
 //Run console initscript
 base->runInitScript();
}

/**
 Runs script from given file, looking in script path first
 @param scriptName name of file with QT Script to run
*/
void ConsoleWindow::run(const QString &scriptName) {
 base->run(scriptName);
}

/**
 Runs script from given file
 @param scriptName name of file with QT Script to run
*/
void ConsoleWindow::runFile(const QString &scriptName) {
 base->run(scriptName,true);
}

/**
 Run given script code
 @param code Code to evaluate
 */
void ConsoleWindow::eval(const QString &code) {
 base->runScript(code);
}

/**
 Run given callback function
 @param func Function to call
 */
void ConsoleWindow::call(const QString &func) {
 base->call(func);
}

/** default destructor */
ConsoleWindow::~ConsoleWindow() {
 base->stopScript();
 delete base;
}

} // namespace gui
