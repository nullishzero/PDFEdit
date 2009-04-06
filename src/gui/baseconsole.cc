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
 Console Base - class containing extra functionalyty present only in commandline
 @author Martin Petricek
*/

#include "baseconsole.h"
#include "consolewriterconsole.h"
#include "util.h"
#include "settings.h"
#include "kernel/cpage.h"
#include <stdlib.h>

namespace gui {

using namespace util;

/**
 Create new Base class
 @param _params Commandline Parameters
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
  conPrintError(gui::Base::tr("No init script found - check your configuration")+"!\n"+gui::Base::tr("Looked for","scripts")+":\n"+initScripts.join("\n"));
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
void BaseConsole::exit(int returnCode/*=0*/) {
 std::exit(returnCode);
}

/**
 Return list of commandline parameters
 @return parameter list
*/
QStringList BaseConsole::parameters() {
 return params;
}

/**
 Return first parameter from list of parameters
 and remove it from the list. Other parameters are shifted to take the empty space
 @return First parameter or NULL if no parameters
*/
QString BaseConsole::takeParameter() {
 if (!params.count()) return QString::null;//No parameters
 QString p0=params[0];
 params.pop_front();
 return p0;
}

double BaseConsole::convertPixmapPosToPdfPos_x ( double fromX, double fromY ) {
 pdfobjects::DisplayParams displayParams = pdfobjects::DisplayParams();
 double toX, toY;
 displayParams.convertPixmapPosToPdfPos( fromX, fromY, toX, toY );
 return toX;
}
double BaseConsole::convertPixmapPosToPdfPos_y ( double fromX, double fromY ) {
 pdfobjects::DisplayParams displayParams = pdfobjects::DisplayParams();
 double toX, toY;
 displayParams.convertPixmapPosToPdfPos( fromX, fromY, toX, toY );
 return toY;
}
double BaseConsole::convertPdfPosToPixmapPos_x ( double fromX, double fromY ) {
 pdfobjects::DisplayParams displayParams = pdfobjects::DisplayParams();
 double toX, toY;
 displayParams.convertPdfPosToPixmapPos( fromX, fromY, toX, toY );
 return toX;
}
double BaseConsole::convertPdfPosToPixmapPos_y ( double fromX, double fromY ) {
 pdfobjects::DisplayParams displayParams = pdfobjects::DisplayParams();
 double toX, toY;
 displayParams.convertPdfPosToPixmapPos( fromX, fromY, toX, toY );
 return toY;
}

} // namespace gui
