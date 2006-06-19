/** @file
 Console Base - class containing extra functionalyty present only in commandline
 @author Martin Petricek
*/

#include "baseconsole.h"
#include "consolewriterconsole.h"
#include "util.h"
#include "settings.h"
#include <stdlib.h>

namespace gui {

using namespace util;

/**
 Create new Base class 
 @param params Commandline Parameters
*/
BaseConsole::BaseConsole(const QStringList &_params) : Base() {
 params=_params;
 //Console writer;
 consoleWriter=new ConsoleWriterConsole();
 setConWriter(consoleWriter);
}

/** destructor */
BaseConsole::~BaseConsole() {
 delete consoleWriter;
}


/**
 Run all initscripts.
 Gets name of initscripts from settings
*/
void BaseConsole::runInitScript() {
 QStringList initScripts=globalSettings->readPath("init_console","script/");
 int scriptsRun=runScriptList(initScripts);
 //Run list of initscripts from settings
 if (!scriptsRun) {
  //No init scripts found - print a warning
  conPrintError(tr("No init script found - check your configuration")+"!\n"+tr("Looked for","scripts")+":\n"+initScripts.join("\n"));
 }
 // Run initscripts from paths listed in settings,
 // initscript with same name is executed only once,
 // initscripts in later paths take priority
 QStringList initScriptPaths=globalSettings->readPath("init_path_console","script/");
 runScriptsFromPath(initScriptPaths);
}

/**
 Terminate the application
 @param returnCode Return code of application
 */
void BaseConsole::exitApp(int returnCode/*=0*/) {
 exit(returnCode);
}

/**
 Return list of commandline parameters
 @return parameter list
*/
QStringList BaseConsole::parameters() {
 return params;
}

} // namespace gui
