#include "pdfedit.h"
#include "settings.h"
#include <stdlib.h>

Settings *global;

/** creates and shows main window */
void newWindow() {
 pdfEditWidget *main=new pdfEditWidget();
 main->show();
}

/** delete settings object (and save settings) at exit */
void saveSettings(void) {
 delete global;//this causes settings to be saved to disk
}

/** main - load settings and launches a main window */
int main(int argc, char *argv[]){
 QApplication app(argc, argv);
 global=new Settings();
 atexit(saveSettings);
 newWindow();
 QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
 return app.exec();
}
