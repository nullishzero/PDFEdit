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
 IconCache - class responsible for loading and caching icons<br>
 @author Martin Petricek
*/

#include "iconcache.h"
#include "settings.h"
#include "util.h"
#include <assert.h>
#include <qfile.h>
#include <qmap.h>
#include <qpixmap.h>
#include QICON
#include <qstring.h>
#include <qstringlist.h>
#include <utils/debug.h>

namespace gui {

using namespace std;

/**
 Number of concurrently existing icon cache instances.
 When last instance is deleted, cached data is deleted too
*/
int iconCacheInstances=0;

//Internal types
 /** Icon Cache type: mapping from icon name to pixmap */
 typedef QMap<QString, QPixmap*> IconCacheData;
 /** Icon Sets Cache type: mapping from icon name to icon set */
 typedef QMap<QString, QIcon*> IconSetsData;

//Internal icon cache data shared by all icon cache instance
 /** List with paths to application icons */
 QStringList iconPath;
 /** Name of current icon style */
 QString iconStyleName;
 /** Cache storing loaded icons */
 IconCacheData iconCache;
 /** Cache storing created icons sets*/
 IconSetsData setCache;

/**
 Default constructor of IconCache
*/
IconCache::IconCache() {
 if (iconCacheInstances>0) {
  iconCacheInstances++;
  return;//Initialization already done
 }
 iconCacheInstances++;
 iconPath=globalSettings->readPath("icon");
 //Read icon style
 iconStyleName=globalSettings->read("icon/theme/current");
 if (iconStyleName=="default") iconStyleName=QString::null;
 if (iconStyleName=="") iconStyleName=QString::null;
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
 if (!iconStyleName.isNull()) {
  //Check icons from style first
  for(QStringList::Iterator it=iconPath.begin();it!=iconPath.end();++it) {
   //Check each path in icon path
   absName=*it+"/"+iconStyleName+"/"+name;
   if (QFile::exists(absName)) return absName;
  }
 }
 //Icon from style not found (or no icon style used) - check default icons
 for(QStringList::Iterator it=iconPath.begin();it!=iconPath.end();++it) {
  //Check each path in icon path
  absName=*it+"/"+name;
  if (QFile::exists(absName)) return absName;
 }
 guiPrintDbg(debug::DBG_WARN,"Icon file not found: " << Q_OUT(name));
 return name;
}

/**
 Returns icon with given name, loading if necessary and caching for later use<br>
 Returns NULL if the icon cannot be loaded.
 Warning: Pixmap will be deleted when last instance of iconcache is deleted.
 If it is desired for application to hold the pixmap after iconcache is destroyed, it should make a copy
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
  guiPrintDbg(debug::DBG_WARN,"File not found:" << Q_OUT(absName));
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
QIcon* IconCache::getIconSet(const QString &name) {
 //Look in cache first
 if (setCache.contains(name)) return setCache[name];
 //Not found in cache - try to create
 QPixmap* pix=getIcon(name);
 if (!pix) return NULL;//File not found ...
 //Pixmap is OK
 QIcon *ico=new QIcon(*pix);
 setCache[name]=ico;
 return ico;
}


/** default destructor */
IconCache::~IconCache() {
 assert(iconCacheInstances>=1);
 iconCacheInstances--;
 if (iconCacheInstances<=0) {
  //This was last instance -> free the internal data
  Q_List<QString> pixmaps=iconCache.keys();
  //Delete all pixmaps from cache
  for (Q_List<QString>::Iterator it=pixmaps.begin();it!=pixmaps.end();++it) {
   QPixmap *rm=iconCache[*it];
   delete rm;
  }
  iconCache.clear();
  Q_List<QString> iconSets=setCache.keys();
  //Delete all icon sets from cache
  for (Q_List<QString>::Iterator it=iconSets.begin();it!=iconSets.end();++it) {
   QIconSet *rm=setCache[*it];
   delete rm;
  }
  setCache.clear();
 }
}

} // namespace gui
