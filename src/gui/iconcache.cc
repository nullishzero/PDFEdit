/** @file
 IconCache - class responsible for loading and caching icons<br>
 @author Martin Petricek
*/

#include <qpixmap.h>
#include <qfile.h>
#include <qiconset.h> 
#include <qstring.h>
#include <qstringlist.h>
#include "iconcache.h"
#include "settings.h"
#include <utils/debug.h>
#include "util.h"

namespace gui {

using namespace std;

/**
 Default constructor of IconCache
*/
IconCache::IconCache() {
 iconPath=globalSettings->readPath("icon");
}

/**
 Given name of the icon, finds and returns full path to the icon,
 considering all relevant settings (icon path ..)
 @param name Name of icon
 @return Full filename of the icon
*/
QString IconCache::getIconFile(const QString &name) {
 if (name.startsWith("/")) { //absolute path -> no change
  return name;
 }
 QString absName;
 for(QStringList::Iterator it=iconPath.begin();it!=iconPath.end();++it) {
  absName=*it+"/"+name;
  if (QFile::exists(absName)) return absName;
 }
 guiPrintDbg(debug::DBG_WARN,"Icon file not found: " << name);
 return name;
}

/**
 Returns icon with given name, loading if necessary and caching for later use<br>
 Returns NULL if the icon cannot be loaded.
 @param name Name of icon to get
 @return Pixmap containing specified icon
*/
QPixmap* IconCache::getIcon(const QString &name) {
 // guiPrintDbg(debug::DBG_INFO,"Loading icon:" << name);
 //Look in cache first
 if (iconCache.contains(name)) return iconCache[name];
 //Not found in cache - try to load
 QString absName=getIconFile(name);
 QFile f(absName);
 if (!f.open(IO_ReadOnly)) {
  //file not found or unreadable or whatever ...
  guiPrintDbg(debug::DBG_WARN,"File not found:" << absName);
  return NULL;
 }
 QByteArray qb=f.readAll();
 f.close();
 //File is OK -> create pixmap
 QPixmap *pix=new QPixmap();
 pix->loadFromData (qb,0,0);
 iconCache[name]=pix;
 return pix; 
}

/**
 Returns icon set with given name, creating from corresponding icon if necessary and caching for later use<br>
 Returns NULL if the icon in the icon set cannot be loaded.
 @param name Name of icon set to get
 @return IconSet containing specified icon set
*/
QIconSet* IconCache::getIconSet(const QString &name) {
 //Look in cache first
 if (setCache.contains(name)) return setCache[name];
 //Not found in cache - try to create
 QPixmap* pix=getIcon(name);
 if (!pix) return NULL;//File not found ... 
 //Pixmap is OK
 QIconSet *ico=new QIconSet(*pix);
 setCache[name]=ico;
 return ico; 
}


/** default destructor */
IconCache::~IconCache() {
 QValueList<QString> pixmaps=iconCache.keys();
 //Delete all pixmaps from cache
 for (QValueList<QString>::Iterator it=pixmaps.begin();it!=pixmaps.end();++it) {
  QPixmap *rm=iconCache[*it];
  delete rm;
 }
 iconCache.clear();
 QValueList<QString> iconSets=setCache.keys();
 //Delete all icon sets from cache
 for (QValueList<QString>::Iterator it=iconSets.begin();it!=iconSets.end();++it) {
  QIconSet *rm=setCache[*it];
  delete rm;
 }
 setCache.clear();
}

} // namespace gui
