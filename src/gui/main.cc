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
 Main function<br>
 Handle commandline options and then open editor window(s)
 - unless commandline parameters specify something different
 @author Martin Petricek
*/

#include "qtcompat.h"
#include "args.h"
#include "config.h"
#include "consolewindow.h"
#include "optionwindow.h"
#include "pdfeditwindow.h"
#include "settings.h"
#include "util.h"
#include "version.h"
#include "kernel/cannotation.h"
#include "kernel/pdfedit-core-dev.h"
#include <iostream>
#include <qapplication.h>
#include <qdir.h>
#include <qtranslator.h>
#include <qstringlist.h>
#include <stdlib.h>
#include <time.h>
#include <utils/debug.h>

using namespace std;
using namespace gui;
using namespace pdfobjects;
using namespace pdfobjects::utils;

/** Path to directory in which the binary resides */
QString appPath;
/** run in commandline mode? */
bool consoleMode=false;
/** Types of scripts to run (commandline mode) */
QStringList runScript;
/** Script parameters (script code/filename) (commandline mode) */
QStringList runScriptParam;

/** delete settings object (and save settings)
 This function is called at application exit
 */
void saveSettings(void) {
 delete globalSettings;//this causes settings to be saved to disk
}

/**
 handle --help parameter<br>
 Print help to STDOUT and exit
*/
void handleHelp(){
 printHeader();
 cout << util::convertFromUnicode(QObject::tr("Usage:"),util::CON)
      << " " << util::convertFromUnicode(binName,util::CON)
      << " [" << util::convertFromUnicode(QObject::tr("option(s)"),util::CON)
      << "] [" << util::convertFromUnicode(QObject::tr("files(s)"),util::CON) << "]" << endl;
 handleHelpOptions();
}

/**
 handle --version parameter<br>
 Print version to STDOUT and exit
 */
void handleVersion(){
 cout << PDFEDIT_VERSION << endl;
 exit(0);
}

/**
 handle -d [n] parameter<br>
 Change debugging level
 @param param Parameter passed
*/
void handleDebug(const QString &param){
 util::setDebugLevel(param);
}

/**
 handle -script [file] parameter<br>
 Run script file
 @param param Parameter passed
*/
void handleScript(const QString &param){
 runScript+="script";
 runScriptParam+=param;
}


/**
 handle -run [file] parameter<br>
 Run script file
 @param param Parameter passed
*/
void handleRun(const QString &param){
 runScript+="run";
 runScriptParam+=param;
}

/**
 handle -eval [code] parameter<br>
 Evaluate script code
 @param param Parameter passed
*/
void handleEval(const QString &param){
 runScript+="eval";
 runScriptParam+=param;
}

/**
 handle -console parameter<br>
 Run in commandline mode
*/
void handleConsole(){
 consoleMode=true;
}

/**
 handle -s [file] parameter<br>
 Run script file + set console mode
 @param param Parameter passed
*/
void handleScriptShort(const QString &param){
 handleConsole();
 handleScript(param);
}

/** QApplication - main application */
QApplication *q_App;

/**
 Run scripts/code specified on commandline in given editor window
 @param w Editor window
*/
void runCmdScripts(PdfEditWindow *w) {
 //Run scripts given on commandline in given PDFedit window
 for (unsigned int i=0;i<runScript.count();i++) {
  QString typ=runScript[i];
  guiPrintDbg(debug::DBG_DBG, "Script cmdline " << Q_OUT(typ) << " " << Q_OUT(runScriptParam[i]));
       if (typ=="run")		w->runFile(runScriptParam[i]);
  else if (typ=="script")	w->run(runScriptParam[i]);
  else if (typ=="eval")		w->eval(runScriptParam[i]);
  else {
   assert(0);
  }
 }
}

/**
 Main - load settings, load translation and launch
 main window(s) according to settings and commandline parameters
 @param argc Argument count
 @param argv Commandline arguments
 @return Executable return code
*/
int main(int argc, char *argv[]){
 //We need this so we can use things like --version and --help even without X11 connection
#ifdef Q_WS_X11
 bool useGUI=getenv("DISPLAY")!=0;
#else
 bool useGUI=true;
#endif
 QApplication app(argc,argv,useGUI);

 q_App=&app;
 //Get application path
 appPath=app.applicationDirPath();
 appPath=QDir(appPath).absPath();
 //Seed random number generator
 srand(time(NULL));
 //Translation support
 QTranslator translator;
 //Try LC_ALL, LC_MESSAGES and LANG - first one that is set is used
 const char *env_lang=getenv("LC_ALL");
 if (!env_lang) env_lang=getenv("LC_MESSAGES");
 if (!env_lang) env_lang=getenv("LANG");
 if (env_lang) {//LC_ALL/LC_MESSAGES/LANG variable is present in environment -> attempt to load localization
  guiPrintDbg(debug::DBG_WARN,"LANG= " << env_lang);
  QString lang=QString("pdfedit_")+env_lang;
  //look for translation file in DATA_PATH
  if (!translator.load(lang,QString(DATA_PATH)+"/lang")) {
   //Look for translation file in config directory in $HOME
   if (!translator.load(lang,QDir::home().path()+"/"+CONFIG_DIR+"/lang")) {
    //Look in binary path - testing compilations and (possibly) windows builds
    if (!translator.load(lang,appPath+"/lang")) {
     guiPrintDbg(debug::DBG_WARN,"Translation file " << Q_OUT(lang) << " not found");
    }
   }
  }
 }
 app.installTranslator(&translator);

 // initializes all global configuration stuff
 if(pdfedit_core_dev_init(&argc, &argv))
 {
   guiPrintDbg(debug::DBG_PANIC, "Unable to initialize pdfedit core");
   return 1;
 }

 //parse commandline parameters
 /*
  Whole name of one parameter should not be prefix of another parameter, as unpredictable behaviour can occur,
  for example -d and -def, in this case there is no way to recognize between -def option and -d option
  with 'ef' as parameter and it is undefined what case of these two will be recognized
 */
 optionHandler("--help",handleHelp,QObject::tr("Print help and exit"));
 optionHandler("--version",handleVersion,QObject::tr("Print version and exit"));
 optionHandlerParam("-d","n",handleDebug,QObject::tr("Set debug messages verbosity")+" "+QObject::tr("(n = -1 .. 5)"));

 optionHandler("-console",handleConsole,QObject::tr("Run in commandline mode"));
 optionHandlerParam("-script",QObject::tr("file"),handleScript,QObject::tr("Run script from script path or current directory if not found"));
 optionHandlerParam("-run",QObject::tr("file"),handleRun,QObject::tr("Run script from current directory"));
 optionHandlerParam("-eval",QObject::tr("code"),handleEval,QObject::tr("Evaluate script code"));
 optionHandlerParam("-s",QObject::tr("file"),handleScriptShort,QObject::tr("Shortcut for")+" '-console -script ["+QObject::tr("file")+"]'");

 optionHandler("--",handleStopOpt,QObject::tr("Stop processing options"));
 QStringList params=handleParams(app.argc(),app.argv());

 guiPrintDbg(debug::DBG_DBG,"Commandline parameters processed");
 guiPrintDbg(debug::DBG_DBG,"App path: " << Q_OUT(appPath));

 //load settings
 globalSettings=Settings::getInstance();
 globalSettings->setName("settings");
 atexit(saveSettings);

 guiPrintDbg(debug::DBG_DBG,"Settings loaded");

 ///Register annotation factories
 boost::shared_ptr<pdfobjects::utils::IAnnotInitializator> null;
 boost::shared_ptr<IAnnotInitializator> ia=CAnnotation::setAnnotInitializator(null);
 boost::shared_ptr<UniversalAnnotInitializer> ua=boost::dynamic_pointer_cast<UniversalAnnotInitializer>(ia);
 string text_t="Text";
 boost::shared_ptr<pdfobjects::utils::IAnnotInitializator> textA(new TextAnnotInitializer());
 boost::shared_ptr<pdfobjects::utils::IAnnotInitializator> linkA(new LinkAnnotInitializer());
 ua->registerInitializer("Text",textA);
 ua->registerInitializer("Link",linkA);

 //We need to set default charset on the beginning
 util::setDefaultCharset(globalSettings->read("editor/charset"));

 if (consoleMode) {
  //Running in console mode
  ConsoleWindow c(params);
  c.init();
  //Run scripts given on commandline
  for (unsigned int i=0;i<runScript.count();i++) {
   QString typ=runScript[i];
   guiPrintDbg(debug::DBG_DBG, "Script cmdline " << Q_OUT(typ) << " " << Q_OUT(runScriptParam[i]));
        if (typ=="run")		c.runFile(runScriptParam[i]);
   else if (typ=="script")	c.run(runScriptParam[i]);
   else if (typ=="eval")	c.eval(runScriptParam[i]);
   else {
    assert(0);
   }
  }
  //Call "default console start function"
  c.call("onConsoleStart");
  //Scripts complete ... exit
  exit(0);
 }

 if (!useGUI) {
  // Up until now DISPLAY was optional. For running GUI, it is mandatory
  printHeaderErr();
  util::fatalError(QObject::tr("Environment variable DISPLAY is not set - cannot run GUI of " APP_NAME));
 }

 //style
 QString style=globalSettings->read("gui/style","");
 if (style!="") {
  if (!app.setStyle(style)) globalSettings->write("gui/style",""); //No such style -> reset
 }

 guiPrintDbg(debug::DBG_DBG,"Style loaded");

 //font
 applyLookAndFeel(false);

 guiPrintDbg(debug::DBG_DBG,"Font and style applied");


 //open editor windows(s)
 int nFiles=params.size();
 if (nFiles) { //open files from cmdline
  guiPrintDbg(debug::DBG_DBG,"Opening files from commandline");
  for (QStringList::Iterator it=params.begin();it!=params.end();++it) {
   QString parameter=*it;
   guiPrintDbg(debug::DBG_INFO,"Opening parameter: " << Q_OUT(parameter));
   runCmdScripts(PdfEditWindow::create(parameter));
  }
 } else { //no parameters
  guiPrintDbg(debug::DBG_DBG,"Opening empty editor window");
  runCmdScripts(PdfEditWindow::create());
 }
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
 int ret = app.exec();
 GlobalParams::destroyGlobalParams();
 return ret;
}
