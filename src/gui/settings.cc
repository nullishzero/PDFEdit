/** @file
 Settings - class handling application settings,
 like window positions, menu, toolbar and keyboard settings
 and other general settings
*/

#include <utils/debug.h>
#include <iostream>
#include <qfile.h>
#include <qvaluelist.h> 
#include <qdir.h>
#include <qregexp.h>
#include "settings.h"
#include "util.h"
#include "toolbutton.h"

using namespace std;

const QString APP_KEY = "/PDFedit/";

/** private initialization function */
void Settings::init() {
 action_index=1;
 initSettings();
 initPaths();
}

/** Given name of the icon, finds and returns full path to the icon,
     considering all relevant settings (icon path ..) */
QString Settings::getIconFile(const QString &name) {
 if (name.startsWith("/")) { //absolute path -> no change
  return name;
 }
 QString absName;
 for(QStringList::Iterator it=iconPath.begin();it!=iconPath.end();++it) {
  absName=*it+"/"+name;
  printDbg(debug::DBG_DBG,"Looking for " <<name << " in: " << *it << " as " << absName);
  if (QFile::exists(absName)) return absName;
 }
 printDbg(debug::DBG_WARN,"Icon file not found: " << name);
 return name;
}

/** Read settings with given key from configuration file and return as QString
 @param key Key to read from settings
 @param defValue default value to use if key not found in settings.
 @return Value of given setting */
QString Settings::read(const QString &key,const QString defValue/*=QString::null*/) {
 QString x=set->readEntry(APP_KEY+key);
 if (x.isNull()) x=staticSet->readEntry(APP_KEY+key,defValue);
 return x;
}

/** Read settings with given key from configuration file and return as bool
 @param key Key to read from settings
 @param defValue default value to use if key not found in settings.
 @return Value of given setting (true or false) */
bool Settings::readBool(const QString &key,bool defValue/*=false*/) {
 QString k=read(key);
 if (k.isNull()) return defValue;
 //Positive integer, "T" and "True" (regardless of case) are considered "true". Everything else "false"
 k=k.stripWhiteSpace().lower();
 int intVal=k.toInt();
 if (intVal>0) return true;
 else if (k=="t") return true;
 else if (k=="true") return true;
 return false;
}

/** Read settings with given key from configuration file and return as QString
 Any environment variable references (in form $VARIABLE) are expanded in the string
 @param key Key to read from settings
 @param defValue default value to use if key not found in settings.
 @return Value of given setting */
QString Settings::readExpand(const QString &key,const QString defValue/*=QString::null*/) {
 QString x=read(key,defValue);
 x=expand(x);
 return x;
}

/** creates and inits new QSettings Object.
    Set paths to config files */
void Settings::initSettings() {
 QDir::home().mkdir(CONFIG_DIR);
 set=new QSettings(QSettings::Ini);
 staticSet=new QSettings(QSettings::Ini);
 #ifdef TESTING
 //Look in current directory in testing versions -> lowest priority
 staticSet->insertSearchPath(QSettings::Unix,QDir::current().path());
 #endif
 staticSet->insertSearchPath(QSettings::Unix,DATA_PATH);
 set->insertSearchPath(QSettings::Unix,QDir::convertSeparators(QDir::home().path()+"/"+CONFIG_DIR));
}

/** load and expand various PATHs from config file.
  Must be done after config defaults are set */
void Settings::initPaths() {
 iconPath=readPath("icon");
}

/** Expand environment variables in given string (like $HOME, etc ..)
 @param s String to expand
 @return QString with variables expanded
*/
QString Settings::expand(QString s) {
 QRegExp r("\\$([a-zA-Z0-9]+)");//todo: not expand \$something, epxand ${something}
 QString var, envVar;
  int pos=0;
  while((pos=r.search(s,pos))!=-1) { //while found some variable
   var=r.cap(1);
   printDbg(debug::DBG_DBG,"Expand: " << var << " in " << s);
   /* home() is special - it is equal to regular $HOME on unixes, but might
    expand to "application data" directory on windows if HOME is not set */
   if (var=="HOME") envVar=QDir::home().path();
   else envVar=getenv(var);
   if (envVar==QString::null) { //variable not found in environment
    printDbg(debug::DBG_DBG,"Expand: " << var << " -> not found ");
    envVar="";
   }
   printDbg(debug::DBG_DBG,"Expand before: " << s);
   s=s.replace(pos,r.matchedLength(),envVar);
   printDbg(debug::DBG_DBG,"Expand after: " << s);
   pos++;
  }
 return s;
}

/** read path element from config file. Expands variables ($HOME, etc ...) and return as string list 
    Path elements are expected to be separated by semicolon
 @param name Identifier of path in config file
 @return QStringList containing expanded path directories
 */
QStringList Settings::readPath(const QString &name) {
 QString path=read(QString("path/")+name,".");
 QStringList s=QStringList::split(";",path);
 QRegExp stripTrail("(.)/+$");
 for(QStringList::Iterator it=s.begin();it!=s.end();++it) {
  //Trim starting and ending whitespace
  *it=(*it).stripWhiteSpace();
  //Expand environment variables in path component
  *it=expand(*it);
  //Trim trailing slashes
  *it=(*it).replace(stripTrail,"\\1");
  *it=QDir::convertSeparators(*it);
 }
 return s;
}

/** read list of values from config file and return as string list 
    List elements are expected to be separated by defined separator (default is comma)
 @param name Identifier of list in config file
 @param separator String separating items in the list
 @return QStringList containing items from list
 */
QStringList Settings::readList(const QString &name,const QString separator/*=","*/) {
 QString lst=read(name,"");
 QStringList s=QStringList::split(separator,lst);
 return s;
}

/** flushes settings, saving all changes to disk */
void Settings::flushSettings() {
 delete set;  
 delete staticSet;
 initSettings();
}

/** Default constructor */
Settings::Settings() {
 init();
}

/** Default destructor */
Settings::~Settings() {
 delete set;
}

/** Save window/widget size and position to settings.
 @param win Widget that will have it's size and position stored
 @param name Name of key to be used in configuration */  
void Settings::saveWindow(QWidget *win,const QString name) {
 printDbg(debug::DBG_DBG,"save window " << name);
 QString line;
 line+=QString::number(win->width());
 line+=",";
 line+=QString::number(win->height());
 line+=",";
 line+=QString::number(win->x());
 line+=",";
 line+=QString::number(win->y());
 write("gui/windowstate/"+name,line);
}

/** Write settings with given key and value to configuration
 @param key Key to write to settings
 @param value Value to write to settings
 */
void Settings::write(const QString &key,const QString &value) {
 set->writeEntry(APP_KEY+key,value);
 emit settingChanged(key);
}

/** Restore window/widget size and position from setting.
 @param win Widget that will be resized and moved
 @param name Name of key to be used in configuration */
void Settings::restoreWindow(QWidget *win,const QString name) {
 printDbg(debug::DBG_DBG,"restore window " << name);
 QWidget *desk = QApplication::desktop();
 QString line=read("gui/windowstate/"+name);
 QStringList pos=explode(',',line);
 if (pos.count()!=4) return;//No previous window state information available, or it is invalid
 int x,y,w,h;
 w=atoi(pos[0]);
 h=atoi(pos[1]);
 x=atoi(pos[2]);
 y=atoi(pos[3]);
 if (w<=0 || h<=0) return;//Negative/null size is invalid
 int dw=desk->width();
 int dh=desk->height(); 
 
 //if window is offscreen, move it to screen
 if (x<-w+1) x=-w+1; //Offscreen -> Onscreen
 if (y<-h+1) y=-h+1; //Offscreen -> Onscreen
 if (x>dw-1) x=dw-1; //Offscreen -> Onscreen
 if (y>dh-1) x=dh-1; //Offscreen -> Onscreen
 win->resize(w,h);
 win->move(x,y);
}

/** Save splitter positions  to settings.
 @param spl Splitter to save positions
 @param name Name of key to be used in configuration */  
void Settings::saveSplitter(QSplitter *spl,const QString name) {
 printDbg(debug::DBG_DBG,"save splitter " << name);
 QValueList<int> siz=spl->sizes();
 QString line;
 int cnt=siz.size();
 for (int i=0;i<cnt;i++) {
  line+=QString::number(siz[i]);
  if (i<cnt-1) line+=",";
 }
 write("gui/windowstate/"+name,line);  
}

/** Restore splitter positions from setting.
 @param spl Splitter to be resized
 @param name Name of key to be used in configuration */
void Settings::restoreSplitter(QSplitter *spl,const QString name) {
 printDbg(debug::DBG_DBG,"restore splitter " << name);
 QString line=read("gui/windowstate/"+name);
 QStringList pos=explode(',',line);
 int cnt=pos.count();
 QValueList<int> splSize;
 if (cnt<2) return;//No previous window state information available, or it is invalid
 for(int i=0;i<cnt;i++) {
  int v=atoi(pos[i]);
  if (v<=0) return;//Negative/null size is invalid
  splSize.append(v);
 }
 spl->setSizes(splSize);
}


/** Adds action to menu, returning newly allocated menu Id or existing menu id if action is already present
 
 @param action Name of action
 @return Menu ID of the specified action
 */
int Settings::addAction(const QString action) {
 if (action_map.contains(action)) return action_map[action];
 action_index++;
 action_map[action]=action_index;
 action_map_i[action_index]=action;
 return action_index;
}

/** return action string from given menu ID
 
 @param index Menu ID of action
 @return Name of the specified action
 */
QString Settings::getAction(int index) {
 return action_map_i[index];
}

/** returns icon with given name, loading if necessary and caching for later use
 @param name Name of icon to load
 @return Pixmap containing specified icon
 */
QPixmap *Settings::getIcon(const QString name) {
 printDbg(debug::DBG_INFO,"Loading icon:" << name);
 if (iconCache.contains(name)) return iconCache[name];
 QString absName=getIconFile(name);
 QFile f(absName);
 if (!f.open(IO_ReadOnly)) {
  printDbg(debug::DBG_WARN,"File not found:" << absName);
  return NULL;//file not found or unreadable or whatever ...
 }
 QByteArray qb=f.readAll();
 f.close();
 QPixmap *pix=new QPixmap();
 pix->loadFromData (qb,0,0);
 iconCache[name]=pix;
 return pix; 
}

/** load one GUI item from config file, exiting application with fatal error if item not found 
 
 @param name Name of the item to read
 @param root Root key to read from (default will be used if none specified)
 @return line from config file  
 */
QString Settings::readItem(const QString name,const QString root/*="gui/items/"*/) {
 QString line=read(root+name);
 line=line.simplifyWhiteSpace();
 if (line.length()==0) fatalError("Missing item in config:\n"+root+name);
 return line; 
}

/**
 load one menu item and insert it into parent menu. Recursively load subitems if item is a submenu.

 @param name name of item to be loaded from config file
 @param isRoot TRUE if main menubar is being loaded
 @param parent parent menu item (if isRoot == TRUE, this is the root menubar to add items to)
 */ 
void Settings::loadItem(const QString name,QMenuData *parent/*=NULL*/,bool isRoot/*=FALSE*/) {
 QPopupMenu *item=NULL;
 if (name=="-" || name=="") { //separator
  parent->insertSeparator();
  return;
 }
 QString line=readItem(name);
 if (line.startsWith("list ")) { // List of values - a submenu, first is name of submenu, others are items in it
  if (!isRoot) {
   item=new QPopupMenu();
  }
  line=line.remove(0,5);
  QStringList qs=explode(',',line);  
  QStringList::Iterator it=qs.begin();
  if (it!=qs.end()) { //add itself as popup menu to parent with given name
   if (!isRoot) parent->insertItem(tr(*it,name),item);
   ++it;
  } else fatalError("Invalid menu item in config:\n"+line);
  for (;it!=qs.end();++it) { //load all subitems
   if (!isRoot) loadItem(*it,item); else loadItem(*it,parent);
  }
 } else if (line.startsWith("item ")) { // A single item
  line=line.remove(0,5);
  //Format: Caption, Action,[,accelerator, [,menu icon]]
  QStringList qs=explode(',',line);
  if (qs.count()<2) fatalError("Invalid menu item in config:\n"+line);
  int menu_id=addAction(qs[1]);
  qs[0]=tr(qs[0],name);
  parent->insertItem(qs[0],menu_id);
  if (qs.count()>=3 && qs[2].length()>0) { //accelerator specified
   parent->setAccel(QKeySequence(qs[2]),menu_id);
  }
  if (qs.count()>=4 && qs[3].length()>0) { //menu icon specified
   QPixmap *pixmap=getIcon(qs[3]);
   if (pixmap) {
    parent->changeItem(menu_id,*pixmap,qs[0]);
   } else {
    printDbg(debug::DBG_WARN, "Pixmap missing: " << qs[3]);
   }
  }
 } else { //something invalid
  fatalError("Invalid menu item in config:\n"+line);
 } 
}

/** Loads menubar from configuration bar, and return it
 
 If menubar can't be loaded, the application is terminated
 Missing menu icons are allowed (if it can't be loaded, there will be no pixmap), missing items in configuration are not.

 @param parent QWidget that will contain the menubar
 @return loaded and initialized menubar
 */
QMenuBar *Settings::loadMenu(QWidget *parent) {
 //menubar can't be cached and must be separate for each window (otherwise weird things happen)
 QMenuBar *menubar=new QMenuBar(parent);//Make new menubar
 loadItem(QString("MainMenu"),menubar,TRUE);//create root menu
 return menubar;
}

/** Load single toolbar item and add it to toolbar
 @param tb Toolbar for addition of item
 @param item Item name in configuration file
 */
void Settings::loadToolBarItem(ToolBar *tb,QString item) {
 if (item=="-" || item=="") {
  tb->addSeparator();
  return;
 }
 QString line=readItem(item);
 if (line.startsWith("item ")) { //Format: Tooltip, Action,[,accelerator, [,icon]]
  line=line.remove(0,5);  
  QStringList qs=explode(',',line);
  if (qs.count()<4) fatalError("Invalid toolbar item in config:\n"+line);
  line=line.remove(0,5);
  QPixmap *pixmap=getIcon(qs[3]);
  int menu_id=addAction(qs[1]);
  if (!pixmap) {
   printDbg(debug::DBG_WARN, "Pixmap missing: " << qs[3]);
  }
  ToolButton *tbutton =new ToolButton(pixmap,qs[0],menu_id,tb);
  if (qs[2].length()>0) { //accelerator specified
   tbutton->setAccel(QKeySequence(qs[2]));
  }
  tb->addButton(tbutton);
  tbutton->show();
 } else {
  fatalError("Invalid toolbar item in config:\n"+line);  
 }
}

/**
 Load single toolbar from configuration file

 @param name Toolbar name in configuration file
 @param parent Parent window
 @return loaded toolbar
 */
ToolBar *Settings::loadToolbar(const QString name,QMainWindow *parent) {
 QString line=readItem(name);
 printDbg(debug::DBG_INFO,"Loading toolbar:" << name);
 if (line.startsWith("list ")) { // List of values - first is name, others are items in it
  line=line.remove(0,5);
  QStringList qs=explode(',',line);
  ToolBar *tb=NULL;
  QStringList::Iterator it=qs.begin();
  if (it!=qs.end()) {
   tb=new ToolBar(*it,parent);
   tb->setName(name);
   ++it;
  } else fatalError("Invalid toolbar item in config:\n"+line);
  for (;it!=qs.end();++it) { //load all subitems
   loadToolBarItem(tb,*it);
  }
  tb->show();
  return tb;
 } else {
  fatalError("Invalid toolbar item in config:\n"+line);
  return NULL;
 }
}

/** Load all toolbars from configuration files and add them to parent window
 
 @param parent parent window for toolbar
 */
ToolBarList Settings::loadToolBars(QMainWindow *parent) {
 ToolBarList list;
 QString line=read("gui/toolbars");
 QStringList tool=explode(',',line);
 for (unsigned int i=0;i<tool.count();i++) {
  list+=loadToolbar(tool[i],parent);
 }
 return list;
}
