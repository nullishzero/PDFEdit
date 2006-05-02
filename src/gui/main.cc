/** @file
 Main function<br>
 Handle commandline options and then open editor window(s)
 - unless commandline parameters specify something different
 @author Martin Petricek
*/

#include "args.h"
#include <stdlib.h>
#include <qtranslator.h>
#include <qapplication.h>
#include <qdir.h>
#include <iostream>
#include "pdfeditwindow.h"
#include "config.h"
#include "util.h"
#include <utils/debug.h>
#include "version.h"
#include "settings.h"
#include "optionwindow.h"

using namespace std;
using namespace gui;

/** delete settings object (and save settings)
 This function is called at application exit
 */
void saveSettings(void) {
 delete globalSettings;//this causes settings to be saved to disk
}

/**
 handle --help parameter<br>
 Prtin help to STDOUT and exit
*/
void handleHelp(){
 cout << APP_NAME << " " << VERSION << endl;
 cout << QObject::tr("Usage: ") << binName << QObject::tr(" [option(s)] [files(s)]") << endl;
 handleHelpOptions();
}

/**
 handle --version parameter<br>
 Print version to STDOUT and exit
 */
void handleVersion(){
 cout << VERSION << endl;
 exit(0);
}

/**
 handle -d [n] parameter<br>
 Change debugging level
 @param param Parameter passed
*/
void handleDebug(const QString &param){
 QString cns=param.upper();
 if (cns.length()) { //Check for symbolic constants
  if (QString("PANIC").startsWith(cns))		{ debug::debugLevel=debug::DBG_PANIC;	return; }
  if (QString("CRITICAL").startsWith(cns))	{ debug::debugLevel=debug::DBG_CRIT;	return; }
  if (QString("ERROR").startsWith(cns))		{ debug::debugLevel=debug::DBG_ERR;	return; }
  if (QString("WARNING").startsWith(cns))	{ debug::debugLevel=debug::DBG_WARN;	return; }
  if (QString("INFO").startsWith(cns))		{ debug::debugLevel=debug::DBG_INFO;	return; }
  if (QString("DEBUG").startsWith(cns))		{ debug::debugLevel=debug::DBG_DBG;	return; }
 }
 //If debuglevel is set outside of limits - no problem, nearest "in ilmits" value is defacto used
 debug::debugLevel=atoi(param);
 //If non-number is given, default 0 is silently used ... :)
}

/** main - load settings and launches a main window */
int main(int argc, char *argv[]){
 QApplication app(argc, argv);

 //Translation support
 QTranslator translator;
 char *lang=getenv("LANG");
 if (lang) {//LANG variable is present in environment -> attempt to load localization
  QString lang=QString("pdfedit_")+lang;
  //look for translation file in DATA_PATH
  if (!translator.load(lang,QString(DATA_PATH)+"/lang")) {
   //Look for translation file in config directory in $HOME
   if (!translator.load(lang,QDir::home().path()+"/"+CONFIG_DIR+"/lang")) {
    #ifdef TESTING
    //look in current directory for testing version
    if (!translator.load(lang,"./lang")) { 
    #endif
     guiPrintDbg(debug::DBG_WARN,"Translation file " << lang << "not found");
    #ifdef TESTING
    }
    #endif
   }
  }
 }
 app.installTranslator(&translator);

 //parse commandline parameters
 /*
  Whole name of one parameter should not be prefix of another parameter, as unpredictable behaviour can occur,
  for example -d and -def, in this case there is no way to recognize between -def option and -d option
  with 'ef' as parameter and it is undefined what case of these two will be recognized
 */
 optionHandler("--help",handleHelp,QObject::tr("Print help and exit"));
 optionHandler("--version",handleVersion,QObject::tr("Print version and exit"));
 optionHandlerParam("-d","n",handleDebug,QObject::tr("Set debug messages verbosity")+" "+QObject::tr("(n = -1 .. 5, default 0)"));
 optionHandler("--",handleStopOpt,QObject::tr("Stop processing options"));
 QStringList params=handleParams(app.argc(),app.argv());

 guiPrintDbg(debug::DBG_DBG,"Commandline parameters processed");

 //load settings
 globalSettings=Settings::getInstance();
 globalSettings->setName("settings");
 atexit(saveSettings);

 guiPrintDbg(debug::DBG_DBG,"Settings loaded");

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
   guiPrintDbg(debug::DBG_INFO,"Opening parameter: " << *it)
   createNewEditorWindow(*it);
  }
 } else { //no parameters
  guiPrintDbg(debug::DBG_DBG,"Opening empty editor window");
  createNewEditorWindow();
 }
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
 return app.exec();
}
