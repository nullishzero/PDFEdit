/** @file
 Settings - class handling application settings,
 like window positions, menu, toolbar and keyboard settings
 and other general settings
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

namespace gui {

using namespace std;

const QString APP_KEY = "/PDFedit/";

/** One object for application, holding all global settings.
 Should be thread-safe. This instance is used from other files */
Settings *globalSettings;

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

/** Given name of the file, finds and returns full path to the file,
     considering all relevant settings
 @param nameOfPath Name of paths in settings for this file (e.g. "icon")
 @param fileName Name of file
 @return QString with full path to the file
*/
QString Settings::getFullPathName( QString nameOfPath , QString fileName ) {
 QStringList path = readPath( nameOfPath );
 if (fileName.startsWith("/")) { //absolute path -> no change
  return fileName;
 }
 QString absName;
 for(QStringList::Iterator it=path.begin();it!=path.end();++it) {
  absName=*it+"/"+fileName;
//  printDbg(debug::DBG_DBG,"Looking for " <<fileName << " in: " << *it << " as " << absName);
  if (QFile::exists(absName)) return absName;
 }
 printDbg(debug::DBG_WARN,"File not found: " << fileName);
 return fileName;
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
//    printDbg(debug::DBG_DBG,"Expand: " << var << " in " << s);
    /* home() is special - it is equal to regular $HOME on unixes, but might
     expand to "application data" directory on windows if HOME is not set */
    if (var=="HOME") envVar=QDir::home().path();
    else envVar=getenv(var);
    if (envVar==QString::null) { //variable not found in environment
//     printDbg(debug::DBG_DBG,"Expand: " << var << " -> not found ");
     envVar="";
    }
   }
//   printDbg(debug::DBG_DBG,"Expand before: " << s);
   s=s.replace(pos,r.matchedLength(),envVar);
//   printDbg(debug::DBG_DBG,"Expand after: " << s);
   pos+=envVar.length();//Move to end of expanded string
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
// printDbg(debug::DBG_DBG,"save window " << name);
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
// printDbg(debug::DBG_DBG,"restore window " << name);
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
// printDbg(debug::DBG_DBG,"save splitter " << name);
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
// printDbg(debug::DBG_DBG,"restore splitter " << name);
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
