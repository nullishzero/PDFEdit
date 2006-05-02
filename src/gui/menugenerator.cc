/** @file
 Generator, checker and translation generator of menu configuration<br>
 This is helper utility used to:<br>
  - check menus for translatable strings and write them to .menu-trans.h,
    so they will be found by lupdate utility<br>
  - check menus for unreferenced items<br>
  - generate initial menu (now obsolete)<br>
 @author Martin Petricek
*/

#include <iostream>
#include <string.h>
#include <qfile.h>
#include <qvaluelist.h> 
#include <qstringlist.h> 
#include <qdir.h>
#include "config.h"
#include "util.h"
#include "menugenerator.h"

using namespace std;

//todo: read from settings.cc/.h
const QString APP_KEY = "/PDFedit/";

/** Constructor */
MenuGenerator::MenuGenerator() {
 set=new QSettings(QSettings::Ini);
 //generate to/from current directory
 set->insertSearchPath(QSettings::Unix,QDir::current().path());
}

/** Destructor */
MenuGenerator::~MenuGenerator() {
 delete set;
}

/**
 Add Toolbar 
 @param id ID of toolbar
 @param name Caption of toolbar
 @param data Comma separated list of subitems
*/
void MenuGenerator::addToolbar(const QString &id,const QString &name,const QString &data) {
 addMenu(id,name,data);
 tbs+=id;
}

/**
 Add Menu item 
 @param id ID of menu item
 @param name Caption of menu item
 @param data Comma separated list: action, shortcut, icon. Shortcut / icon may be omitted or empty
*/
void MenuGenerator::addItem(const QString &id,const QString &name,const QString &data) {
 set->writeEntry("gui/items/"+id,QString("item ")+name+","+data);
}

/**
 Add Menu or submenu
 @param id ID of menu
 @param name Caption of menu
 @param data Comma separated list of subitems
*/
void MenuGenerator::addMenu(const QString &id,const QString &name,const QString &data) {
 set->writeEntry("gui/items/"+id,QString("list ")+name+","+data);
}

/** generate some initial/testing menu */
void MenuGenerator::generate() {
 set->beginGroup(APP_KEY);
 set->writeEntry("path/icon", "$HOME/" CONFIG_DIR "/icon;" DATA_PATH "/icon"
#ifdef TESTING
 ";./icon"
#endif
);
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

/**
 Set menu item as "reachable" (increases its reference count) and add translation on it.
 Recursively run on subitems if item is list
 @param name Id of menu item
*/
void MenuGenerator::setAvail(const QString &name) {
 if (name=="-") return; //return if separator
 avail[name]+=1;
 if (avail[name]>=2) return; //already seen this one
 QString line=set->readEntry("gui/items/"+name);
 if (line.startsWith("list ")) { // List of values - a submenu, first is name of submenu, others are items in it
  line=line.remove(0,5);
  QStringList qs=explode(',',line);  
  QStringList::Iterator it=qs.begin();
  if (it!=qs.end()) {
   if (*it!="-") { //not a separator 
    addLocString(name,*it);
   }
   ++it;
  } else fatalError("Invalid menu item in config:\n"+line);
  for (;it!=qs.end();++it) { //load all subitems
   setAvail(*it);
  }
 } else if (line.startsWith("item ")) { // A single item
  line=line.remove(0,5);
  QStringList qs=explode(',',line);
  addLocString(name,qs[0]);
  if (qs.count()<2) fatalError("Invalid menu item in config:\n"+line);
 } else { //something invalid
  fatalError("Invalid menu item in config:\n"+line);
 } 

}

/** 
 Check menu structure, print items and their reference counts
 Warn about unreferenced items 
*/
void MenuGenerator::check() {
 set->beginGroup(APP_KEY);
 QStringList items=set->entryList("gui/items");
 QStringList toolb=QStringList::split(",",set->readEntry("gui/toolbars"));

 //Toolbars are root items
 for (QStringList::Iterator it=toolb.begin();it!=toolb.end();++it) {
  setAvail(*it);
 }

 //Main menu is root item
 setAvail("MainMenu");

 int ava;
 for (QStringList::Iterator it=items.begin();it!=items.end();++it) {
  cout << "Item : ";
  cout.width(20);
  cout.flags(ios::left);
  cout << *it;
  ava=avail[*it];
  if (!ava) cout << " (unreachable!)";
  else  cout << " (" << ava << " refs)";
  cout << endl;
 }
 set->endGroup();
}

/** Add menu to localization list
 @param id Id of menu item
 @param name Caption of menu item
*/
void MenuGenerator::addLocString(const QString &id,const QString &name) {
 trans+=QString("QT_TRANSLATE_NOOP( \"gui::Settings\",\"")+name+"\",\""+id+"\")";
 cout << id << " = " << name << endl;
}

/** Produce dummy header used for menu items localization
*/
void MenuGenerator::translate() {
 check();
 QString trx=trans.join("\n");
 QFile file(".menu-trans.h");
 if ( file.open( IO_WriteOnly ) ) {
  QTextStream stream( &file );
  stream << "//File automatically generated by menugenerator from pdfeditrc" << endl;
  stream << "//Do not edit, any changes will be overwritten" << endl;
  stream << trx;
  file.close();
 } else {
  fatalError("Cannot open file on write!");
 }
}

/** Main of menugenerator */
int main(int argc, char *argv[]){
 MenuGenerator m;
 cout << "Usage: \"menugenerator -generate\" to generate default menus" << endl
      << "       \"menugenerator\" to check menus" <<endl
      << "       \"menugenerator -trans\" to generate translation" <<endl;
 if (argc>1) {
  //Generate something .... (obsolete)
  if (strcmp(argv[1],"-generate")==0) {
   cout << "Generating menu" << endl;
   m.generate();
   cout << "Done generating menu" << endl;
  }
  if (strcmp(argv[1],"-trans")==0) {
   cout << "Checking menu" << endl;
   m.translate();
   cout << "Done checking menu" << endl;
  }
 } else { //check menus, do localization
   cout << "Checking menu" << endl;
   //TODO: check accelerator conflicts in single menu
   m.check();
   cout << "Done checking menu" << endl;
 }
}
