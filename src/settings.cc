#include "settings.h"
#include <iostream>

using namespace std;

const QString APP_KEY = "/PDFedit/";

void Settings::init() {
 action_index=1;
 set=new QSettings(QSettings::Ini);
// set->Scope=QSettings::User;
 set->insertSearchPath(QSettings::Unix,DATA_PATH);
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

int Settings::addAction(QString action) {
 if (action_map.contains(action)) return action_map[action];
 action_index++;
 action_map[action]=action_index;
 action_map_i[action_index]=action;
 return action_index;
}

QString Settings::getAction(int index) {
 return action_map_i[index];
}

/**
 load menu item - returns created popup menu if the item was nested menu, otherwise returns null (single item added to menu) 

 isRoot - TRUE if main menubar is being loaded
 parent - parent menu item (if isRoot == TRUE, this is the root menubar to add items to)

 */
 
  
void Settings::loadItem(QString name,QMenuData *parent,bool isRoot) {
 QPopupMenu *item=NULL;
 if (name=="-") { //separator
  parent->insertSeparator();
  return;
 }
 QString line=set->readEntry( APP_KEY+"gui/items/"+name);
 line=line.simplifyWhiteSpace();
 if (line.length()==0) fatalError("Missing menu item in config:\n"+name);
// cout << "CFG:" << line << endl;
 if (line.startsWith("list ")) { // List of values - a submenu, first is name of submenu, others are items in it
  if (!isRoot) {
   item=new QPopupMenu();
  }
  line=line.remove(0,5);
  QStringList qs=QStringList::split(',',line);  
  QStringList::Iterator it=qs.begin();
  if (it!=qs.end()) { //add itself as popup menu to parent with given name
   if (!isRoot) parent->insertItem(*it,item);
//   cout << "CFG:selfname:" << *it << endl;
   ++it;
  } else fatalError("Invalid menu item in config:\n"+line);
  for (;it!=qs.end();++it) { //load all subitems
//   cout << "CFG:loaditem:" << *it << endl;
   if (!isRoot) loadItem(*it,item); else loadItem(*it,parent);
  }
 } else if (line.startsWith("item ")) { // A single item
  line=line.remove(0,5);
  QStringList qs=QStringList::split(',',line,TRUE);  //Format: Caption, Action,[,accelerator, [,pixmap]]
  if (qs.count()<2) fatalError("Invalid menu item in config:\n"+line);
  QString label=qs[0];
  QString action=qs[1];
  action=action.stripWhiteSpace();
  int menu_id=addAction(action);
  parent->insertItem(label,menu_id);
  cout << "Accel?" << label <<endl;
  if (qs.count()>=3 && qs[2].length()>0) { //accelerator specified
   cout << label << " Setaccel:" << qs[2] <<endl;
   parent->setAccel(QKeySequence(qs[2]),menu_id);
  }
 } else { //something invalid
  fatalError("Invalid menu item in config:\n"+line);
 } 
}

QMenuBar *Settings::loadMenu(QWidget *parent) {
 QMenuBar *q=new QMenuBar(parent);
 loadItem(QString("MainMenu"),q,TRUE);//create root menu
 //TODO: cache Qmenubar one loaded?
 return q;
}
