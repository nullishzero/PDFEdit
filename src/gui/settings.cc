/** @file
 Settings - class handling basic application settings,
 reading and writing values with keys and also storing and 
 restoring more complex settings (window and toolbar positions),
 reading Paths and expanding environment variables in settings.
 QSettings with two files (user file in $HOME (readwrite)
 and system file with default settings (readonly)) is used as
 backend for storing the data
 @author Martin Petricek
*/

#include <utils/debug.h>
#include <qdir.h>
#include <qapplication.h>
#include <qregexp.h>
#include "settings.h"
#include "util.h"
#include <qsettings.h>
#include <qsplitter.h>
#include <qstringlist.h> 
#include <qstring.h>
#include "main.h"
#include "config.h"

namespace gui {

using namespace std;
using namespace util;

const QString APP_KEY = "/PDFedit/";
/** Name of variable which will be expanded to full path to the executable */
const QString APP_PATH_VAR = "PDFEDIT_BIN";
/** Name of variable which will be expanded to data directory */
const QString DATA_PATH_VAR = "PDFEDIT_DATA";

/** One object for application, holding all global settings.
 Should be thread-safe. This instance is used from other files */
Settings *globalSettings=NULL;

/**
 Return Instance of Settings.
 Ensures only one instance of Settings exists at any time (singleton)
 If no instance exists, it is created.
 @return existing Settings object
*/
Settings* Settings::getInstance() {
//  static Settings* globalSettings=NULL;
 if (!globalSettings) globalSettings=new Settings();
 return globalSettings;
}

/** private initialization function */
void Settings::init() {
 initSettings();
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

/** Read settings with given key from configuration file and return as integer
 @param key Key to read from settings
 @param defValue default value to use if key not found in settings.
 @return Value of given setting */
int Settings::readNum(const QString &key,int defValue/*=0*/) {
 QString k=read(key);
 bool ok;
 int x=k.toInt(&ok);
 if (!ok) return defValue;
 return x;
}

/** Read settings with given key from configuration file and return as QString
 Any environment variable references (in form $VARIABLE) are expanded in the string.
 \note Some internal variables (beginning with $PDFEDIT_) can override corresponding environment variables. See documentation for details on them
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
 //Look in directory with the binary -> lowest priority
 staticSet->insertSearchPath(QSettings::Unix,appPath);
 staticSet->insertSearchPath(QSettings::Windows,appPath);
 //Look in data directory
 staticSet->insertSearchPath(QSettings::Unix,DATA_PATH);
 //Look in $HOME (or something like that, for example QT maps this to "Application Data" in windows)
 QString homeDir=QDir::convertSeparators(QDir::home().path()+"/"+CONFIG_DIR);
 set->insertSearchPath(QSettings::Unix,homeDir);
 set->insertSearchPath(QSettings::Windows,homeDir);
}

/**
 Given name of the file, finds and returns full path to the file,
 considering all relevant settings
 \see readPath
 @param nameOfPath Name of paths in settings for this file (e.g. "icon")
 @param fileName Name of file. If not specified, the path itself is the filename we want to find
 @param prefix Path prefix (Can be specified if desired to read from different configuration key than default)
 @return QString with full path to the file
*/
QString Settings::getFullPathName(QString nameOfPath,QString fileName/*=QString::null*/,const QString &prefix/*=QString::null*/) {
 if (fileName.isNull()) {
  fileName="";
 }
 if (fileName.startsWith("/")) { //absolute path -> no change
  return fileName;
 }
 QStringList path;
 if (prefix.isNull()) {
  path=readPath(nameOfPath);
 } else {
  path=readPath(nameOfPath,prefix);
 }
 QString absName;
 for(QStringList::Iterator it=path.begin();it!=path.end();++it) {
  if (fileName=="") {
   absName=*it;
  } else {
   absName=*it+"/"+fileName;
  }
  if (QFile::exists(absName)) return absName;
 }
 guiPrintDbg(debug::DBG_WARN,"File not found: " << fileName);
 return fileName;
}

/** 
 Remove key from user settings, effectively restoring the setting to its default value
 @param key Key to remove
*/
void Settings::remove(const QString &key) {
 set->removeEntry(key);
}

/** 
 Remove all keys directly under specified key according to hiararchic structure
 @param key Key to remove
*/
void Settings::removeAll(const QString &key) {
 QStringList all=set->entryList(key);
 for(size_t i=0;i<all.count();i++) {
  set->removeEntry(key+"/"+all[i]); 
 }
 //At last, try to remove the key itself
 set->removeEntry(key);
}

/** Expand environment variables in given string (like $HOME, etc ..)
 @param s String to expand
 @return QString with variables expanded
*/
QString Settings::expand(QString s) {
 QRegExp r("\\$([a-zA-Z0-9_]+|\\{[_a-zA-Z0-9]+\\})|(\\\\.)");
 QString var, envVar;
  int pos=0;
  while((pos=r.search(s,pos))!=-1) { //while found some variable
   if (r.cap(2).length()>0) {// Found \something -> skip
    envVar=r.cap(2).mid(1);
   } else { //Found $VARIABLE
    var=r.cap(1);
    if (var[0]=='{') var=var.mid(1,var.length()-2);
    /* home() is special - it is equal to regular $HOME on unixes, but might
     expand to "application data" directory on windows if HOME is not set */
    if (var=="HOME") envVar=QDir::home().path();
    else if (var==APP_PATH_VAR) envVar=appPath;
    else if (var==DATA_PATH_VAR) envVar=DATA_PATH;
    else envVar=getenv(var);
    if (envVar==QString::null) { //variable not found in environment
     envVar="";
    }
   }
   s=s.replace(pos,r.matchedLength(),envVar);
   pos+=envVar.length();//Move to end of expanded string
  }
 return s;
}

/**
 Read path list element from config file.
 Expands variables ($HOME, etc ...) and return as string list 
 Path elements are expected to be separated by semicolon
 Trailing slashes are removed from path elements
 @param name Identifier of path in config file
 @param prefix Path prefix (Can be specified if desired to read from different configuration key than default "path"/)
 @return QStringList containing expanded path directories
 */
QStringList Settings::readPath(const QString &name,const QString &prefix/*="path/"*/) {
 QString path=read(prefix+name,".");
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
//  guiPrintDbg(debug::DBG_DBG,"Path " << (prefix+name) << " -> " << *it);
 }
 return s;
}

/** read list of values from config file and return as string list 
    List elements are expected to be separated by defined separator (default is comma)
 @param name Identifier of list in config file
 @param separator String separating items in the list
 @return QStringList containing items from list
 */
QStringList Settings::readList(const QString &name,const QString &separator/*=","*/) {
 QString lst=read(name,"");
 QStringList s=QStringList::split(separator,lst);
 return s;
}

/** Default constructor */
Settings::Settings() {
 init();
}

/** flushes settings, saving all changes to disk */
void Settings::flushSettings() {
 delete set;  
 delete staticSet;
 initSettings();
}

/** Default destructor */
Settings::~Settings() {
 delete set;
 delete staticSet;
}

/** Save window/widget size and position to settings.
 @param win Widget that will have it's size and position stored
 @param name Name of key to be used in configuration */  
void Settings::saveWindow(QWidget *win,const QString name) {
// guiPrintDbg(debug::DBG_DBG,"save window " << name);
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

/** Write settings with given key and value to configuration
 @param key Key to write to settings
 @param value Value to write to settings
 */
void Settings::write(const QString &key, int value) {
 set->writeEntry(APP_KEY+key,value);
 emit settingChanged(key);
}

/** Restore window/widget size and position from setting.
 @param win Widget that will be resized and moved
 @param name Name of key to be used in configuration */
void Settings::restoreWindow(QWidget *win,const QString name) {
// guiPrintDbg(debug::DBG_DBG,"restore window " << name);
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
// guiPrintDbg(debug::DBG_DBG,"save splitter " << name);
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
// guiPrintDbg(debug::DBG_DBG,"restore splitter " << name);
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

} // namespace gui
