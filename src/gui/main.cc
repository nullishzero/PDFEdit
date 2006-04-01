/** @file
 Main function
*/
#include "config.h"
#include "pdfeditwindow.h"
#include "settings.h"
#include <stdlib.h>
#include <qtranslator.h>
#include <qdir.h>

/** One object for application, holding all global settings.
 Should be thread-safe */
Settings *globalSettings;

/** delete settings object (and save settings)
 This function is called at application exit
 */
void saveSettings(void) {
 delete globalSettings;//this causes settings to be saved to disk
}

/** main - load settings and launches a main window */
int main(int argc, char *argv[]){
 QApplication app(argc, argv);

 //Translation support
 QTranslator translator;
 QString lang=QString("pdfedit_")+getenv("LANG");
 if (!translator.load(lang,QString(DATA_PATH)+"/lang")) { 
  if (!translator.load(lang,QDir::home().path()+"/"+CONFIG_DIR+"/lang")) { 
   #ifdef TESTING
   //look in current directory for testing version
   if (!translator.load(lang,"./lang")) { 
   #endif
    printDbg(debug::DBG_WARN,"Translation file " << lang << "not found");
   #ifdef TESTING
   }
   #endif
  }
 }
 app.installTranslator(&translator);

 globalSettings=new Settings();
 globalSettings->setName("settings");
 atexit(saveSettings);
 createNewEditorWindow();
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
 return app.exec();
}
