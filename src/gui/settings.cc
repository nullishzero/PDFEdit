/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
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

#include "settings.h"
#include "qtcompat.h"
#include "config.h"
#include "main.h"
#include "staticsettings.h"
#include "util.h"
#include <qapplication.h>
#include <qdir.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qsplitter.h>
#include <qstring.h>
#include <qstringlist.h>
#include <stdlib.h>
#include <utils/debug.h>

namespace gui {

using namespace std;
using namespace util;

/** "Root" item of all configuration settings, which are relative to this item */
const QString APP_KEY = "/PDFedit/";
/** Name of configuration file */
const QString CONFIG_FILE = "pdfeditrc";
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
 @return Value of given setting
*/
QString Settings::read(const QString &key,const QString &defValue/*=QString::null*/) {
 QString x=set->readEntry(APP_KEY+key);
 if (x.isNull()) x=staticSet->readEntry(key,defValue);
 return x;
}

/** Read settings with given key from configuration file and return as bool
 @param key Key to read from settings
 @param defValue default value to use if key not found in settings.
 @return Value of given setting (true or false)
*/
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
 @return Value of given setting
*/
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
 @return Value of given setting
 */
QString Settings::readExpand(const QString &key,const QString &defValue/*=QString::null*/) {
 QString x=read(key,defValue);
 x=expand(x);
 return x;
}

/**
 creates and inits new QSettings Object.
 Set paths to config files
*/
void Settings::initSettings() {
 QDir::home().mkdir(CONFIG_DIR);
 set=new QSettings(QSettings::Ini);
 staticSet=new StaticSettings();
 //Look in data directory
 bool haveSettings=staticSet->tryLoad(DATA_PATH,CONFIG_FILE);
 //Look in directory with the binary -> lowest priority
 if (!haveSettings) haveSettings=staticSet->tryLoad(appPath,CONFIG_FILE);
 if (!haveSettings) {
  //Main settings file not found
  guiPrintDbg(debug::DBG_ERR,"Main configuration file not found. PDFedit is unlikely to start");
  guiPrintDbg(debug::DBG_ERR,"Searched in: " << DATA_PATH << "/" << Q_OUT(CONFIG_FILE));
  guiPrintDbg(debug::DBG_ERR,"Searched in: " << Q_OUT(appPath) << "/" << Q_OUT(CONFIG_FILE));
 }
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
 @return QString with full path to the file, or NULL if no file found in path
*/
QString Settings::getFullPathName(const QString &nameOfPath,QString fileName/*=QString::null*/,const QString &prefix/*=QString::null*/) {
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
 guiPrintDbg(debug::DBG_WARN,"File not found (" << Q_OUT(nameOfPath) << "): " << Q_OUT(fileName));
 return QString::null;
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

/**
 Expand environment variables in given string (like $HOME, etc ..)
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
 Expands variables ($HOME, etc ...) and return as string list.
 Path elements are expected to be separated by semicolon.
 Trailing slashes are removed from path elements.
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

/**
 Read list of values from config file and return as string list.
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
void Settings::flush() {
 delete set;
 delete staticSet;
 initSettings();
}

/** Default destructor */
Settings::~Settings() {
 delete set;
 delete staticSet;
}

/**
 Save window/widget size and position to settings.
 @param win Widget that will have it's size and position stored
 @param name Name of key to be used in configuration
*/
void Settings::saveWindow(QWidget *win,const QString &name) {
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

/**
 Write settings with given key and value to configuration
 @param key Key to write to settings
 @param value Value to write to settings
*/
void Settings::write(const QString &key,const QString &value) {
 set->writeEntry(APP_KEY+key,value);
 emit settingChanged(key);
}

/**
 Write settings with given key and value to configuration
 @param key Key to write to settings
 @param value Value to write to settings
*/
void Settings::write(const QString &key, int value) {
 set->writeEntry(APP_KEY+key,value);
 emit settingChanged(key);
}

/**
 Restore window/widget size and position from setting.
 @param win Widget that will be resized and moved
 @param name Name of key to be used in configuration
*/
void Settings::restoreWindow(QWidget *win,const QString &name) {
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

/**
 Save splitter positions  to settings.
 @param spl Splitter to save positions
 @param name Name of key to be used in configuration
*/
void Settings::saveSplitter(QSplitter *spl,const QString &name) {
// guiPrintDbg(debug::DBG_DBG,"save splitter " << name);
 Q_List<int> siz=spl->sizes();
 QString line;
 int cnt=siz.size();
 for (int i=0;i<cnt;i++) {
  line+=QString::number(siz[i]);
  if (i<cnt-1) line+=",";
 }
 write("gui/windowstate/"+name,line);
}

/**
 Restore splitter positions from setting.
 @param spl Splitter to be resized
 @param name Name of key to be used in configuration
*/
void Settings::restoreSplitter(QSplitter *spl,const QString &name) {
// guiPrintDbg(debug::DBG_DBG,"restore splitter " << name);
 QString line=read("gui/windowstate/"+name);
 QStringList pos=explode(',',line);
 int cnt=pos.count();
 Q_List<int> splSize;
 if (cnt<2) return;//No previous window state information available, or it is invalid
 for(int i=0;i<cnt;i++) {
  int v=atoi(pos[i]);
  if (v<=0) return;//Negative/null size is invalid
  splSize.append(v);
 }
 spl->setSizes(splSize);
}

} // namespace gui
