#include "settings.h"
#include <iostream>
#include <qfile.h>
#include <qdir.h>

using namespace std;

const QString APP_KEY = "/PDFedit/";

/** private initialization function */
void Settings::init() {
 action_index=1;
 initSettings();
 //DEBUG: aktualni adresar, ve finalni verzi nebude
 // settings.insertSearchPath( QSettings::Unix,".");

 /* DEBUG : will get out very soon*/
 set->beginGroup(APP_KEY);
 if (set->readEntry("debugTrashSaved")!="3") {
  set->writeEntry("debugTrashSaved", "3");
  set->writeEntry("keyboard/CtrlA", "dosomethingeverywhere()");
  set->writeEntry("keyboard/context/CtrlA", "dosomethingincontext()");
  set->writeEntry("gui/items/MainMenu",  "list Main menu,file,help");
  set->writeEntry("gui/items/file", "list File,load,save,neww,closew,quit");
  set->writeEntry("gui/items/help", "list Help,about,index");
  set->writeEntry("gui/items/neww",  "item &New Window, newwindow,Ctrl+N");
  set->writeEntry("gui/items/closew","item &Close Window, closewindow");
  set->writeEntry("gui/items/quit",  "item &Quit, quit");
  set->writeEntry("gui/items/load",  "item &Load, loadFile(),, iconload.png");
  set->writeEntry("gui/items/save",  "item &Save, saveFile(),, iconsave.png");
  set->writeEntry("gui/items/about",  "item &About, about()");
  set->writeEntry("gui/items/index",  "item &Help index, showhelp(index)");
  set->writeEntry("gui/items/MainToolbar",  "list popup,load,save");
  QStringList vi;
  vi+="MainMenu";
  vi+="MainToolbar";
  set->writeEntry("gui/visibleitems",vi.join(","));/**/
 }
 set->endGroup();
}

/** creates and inits new QSettings Object */
void Settings::initSettings() {
 set=new QSettings(QSettings::Ini);
 set->insertSearchPath(QSettings::Unix,DATA_PATH);
}

/** flushes settings, saving all changes to disk */
void Settings::flushSettings() {
 delete set;  
 initSettings();
}


Settings::Settings() {
 init();
}

Settings::~Settings() {
 delete set;
}

void Settings::fatalError(QString q){
 cout << "Fatal Error: " << q << endl;
 exit(-1);
}

/** Adds action to menu, returning newly allocated menu Id or existing menu id if action is already present */
int Settings::addAction(QString action) {
 if (action_map.contains(action)) return action_map[action];
 action_index++;
 action_map[action]=action_index;
 action_map_i[action_index]=action;
 return action_index;
}

/** return action string from given menu ID */
QString Settings::getAction(int index) {
 return action_map_i[index];
}

/** returns icon with given name, loading if necessary and caching for later use */
QPixmap *Settings::getIcon(QString name) {
 cout << "Loading:" << name << endl; 
 if (iconCache.contains(name)) return iconCache[name];
 QFile f(name);
 if (!f.open(IO_ReadOnly)) {
  cout << "File not found:" << name << endl;
  return NULL;//file not found or unreadable or whatever ...
 }
 QByteArray qb=f.readAll();
 f.close();
 QPixmap *pix=new QPixmap();
 pix->loadFromData (qb,0,0);
 iconCache[name]=pix;
 return pix; 
}

/**
 load one menu item and insert it into parent menu. Recursively load subitems if item is a submenu.

 name - name of item to be loaded from config file
 isRoot - TRUE if main menubar is being loaded
 parent - parent menu item (if isRoot == TRUE, this is the root menubar to add items to)
 */ 
void Settings::loadItem(QString name,QMenuData *parent,bool isRoot) {
 QPopupMenu *item=NULL;
 if (name=="-" || name=="") { //separator
  parent->insertSeparator();
  return;
 }
 QString line=set->readEntry( APP_KEY+"gui/items/"+name);
 line=line.simplifyWhiteSpace();
 if (line.length()==0) fatalError("Missing menu item in config:\n"+name);
 if (line.startsWith("list ")) { // List of values - a submenu, first is name of submenu, others are items in it
  if (!isRoot) {
   item=new QPopupMenu();
  }
  line=line.remove(0,5);
  QStringList qs=QStringList::split(',',line);  
  QStringList::Iterator it=qs.begin();
  if (it!=qs.end()) { //add itself as popup menu to parent with given name
   if (!isRoot) parent->insertItem(*it,item);
   ++it;
  } else fatalError("Invalid menu item in config:\n"+line);
  for (;it!=qs.end();++it) { //load all subitems
   if (!isRoot) loadItem(*it,item); else loadItem(*it,parent);
  }
 } else if (line.startsWith("item ")) { // A single item
  line=line.remove(0,5);
  //Format: Caption, Action,[,accelerator, [,menu icon]]
  QStringList qs=QStringList::split(',',line,TRUE);
  if (qs.count()<2) fatalError("Invalid menu item in config:\n"+line);
  QString label=qs[0];
  QString action=qs[1];
  action=action.stripWhiteSpace();
  int menu_id=addAction(action);
  parent->insertItem(label,menu_id);
  if (qs.count()>=3 && qs[2].length()>0) { //accelerator specified
   cout << label << " Setaccel:" << qs[2] <<endl;
   parent->setAccel(QKeySequence(qs[2].stripWhiteSpace()),menu_id);
  }
  if (qs.count()>=4 && qs[3].length()>0) { //menu icon specified
   cout << label << " SetPixmap:" << qs[3] <<endl;
   QPixmap *pixmap=getIcon(qs[3].stripWhiteSpace());
   if (pixmap) parent->changeItem(menu_id,*pixmap,label); else cout << "Pixmap missing: " << qs[3] << endl;
  }
 } else { //something invalid
  fatalError("Invalid menu item in config:\n"+line);
 } 
}

QMenuBar *Settings::loadMenu(QWidget *parent) {
 //menubar can't be cached and must be separate for each window (otherwise weird things happen)
 QMenuBar *menubar=new QMenuBar(parent);//Make new menubar
 loadItem(QString("MainMenu"),menubar,TRUE);//create root menu
 return menubar;
}
