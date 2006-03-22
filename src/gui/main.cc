/** @file
 Main function
*/
#include "pdfeditwindow.h"
#include "settings.h"
#include <stdlib.h>

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
 globalSettings=new Settings();
 globalSettings->setName("settings");
 atexit(saveSettings);
 createNewEditorWindow();
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
 return app.exec();
}
