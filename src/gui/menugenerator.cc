/** @file
 Generator of menu configuration
*/

#include <utils/debug.h>
#include <iostream>
#include <qfile.h>
#include <qvaluelist.h> 
#include <qdir.h>
#include "config.h"
#include "menugenerator.h"

using namespace std;

//todo: read from settings.cc/.h
const QString APP_KEY = "/PDFedit/";

/** Constructor */
MenuGenerator::MenuGenerator() {
 set=new QSettings(QSettings::Ini);
 //generate to current directory
 set->insertSearchPath(QSettings::Unix,QDir::current().path());
}

MenuGenerator::~MenuGenerator() {
 delete set;
}

void MenuGenerator::addToolbar(const QString &id,const QString &name,const QString &data) {
 addMenu(id,name,data);
 tbs+=id;
}

void MenuGenerator::addItem(const QString &id,const QString &name,const QString &data) {
 set->writeEntry("gui/items/"+id,QString("item ")+name+","+data);
}

void MenuGenerator::addMenu(const QString &id,const QString &name,const QString &data) {
 set->writeEntry("gui/items/"+id,QString("list ")+name+","+data);
}

void MenuGenerator::generate() {
 set->beginGroup(APP_KEY);
 set->writeEntry("path/icon", "$HOME/" CONFIG_DIR "/icon;" DATA_PATH "/icon"
#ifdef TESTING
 ";./icon"
#endif
);
 set->writeEntry("keyboard/CtrlA", "dosomethingeverywhere()");
 set->writeEntry("keyboard/context/CtrlA", "dosomethingincontext()");

 addMenu   ("MainMenu",	"Main menu",	"file,help");
 addMenu   ("file",	"File",		"load,save,neww,closew,quit,help");
 addMenu   ("help",	"Help",		"about,index");

 addItem   ("neww",	"&New Window",	"newwindow(),Ctrl+N");
 addItem   ("closew",	"&Close Window","closewindow");
 addItem   ("quit",	"&Quit",	"quit");
 addItem   ("load",	"&Load",	"loadFile(),, load.png");
 addItem   ("save",	"&Save",	"saveFile(),, save.png");
 addItem   ("about",	"&About",	"about(),, about.png");
 addItem   ("index",	"&Help",	"index, showhelp('index')");

 addToolbar("MainToolbar",  "Main Toolbar",	"load,save,about");
 addToolbar("OtherToolbar", "Other Toolbar",	"save,load");

 set->writeEntry("gui/toolbars",tbs.join(","));/**/
 set->endGroup();
}

int main(int argc, char *argv[]){
 MenuGenerator m;
 m.generate();
}
