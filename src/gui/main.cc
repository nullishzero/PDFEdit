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

/** Path to directory in which the binary resides */
QString appPath;

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
 util::setDebugLevel(param);
}

/** QApplication */
QApplication *qApp;

/** main - load settings and launches a main window */
int main(int argc, char *argv[]){
 //We need this so we can use things like --version and --help even without X11 connection
#ifdef Q_WS_X11
 bool useGUI=getenv("DISPLAY")!=0;
#else
 bool useGUI=true;
#endif
 QApplication app(argc,argv,useGUI);
 qApp=&app;
 //Get application path
 appPath=app.applicationDirPath();
 appPath=QDir(appPath).absPath();
 //Translation support
 QTranslator translator;
 const char *env_lang=getenv("LANG");
 if (env_lang) {//LANG variable is present in environment -> attempt to load localization
  guiPrintDbg(debug::DBG_WARN,"LANG= " << env_lang);
  QString lang=QString("pdfedit_")+env_lang;
  //look for translation file in DATA_PATH
  if (!translator.load(lang,QString(DATA_PATH)+"/lang")) {
   //Look for translation file in config directory in $HOME
   if (!translator.load(lang,QDir::home().path()+"/"+CONFIG_DIR+"/lang")) {
    //Look in binary path - testing compilations and (possibly) windows builds
    if (!translator.load(lang,appPath+"/lang")) { 
     guiPrintDbg(debug::DBG_WARN,"Translation file " << lang << " not found");
    }
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
 optionHandlerParam("-d","n",handleDebug,QObject::tr("Set debug messages verbosity")+" "+QObject::tr("(n = -1 .. 5)"));
 optionHandler("--",handleStopOpt,QObject::tr("Stop processing options"));
 QStringList params=handleParams(app.argc(),app.argv());

 guiPrintDbg(debug::DBG_DBG,"Commandline parameters processed");
 guiPrintDbg(debug::DBG_DBG,"App path: " << appPath);

 //load settings
 globalSettings=Settings::getInstance();
 globalSettings->setName("settings");
 atexit(saveSettings);

 guiPrintDbg(debug::DBG_DBG,"Settings loaded");

 if (!useGUI) {
  // Up until now DISPLAY was optional. For running GUI, it is mandatory
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
   guiPrintDbg(debug::DBG_INFO,"Opening parameter: " << *it)
   PdfEditWindow::create(*it);
  }
 } else { //no parameters
  guiPrintDbg(debug::DBG_DBG,"Opening empty editor window");
  PdfEditWindow::create();
 }
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
 return app.exec();
}
