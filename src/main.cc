#include "pdfedit.h"
#include "settings.h"
#include <stdlib.h>

Settings *global;

/** delete settings object (and save settings) at exit */
void saveSettings(void) {
 delete global;//this causes settings to be saved to disk
}

/** main - load settings and launches a main window */
int main(int argc, char *argv[]){
 QApplication app(argc, argv);
 global=new Settings();
 atexit(saveSettings);
 createNewEditorWindow();
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
 return app.exec();
}
