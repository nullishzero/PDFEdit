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
